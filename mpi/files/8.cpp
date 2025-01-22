#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>

void exchange_messages(int msg_size, int num_exchanges, int rank) {
    char* send_buf = new char[msg_size];
    char* recv_buf = new char[msg_size];
    std::memset(send_buf, 0, msg_size);
    std::memset(recv_buf, 0, msg_size);

    for (int i = 0; i < num_exchanges; ++i) {
        if (rank == 0) {
            MPI_Sendrecv(send_buf, msg_size, MPI_CHAR, 1, 0,
                         recv_buf, msg_size, MPI_CHAR, 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else if (rank == 1) {
            MPI_Sendrecv(send_buf, msg_size, MPI_CHAR, 0, 0,
                         recv_buf, msg_size, MPI_CHAR, 0, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    delete[] send_buf;
    delete[] recv_buf;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    std::vector<int> msg_sizes = {1, 10, 100, 1000, 10000, 100000, 1000000};

    if (rank == 0) {
        std::cout << "msg_size,exchange_count,avg_time\n";
    }

    for (int msg_size : msg_sizes) {
        MPI_Barrier(MPI_COMM_WORLD);

        auto start = std::chrono::high_resolution_clock::now();

        exchange_messages(msg_size, 10, rank);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        if (rank == 0) {
            std::cout << msg_size << ","
                      << elapsed.count() / 10 << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
