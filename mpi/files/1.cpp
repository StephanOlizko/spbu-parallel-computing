#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>

int calc_seq_min(const std::vector<int>& data) {
    return *std::min_element(data.begin(), data.end());
}

int calc_par_min(const std::vector<int>& data, int local_size, int rank, int size) {
    std::vector<int> local_data(local_size);

    MPI_Scatter(data.data(), local_size, MPI_INT, 
                local_data.data(), local_size, MPI_INT, 
                0, MPI_COMM_WORLD);

    int local_min = *std::min_element(local_data.begin(), local_data.end());

    int global_min;
    MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    return global_min;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> vec_sizes = {1000, 10000, 100000, 1000000, 10000000};

    if (rank == 0) {
        std::cout << "vec_size,proc_count,seq_time,par_time,min\n";
    }

    for (int N : vec_sizes) {
        std::vector<int> data;
        int local_size = N / size; 

        double seq_time = 0.0;
        int seq_min = 0;
        if (rank == 0) {

            data.resize(N);
            std::srand(static_cast<unsigned>(std::time(0)));
            for (int i = 0; i < N; ++i) {
                data[i] = std::rand() % 1000;
            }

            auto seq_start = std::chrono::high_resolution_clock::now();
            seq_min = calc_seq_min(data);
            auto seq_end = std::chrono::high_resolution_clock::now();
            seq_time = std::chrono::duration<double>(seq_end - seq_start).count();
        }

        int global_min = 0;
        double par_time = 0.0;

        auto par_start = std::chrono::high_resolution_clock::now();
        global_min = calc_par_min(data, local_size, rank, size);
        auto par_end = std::chrono::high_resolution_clock::now();
        par_time = std::chrono::duration<double>(par_end - par_start).count();

        if (rank == 0) {
            std::cout << N << ","
                      << size << ","
                      << seq_time << ","
                      << par_time << ","
                      << global_min << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
