#include <benchmark/benchmark.h>

#include <algorithm>
#include <execution>
#include <random>
#include <iterator>
#include <vector>
#include <cstdint>
#include <iostream>
#include <memory>

// g++ dot_product_simd.cpp -o build/vec_simd_opt -O2  -std=c++20 -lbenchmark -lpthread  -ftree-vectorize -march=native

// ftree-vectorize :  enable  vectorization 
// -mrach : optmize for native arcitecture
// c++20 for ranges 


static void dp_bench( benchmark::State &s) {

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution dist(0.0f, 1.0f);

    const uint32_t elements {1 << 15};
    std::vector<float> v1;
    std::vector<float> v2;
    // generate random elements for vectors
    std::ranges::generate_n(std::back_inserter(v1), elements, [&]{return dist(mt);} );
    std::ranges::generate_n(std::back_inserter(v2), elements, [&]{return dist(mt);} );

    std::unique_ptr<float> sink = std::make_unique<float>();
    for (auto _: s) {
        // perform dot product of v1 & v2 using transform reduce
        *sink = std::transform_reduce(std::execution::unseq, v1.begin(), v1.end(), v2.begin(), 0.0f);
    }    
}

BENCHMARK(dp_bench)->Unit(benchmark::kMicrosecond)->UseRealTime();
BENCHMARK_MAIN();

