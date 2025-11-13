/* busy waiting spin lock */
#include <algorithm>
#include <random>
#include <chrono>
#include <list>
#include <thread>
#include <cstdint>
#include <pthread.h>
#include <iostream>


void c_style_spinlock() {

    constexpr uint64_t lsize {1 << 10};
    constexpr uint16_t num_of_threads {8};


    std::random_device rd;
    std::mt19937 mt(rd());

    // 4 bins to generate random number with uniform dist.
    std::uniform_int_distribution bin(10, 25);

    std::list<uint16_t> lst;
    std::generate_n(std::back_inserter(lst), lsize, [&](){ return bin(mt); } );

    // define spin lock
    pthread_spinlock_t spinlock;
    pthread_spin_init(&spinlock, 0);

    auto work = [&]() {
        while (true) {
            pthread_spin_lock(&spinlock); // faster then std::mutex && lock_guard / unique_lock
            if(lst.empty()) {
                // release the lock and break
                pthread_spin_unlock(&spinlock);
                break;
            }
            lst.pop_back();
            pthread_spin_unlock(&spinlock);
        }
        //std::cout << "break out with list size: " << lst.size() << std::endl;
    };
    // Spawn 
    std::vector<std::jthread> threads;
    for (uint8_t i=0; i< num_of_threads; i++) 
        threads.emplace_back(work);

}

int main() {
    c_style_spinlock();

    return 0;
}
