#include <thread>
#include <vector>
#include <cstdint>
#include <span>
#include <semaphore>
#include <random>

void generate(std::span<uint16_t> data) {
    std::random_device rd;
    std::mt19937 mt(rd());

    std::uniform_int_distribution dist(1, 100);
    for (auto &val: data)
        val = dist(mt);
}

void process(std::span<uint16_t> data) {
    for (auto &val: data)
        val %= (100-val);
}

/* g++ double_buffer.cpp -o double_buff -O3 -lpthread --std=c++20 */
int main() {

    constexpr uint16_t iter = 100;

    std::vector<uint16_t> data_1;
    data_1.resize(1 << 20);
    std::vector<uint16_t> data_2;
    data_2.resize(1 << 20);

    std::binary_semaphore sig_process {0};
    std::binary_semaphore sig_generator {1};

    auto generator = [&]() {
        //swap 100 times
        for (uint16_t i = 0; i < iter; i++) {
            generate(data_1);
            sig_generator.acquire(); /* wait for processor release data_2 for swap*/
            data_1.swap(data_2);
            sig_process.release(); /* signal consumer to start process data_2 after swap */

        }
    };

    auto processor = [&]() {
        for (uint16_t i = 0; i < iter; i++) {
            sig_process.acquire(); /* wait for generator to generate data_2 */
            process(data_2);
            sig_generator.release(); /* signal generator to generate swap data_2 */
        }
    };

    std::jthread gen(generator);
    std::jthread proc(processor); 

    return 0;
};


