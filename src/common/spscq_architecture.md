# SPSC Queue Architecture Requirements

This document outlines the requirements for a Single-Producer, Single-Consumer (SPSC) queue designed for optimal performance on both multi-threaded PC environments and potentially single-threaded embedded systems.

## 1. Functional Requirements

- **FIFO Ordering:** Elements must be processed in First-In, First-Out order.
- **Push Operation:**
  - Must allow a single producer to add elements to the queue.
  - Should offer both a blocking (`push`) and non-blocking (`try_push`) variant.
  - If blocking, it should wait until space is available.
  - If non-blocking, it should return `true` on success and `false` if the queue is full.
- **Pop Operation:**
  - Must allow a single consumer to remove elements from the queue.
  - Should offer both a blocking (`pop`) and non-blocking (`try_pop`) variant.
  - If blocking, it should wait until an element is available.
  - If non-blocking, it should return `true` on success (with the popped element) and `false` if the queue is empty.
- **Capacity Management:**
  - Must have a fixed, pre-allocated capacity defined at construction time.
  - Should provide methods to query `size()`, `empty()`, and `full()`.
- **Element Types:**
  - Must be a template, capable of storing any `T` that is movable or copyable.
  - For performance, prefer `T` to be cheaply movable (e.g., primitive types, `std::unique_ptr`, `std::vector` if using move semantics).

## 2. Performance Requirements

- **Low Latency (Critical):** Minimize the time taken for `push` and `pop` operations.
- **High Throughput:** Maximize the number of `push`/`pop` operations per unit of time.
- **Lock-Freedom/Minimal Locking (Multi-threaded PC):**
  - Prefer a lock-free implementation using `std::atomic` operations for pointer manipulation (head/tail indices) to avoid mutex overhead and contention.
  - If a purely lock-free approach is overly complex or introduces subtle bugs, a minimal-lock approach (e.g., using `std::mutex` and `std::condition_variable` only for blocking operations) is acceptable, but lock-free is strongly preferred for the `try_push`/`try_pop` paths.
- **Zero/Minimal Overhead (Single-threaded Embedded):**
  - On a single-threaded system, `std::atomic` operations should ideally compile down to regular memory accesses (no-ops for atomicity, potential for implicit memory barriers if needed by compiler for reordering).
  - Avoid any overhead associated with threading primitives (mutexes, condition variables) if they are not actively used (e.g., in `try_push`/`try_pop` or if blocking is not enabled/used).
  - Memory accesses should be cache-friendly, particularly for embedded caches.

## 3. Resource Requirements

- **Fixed Memory Allocation:** The queue's buffer memory must be allocated once at construction and not resize dynamically during operation. This is crucial for embedded systems with limited and often fragmented heap memory.
- **Minimal RAM Footprint:** Optimize for memory efficiency, especially for the buffer and control variables.
- **No Dynamic Allocations After Initialization:** Avoid `new`/`delete` calls during `push` or `pop` operations to prevent heap fragmentation and non-deterministic latencies. This implies storing elements directly in the buffer (or using placement new if `T` is complex and doesn't handle assignment well).

## 4. Design Requirements

- **Circular Buffer Implementation:** The internal buffer should be a circular array (or `std::vector` pre-sized) to efficiently reuse memory.
- **Pointer/Index Management:** Use `std::atomic<size_t>` for head (read) and tail (write) indices for thread-safety.
- **Memory Ordering:** Correct use of `std::memory_order` (e.g., `acquire`, `release`, `relaxed`) for atomic operations to ensure visibility and prevent reordering where necessary.
- **Distinguishing Full/Empty:** A robust mechanism to differentiate between a full and an empty queue (e.g., using `capacity + 1` slots, or a count).
- **C++ Standard:** Must be compatible with C++17 (or C++20 for optional features like `std::atomic_ref` if deemed beneficial and supported by toolchain). No reliance on OS-specific threading APIs where `std::thread` and `std::atomic` suffice.
- **Error Handling:**
  - Non-blocking operations should indicate success/failure via boolean return values.
  - Blocking operations might throw exceptions on unexpected errors (e.g., queue destroyed while waiting), but ideally would handle graceful shutdown. For embedded, avoiding exceptions is often preferred; error codes or assertions might be more appropriate.

## 5. Portability Requirements

- **Cross-Platform Compatibility:** The implementation should compile and run correctly on various platforms, including:
  - Linux (x86, ARM)
  - Windows (x86)
  - Embedded RTOS/Bare Metal (ARM Cortex-M, RISC-V, etc.)
- **Compiler Compatibility:** Support common compilers (GCC, Clang, MSVC) that provide robust `std::atomic` support.

## 6. Implementation Notes & Considerations

- **Padding:** Consider cache line padding for atomic variables if multiple SPSC queues or other shared state are on the same cache line, to avoid false sharing.
- **`std::atomic_ref` (C++20):** If using raw arrays, `std::atomic_ref` could be used for accessing elements atomically if `T` is trivially copyable and lock-free for `atomic_ref`. However, storing elements directly in a `std::vector` and atomically managing indices is generally simpler and sufficient.
- **Spin-waiting vs. Yielding/Sleeping:** For non-blocking operations, if `try_push`/`try_pop` fails, the calling thread can spin-wait, `std::this_thread::yield()`, or `std::this_thread::sleep_for()` based on the real-time requirements and CPU budget. For embedded, simple spin-waiting for very short durations is common, or relying on higher-level system design to ensure consumer/producer rates match.
- **Value Semantics:** The queue will hold copies of values. For large objects, consider storing `std::unique_ptr<T>` to avoid large copies, but this reintroduces dynamic allocation for the pointee. For small, frequently moved objects (e.g., `std::vector` of samples/features), move semantics are critical for performance.
