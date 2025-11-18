#include <cassert>
#include <iostream>

// g++ openmp_parallel.cpp -0 build/openmp_parallel -fopenmp -O3 -std=c++20 

int main() {

    const int iterations {1 << 20};
    const int nthreads {8};
    const int per_thread {iterations/nthreads};

    int sink {0};

    #pragma omp parallel num_threads(8) 
    {
        for (int i = 0; i < per_thread; i++) {
            #pragma omp atomic
            sink++; 
        }
    }
    assert(sink == iterations);
    std::cout << "iterations  = " << iterations << std::endl;
    return 0;
} 