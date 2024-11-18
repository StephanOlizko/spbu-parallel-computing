#include <iostream>
#include <omp.h>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <fstream>

// Функция для выполнения "тяжёлых" вычислений
void heavy_computation(int& result) {
    for (int i = 0; i < 100000; ++i) {
        result += rand() % 10; // Генерация случайного числа и его добавление
    }
}

// Основная функция
void test_schedule(omp_sched_t schedule_type, const std::string& schedule_name, std::ofstream& log_file) {
    int num_iterations = 1000;
    std::vector<int> results(num_iterations, 0); // Массив для хранения результатов

    // Установка типа планирования для цикла
    omp_set_schedule(schedule_type, 0);

    // Замер времени выполнения параллельного метода
    double parallel_time = 0.0;
    const int num_tests = 5;
    for (int i = 0; i < num_tests; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        #pragma omp parallel for schedule(runtime)
        for (int i = 0; i < num_iterations; ++i) {
            if (i % 10 == 0) { // На каждых 10 итерациях выполняем сложные вычисления
                heavy_computation(results[i]);
            } else { // На остальных - простое присваивание
                results[i] = i;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        parallel_time += std::chrono::duration<double>(end - start).count();
    }
    parallel_time /= num_tests;

    // Замер времени выполнения последовательного метода
    double sequential_time = 0.0;
    for (int i = 0; i < num_tests; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_iterations; ++i) {
            if (i % 10 == 0) { // На каждых 10 итерациях выполняем сложные вычисления
                heavy_computation(results[i]);
            } else { // На остальных - простое присваивание
                results[i] = i;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        sequential_time += std::chrono::duration<double>(end - start).count();
    }
    sequential_time /= num_tests;

    log_file << "Schedule: " << schedule_name << "\n";
    log_file << "Sequential method time: " << sequential_time << " seconds\n";
    log_file << "Parallel method time: " << parallel_time << " seconds\n";
    log_file << "--------------------------------------\n";
}

int main() {
    std::ofstream log_file("6_log.txt");
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    // Установка режима выполнения (runtime) для использования omp_set_schedule
    omp_set_dynamic(0);

    log_file << "Performance of different schedules with non-uniform workload:\n";

    // Тестируем статический режим
    test_schedule(omp_sched_static, "static", log_file);

    // Тестируем динамический режим
    test_schedule(omp_sched_dynamic, "dynamic", log_file);

    // Тестируем направляемый режим
    test_schedule(omp_sched_guided, "guided", log_file);

    log_file.close();
    return 0;
}