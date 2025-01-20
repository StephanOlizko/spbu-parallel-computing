#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <numeric> // Для std::inner_product

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Ранг процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Число процессов

    // Массив размеров векторов для тестирования
    std::vector<int> vector_sizes = {1000, 10000, 100000, 1000000, 10000000};

    if (rank == 0) {
        // Вывод заголовка таблицы
        std::cout << "vector_size,process_count,seq_time,par_time,result\n";
    }

    for (int N : vector_sizes) {
        std::vector<int> vec1, vec2;
        int local_size = N / size; // Размер подмассива для каждого процесса
        std::vector<int> local_vec1(local_size), local_vec2(local_size);

        double seq_start_time = 0.0, seq_end_time = 0.0;
        long long scalar_result_seq = 0;

        if (rank == 0) {
            // Генерация данных
            vec1.resize(N);
            vec2.resize(N);
            std::srand(static_cast<unsigned>(std::time(0)));
            for (int i = 0; i < N; ++i) {
                vec1[i] = std::rand() % 100;
                vec2[i] = std::rand() % 100;
            }

            // Последовательное вычисление скалярного произведения
            seq_start_time = MPI_Wtime();
            scalar_result_seq = std::inner_product(vec1.begin(), vec1.end(), vec2.begin(), 0LL);
            seq_end_time = MPI_Wtime();
        }

        // Начало замера параллельного времени
        double par_start_time = MPI_Wtime();

        // Распределение данных между процессами
        MPI_Scatter(vec1.data(), local_size, MPI_INT, local_vec1.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(vec2.data(), local_size, MPI_INT, local_vec2.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

        // Локальное вычисление скалярного произведения
        long long local_result = std::inner_product(local_vec1.begin(), local_vec1.end(), local_vec2.begin(), 0LL);

        // Глобальное суммирование локальных результатов
        long long global_result = 0;
        MPI_Reduce(&local_result, &global_result, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

        // Конец замера параллельного времени
        double par_end_time = MPI_Wtime();

        // Вывод результатов в таблицу
        if (rank == 0) {
            std::cout << N << ","
                      << size << ","
                      << seq_end_time - seq_start_time << ","
                      << par_end_time - par_start_time << ","
                      << global_result << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
