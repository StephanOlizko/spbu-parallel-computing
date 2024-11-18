#include <iostream>
#include <omp.h>
#include <chrono>
#include <fstream>

double f(double x) {
    return x * x; // Пример функции f(x) = x^2
}

double integrate_parallel(double a, double b, int n) {
    double h = (b - a) / n;
    double integral = 0.0;

    #pragma omp parallel for reduction(+:integral)
    for (int i = 0; i < n; i++) {
        double x = a + i * h;
        integral += f(x) * h;
    }

    return integral;
}

double integrate_sequential(double a, double b, int n) {
    double h = (b - a) / n;
    double integral = 0.0;

    for (int i = 0; i < n; i++) {
        double x = a + i * h;
        integral += f(x) * h;
    }

    return integral;
}

int main() {
    std::ofstream log_file("3_log.txt");
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    const int num_tests = 5;
    for (auto [a, b] : {std::make_pair(0.0, 1.0), std::make_pair(0.0, 10.0), std::make_pair(0.0, 100.0), std::make_pair(0.0, 1000.0), std::make_pair(0.0, 10000.0)}) {
        int n = 1000000;
        double parallel_time = 0.0;
        double sequential_time = 0.0;

        for (int i = 0; i < num_tests; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            integrate_parallel(a, b, n);
            auto end = std::chrono::high_resolution_clock::now();
            parallel_time += std::chrono::duration<double, std::milli>(end - start).count();

            start = std::chrono::high_resolution_clock::now();
            integrate_sequential(a, b, n);
            end = std::chrono::high_resolution_clock::now();
            sequential_time += std::chrono::duration<double, std::milli>(end - start).count();
        }

        parallel_time /= num_tests;
        sequential_time /= num_tests;

        log_file << "Integration limits: [" << a << ", " << b << "]\n";
        log_file << "Sequential method time: " << sequential_time << " ms\n";
        log_file << "Parallel method time: " << parallel_time << " ms\n";
        log_file << "--------------------------------------\n";
    }

    log_file.close();
    return 0;
}