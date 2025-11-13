/* busy waiting spin lock */
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <cstdint>
#include <pthread.h>
#include <iostream>
#include <atomic>
#include <vector>
#include <emmintrin.h> // _mm_pause()


constexpr uint16_t num_of_threads {8};
constexpr uint8_t busywait_iters {99};

/*
std::array<uint64_t, num_of_threads> may suffer from false sharing
all threads write to adjacent 8-byte slots on the same cache line
so we aligned each entry to different cache line 
*/
struct alignas(64) Padded {
    uint64_t value;
}; 

// ticket spinlock
class Spinlock {
private:
    std::atomic<uint64_t> taken_in_line {0};
    std::atomic<uint64_t> serving{0}; 
public:
    Spinlock() = default;
    void lock() {
        uint64_t next_place = taken_in_line.fetch_add(1, std::memory_order_relaxed);
        /* check if it next_place turn if not delay using _mm_pause()
           if it is whating to much allow other threads to run */
        while (serving.load(std::memory_order_acquire) != next_place) {
            for (int i = 0; i < busywait_iters; ++i) _mm_pause();
            std::this_thread::yield();
        }
    }

    void unlock() {
        // asm volatile("" : : : "memory"); 
        serving.fetch_add(1, std::memory_order_release); // serving next
    }
};

void custom_spinlock(std::array<Padded, num_of_threads> &max_wait) {

    const uint64_t num_of_iter {1 << 10};

    Spinlock spinlock;

    auto work = [&](uint8_t tid) {
        uint64_t max {0};
        for (uint64_t i=0; i < num_of_iter; i++ ) {
            auto start = std::chrono::steady_clock::now();
            spinlock.lock();
            auto stop = std::chrono::steady_clock::now();
            spinlock.unlock();
            uint64_t diff = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
            max = std::max(max, diff);
        }
        max_wait[tid].value = max;
    };

    // Spawn 
    std::vector<std::jthread> threads;
    for (uint8_t i=0; i< num_of_threads; i++) 
        threads.emplace_back(work, i);

}

int main() {

    std::array<Padded, num_of_threads> max_wait = {};
    custom_spinlock(max_wait);

    for (auto &max: max_wait) {
        std::cout << max.value << ' ' ;
    }
    std::cout << "\n";
    return 0;
}

