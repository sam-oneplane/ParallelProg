#include <chrono>
#include <thread>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <atomic>
#include <vector>

void non_blocking() {
    constexpr uint32_t iterations {1 << 15};
    constexpr uint8_t num_threads {8};

    std::atomic<uint32_t> sink {0};

    auto work = [&] () {

        while (true) {
            uint32_t desired;
            uint32_t expected = sink.load();
            do {
                if (expected == iterations) return;
                desired = expected + 1;
            } while (!sink.compare_exchange_strong(expected, desired));
        }
    };

    auto slow_work = [&] () {

        while (true) {
            uint32_t desired;
            uint32_t expected = sink.load();
            do {
                if (expected == iterations) return;
                desired = expected + 1;
            } while (!sink.compare_exchange_strong(expected, desired));
             // try to slow done thread exec in a non blocking case (will not effect the total time) 
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    };

    // wait free work for multi thread
    auto wait_free_work = [&] () {
        for (int i = 0; i < (iterations/ num_threads) ; i++) 
            sink++;
    } ;

    // Spawn 
    std::vector<std::jthread> threads;
    for (uint8_t i=0; i< num_threads-2; i++) 
        threads.emplace_back(work);

    threads.emplace_back(slow_work);
    threads.emplace_back(slow_work);

    // join threads to count iterations
    for (auto &thread: threads) thread.join();

    std::cout << "Completed : " << sink << " iterations\n";
}


int main() {
    non_blocking();
    return 0;
}