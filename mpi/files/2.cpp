#include <mpi.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <chrono>

long long seq_dot_prod(const std::vector<int>& v1, const std::vector<int>& v2) {
    return std::inner_product(v1.begin(), v1.end(), v2.begin(), 0LL);
}

long long par_dot_prod(const std::vector<int>& v1, const std::vector<int>& v2, int rank, int size) {
    int N = v1.size();
    int local_N = N / size;
    std::vector<int> local_v1(local_N), local_v2(local_N);

    MPI_Scatter(v1.data(), local_N, MPI_INT, local_v1.data(), local_N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2.data(), local_N, MPI_INT, local_v2.data(), local_N, MPI_INT, 0, MPI_COMM_WORLD);

    long long local_res = std::inner_product(local_v1.begin(), local_v1.end(), local_v2.begin(), 0LL);

    long long global_res = 0;
    MPI_Reduce(&local_res, &global_res, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    return global_res;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> vec_sizes = {1000, 10000, 100000, 1000000, 10000000};

    if (rank == 0) {
        std::cout << "vec_size,proc_count,seq_time,par_time,result\n";
    }

    for (int N : vec_sizes) {
        std::vector<int> v1, v2;
        if (rank == 0) {
            v1.resize(N);
            v2.resize(N);
            std::srand(static_cast<unsigned>(std::time(0)));
            for (int i = 0; i < N; ++i) {
                v1[i] = std::rand() % 1000;
                v2[i] = std::rand() % 1000;
            }
        }

        auto seq_start = std::chrono::high_resolution_clock::now();
        long long seq_res = 0;
        if (rank == 0) {
            seq_res = seq_dot_prod(v1, v2);
        }
        auto seq_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> seq_time = seq_end - seq_start;

        auto par_start = std::chrono::high_resolution_clock::now();
        long long global_res = par_dot_prod(v1, v2, rank, size);
        auto par_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> par_time = par_end - par_start;

        if (rank == 0) {
            std::cout << N << ","
                      << size << ","
                      << seq_time.count() << ","
                      << par_time.count() << ","
                      << global_res << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
