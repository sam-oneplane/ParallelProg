#include <benchmark/benchmark.h>
#include <immintrin.h>

#include <random>
#include <cstdlib>
#include <cstring>
#include <iostream>

// g++ dot_product_intrinsic.cpp -o build/vec_intrinsic -O3  -std=c++20 -lbenchmark -lpthread -march=native

float dot_product(const __m256 *v1, const __m256 *v2, size_t packed_elements) {
    auto tmp {0};
    for (size_t i = 0; i < packed_elements; i++) {
        float unpacked[8];
        __m256 result =_mm256_dp_ps(v1[i], v2[i], 0xf1); // multiply and sum into [0] and [4]
        // unpeck _m256 into 8*32bit floats
        std::memcpy(unpacked, &result, sizeof(float)*8);
        tmp += unpacked[0] + unpacked[4]; // final addition after _mm256_dp_ps()
    }
    return tmp;
}

static void dp_intrinsics(benchmark::State &s) {

    const size_t elements {1 << 15};
    // 256 bit packed values
    const size_t num_packed = elements / 8;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution dist(0.0f, 1.0f);

    // align 256 bit into 32 bytes single cache line * num_packed 
    auto v1 = static_cast<__m256 *>(std::aligned_alloc(32, num_packed * sizeof(__m256)));
    auto v2 = static_cast<__m256 *>(std::aligned_alloc(32, num_packed * sizeof(__m256)));
    
    // 8 * 32bit random float in _m256 vector element 
    for (size_t i = 0; i < num_packed; i++) {
        v1[i] = _mm256_set_ps(dist(mt), dist(mt), dist(mt), dist(mt), dist(mt), dist(mt), dist(mt), dist(mt));
        v2[i] = _mm256_set_ps(dist(mt), dist(mt), dist(mt), dist(mt), dist(mt), dist(mt), dist(mt), dist(mt));
    }

    float *result = new float;
    for (auto _: s) {
        // perform dot product of v1 & v2 using transform reduce
        *result = dot_product(v1, v2, num_packed);
    }    

    free(v1);
    free(v2);
    std::cout << *result << std::endl;
}

BENCHMARK(dp_intrinsics)->Unit(benchmark::kMicrosecond)->UseRealTime();
BENCHMARK_MAIN();