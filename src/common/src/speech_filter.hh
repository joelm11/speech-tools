#pragma once

#include <atomic>
#include <thread>

namespace SpeechTools {

/** @brief Base class for all filters. Deriving filters implement the process()
 * method, which gets called in the base class's processLoop().
 *
 */
template <class InType, class OutType>
class SpeechFilter {
  using ThreadType = std::thread;

 public:
  SpeechFilter(InType& in, OutType& out) : inQueue_(in), outQueue_(out) {}

  virtual ~SpeechFilter() {
    stop();
    if (proc_thread_.joinable()) {
      proc_thread_.join();
    }
  }

 protected:
  void processLoop() {
    InType input_data;

    while (running_.load(std::memory_order_relaxed)) {
      if (inQueue_.try_pop(input_data)) {
        OutType output_data = processData(input_data);
        while (!outQueue_.try_push(output_data) &&
               running_.load(std::memory_order_relaxed)) {
          // Do something if push repeatedly fails (?)
        }
      } else {
        // Do something if the queue is full (?)
        std::this_thread::yield();
      }
    }
  }

 private:
  virtual OutType process() = 0;

  void start() { running_ = true; }
  void stop() { running_ = false; }

  std::atomic<bool> running_ = false;
  InType inQueue_;
  OutType outQueue_;
  ThreadType proc_thread_;
};
}  // namespace SpeechTools