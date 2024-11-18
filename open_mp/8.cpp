#include <iostream>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>

int main() {
    const int N = 1000;  // Размер векторов

    // Создаем два вектора
    std::vector<int> vec1(N), vec2(N);

    // Генерация случайных чисел для векторов с разными начальными значениями (seed)
    auto start = std::chrono::high_resolution_clock::now();
    #pragma omp parallel sections
    {
        // Генерация первого вектора
        #pragma omp section
        {
            // Генератор случайных чисел с уникальным seed для каждого потока
            std::random_device rd;  // Источник случайных чисел
            std::mt19937 gen(rd()); // Генератор случайных чисел
            std::uniform_int_distribution<> dis(0, 99); // Диапазон значений

            for (int i = 0; i < N; ++i) {
                vec1[i] = dis(gen);  // Генерация случайных чисел для первого вектора
            }
        }

        // Генерация второго вектора
        #pragma omp section
        {
            // Генератор случайных чисел с уникальным seed для каждого потока
            std::random_device rd;  // Источник случайных чисел
            std::mt19937 gen(rd()); // Генератор случайных чисел
            std::uniform_int_distribution<> dis(0, 99); // Диапазон значений

            for (int i = 0; i < N; ++i) {
                vec2[i] = dis(gen);  // Генерация случайных чисел для второго вектора
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> gen_duration = end - start;
    std::cout << "Time for generating vectors: " << gen_duration.count() << " seconds\n";

    // Вывод первых 10 элементов каждого вектора
    std::cout << "First 10 elements of vec1: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << vec1[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "First 10 elements of vec2: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << vec2[i] << " ";
    }
    std::cout << std::endl;

    // Вычисление скалярного произведения
    int dot_product = 0;
    start = std::chrono::high_resolution_clock::now();
    #pragma omp parallel for reduction(+:dot_product)
    for (int i = 0; i < N; ++i) {
        dot_product += vec1[i] * vec2[i];
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dot_product_duration = end - start;
    
    std::cout << "Dot product: " << dot_product << "\n";
    std::cout << "Time for calculating dot product: " << dot_product_duration.count() << " seconds\n";

    return 0;
}
