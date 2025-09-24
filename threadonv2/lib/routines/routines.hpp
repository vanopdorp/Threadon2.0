#include <coroutine>
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <cassert>
#include <unistd.h>
#include <fstream>
#include <memory>
#include <map>

struct Task {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    Task() : coro(nullptr) {}
    explicit Task(handle_type h) : coro(h) {}

    Task(Task&& other) noexcept : coro(other.coro) { other.coro = nullptr; }
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (coro) coro.destroy();
            coro = other.coro;
            other.coro = nullptr;
        }
        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    ~Task() { if (coro) coro.destroy(); }

    void resume() { if (coro && !coro.done()) coro.resume(); }
    bool done() const { return !coro || coro.done(); }
    void resume_if_needed() {
        if (!done()) resume();
    }

private:
    handle_type coro;

public:
    struct promise_type {
        Task get_return_object() { return Task{handle_type::from_promise(*this)}; }
        std::suspend_always initial_suspend() { return {}; }   // start suspended
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};

// Lock-free SPSC ring buffer voor Tasks
class SpscQueue {
public:
    explicit SpscQueue(size_t capacity_power_of_two = 1 << 20) // default 1,048,576
        : capacity(capacity_power_of_two),
          mask(capacity_power_of_two - 1),
          buffer(capacity_power_of_two) {
        // capacity moet macht van 2 zijn
        assert((capacity & mask) == 0 && "capacity must be power of two");
    }

    bool push(Task&& t) {
        auto h = head.load(std::memory_order_relaxed);
        auto next = (h + 1) & mask;
        if (next == tail.load(std::memory_order_acquire)) {
            return false; // full
        }
        buffer[h] = std::move(t);
        head.store(next, std::memory_order_release);
        return true;
    }
    bool pop_batch(std::vector<Task>& out, size_t max_batch) {
        size_t count = 0;
        auto t = tail.load(std::memory_order_relaxed);
        auto h = head.load(std::memory_order_acquire);

        while (t != h && count < max_batch) {
            out[count++] = std::move(buffer[t]);
            t = (t + 1) & mask;
        }
        if (count > 0) {
            tail.store(t, std::memory_order_release);
            out.resize(count);
            return true;
        }
        return false;
    }

    bool pop(Task& out) {
        auto t = tail.load(std::memory_order_relaxed);
        if (t == head.load(std::memory_order_acquire)) {
            return false; // empty
        }
        out = std::move(buffer[t]);
        tail.store((t + 1) & mask, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return tail.load(std::memory_order_acquire) ==
               head.load(std::memory_order_acquire);
    }

private:
    const size_t capacity;
    const size_t mask;
    std::vector<Task> buffer;
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};
};

class Scheduler {
public:
    explicit Scheduler(size_t ring_capacity_pow2 = 1 << 21)
        : queue(ring_capacity_pow2) {}

    void spawn(Task (*func)(std::string), std::string name) {
        Task t = func(std::move(name));
        while (!queue.push(std::move(t))) {
            std::this_thread::yield();
        }
    }

    // Start de scheduler in een aparte thread
    void run() {
        worker = std::thread([this] { run_loop(); });
    }

    // Stop de scheduler
    void stop_scheduler() {
        running.store(false, std::memory_order_relaxed);
    }
void enqueue_resume(std::coroutine_handle<> handle) {
    Task t{Task::handle_type::from_address(handle.address())};
    while (!queue.push(std::move(t))) {
        std::this_thread::yield();
    }
}

    // Wacht tot de thread klaar is
    void join() {
        if (worker.joinable()) {
            worker.join();
        }
    }
    void schedule_after(std::coroutine_handle<> h,
                        std::chrono::milliseconds d) {
        auto tp = std::chrono::steady_clock::now() + d;
        {
            std::lock_guard<std::mutex> lock(timer_mutex);
            timers.emplace(tp, h);
        }
    }
private:
    void run_loop() {
        std::vector<Task> batch;

        while (running.load()) {
            batch.clear();
            batch.resize(64);

            // gewone taken
            if (queue.pop_batch(batch, batch.size())) {
                for (auto& task : batch) {
                    task.resume_if_needed();
                }
            }

            // timers checken
            auto now = std::chrono::steady_clock::now();
            std::vector<std::coroutine_handle<>> ready;
            {
                std::lock_guard<std::mutex> lock(timer_mutex);
                while (!timers.empty() && timers.begin()->first <= now) {
                    ready.push_back(timers.begin()->second);
                    timers.erase(timers.begin());
                }
            }
            for (auto h : ready) {
                enqueue_resume(h);
            }

            // kleine pauze om busy‑loop te vermijden
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    }

    SpscQueue queue;
    std::atomic<bool> running{true};
    std::thread worker;

    std::multimap<std::chrono::steady_clock::time_point,
                  std::coroutine_handle<>> timers;
    std::mutex timer_mutex;
};


alignas(64) std::atomic<int> completed{0};
