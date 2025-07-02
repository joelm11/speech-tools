#pragma once

#include <atomic>
#include <thread>

namespace SpeechTools {

/** @brief Base class for all filters. Deriving filters implement the process()
 * method, which gets called in the base class's processLoop().
 *
 */
template <template <typename> class QueueType, class InType, class OutType>
class SpeechFilter {
  using ThreadType = std::thread;

 public:
  SpeechFilter(QueueType<InType>& in, QueueType<OutType>& out)
      : inQueue_(in), outQueue_(out) {
    running_ = true;
    proc_thread_ = ThreadType([this]() { processLoop(); });
  }

  virtual ~SpeechFilter() {
    stop();
    if (proc_thread_.joinable()) {
      proc_thread_.join();
    }
  }

  void start() {
    if (!running_) {
      running_ = true;
      proc_thread_ = ThreadType([this] { processLoop(); });
    }
  }

  void stop() { running_ = false; }

 protected:
  virtual OutType process(const InType& input_data) = 0;

  void processLoop() {
    InType input_data;

    while (running_.load(std::memory_order_relaxed)) {
      if (inQueue_.try_pop(input_data)) {
        OutType output_data = process(input_data);
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
  std::atomic<bool> running_ = false;
  QueueType<InType>& inQueue_;
  QueueType<OutType>& outQueue_;
  ThreadType proc_thread_;
};
}  // namespace SpeechTools