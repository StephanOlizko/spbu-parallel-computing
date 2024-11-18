#include <iostream>
#include <vector>
#include <omp.h>
#include <limits>
#include <chrono>
#include <fstream>

// Функция для последовательного выполнения
int max_of_mins_sequential(const std::vector<std::vector<int>>& matrix) {
    int max_of_mins = std::numeric_limits<int>::min();
    for (const auto& row : matrix) {
        int min_in_row = std::numeric_limits<int>::max();
        for (int val : row) {
            if (val < min_in_row) min_in_row = val;
        }
        if (min_in_row > max_of_mins) max_of_mins = min_in_row;
    }
    return max_of_mins;
}

// Функция для параллельного выполнения
int max_of_mins_parallel(const std::vector<std::vector<int>>& matrix) {
    int max_of_mins = std::numeric_limits<int>::min();
    #pragma omp parallel for
    for (int i = 0; i < matrix.size(); i++) {
        int min_in_row = std::numeric_limits<int>::max();
        for (int j = 0; j < matrix[i].size(); j++) {
            if (matrix[i][j] < min_in_row) min_in_row = matrix[i][j];
        }
        #pragma omp critical
        {
            if (min_in_row > max_of_mins) max_of_mins = min_in_row;
        }
    }
    return max_of_mins;
}

int main() {
    std::ofstream log_file("4_log.txt");
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    const int num_tests = 5;
    for (int size : {100, 1000, 10000}) {
        std::vector<std::vector<int>> matrix(size, std::vector<int>(size));
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
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

        // Parallel method
        double parallel_time = 0.0;
        for (int i = 0; i < num_tests; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            max_of_mins_parallel(matrix);
            auto end = std::chrono::high_resolution_clock::now();
            parallel_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        parallel_time /= num_tests;

        // Log results
        log_file << "Matrix size: " << size << std::endl;
        log_file << "Sequential method time: " << sequential_time << " ms" << std::endl;
        log_file << "Parallel method time: " << parallel_time << " ms" << std::endl;
        log_file << "--------------------------------------" << std::endl;
    }

    log_file.close();
    return 0;
}