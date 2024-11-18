#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <fstream>

int main() {
    std::ofstream log_file("2_log.txt");
    const int num_runs = 5; // Количество запусков для вычисления среднего времени

    for (int size : {1000, 10000, 100000, 1000000, 10000000, 100000000}) {
        std::vector<int> vec1(size, 1);
        std::vector<int> vec2(size, 2);
        int dot_product = 0;

        // Sequential method
        double total_sequential_time = 0;
        for (int run = 0; run < num_runs; run++) {
            auto start = std::chrono::high_resolution_clock::now();
            dot_product = 0;
            for (int i = 0; i < size; i++) {
                dot_product += vec1[i] * vec2[i];
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> sequential_time = end - start;
            total_sequential_time += sequential_time.count();
        }
        double average_sequential_time = total_sequential_time / num_runs;

        // Parallel method with reduction
        double total_reduction_time = 0;
        for (int run = 0; run < num_runs; run++) {
            auto start = std::chrono::high_resolution_clock::now();
            dot_product = 0;
            #pragma omp parallel for reduction(+:dot_product)
            for (int i = 0; i < size; i++) {
                dot_product += vec1[i] * vec2[i];
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> reduction_time = end - start;
            total_reduction_time += reduction_time.count();
        }
        double average_reduction_time = total_reduction_time / num_runs;

        // Log results
        log_file << "Vector size: " << size << std::endl;
        log_file << "Sequential method time: " << average_sequential_time << " ms" << std::endl;
        log_file << "Reduction method time: " << average_reduction_time << " ms" << std::endl;
        log_file << "--------------------------------------" << std::endl;
    }

    log_file.close();
    return 0;
}