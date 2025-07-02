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

 protected:
  void processLoop() {
    InType input_data;
    while (running_.load(std::memory_order_relaxed)) {
      if (inQueue_.try_pop(input_data)) {
        OutType output_data = processData(input_data);
        // Busy-wait or add a small sleep if push fails repeatedly
        // In real-time, you might just drop the frame if the next stage is
        // backed up
        while (!outQueue_.try_push(output_data) &&
               running_.load(std::memory_order_relaxed)) {
          // Optionally, log or handle backpressure.
          // For embedded, a `std::this_thread::yield()` or short `sleep` might
          // be used to prevent 100% CPU usage if the queue is persistently
          // full. However, in a tightly coupled real-time pipeline, if a stage
          // can't keep up, it often means a fundamental design issue or
          // overloaded hardware.
        }
      } else {
        // Input queue is empty, wait a bit or yield to other threads
        // For a truly real-time system, you might not sleep,
        // but spin-wait or rely on external timing/interrupts to provide data.
        // For illustration:
        std::this_thread::yield();  // Or a very short sleep
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