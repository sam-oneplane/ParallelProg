#include <tbb/parallel_for.h>

#include <execution>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

// g++ tbb_multithread.cpp -o build/tbb_mt -O3 -std=c++20 -lbenchmark -lbenchmark_main -ltbb -lpthread   

static void tbb_bl(benchmark::State &s) {
    const uint64_t lsize = 1 << 20;

    std::random_device rd;
    std::mt19937 mt(rd());

    std::uniform_real_distribution dist(0.0f, 1.0f);
    std::vector<float> v_in;            
    std::generate_n(std::back_inserter(v_in), lsize, [&](){ return dist(mt); } );

    std::vector<float> v_out(lsize);

    for (auto _ : s) {
        tbb::parallel_for(tbb::blocked_range<int>(0, lsize), 
            [&](tbb::blocked_range<int> r) {
                for(int i = r.begin(); i < r.end(); i++)
                    v_out[i] = v_in[i] * v_in[i];
            }
        );
    }
}

BENCHMARK(tbb_bl)->Unit(benchmark::kMicrosecond)->UseRealTime();
BENCHMARK_MAIN();