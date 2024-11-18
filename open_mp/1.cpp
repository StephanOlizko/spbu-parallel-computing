#include <iostream>
#include <vector>
#include <omp.h>
#include <limits>
#include <chrono>
#include <fstream>

void no_reduction_method(const std::vector<int> &vec, int &max_val, int &min_val) {
    max_val = std::numeric_limits<int>::min();
    min_val = std::numeric_limits<int>::max();

#pragma omp parallel for
    for (int i = 0; i < vec.size(); i++) {
#pragma omp critical
        {
            if (vec[i] > max_val)
                max_val = vec[i];
            if (vec[i] < min_val)
                min_val = vec[i];
        }
    }
}

void reduction_method(const std::vector<int> &vec, int &max_val, int &min_val) {
    max_val = std::numeric_limits<int>::min();
    min_val = std::numeric_limits<int>::max();

#pragma omp parallel for reduction(max : max_val) reduction(min : min_val)
    for (int i = 0; i < vec.size(); i++) {
        if (vec[i] > max_val)
            max_val = vec[i];
        if (vec[i] < min_val)
            min_val = vec[i];
    }
}

void sequential_method(const std::vector<int> &vec, int &max_val, int &min_val) {
    max_val = std::numeric_limits<int>::min();
    min_val = std::numeric_limits<int>::max();

    for (int i = 0; i < vec.size(); i++) {
        if (vec[i] > max_val)
            max_val = vec[i];
        if (vec[i] < min_val)
            min_val = vec[i];
    }
}

int main() {
    std::ofstream log_file("1_log.txt");
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    const int num_tests = 5;

    for (size_t size : {1000, 10000, 100000, 1000000, 10000000}) {
        std::vector<int> vec(size);
        for (size_t i = 0; i < size; i++) {
            vec[i] = rand() % 1000000; // Fill with random values
        }

        int max_val, min_val;

        // Sequential method
        double sequential_time = 0.0;
        for (int i = 0; i < num_tests; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            sequential_method(vec, max_val, min_val);
            auto end = std::chrono::high_resolution_clock::now();
            sequential_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        sequential_time /= num_tests;

        // No reduction method
        double no_reduction_time = 0.0;
        for (int i = 0; i < num_tests; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            no_reduction_method(vec, max_val, min_val);
            auto end = std::chrono::high_resolution_clock::now();
            no_reduction_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        no_reduction_time /= num_tests;

        // Reduction method
        double reduction_time = 0.0;
        for (int i = 0; i < num_tests; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            reduction_method(vec, max_val, min_val);
            auto end = std::chrono::high_resolution_clock::now();
            reduction_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        reduction_time /= num_tests;

        // Log results
        log_file << "Vector size: " << size << "\n";
        log_file << "Sequential method time: " << sequential_time << " ms\n";
        log_file << "No reduction method time: " << no_reduction_time << " ms\n";
        log_file << "Reduction method time: " << reduction_time << " ms\n";
        log_file << "--------------------------------------\n";
    }

    log_file.close();
    return 0;
}
