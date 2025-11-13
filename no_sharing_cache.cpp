#include <array>
#include <vector>
#include <atomic>
#include <thread>
#include <cstdint>

// align to cache lines size to reduce cache miss 
struct AlignedAtomic {
    alignas(128) std::atomic<uint64_t> counter = 0;
};

int main() {

    constexpr uint64_t num_iter = 1 << 27;
    constexpr uint8_t num_threads = 4;
    constexpr uint64_t elements_per_threads = num_iter/num_threads;
    
    std::atomic<uint64_t> final = 0;
    std::array<AlignedAtomic, num_threads> counters; // array of 128 bytes aligned atomic counter

    // lambda per thread
    auto work = [&](uint8_t thread_id) {
        for (uint64_t i = 0; i < elements_per_threads; i++)
            counters[thread_id].counter.fetch_add(1);
    };

    // Spawn threads
    std::vector<std::jthread> threads;
    for (uint8_t i=0 ; i < num_threads; i++) 
        threads.emplace_back(work, i);

    return 0;
}