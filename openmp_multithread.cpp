#include <random>
#include <vector>
#include <omp.h>
#include <benchmark/benchmark.h>

// g++-12 openmp_multithread.cpp -o build/openmd_mt -fopenmp -O3 -std=c++20 -lbenchmark -lpthread

static void openmp_bl(benchmark::State &s) {
    const int lsize = 1 << 20;

    std::random_device rd;
    std::mt19937 mt(rd());

    std::uniform_real_distribution dist(0.0f, 1.0f);
    std::vector<float> v_in;            
    std::generate_n(std::back_inserter(v_in), lsize, [&](){ return dist(mt); } );

    std::vector<float> v_out(lsize);

    for (auto _: s) {
        // openmp c++11 attribute 
        [[omp::sequence(directive(parallel), directive(for))]]
        for (int i=0; i < lsize; i++ )
            v_out[i] = v_in[i] * v_in[i];
    }   
}
BENCHMARK(openmp_bl)->Unit(benchmark::kMicrosecond)->UseRealTime();
BENCHMARK_MAIN();
