#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

void mpi_broadcast(int* data, int size, int rank, int n_procs) {
    int partner;
    for (int step = 1; step < n_procs; step *= 2) {
        partner = rank ^ step;
        if (partner < n_procs) {
            if (rank < partner) {
                MPI_Send(data, size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            } else {
                MPI_Recv(data, size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}

void mpi_scatter(int* data, int* local_data, int size, int rank, int n_procs) {
    int chunk_size = size / n_procs;
    int partner;
    for (int step = 1; step < n_procs; step *= 2) {
        partner = rank ^ step;
        if (partner < n_procs) {
            if (rank < partner) {
                MPI_Send(data + rank * chunk_size, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            } else {
                MPI_Recv(local_data, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}

void mpi_gather(int* local_data, int* data, int size, int rank, int n_procs) {
    int chunk_size = size / n_procs;
    int partner;
    for (int step = 1; step < n_procs; step *= 2) {
        partner = rank ^ step;
        if (partner < n_procs) {
            if (rank < partner) {
                MPI_Recv(data + partner * chunk_size, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Send(local_data, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            }
        }
    }
}

void mpi_reduce(int* data, int* result, int size, int rank, int n_procs) {
    int partner;
    for (int step = 1; step < n_procs; step *= 2) {
        partner = rank ^ step;
        if (partner < n_procs) {
            if (rank < partner) {
                MPI_Send(data, size, MPI_INT, partner, 0, MPI_COMM_WORLD);
                break;
            } else {
                MPI_Recv(result, size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (int i = 0; i < size; i++) {
                    result[i] = std::min(result[i], data[i]);
                }
                break;
            }
        }
    }
}

void mpi_allgather(int* local_data, int* data, int size, int rank, int n_procs) {
    int chunk_size = size / n_procs;
    int partner;
    for (int step = 1; step < n_procs; step *= 2) {
        partner = rank ^ step;
        if (partner < n_procs) {
            if (rank < partner) {
                MPI_Recv(data + partner * chunk_size, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Send(local_data, chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            }
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    int vec_size = 10000;
    std::vector<int> data(vec_size, 0);
    std::vector<int> local_data(vec_size / n_procs, 0);

    if (rank == 0) {
        for (int i = 0; i < vec_size; i++) {
            data[i] = rand() % 100;
        }
    }

    if (rank == 0) {
        std::cout << "Operation,Time (seconds)" << std::endl;
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        mpi_broadcast(data.data(), vec_size, rank, n_procs);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "Broadcast (pairwise)," << elapsed.count() << std::endl;
    } else {
        mpi_broadcast(data.data(), vec_size, rank, n_procs);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        mpi_scatter(data.data(), local_data.data(), vec_size, rank, n_procs);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "Scatter (pairwise)," << elapsed.count() << std::endl;
    } else {
        mpi_scatter(data.data(), local_data.data(), vec_size, rank, n_procs);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        mpi_gather(local_data.data(), data.data(), vec_size, rank, n_procs);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "Gather (pairwise)," << elapsed.count() << std::endl;
    } else {
        mpi_gather(local_data.data(), data.data(), vec_size, rank, n_procs);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        mpi_reduce(data.data(), data.data(), vec_size, rank, n_procs);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "Reduce (pairwise)," << elapsed.count() << std::endl;
    } else {
        mpi_reduce(data.data(), data.data(), vec_size, rank, n_procs);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        mpi_allgather(local_data.data(), data.data(), vec_size, rank, n_procs);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "AllGather (pairwise)," << elapsed.count() << std::endl;
    } else {
        mpi_allgather(local_data.data(), data.data(), vec_size, rank, n_procs);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        MPI_Bcast(data.data(), vec_size, MPI_INT, 0, MPI_COMM_WORLD);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "Broadcast (MPI_Bcast)," << elapsed.count() << std::endl;
    } else {
        MPI_Bcast(data.data(), vec_size, MPI_INT, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        MPI_Scatter(data.data(), vec_size / n_procs, MPI_INT, local_data.data(), vec_size / n_procs, MPI_INT, 0, MPI_COMM_WORLD);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "Scatter (MPI_Scatter)," << elapsed.count() << std::endl;
    } else {
        MPI_Scatter(data.data(), vec_size / n_procs, MPI_INT, local_data.data(), vec_size / n_procs, MPI_INT, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        MPI_Gather(local_data.data(), vec_size / n_procs, MPI_INT, data.data(), vec_size / n_procs, MPI_INT, 0, MPI_COMM_WORLD);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "Gather (MPI_Gather)," << elapsed.count() << std::endl;
    } else {
        MPI_Gather(local_data.data(), vec_size / n_procs, MPI_INT, data.data(), vec_size / n_procs, MPI_INT, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        MPI_Reduce(local_data.data(), data.data(), vec_size / n_procs, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "Reduce (MPI_Reduce)," << elapsed.count() << std::endl;
    } else {
        MPI_Reduce(local_data.data(), data.data(), vec_size / n_procs, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        auto start_time = high_resolution_clock::now();
        MPI_Allgather(local_data.data(), vec_size / n_procs, MPI_INT, data.data(), vec_size / n_procs, MPI_INT, MPI_COMM_WORLD);
        auto end_time = high_resolution_clock::now();
        duration<double> elapsed = end_time - start_time;
        std::cout << "AllGather (MPI_Allgather)," << elapsed.count() << std::endl;
    } else {
        MPI_Allgather(local_data.data(), vec_size / n_procs, MPI_INT, data.data(), vec_size / n_procs, MPI_INT, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
