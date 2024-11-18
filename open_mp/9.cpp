#include <iostream>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <climits>
#include <fstream>

int max_of_mins_sequential(const std::vector<std::vector<int>>& matrix) {
    int max_of_mins = INT_MIN;
    for (const auto& row : matrix) {
        int min_in_row = INT_MAX;
        for (int val : row) {
            if (val < min_in_row) min_in_row = val;
        }
        if (min_in_row > max_of_mins) max_of_mins = min_in_row;
    }
    return max_of_mins;
}

int main() {
    std::ofstream log_file("9_log.txt");
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    const int num_tests = 5;
    for (int N : {10, 100, 1000}) {
        std::vector<std::vector<int>> matrix(N, std::vector<int>(N));
        srand(time(0));
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                matrix[i][j] = rand() % 1000;
            }
        }

        // Sequential method
        double sequential_time = 0.0;
        for (int i = 0; i < num_tests; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            max_of_mins_sequential(matrix);
            auto end = std::chrono::high_resolution_clock::now();
            sequential_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        sequential_time /= num_tests;

        // Nested parallelism
        omp_set_nested(1);
        double nested_time = 0.0;
        for (int i = 0; i < num_tests; i++) {
            int global_max_nested = 0;
            auto start = std::chrono::high_resolution_clock::now();
            #pragma omp parallel
            {
                int local_min;
                #pragma omp for nowait
                for (int i = 0; i < N; ++i) {
                    local_min = INT_MAX;
                    #pragma omp parallel for reduction(min: local_min)
                    for (int j = 0; j < N; ++j) {
                        local_min = std::min(local_min, matrix[i][j]);
                    }
                    #pragma omp critical
                    {
                        global_max_nested = std::max(global_max_nested, local_min);
                    }
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            nested_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        nested_time /= num_tests;

        // Non-nested parallelism
        omp_set_nested(0);
        double non_nested_time = 0.0;
        for (int i = 0; i < num_tests; i++) {
            int global_max_non_nested = 0;
            auto start = std::chrono::high_resolution_clock::now();
            #pragma omp parallel for
            for (int i = 0; i < N; ++i) {
                int local_min = INT_MAX;
                for (int j = 0; j < N; ++j) {
                    local_min = std::min(local_min, matrix[i][j]);
                }
                #pragma omp critical
                {
                    global_max_non_nested = std::max(global_max_non_nested, local_min);
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            non_nested_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        non_nested_time /= num_tests;

        // Log results
        log_file << "Matrix size: " << N << std::endl;
        log_file << "Sequential method time: " << sequential_time << " ms" << std::endl;
        log_file << "Nested parallelism time: " << nested_time << " ms" << std::endl;
        log_file << "Non-nested parallelism time: " << non_nested_time << " ms" << std::endl;
        log_file << "--------------------------------------" << std::endl;
    }

    log_file.close();
    return 0;
}