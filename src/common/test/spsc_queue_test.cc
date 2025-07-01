#include "../src/spsc_queue.hh"

#include <gtest/gtest.h>

#include <thread>

// Basic FIFO test
TEST(SPSCLockFreeQueueTest, FifoOrder) {
  SPSCLockFreeQueue<int> q(4);
  EXPECT_TRUE(q.try_push(1));
  EXPECT_TRUE(q.try_push(2));
  EXPECT_TRUE(q.try_push(3));
  int v;
  EXPECT_TRUE(q.try_pop(v));
  EXPECT_EQ(v, 1);
  EXPECT_TRUE(q.try_pop(v));
  EXPECT_EQ(v, 2);
  EXPECT_TRUE(q.try_pop(v));
  EXPECT_EQ(v, 3);
  EXPECT_TRUE(q.empty());
}

// Full/empty detection
TEST(SPSCLockFreeQueueTest, FullAndEmpty) {
  SPSCLockFreeQueue<int> q(2);
  EXPECT_TRUE(q.empty());
  EXPECT_TRUE(q.try_push(1));
  EXPECT_TRUE(q.try_push(2));
  EXPECT_TRUE(q.full());
  EXPECT_FALSE(q.try_push(3));
  int v;
  EXPECT_TRUE(q.try_pop(v));
  EXPECT_EQ(v, 1);
  EXPECT_TRUE(q.try_pop(v));
  EXPECT_EQ(v, 2);
  EXPECT_TRUE(q.empty());
  EXPECT_FALSE(q.try_pop(v));
}

// Size reporting
TEST(SPSCLockFreeQueueTest, Size) {
  SPSCLockFreeQueue<int> q(3);
  EXPECT_EQ(q.size(), 0u);
  q.try_push(1);
  EXPECT_EQ(q.size(), 1u);
  q.try_push(2);
  EXPECT_EQ(q.size(), 2u);
  int v;
  q.try_pop(v);
  EXPECT_EQ(q.size(), 1u);
  q.try_pop(v);
  EXPECT_EQ(q.size(), 0u);
}

// Move-only type support
TEST(SPSCLockFreeQueueTest, MoveOnlyType) {
  SPSCLockFreeQueue<std::unique_ptr<int>> q(2);
  EXPECT_TRUE(q.try_push(std::make_unique<int>(42)));
  std::unique_ptr<int> v;
  EXPECT_TRUE(q.try_pop(v));
  ASSERT_TRUE(v);
  EXPECT_EQ(*v, 42);
}

// Single-producer, single-consumer concurrency
TEST(SPSCLockFreeQueueTest, SPSCConcurrent) {
  SPSCLockFreeQueue<int> q(1000);
  std::vector<int> results;
  std::thread producer([&]() {
    for (int i = 0; i < 500; ++i) {
      while (!q.try_push(i)) {
      }
    }
  });
  std::thread consumer([&]() {
    int v;
    for (int i = 0; i < 500; ++i) {
      while (!q.try_pop(v)) {
      }
      results.push_back(v);
    }
  });
  producer.join();
  consumer.join();
  for (int i = 0; i < 500; ++i) {
    EXPECT_EQ(results[i], i);
  }
}
