#include <atomic>
#include <thread>
#include <vector>
#include <cassert>

#include <benchmark/benchmark.h>

// g++ thread_affinity.cpp -o build/thread_affinity -O3  -std=c++20 -lbenchmark -lpthread

// align to cache lines size to reduce cache miss 
struct AlignedAtomic {
    alignas(128) std::atomic<uint32_t> counter = 0;
};

void os_scheduler() {

    AlignedAtomic a1;
    AlignedAtomic a2;
    // cpu sets for pair of thread 0,1 and 2,3 running the same atomic counter
    cpu_set_t cpu_set_0;
    cpu_set_t cpu_set_1;

    CPU_ZERO(&cpu_set_0);
    CPU_ZERO(&cpu_set_1);
    // set cpu cores 0,1
    CPU_SET(0, &cpu_set_0);
    CPU_SET(1, &cpu_set_1);

    auto work = [&](AlignedAtomic &a) {
        for(uint32_t i = 0; i < (1 << 20); i++) a.counter.fetch_add(1);
    };

    std::vector<std::jthread> thread_vec;

    thread_vec.emplace_back(work, std::ref(a1)); 
    thread_vec.emplace_back(work, std::ref(a1)); 
    thread_vec.emplace_back(work, std::ref(a2)); 
    thread_vec.emplace_back(work, std::ref(a2)); 

    // set cpu affinity mask of the threads 0..3 to cpu_set point 0 or one
    assert(pthread_setaffinity_np(thread_vec[0].native_handle(), sizeof(cpu_set_t), &cpu_set_0)== 0);
    assert(pthread_setaffinity_np(thread_vec[1].native_handle(), sizeof(cpu_set_t), &cpu_set_0)== 0);
    assert(pthread_setaffinity_np(thread_vec[2].native_handle(), sizeof(cpu_set_t), &cpu_set_1)== 0);
    assert(pthread_setaffinity_np(thread_vec[3].native_handle(), sizeof(cpu_set_t), &cpu_set_1)== 0);
    // native_handle : get handler from <thread> to pthread
}

static void os_scheduling(benchmark::State &s) {
    while (s.KeepRunning()) 
        os_scheduler(); 
}

BENCHMARK(os_scheduling)->Unit(benchmark::kMicrosecond)->UseRealTime();
BENCHMARK_MAIN();
