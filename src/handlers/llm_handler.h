#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "chat_template/chat_template.h"
#include "engine/engine.h"
#include "request/output.h"
#include "sampling_params.h"
#include "scheduler/continuous_scheduler.h"

namespace llm {

// callback function for output, return true to continue, false to stop/cancel
using OutputCallback = std::function<bool(RequestOutput output)>;

// NOLINTNEXTLINE
class LLMHandler {
 public:
  struct Options {
    DEFINE_ARG(std::string, model_path);

    DEFINE_ARG(std::optional<std::string>, devices);

    DEFINE_ARG(std::optional<std::string>, draft_model_path);

    DEFINE_ARG(std::optional<std::string>, draft_devices);

    // the number of slots per block, default 16, value must be multiple of 16
    DEFINE_ARG(int32_t, block_size) = 16;

    // the maximum cache size in bytes, default 10GB
    DEFINE_ARG(int64_t, max_cache_size) = static_cast<int64_t>(10) * 1024 *
                                          1024 * 1024;

    // maximum memory utilization allowed, default 0.9
    DEFINE_ARG(double, max_memory_utilization) = 0.9;

    // enable prefix cache
    DEFINE_ARG(bool, enable_prefix_cache) = true;

    // enable cuda graph
    DEFINE_ARG(bool, enable_cuda_graph) = true;

    // max sequence length used to capture cuda graphs
    DEFINE_ARG(int64_t, cuda_graph_max_seq_len) = 2048;

    // batch sizes to capture cuda graphs
    DEFINE_ARG(std::optional<std::vector<uint32_t>>, cuda_graph_batch_sizes);

    // batch sizes to capture cuda graphs for draft model
    DEFINE_ARG(std::optional<std::vector<uint32_t>>,
               draft_cuda_graph_batch_sizes);

    // the maximum number of tokens per batch
    DEFINE_ARG(int32_t, max_tokens_per_batch) = 256;

    // the maximum number of sequences per batch
    DEFINE_ARG(int32_t, max_seqs_per_batch) = 64;

    // the number of speculative tokens per step
    DEFINE_ARG(int32_t, num_speculative_tokens) = 0;
  };

  LLMHandler(const Options& options);

  ~LLMHandler();

  // schedule a request, the engine will execute the request asynchronously
  // and call the callback with output when the request is done
  // the callback will be called multiple times if the request is a streaming
  // request
  void schedule_async(std::string prompt,
                      SamplingParams sp,
                      Priority priority,
                      bool stream,
                      OutputCallback callback);

  void schedule_chat_async(std::vector<Message> messages,
                           SamplingParams sp,
                           Priority priority,
                           bool stream,
                           OutputCallback callback);

  // start the handling loop
  void start();

  // stop the engine
  void stop();

  // run until complete, blocking call
  void run_until_complete();

 private:
  std::unique_ptr<Request> create_request(std::string prompt,
                                          const SamplingParams& sp,
                                          Priority priority,
                                          bool stream,
                                          OutputCallback callback);

  std::unique_ptr<Request> create_chat_request(
      const std::vector<Message>& messages,
      const SamplingParams& sp,
      Priority priority,
      bool stream,
      OutputCallback callback);

  const Options options_;

  std::unique_ptr<Engine> engine_;

  std::unique_ptr<Scheduler> scheduler_;

  // tokenizer instance
  std::unique_ptr<Tokenizer> tokenizer_;

  // model args
  ModelArgs model_args_;

  // thread pool for handling requests
  ThreadPool thread_pool_;

  // chat template instance
  std::unique_ptr<ChatTemplate> chat_template_;

  std::thread loop_thread_;

  std::atomic_bool stoped_{false};

  std::atomic_bool running_{false};
};

}  // namespace llm
