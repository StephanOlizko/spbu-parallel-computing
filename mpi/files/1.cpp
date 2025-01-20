#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Ранг процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Число процессов

    // Массив размеров векторов для тестирования
    std::vector<int> vector_sizes = {1000, 10000, 100000, 1000000, 10000000};

    if (rank == 0) {
        // Вывод заголовка таблицы
        std::cout << "vector_size,process_count,seq_time,par_time,min,max\n";
    }

    for (int N : vector_sizes) {
        std::vector<int> data;
        int local_size = N / size; // Размер подмассива для каждого процесса
        std::vector<int> local_data(local_size);

        double seq_start_time = 0.0, seq_end_time = 0.0;

        if (rank == 0) {
            // Генерация данных
            data.resize(N);
            std::srand(static_cast<unsigned>(std::time(0)));
            for (int i = 0; i < N; ++i) {
                data[i] = std::rand() % 1000; // Заполнение случайными числами
            }

            // Последовательный поиск минимума и максимума
            seq_start_time = MPI_Wtime();
            int seq_min = *std::min_element(data.begin(), data.end());
            int seq_max = *std::max_element(data.begin(), data.end());
            seq_end_time = MPI_Wtime();
        }

        // Начало замера параллельного времени
        double par_start_time = MPI_Wtime();

        // Распределение данных между процессами
        MPI_Scatter(data.data(), local_size, MPI_INT, 
                    local_data.data(), local_size, MPI_INT, 
                    0, MPI_COMM_WORLD);

        // Локальный поиск минимума и максимума
        int local_min = *std::min_element(local_data.begin(), local_data.end());
        int local_max = *std::max_element(local_data.begin(), local_data.end());

        // Глобальное уменьшение для нахождения глобального минимума и максимума
        int global_min, global_max;
        MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

        // Конец замера параллельного времени
        double par_end_time = MPI_Wtime();

        // Вывод результатов в таблицу
        if (rank == 0) {
            std::cout << N << ","
                      << size << ","
                      << seq_end_time - seq_start_time << ","
                      << par_end_time - par_start_time << ","
                      << global_min << ","
                      << global_max << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
