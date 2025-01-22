#include <mpi.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <chrono>

void do_computations(int delay_us) {
    usleep(delay_us);
}

void send_data(int msg_size, int rank, int size) {
    std::vector<char> send_buf(msg_size, rank);
    MPI_Request req;
    for (int i = 1; i < size; ++i) {
        MPI_Isend(send_buf.data(), msg_size, MPI_CHAR, i, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
    }
}

void receive_data(int msg_size, int rank) {
    std::vector<char> recv_buf(msg_size, 0);
    MPI_Request req;
    MPI_Irecv(recv_buf.data(), msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &req);
    MPI_Wait(&req, MPI_STATUS_IGNORE);
}

void send_back_data(int msg_size, int rank) {
    std::vector<char> send_buf(msg_size, rank);
    MPI_Request req;
    MPI_Isend(send_buf.data(), msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &req);
    MPI_Wait(&req, MPI_STATUS_IGNORE);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> comp_delays = {1000, 10000, 100000, 1000000};
    std::vector<int> msg_sizes = {1024, 10240, 102400, 1048576};

    if (rank == 0) {
        std::cout << "Comp_Delay (us),Msg_Size (bytes),Exec_Time (s)\n";
    }

    for (int delay_us : comp_delays) {
        for (int msg_size : msg_sizes) {
            auto start = std::chrono::high_resolution_clock::now();

            if (rank == 0) {
                send_data(msg_size, rank, size);
                for (int i = 1; i < size; ++i) {
                    receive_data(msg_size, rank);
                }
            } else {
                receive_data(msg_size, rank);
                do_computations(delay_us / size);
                send_back_data(msg_size, rank);
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> exec_time = end - start;

            if (rank == 0) {
                std::cout << delay_us << ","
                          << msg_size << ","
                          << exec_time.count() << "\n";
            }
        }
    }

    MPI_Finalize();
    return 0;
}
