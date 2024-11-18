#include <iostream>
#include <omp.h>
#include <vector>
#include <chrono>
#include <mutex>
#include <fstream>

void log_results(const std::string& method, int vector_size, double time, std::ofstream& log_file) {
    log_file << "Vector size: " << vector_size << "\n";
    log_file << method << " time: " << time << " ms\n";
    log_file << "--------------------------------------\n";
}

int main() {
    std::ofstream log_file("7_log.txt");
    std::vector<int> sizes = {1000, 10000, 100000, 1000000};
    const int num_tests = 5;

    for (int num_elements : sizes) {
        std::vector<int> data(num_elements, 1);
        int sum = 0;
        int expected_sum = num_elements;
        double total_time = 0.0;

        // Последовательное выполнение
        for (int t = 0; t < num_tests; ++t) {
            sum = 0;
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < num_elements; ++i) {
                sum += data[i];
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            total_time += duration.count();
        }
        log_results("Sequential method", num_elements, total_time / num_tests, log_file);

        // Атомарные операции
        total_time = 0.0;
        for (int t = 0; t < num_tests; ++t) {
            sum = 0;
            auto start = std::chrono::high_resolution_clock::now();
            #pragma omp parallel for
            for (int i = 0; i < num_elements; ++i) {
                #pragma omp atomic
                sum += data[i];
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            total_time += duration.count();
        }
        log_results("Atomic method", num_elements, total_time / num_tests, log_file);

        // Критические секции
        total_time = 0.0;
        for (int t = 0; t < num_tests; ++t) {
            sum = 0;
            auto start = std::chrono::high_resolution_clock::now();
            #pragma omp parallel for
            for (int i = 0; i < num_elements; ++i) {
                #pragma omp critical
                sum += data[i];
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            total_time += duration.count();
        }
        log_results("Critical method", num_elements, total_time / num_tests, log_file);

        // Замки (мьютексы)
        total_time = 0.0;
        for (int t = 0; t < num_tests; ++t) {
            sum = 0;
            omp_lock_t lock;
            omp_init_lock(&lock);
            auto start = std::chrono::high_resolution_clock::now();
            #pragma omp parallel for
            for (int i = 0; i < num_elements; ++i) {
                omp_set_lock(&lock);
                sum += data[i];
                omp_unset_lock(&lock);
            }
            auto end = std::chrono::high_resolution_clock::now();
            omp_destroy_lock(&lock);
            std::chrono::duration<double, std::milli> duration = end - start;
            total_time += duration.count();
        }
        log_results("Lock method", num_elements, total_time / num_tests, log_file);

        // Параметр reduction
        total_time = 0.0;
        for (int t = 0; t < num_tests; ++t) {
            sum = 0;
            auto start = std::chrono::high_resolution_clock::now();
            #pragma omp parallel for reduction(+:sum)
            for (int i = 0; i < num_elements; ++i) {
                sum += data[i];
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            total_time += duration.count();
        }
        log_results("Reduction clause method", num_elements, total_time / num_tests, log_file);
    }

    log_file.close();
    return 0;
}