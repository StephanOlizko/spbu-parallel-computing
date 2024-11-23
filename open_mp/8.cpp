#include <iostream>
#include <vector>
#include <cstdlib>  // Для rand()
#include <omp.h>
#include <chrono>
#include <fstream>

const int VECTOR_SIZE = 1000;  // Размер каждого вектора
const int NUM_VECTORS = 10;    // Количество пар векторов для обработки

// Функция генерации случайного вектора
std::vector<int> generate_random_vector(int size) {
    std::vector<int> vec(size);
    for (int i = 0; i < size; ++i) {
        vec[i] = rand() % 100;  // Случайное число от 0 до 99
    }
    return vec;
}

// Функция вычисления скалярного произведения двух векторов
int compute_dot_product(const std::vector<int>& vec1, const std::vector<int>& vec2) {
    int dot_product = 0;
    for (int i = 0; i < vec1.size(); ++i) {
        dot_product += vec1[i] * vec2[i];
    }
    return dot_product;
}

int main() {
    std::ofstream log_file("8_log.txt");
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    const int num_tests = 5;
    std::vector<int> sizes = {1000, 10000, 100000, 1000000, 10000000};

    for (int size : sizes) {
        std::vector<std::vector<int>> vectors1(NUM_VECTORS);
        std::vector<std::vector<int>> vectors2(NUM_VECTORS);
        std::vector<int> results(NUM_VECTORS);

        srand(static_cast<unsigned>(time(0)));


        // Последовательное выполнение
        double sequential_time = 0.0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int t = 0; t < num_tests; ++t) {
            // Генерация векторов
            for (int i = 0; i < NUM_VECTORS; ++i) {
                vectors1[i] = generate_random_vector(size);
                vectors2[i] = generate_random_vector(size);
            }

            for (int i = 0; i < NUM_VECTORS; ++i) {
                results[i] = compute_dot_product(vectors1[i], vectors2[i]);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        sequential_time += std::chrono::duration<double, std::milli>(end - start).count();
        sequential_time /= num_tests;

        // Параллельное выполнение
        double parallel_time = 0.0;
        for (int t = 0; t < num_tests; ++t) {
            auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel sections
            {
#pragma omp section
                {
                    // Генерация векторов
                    for (int i = 0; i < NUM_VECTORS; ++i) {
                        vectors1[i] = generate_random_vector(size);
                        vectors2[i] = generate_random_vector(size);
                    }
                }

#pragma omp section
                {
                    // Вычисление скалярного произведения
                    for (int i = 0; i < NUM_VECTORS; ++i) {
                        // Ожидаем генерации векторов
                        while (vectors1[i].empty() || vectors2[i].empty()) {
                            // Active waiting
                        }
                        results[i] = compute_dot_product(vectors1[i], vectors2[i]);
                    }
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            parallel_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        parallel_time /= num_tests;

        // Логирование результатов
        log_file << "Vector size: " << size << "\n";
        log_file << "Sequential method time: " << sequential_time << " ms\n";
        log_file << "Parallel method time: " << parallel_time << " ms\n";
        log_file << "--------------------------------------\n";
    }

    log_file.close();
    std::cout << "All computations completed.\n";
    return 0;
}