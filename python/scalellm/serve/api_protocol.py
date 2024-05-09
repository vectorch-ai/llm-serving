# Adapted from https://github.com/lm-sys/FastChat
import time
from typing import Dict, List, Literal, Optional, Union

import shortuuid
from pydantic import BaseModel, Field


class ErrorResponse(BaseModel):
    message: Optional[str] = None
    code: Optional[int] = None


class ModelPermission(BaseModel):
    id: str = Field(default_factory=lambda: f"modelperm-{shortuuid.random()}")
    object: str = "model_permission"
    created: int = Field(default_factory=lambda: int(time.time()))
    allow_create_engine: bool = False
    allow_sampling: bool = True
    allow_logprobs: bool = False
    allow_search_indices: bool = False
    allow_view: bool = True
    allow_fine_tuning: bool = False
    organization: str = "*"
    group: Optional[str] = None
    is_blocking: str = False


class ModelCard(BaseModel):
    id: str
    object: str = "model"
    created: int = Field(default_factory=lambda: int(time.time()))
    owned_by: str = "scalellm"
    root: Optional[str] = None
    parent: Optional[str] = None
    permission: List[ModelPermission] = []


class ModelList(BaseModel):
    object: str = "list"
    data: List[ModelCard] = []


class UsageInfo(BaseModel):
    prompt_tokens: int = 0
    total_tokens: int = 0
    completion_tokens: Optional[int] = 0


class LogProbs(BaseModel):
    text_offset: List[int] = Field(default_factory=list)
    token_logprobs: List[Optional[float]] = Field(default_factory=list)
    tokens: List[str] = Field(default_factory=list)
    top_logprobs: List[Optional[Dict[str, float]]] = Field(default_factory=list)


class ChatCompletionMessage(BaseModel):
    role: Literal["user", "system", "assistant"]
    content: str


class ChatCompletionRequest(BaseModel):
    model: str
    messages: List[ChatCompletionMessage]
    priority: Optional[Literal["default", "low", "normal", "high"]] = None
    n: Optional[int] = 1
    max_tokens: Optional[int] = 16
    stream: Optional[bool] = False
    temperature: Optional[float] = 0.7
    presence_penalty: Optional[float] = 0.0
    frequency_penalty: Optional[float] = 0.0
    repetition_penalty: Optional[float] = 1.0
    top_p: Optional[float] = 1.0
    top_k: Optional[int] = -1
    # user: Optional[str] = None
    skip_special_tokens: Optional[bool] = True
    ignore_eos: Optional[bool] = False
    stop: Optional[Union[str, List[str]]] = None
    stop_token_ids: Optional[List[int]] = None


class ChatMessage(BaseModel):
    role: str
    content: str


class ChatCompletionResponseChoice(BaseModel):
    index: int
    message: ChatMessage
    finish_reason: Optional[Literal["stop", "length"]] = None


class ChatCompletionResponse(BaseModel):
    id: str = Field(default_factory=lambda: f"chatcmpl-{shortuuid.random()}")
    object: str = "chat.completion"
    created: int = Field(default_factory=lambda: int(time.time()))
    model: str
    choices: List[ChatCompletionResponseChoice]
    usage: Optional[UsageInfo] = None


class DeltaMessage(BaseModel):
    role: Optional[str] = None
    content: Optional[str] = None


class ChatCompletionResponseStreamChoice(BaseModel):
    index: int
    delta: DeltaMessage
    logprobs: Optional[LogProbs] = None
    finish_reason: Optional[Literal["stop", "length"]] = None


class ChatCompletionStreamResponse(BaseModel):
    id: str = Field(default_factory=lambda: f"chatcmpl-{shortuuid.random()}")
    object: str = "chat.completion.chunk"
    created: int = Field(default_factory=lambda: int(time.time()))
    model: str
    choices: List[ChatCompletionResponseStreamChoice]
    usage: Optional[UsageInfo] = None


class CompletionRequest(BaseModel):
    model: str
    prompt: str
    priority: Optional[Literal["default", "low", "normal", "high"]] = None
    # suffix: Optional[str] = None
    n: Optional[int] = 1
    max_tokens: Optional[int] = 16
    stream: Optional[bool] = False
    # logprobs: Optional[int] = None
    echo: Optional[bool] = False
    temperature: Optional[float] = 0.7
    presence_penalty: Optional[float] = 0.0
    frequency_penalty: Optional[float] = 0.0
    repetition_penalty: Optional[float] = 1.0
    top_p: Optional[float] = 1.0
    top_k: Optional[int] = -1
    # user: Optional[str] = None
    skip_special_tokens: Optional[bool] = True
    ignore_eos: Optional[bool] = False
    stop: Optional[Union[str, List[str]]] = None
    stop_token_ids: Optional[List[int]] = None
    # use_beam_search: Optional[bool] = False
    # best_of: Optional[int] = None


class CompletionResponseChoice(BaseModel):
    index: int
    text: str
    logprobs: Optional[LogProbs] = None
    finish_reason: Optional[Literal["stop", "length"]] = None


class CompletionResponse(BaseModel):
    id: str = Field(default_factory=lambda: f"cmpl-{shortuuid.random()}")
    object: str = "text_completion"
    created: int = Field(default_factory=lambda: int(time.time()))
    model: str
    choices: List[CompletionResponseChoice]
    usage: Optional[UsageInfo] = None


class CompletionResponseStreamChoice(BaseModel):
    index: int
    text: str
    logprobs: Optional[LogProbs] = None
    finish_reason: Optional[Literal["stop", "length"]] = None


class CompletionStreamResponse(BaseModel):
    id: str = Field(default_factory=lambda: f"cmpl-{shortuuid.random()}")
    object: str = "text_completion"
    created: int = Field(default_factory=lambda: int(time.time()))
    model: str
    choices: List[CompletionResponseStreamChoice]
    usage: Optional[UsageInfo] = None
