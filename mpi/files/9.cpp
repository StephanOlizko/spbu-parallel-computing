#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

// Реализация операции Broadcast с использованием парных обменов
void mpi_broadcast_pairwise(int* data, int size, int rank, int n_processes) {
    int partner;
    // Параллельный обмен данных между процессами
    for (int step = 1; step < n_processes; step *= 2) {
        partner = rank ^ step; // Вычисляем партнера для обмена (по принципу XOR)
        if (partner < n_processes) {
            if (rank < partner) {
                // Если процесс с меньшим рангом, он отправляет данные
                MPI_Send(data, size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            } else {
                // Если процесс с большим рангом, он получает данные
                MPI_Recv(data, size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}

// Реализация операции Scatter с использованием парных обменов
void mpi_scatter_pairwise(int* data, int* local_data, int size, int rank, int n_processes) {
    int chunk_size = size / n_processes; // Размер блока данных для каждого процесса
    int partner;
    // Параллельный обмен данных
    for (int step = 1; step < n_processes; step *= 2) {
        partner = rank ^ step;
        if (partner < n_processes) {
            if (rank < partner) {
                // Если процесс с меньшим рангом, он отправляет данные
                MPI_Send(data + rank * chunk_size, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            } else {
                // Если процесс с большим рангом, он получает данные
                MPI_Recv(local_data, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}

// Реализация операции Gather с использованием парных обменов
void mpi_gather_pairwise(int* local_data, int* data, int size, int rank, int n_processes) {
    int chunk_size = size / n_processes; // Размер блока данных для каждого процесса
    int partner;
    // Параллельный обмен данных
    for (int step = 1; step < n_processes; step *= 2) {
        partner = rank ^ step;
        if (partner < n_processes) {
            if (rank < partner) {
                // Если процесс с меньшим рангом, он получает данные
                MPI_Recv(data + partner * chunk_size, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                // Если процесс с большим рангом, он отправляет данные
                MPI_Send(local_data, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            }
        }
    }
}

// Реализация операции Reduce с использованием парных обменов
void mpi_reduce_pairwise(int* data, int* result, int size, int rank, int n_processes) {
    int partner;
    // Параллельный обмен данных с операцией свертки (например, минимум)
    for (int step = 1; step < n_processes; step *= 2) {
        partner = rank ^ step;
        if (partner < n_processes) {
            if (rank < partner) {
                // Если процесс с меньшим рангом, он отправляет данные
                MPI_Send(data, size, MPI_INT, partner, 0, MPI_COMM_WORLD);
                break;
            } else {
                // Если процесс с большим рангом, он получает данные и выполняет операцию
                MPI_Recv(result, size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (int i = 0; i < size; i++) {
                    result[i] = std::min(result[i], data[i]); // Например, минимизация
                }
                break;
            }
        }
    }
}

// Реализация операции AllGather с использованием парных обменов
void mpi_allgather_pairwise(int* local_data, int* data, int size, int rank, int n_processes) {
    int chunk_size = size / n_processes; // Размер блока данных для каждого процесса
    int partner;
    // Параллельный обмен данных
    for (int step = 1; step < n_processes; step *= 2) {
        partner = rank ^ step;
        if (partner < n_processes) {
            if (rank < partner) {
                // Если процесс с меньшим рангом, он получает данные
                MPI_Recv(data + partner * chunk_size, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                // Если процесс с большим рангом, он отправляет данные
                MPI_Send(local_data, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            }
        }
    }
}


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Ранг процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Число процессов

    int vector_size = 10000; // Размер вектора для тестирования
    std::vector<int> data(vector_size, 0);
    std::vector<int> local_data(vector_size / size, 0);

    // Инициализация данных в процессе с рангом 0
    if (rank == 0) {
        for (int i = 0; i < vector_size; i++) {
            data[i] = rand() % 100; // Заполнение случайными числами
        }
    }

    double start_time, end_time;

    if (rank == 0) {
        std::cout << "Operation,Time (seconds)" << std::endl;
    }

    // Тестирование Broadcast с использованием парных обменов
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    mpi_broadcast_pairwise(data.data(), vector_size, rank, size);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Broadcast (pairwise)," << end_time - start_time << std::endl;
    }

    // Тестирование Scatter с использованием парных обменов
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    mpi_scatter_pairwise(data.data(), local_data.data(), vector_size, rank, size);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Scatter (pairwise)," << end_time - start_time << std::endl;
    }

    // Тестирование Gather с использованием парных обменов
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    mpi_gather_pairwise(local_data.data(), data.data(), vector_size, rank, size);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Gather (pairwise)," << end_time - start_time << std::endl;
    }

    // Тестирование Reduce с использованием парных обменов
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    mpi_reduce_pairwise(data.data(), data.data(), vector_size, rank, size);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Reduce (pairwise)," << end_time - start_time << std::endl;
    }

    // Тестирование AllGather с использованием парных обменов
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    mpi_allgather_pairwise(local_data.data(), data.data(), vector_size, rank, size);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "AllGather (pairwise)," << end_time - start_time << std::endl;
    }

    // Сравнение с встроенными функциями MPI

    // Тестирование Broadcast с использованием MPI_Bcast
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    MPI_Bcast(data.data(), vector_size, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Broadcast (MPI_Bcast)," << end_time - start_time << std::endl;
    }

    // Тестирование Scatter с использованием MPI_Scatter
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    MPI_Scatter(data.data(), vector_size / size, MPI_INT, local_data.data(), vector_size / size, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Scatter (MPI_Scatter)," << end_time - start_time << std::endl;
    }

    // Тестирование Gather с использованием MPI_Gather
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    MPI_Gather(local_data.data(), vector_size / size, MPI_INT, data.data(), vector_size / size, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Gather (MPI_Gather)," << end_time - start_time << std::endl;
    }

    // Тестирование Reduce с использованием MPI_Reduce
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    MPI_Reduce(local_data.data(), data.data(), vector_size / size, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Reduce (MPI_Reduce)," << end_time - start_time << std::endl;
    }

    // Тестирование AllGather с использованием MPI_AllGather
    if (rank == 0) {
        start_time = MPI_Wtime();
    }
    MPI_Allgather(local_data.data(), vector_size / size, MPI_INT, data.data(), vector_size / size, MPI_INT, MPI_COMM_WORLD);
    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "AllGather (MPI_Allgather)," << end_time - start_time << std::endl;
    }

    MPI_Finalize();
    return 0;
}