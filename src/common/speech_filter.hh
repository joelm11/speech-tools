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
  void processLoop() { return; }

 private:
  virtual void process() = 0;

  void start() { running_ = true; }
  void stop() { running_ = false; }

  std::atomic<bool> running_ = false;
  InType inQueue_;
  OutType outQueue_;
  ThreadType proc_thread_;
};
}  // namespace SpeechTools