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

// SPSC with random producer/consumer delays
TEST(SPSCLockFreeQueueTest, SPSCConcurrentRandomDelays) {
  SPSCLockFreeQueue<int> q(256);
  std::vector<int> results;
  std::atomic<bool> done{false};
  std::thread producer([&]() {
    for (int i = 0; i < 200; ++i) {
      while (!q.try_push(i)) {
      }
      if (i % 10 == 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(rand() % 200));
      }
    }
    done = true;
  });
  std::thread consumer([&]() {
    int v;
    while (!done || !q.empty()) {
      if (q.try_pop(v)) {
        results.push_back(v);
        if (v % 13 == 0) {
          std::this_thread::sleep_for(std::chrono::microseconds(rand() % 150));
        }
      } else {
        std::this_thread::yield();
      }
    }
  });
  producer.join();
  consumer.join();
  ASSERT_EQ(results.size(), 200u);
  for (int i = 0; i < 200; ++i) {
    EXPECT_EQ(results[i], i);
  }
}

// SPSC with bursty producer and slow consumer
TEST(SPSCLockFreeQueueTest, SPSCProducerBurstConsumerSlow) {
  SPSCLockFreeQueue<int> q(32);
  std::vector<int> results;
  std::thread producer([&]() {
    for (int i = 0; i < 100; ++i) {
      while (!q.try_push(i)) {
      }
      if (i % 8 == 0) {
        std::this_thread::sleep_for(
            std::chrono::microseconds(100 + rand() % 200));
      }
    }
  });
  std::thread consumer([&]() {
    int v;
    for (int i = 0; i < 100; ++i) {
      while (!q.try_pop(v)) {
      }
      results.push_back(v);
      if (i % 7 == 0) {
        std::this_thread::sleep_for(
            std::chrono::microseconds(150 + rand() % 250));
      }
    }
  });
  producer.join();
  consumer.join();
  ASSERT_EQ(results.size(), 100u);
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(results[i], i);
  }
}

// SPSC with slow producer and fast consumer
TEST(SPSCLockFreeQueueTest, SPSCProducerSlowConsumerFast) {
  SPSCLockFreeQueue<int> q(16);
  std::vector<int> results;
  std::thread producer([&]() {
    for (int i = 0; i < 50; ++i) {
      while (!q.try_push(i)) {
      }
      std::this_thread::sleep_for(
          std::chrono::microseconds(200 + rand() % 200));
    }
  });
  std::thread consumer([&]() {
    int v;
    for (int i = 0; i < 50; ++i) {
      while (!q.try_pop(v)) {
      }
      results.push_back(v);
    }
  });
  producer.join();
  consumer.join();
  ASSERT_EQ(results.size(), 50u);
  for (int i = 0; i < 50; ++i) {
    EXPECT_EQ(results[i], i);
  }
}
