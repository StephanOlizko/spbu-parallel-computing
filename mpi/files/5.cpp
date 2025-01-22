#include <mpi.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <chrono>

void do_computations(int delay_us) {
    usleep(delay_us);
}

void send_receive(int msg_size, int rank, int size, int transfers) {
    std::vector<char> send_buf(msg_size, rank);
    std::vector<char> recv_buf(msg_size, 0);

    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            MPI_Send(send_buf.data(), msg_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }

        for (int t = 0; t < transfers; t++) {
            for (int i = 1; i < size; i++) {
                MPI_Recv(recv_buf.data(), msg_size, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    } else {
        MPI_Recv(recv_buf.data(), msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        do_computations(1000 / size);

        for (int t = 0; t < transfers; t++) {
            MPI_Send(send_buf.data(), msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> delays = {1000, 10000, 100000, 1000000};
    std::vector<int> msg_sizes = {1024, 10240, 102400, 1048576};

    if (rank == 0) {
        std::cout << "delay_us,msg_size_bytes,exec_time_sec\n";
    }

    for (int delay : delays) {
        for (int msg_size : msg_sizes) {
            auto start = std::chrono::high_resolution_clock::now();

            send_receive(msg_size, rank, size);

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> exec_time = end - start;

            if (rank == 0) {
                std::cout << delay << ","
                          << msg_size << ","
                          << exec_time.count() << "\n";
            }
        }
    }

    MPI_Finalize();
    return 0;
}
