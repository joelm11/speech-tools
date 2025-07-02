#include "../src/speech_filter.hh"

#include "../src/spsc_queue.hh"
#include "gtest/gtest.h"

using IntQueue = SPSCLockFreeQueue<int>;

// Dummy filter: multiplies input by 2
class DummyFilter
    : public SpeechTools::SpeechFilter<int, int, SPSCLockFreeQueue> {
 public:
  DummyFilter(SPSCLockFreeQueue<int>& in, SPSCLockFreeQueue<int>& out)
      : SpeechTools::SpeechFilter<int, int, SPSCLockFreeQueue>(in, out) {}

 protected:
  virtual int process(const int& input_data) override { return input_data * 2; }
};

TEST(SpeechFilterTest, ConstructionAndDestruction) {
  IntQueue in(8), out(8);
  {
    DummyFilter filter(in, out);
    // Construction should not throw
  }
  // Destruction should not throw
}

TEST(SpeechFilterTest, StartAndStopProcessing) {
  IntQueue in(8), out(8);
  DummyFilter filter(in, out);

  // Add input
  in.try_push(3);
  in.try_push(7);

  // Wait for processing
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  int val1 = 0, val2 = 0;
  bool got1 = out.try_pop(val1);
  bool got2 = out.try_pop(val2);

  // The filter should have processed both values
  EXPECT_TRUE(got1);
  EXPECT_TRUE(got2);
  EXPECT_TRUE((val1 == 6 && val2 == 14) || (val1 == 14 && val2 == 6))
      << "Actual: " << val1 << " " << val2 << "\n";
}