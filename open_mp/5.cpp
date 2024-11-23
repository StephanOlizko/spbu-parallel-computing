#include <iostream>
#include <vector>
#include <limits>
#include <omp.h>
#include <chrono>
#include <fstream>

// Генерация ленточной матрицы
std::vector<std::vector<int>> generate_band_matrix(int size, int bandwidth) {
    std::vector<std::vector<int>> matrix(size, std::vector<int>(size, 0));
    for (int i = 0; i < size; i++) {
        for (int j = std::max(0, i - bandwidth); j <= std::min(size - 1, i + bandwidth); j++) {
            matrix[i][j] = rand() % 100;
        }
    }
    return matrix;
}

// Генерация нижнетреугольной матрицы
std::vector<std::vector<int>> generate_lower_triangular_matrix(int size) {
    std::vector<std::vector<int>> matrix(size, std::vector<int>(size, 0));
    for (int i = 0; i < size; i++) {
        for (int j = 0; j <= i; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
    return matrix;
}

// Функция для поиска максимума среди минимумов строк матрицы (параллельная)
int max_of_row_mins_parallel(const std::vector<std::vector<int>>& matrix, const std::string& schedule_type) {
    int max_of_mins = std::numeric_limits<int>::min();

    // Параллельная секция с выбором типа распределения итераций
    #pragma omp parallel for schedule(runtime) reduction(max:max_of_mins)
    for (int i = 0; i < matrix.size(); i++) {
        int min_in_row = std::numeric_limits<int>::max();
        for (int j = 0; j < matrix[i].size(); j++) {
            if (matrix[i][j] < min_in_row) {
                min_in_row = matrix[i][j];
            }
        }
        if (min_in_row > max_of_mins) {
            max_of_mins = min_in_row;
        }
    }
    return max_of_mins;
}

// Функция для поиска максимума среди минимумов строк матрицы (последовательная)
int max_of_row_mins_sequential(const std::vector<std::vector<int>>& matrix) {
    int max_of_mins = std::numeric_limits<int>::min();
    for (const auto& row : matrix) {
        int min_in_row = std::numeric_limits<int>::max();
        for (int val : row) {
            if (val < min_in_row) {
                min_in_row = val;
            }
        }
        if (min_in_row > max_of_mins) {
            max_of_mins = min_in_row;
        }
    }
    return max_of_mins;
}

int main() {
    std::ofstream log_file("5_log.txt");
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    std::vector<int> sizes = {10, 100, 1000, 10000}; // Размеры матриц
    int bandwidth = 5;     // Ширина ленты для ленточной матрицы
    const int num_tests = 5; // Количество тестов для усреднения времени

    // Массив типов распределения
    std::vector<std::string> schedules = {"static", "dynamic", "guided"};

    for (int size : sizes) {
        // Генерация матриц
        auto band_matrix = generate_band_matrix(size, bandwidth);
        auto lower_triangular_matrix = generate_lower_triangular_matrix(size);

        // Тестирование для ленточной матрицы
        log_file << "\nBand matrix results for size " << size << ":\n";
        for (const auto& schedule : schedules) {
            omp_set_schedule(omp_sched_t::omp_sched_static, 0); // Выбор типа распределения

            double parallel_time = 0.0;
            double sequential_time = 0.0;

            for (int i = 0; i < num_tests; i++) {
                auto start = std::chrono::high_resolution_clock::now();
                max_of_row_mins_parallel(band_matrix, schedule);
                auto end = std::chrono::high_resolution_clock::now();
                parallel_time += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                max_of_row_mins_sequential(band_matrix);
                end = std::chrono::high_resolution_clock::now();
                sequential_time += std::chrono::duration<double, std::milli>(end - start).count();
            }

            parallel_time /= num_tests;
            sequential_time /= num_tests;

            log_file << "Schedule: " << schedule << "\n";
            log_file << "Sequential method time: " << sequential_time << " ms\n";
            log_file << "Parallel method time: " << parallel_time << " ms\n";
            log_file << "--------------------------------------\n";
        }

        // Тестирование для нижнетреугольной матрицы
        log_file << "\nLower triangular matrix results for size " << size << ":\n";
        for (const auto& schedule : schedules) {
            omp_set_schedule(omp_sched_t::omp_sched_static, 0); // Выбор типа распределения

            double parallel_time = 0.0;
            double sequential_time = 0.0;

            for (int i = 0; i < num_tests; i++) {
                auto start = std::chrono::high_resolution_clock::now();
                max_of_row_mins_parallel(lower_triangular_matrix, schedule);
                auto end = std::chrono::high_resolution_clock::now();
                parallel_time += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                max_of_row_mins_sequential(lower_triangular_matrix);
                end = std::chrono::high_resolution_clock::now();
                sequential_time += std::chrono::duration<double, std::milli>(end - start).count();
            }

            parallel_time /= num_tests;
            sequential_time /= num_tests;

            log_file << "Schedule: " << schedule << "\n";
            log_file << "Sequential method time: " << sequential_time << " ms\n";
            log_file << "Parallel method time: " << parallel_time << " ms\n";
            log_file << "--------------------------------------\n";
        }
    }

    log_file.close();
    return 0;
}