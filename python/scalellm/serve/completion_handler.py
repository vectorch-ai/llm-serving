import time

import shortuuid
from fastapi.responses import StreamingResponse
from scalellm import AsyncLLMEngine, OutputError, SamplingParams
from scalellm.serve.api_protocol import (CompletionRequest, CompletionResponse,
                                         CompletionResponseChoice,
                                         CompletionResponseStreamChoice,
                                         CompletionStreamResponse,
                                         ErrorResponse, UsageInfo)
from scalellm.serve.common import jsonify_model, to_priority


def to_sampling_params(request: CompletionRequest) -> SamplingParams:
    sp = SamplingParams()
    sp.max_tokens = request.max_tokens
    sp.n = request.n
    sp.echo = request.echo
    sp.frequency_penalty = request.frequency_penalty
    sp.presence_penalty = request.presence_penalty
    sp.repetition_penalty = request.repetition_penalty
    sp.temperature = request.temperature
    sp.top_p = request.top_p
    sp.top_k = request.top_k
    sp.skip_special_tokens = request.skip_special_tokens
    sp.stop = request.stop
    sp.ignore_eos = request.ignore_eos
    sp.stop_token_ids = request.stop_token_ids
    return sp


async def generate_completion_response(
    request: CompletionRequest, engine: AsyncLLMEngine
) -> CompletionResponse:
    request_id = f"cmpl-{shortuuid.random()}"
    created_time = int(time.time())
    model = request.model

    sampling_params = to_sampling_params(request)
    priority = to_priority(request.priority)
    output_stream = await engine.schedule_async(
        request.prompt, sampling_params, priority, request.stream
    )

    # only one output is expected for non-streaming request
    output = await output_stream.__anext__()
    usage = None
    if output.usage:
        usage = UsageInfo(
            prompt_tokens=output.usage.num_prompt_tokens,
            total_tokens=output.usage.num_total_tokens,
            completion_tokens=output.usage.num_generated_tokens,
        )
    choices = []
    for seq_output in output.outputs:
        choices.append(
            CompletionResponseChoice(
                index=seq_output.index,
                text=seq_output.text,
                finish_reason=seq_output.finish_reason,
            )
        )
    return CompletionResponse(
        id=request_id,
        object="text_completion",
        created=created_time,
        model=model,
        choices=choices,
        usage=usage,
    )


async def generate_completion_stream_response(
    request: CompletionRequest, engine: AsyncLLMEngine
) -> StreamingResponse:
    request_id = f"cmpl-{shortuuid.random()}"
    created_time = int(time.time())
    chunk_object_type = "text_completion"
    model = request.model

    sampling_params = to_sampling_params(request)
    priority = to_priority(request.priority)
    output_stream = await engine.schedule_async(
        request.prompt, sampling_params, priority, request.stream
    )

    async def generate_stream_content():
        try:
            async for output in output_stream:
                for seq_output in output.outputs:
                    # send chunk with delta message
                    response = CompletionStreamResponse(
                        id=request_id,
                        object=chunk_object_type,
                        created=created_time,
                        model=model,
                        choices=[
                            CompletionResponseStreamChoice(
                                index=seq_output.index,
                                text=seq_output.text,
                            )
                        ],
                    )
                    yield f"data: {jsonify_model(response)}\n\n"
                    # send the final chunk with finish reason
                    if seq_output.finish_reason is not None:
                        response = CompletionStreamResponse(
                            id=request_id,
                            object=chunk_object_type,
                            created=created_time,
                            model=model,
                            choices=[
                                CompletionResponseStreamChoice(
                                    index=seq_output.index,
                                    text="",
                                    finish_reason=seq_output.finish_reason,
                                )
                            ],
                        )
                        yield f"data: {jsonify_model(response)}\n\n"
            # send additional chunk for usage info
            if output.usage:
                response = CompletionStreamResponse(
                    id=request_id,
                    object=chunk_object_type,
                    created=created_time,
                    model=model,
                    choices=[],
                    usage=UsageInfo(
                        prompt_tokens=output.usage.num_prompt_tokens,
                        total_tokens=output.usage.num_total_tokens,
                        completion_tokens=output.usage.num_generated_tokens,
                    ),
                )
                yield f"data: {jsonify_model(response)}\n\n"
            yield "data: [DONE]\n\n"
        except OutputError as e:
            yield f"error: {jsonify_model(ErrorResponse(object='error', message=e.message, code=e.code))}\n\n"

    return StreamingResponse(
        content=generate_stream_content(), media_type="text/event-stream"
    )
