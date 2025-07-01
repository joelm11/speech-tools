#include <atomic>
#include <memory>
#include <stdexcept>
#include <utility>

/**
 * @brief A Single-Producer, Single-Consumer (SPSC) lock-free queue.
 *
 * This queue is designed for high-performance communication between exactly
 * one producer thread and one consumer thread. It uses a fixed-size circular
 * buffer and atomic operations for its head and tail pointers to avoid
 * mutex overhead, making it suitable for real-time and embedded systems.
 *
 * @tparam T The type of elements to store in the queue. T must be movable or
 * copyable.
 */
template <typename T>
class SPSCLockFreeQueue {
 public:
  /**
   * @brief Constructs an SPSC queue with a specified capacity.
   * @param capacity The maximum number of elements the queue can hold.
   * @throws std::runtime_error If capacity is zero.
   */
  explicit SPSCLockFreeQueue(size_t capacity)
      : buffer_(std::make_unique<T[]>(capacity + 1)),  // Allocate N+1 slots
        capacity_(capacity)  // Store N as usable capacity
  {
    if (capacity == 0) {
      throw std::runtime_error("SPSCLockFreeQueue capacity cannot be zero.");
    }
  }

  // Delete copy constructor and assignment operator to prevent accidental
  // copies and ensure single ownership/instance behavior.
  SPSCLockFreeQueue(const SPSCLockFreeQueue&) = delete;
  SPSCLockFreeQueue& operator=(const SPSCLockFreeQueue&) = delete;

  // Delete move constructor and assignment operator for simplicity and to avoid
  // complications with `const capacity_` and atomic state transfer.
  // The queue is typically constructed once and used.
  SPSCLockFreeQueue(SPSCLockFreeQueue&&) = delete;
  SPSCLockFreeQueue& operator=(SPSCLockFreeQueue&&) = delete;

  /**
   * @brief Attempts to push an element into the queue (non-blocking, copy).
   * @param value The element to copy into the queue.
   * @return true if the element was successfully pushed, false if the queue is
   * full.
   */
  bool try_push(const T& value) {
    size_t current_tail = tail_.load(std::memory_order_relaxed);
    size_t next_tail = next_index(current_tail);

    // Check if the queue is full by comparing the next write position with the
    // read position. Use acquire memory order for head_ to ensure visibility of
    // consumer's progress.
    if (next_tail == head_.load(std::memory_order_acquire)) {
      return false;  // Queue is full
    }

    buffer_[current_tail] = value;  // Copy assignment
    // Release memory order for tail_ to ensure the value written is visible
    // to the consumer before the consumer sees the updated tail_.
    tail_.store(next_tail, std::memory_order_release);
    return true;
  }

  /**
   * @brief Attempts to push an element into the queue (non-blocking, move).
   * @param value The element to move into the queue.
   * @return true if the element was successfully pushed, false if the queue is
   * full.
   */
  bool try_push(T&& value) {
    size_t current_tail = tail_.load(std::memory_order_relaxed);
    size_t next_tail = next_index(current_tail);

    // Check if the queue is full.
    if (next_tail == head_.load(std::memory_order_acquire)) {
      return false;  // Queue is full
    }

    buffer_[current_tail] = std::move(value);  // Move assignment
    tail_.store(next_tail, std::memory_order_release);
    return true;
  }

  /**
   * @brief Attempts to pop an element from the queue (non-blocking).
   * @param value A reference to store the popped element.
   * @return true if an element was successfully popped, false if the queue is
   * empty.
   */
  bool try_pop(T& value) {
    size_t current_head = head_.load(std::memory_order_relaxed);
    // Check if the queue is empty by comparing the read position with the write
    // position. Use acquire memory order for tail_ to ensure visibility of
    // producer's written data.
    if (current_head == tail_.load(std::memory_order_acquire)) {
      return false;  // Queue is empty
    }

    value = std::move(buffer_[current_head]);  // Move element out
    // Release memory order for head_ to ensure the read is complete and the
    // slot is logically free before the producer sees the updated head_.
    head_.store(next_index(current_head), std::memory_order_release);
    return true;
  }

  /**
   * @brief Returns the approximate number of elements currently in the queue.
   * @note This is an approximation in a lock-free SPSC queue without a separate
   * counter, as head and tail can be updated concurrently by different threads.
   * @return The current size of the queue.
   */
  size_t size() const {
    size_t current_head = head_.load(std::memory_order_relaxed);
    size_t current_tail = tail_.load(std::memory_order_relaxed);
    if (current_tail >= current_head) {
      return current_tail - current_head;
    } else {
      return capacity_ + current_tail - current_head;
    }
  }

  /**
   * @brief Checks if the queue is empty.
   * @return true if the queue contains no elements, false otherwise.
   */
  bool empty() const {
    // Use acquire memory order to ensure visibility of the latest tail_ update.
    return head_.load(std::memory_order_acquire) ==
           tail_.load(std::memory_order_acquire);
  }

  /**
   * @brief Checks if the queue is full.
   * @return true if the queue has reached its maximum capacity, false
   * otherwise.
   */
  bool full() const {
    // Use acquire memory order to ensure visibility of the latest head_ update.
    return next_index(tail_.load(std::memory_order_acquire)) ==
           head_.load(std::memory_order_acquire);
  }

 private:
  /**
   * @brief Calculates the next index in the circular buffer.
   * @param current_index The current index.
   * @return The next index, wrapping around if necessary.
   */
  size_t next_index(size_t current_index) const {
    return (current_index + 1) % (capacity_ + 1);
  }

  const size_t capacity_;
  std::atomic<size_t> head_ = 0;
  std::atomic<size_t> tail_ = 0;
  // The underlying buffer for storing elements.
  // Allocated with capacity_ + 1 slots to distinguish full from empty.
  std::unique_ptr<T[]> buffer_;
};