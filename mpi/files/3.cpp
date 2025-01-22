#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>

void exchange_msgs(int rk, int msg_size, int num_exch) {
    char* snd_buf = new char[msg_size];
    char* rcv_buf = new char[msg_size];
    std::memset(snd_buf, 0, msg_size);
    std::memset(rcv_buf, 0, msg_size);

    for (int i = 0; i < num_exch; ++i) {
        if (rk == 0) {
            MPI_Send(snd_buf, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(rcv_buf, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else if (rk == 1) {
            MPI_Recv(rcv_buf, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(snd_buf, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
    }

    delete[] snd_buf;
    delete[] rcv_buf;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rk, sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &rk);
    MPI_Comm_size(MPI_COMM_WORLD, &sz);

    std::vector<int> msg_sizes = {1, 10, 100, 1000, 10000, 100000, 1000000};

    if (rk == 0) {
        std::cout << "Размер сообщения (байт),Среднее время обмена (сек)\n";
    }

    for (int msg_size : msg_sizes) {
        MPI_Barrier(MPI_COMM_WORLD);

        auto start = std::chrono::high_resolution_clock::now();

        exchange_msgs(rk, msg_size, 10);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> total_time = end - start;

        if (rk == 0) {
            std::cout << msg_size << "," 
                      << total_time.count() / 10 << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
