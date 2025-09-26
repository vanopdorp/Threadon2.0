#include "routines.hpp"
#include <iostream>

// Single-shot coroutine: 1 resume, geen suspend binnenin, minimale overhead.
Task benchmarkRoutine(std::string /*name*/) {
    completed.fetch_add(1, std::memory_order_relaxed);
    co_return;
}
int main() {
    constexpr int numCoroutines = 5000; // Aantal coroutines voor de benchmark
    Scheduler scheduler;

    auto start = std::chrono::high_resolution_clock::now();

    std::thread worker([&] { scheduler.run(); });

    for (int i = 0; i < numCoroutines; ++i) {
        scheduler.spawn(benchmarkRoutine, "routine_" + std::to_string(i));
    }

    while (completed.load() < numCoroutines) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    scheduler.stop_scheduler();
    worker.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Benchmark: " << numCoroutines << " coroutines in "
              << elapsed.count() << " seconds\n";
    std::cout << "Completed: " << completed.load() << "\n";
}
