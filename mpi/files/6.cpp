#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>

int dot_product_simple(const std::vector<int>& A, const std::vector<int>& B, int N) {
    int result = 0;
    for (int i = 0; i < N; ++i) {
        result += A[i] * B[i];
    }
    return result;
}

int dot_product_parallel(const std::vector<int>& A, const std::vector<int>& B, int N, int rank, int size, const std::string& mode) {
    int local_N = N / size;
    std::vector<int> local_A(local_N), local_B(local_N);

    MPI_Scatter(A.data(), local_N, MPI_INT, local_A.data(), local_N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(B.data(), local_N, MPI_INT, local_B.data(), local_N, MPI_INT, 0, MPI_COMM_WORLD);

    int local_result = dot_product_simple(local_A, local_B, local_N);

    int global_result = 0;
    MPI_Reduce(&local_result, &global_result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    return global_result;
}

bool verify_result(int seq_result, int parallel_result) {
    return seq_result == parallel_result;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> vec_sizes = {10000, 100000, 1000000};
    std::vector<std::string> modes = {"synchronous", "ready", "buffered"};

    if (rank == 0) {
        std::cout << "vec_size,mode,num_procs,exec_time,correctness\n";
    }

    for (int N : vec_sizes) {
        for (const auto& mode : modes) {
            std::vector<int> A(N), B(N);

            if (rank == 0) {
                std::srand(static_cast<unsigned>(std::time(0)));
                for (int i = 0; i < N; ++i) {
                    A[i] = std::rand() % 1000;
                    B[i] = std::rand() % 1000;
                }
            }

            int seq_result = 0;
            if (rank == 0) {
                seq_result = dot_product_simple(A, B, N);
            }

            auto start_time = std::chrono::high_resolution_clock::now();
            int parallel_result = dot_product_parallel(A, B, N, rank, size, mode);
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> parallel_duration = end_time - start_time;
            double parallel_time = parallel_duration.count();

            if (rank == 0) {
                bool correct = verify_result(seq_result, parallel_result);
                std::cout << N << "," << mode << "," << size << "," << parallel_time << "," << (correct ? "Yes" : "No") << "\n";
            }
        }
    }

    MPI_Finalize();
    return 0;
}
