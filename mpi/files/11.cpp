#include <mpi.h>
#include <iostream>
#include <chrono>
#include <cmath>

void all_reduce(int rank, int& global_sum) {
    int value = rank;
    MPI_Reduce(&value, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
}

void row_reduce(int rank, MPI_Comm row_comm, int& row_sum) {
    int value = rank;
    MPI_Reduce(&value, &row_sum, 1, MPI_INT, MPI_SUM, 0, row_comm);
}

void col_reduce(int rank, MPI_Comm col_comm, int& col_sum) {
    int value = rank;
    MPI_Reduce(&value, &col_sum, 1, MPI_INT, MPI_SUM, 0, col_comm);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int grid_size = static_cast<int>(sqrt(size));
    int row = rank / grid_size;
    int col = rank % grid_size;

    MPI_Comm row_comm;
    MPI_Comm_split(MPI_COMM_WORLD, row, rank, &row_comm);

    MPI_Comm col_comm;
    MPI_Comm_split(MPI_COMM_WORLD, col, rank, &col_comm);

    const int num_meas = 10;
    double total_all_time = 0.0;
    double total_row_time = 0.0;
    double total_col_time = 0.0;

    for (int i = 0; i < num_meas; ++i) {
        int global_sum = 0;
        auto start_time = std::chrono::high_resolution_clock::now();
        all_reduce(rank, global_sum);
        auto end_time = std::chrono::high_resolution_clock::now();
        total_all_time += std::chrono::duration<double>(end_time - start_time).count();

        int row_sum = 0;
        start_time = std::chrono::high_resolution_clock::now();
        row_reduce(rank, row_comm, row_sum);
        end_time = std::chrono::high_resolution_clock::now();
        total_row_time += std::chrono::duration<double>(end_time - start_time).count();

        int col_sum = 0;
        start_time = std::chrono::high_resolution_clock::now();
        col_reduce(rank, col_comm, col_sum);
        end_time = std::chrono::high_resolution_clock::now();
        total_col_time += std::chrono::duration<double>(end_time - start_time).count();
    }

    double avg_all_time = total_all_time / num_meas;
    double avg_row_time = total_row_time / num_meas;
    double avg_col_time = total_col_time / num_meas;

    if (rank == 0) {
        std::cout << "Grid Size,Avg All Time,Avg Row Time,Avg Col Time" << std::endl;
        std::cout << grid_size << "x" << grid_size << "," << avg_all_time << "," << avg_row_time << "," << avg_col_time << std::endl;
    }

    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);

    MPI_Finalize();
    return 0;
}
