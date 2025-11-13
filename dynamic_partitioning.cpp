#include <algorithm>
#include <atomic>
#include <random>
#include <chrono>
#include <iterator>
#include <thread>
#include <vector>
#include <cstdint>

void dync_patitions() {

    std::random_device rd;
    std::mt19937 mt(rd());

    // 4 bins to generate random number with uniform dist.
    std::uniform_int_distribution bin_1(1, 25);
    std::uniform_int_distribution bin_2(26, 50);
    std::uniform_int_distribution bin_3(50, 75);
    std::uniform_int_distribution bin_4(76, 100);

    uint64_t num_work_items = (1 << 18);
    uint16_t num_bins {4};
    uint64_t elements_per_bin {num_work_items/num_bins};
    uint16_t num_of_threads {8};


    std::vector<uint16_t> work_items;
    // populate vector with random numbers from bin_1 - bint_4
    std::generate_n(std::back_inserter(work_items), elements_per_bin, [&] {return bin_1(mt);}); 
    std::generate_n(std::back_inserter(work_items), elements_per_bin, [&] {return bin_2(mt);}); 
    std::generate_n(std::back_inserter(work_items), elements_per_bin, [&] {return bin_3(mt);});
    std::generate_n(std::back_inserter(work_items), elements_per_bin, [&] {return bin_4(mt);});

    std::atomic<uint64_t> index {0}; // single atomic index

    // define worker lambda
    auto work = [&]() {
        for (uint64_t i = index.fetch_add(1); i < num_work_items ; i = index.fetch_add(1)) {
            // sleep for work_items[i]
            std::this_thread::sleep_for(std::chrono::microseconds(work_items[i]));
        }
    };

    std::vector<std::jthread> threads;
    for (int i=0; i < num_of_threads; i++) {
        threads.emplace_back(work);
    }
}

// g++ dync_partitioning.cpp -o dync_partitioning -O3 -lpthread --std=c++20
int main() {
    dync_patitions();
    return 0;
}