#include "../src/noise_reduction.hh"

#include <gtest/gtest.h>

#include "../../common/src/spsc_queue.hh"

using namespace SpeechTools;

TEST(NoiseFilterTest, ConstructorDestructor) {
  SPSCLockFreeQueue<std::vector<std::vector<float>>> in_queue(4);
  SPSCLockFreeQueue<std::vector<std::vector<float>>> out_queue(4);
  // Scope to test constructor and destructor
  {
    NoiseFilter filter(in_queue, out_queue);
    // No exception should be thrown, object should be constructed
  }
  // Destructor called at end of scope
}
