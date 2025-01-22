#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <ctime>
#include <chrono>

struct Data {
    int id;
    float val;
    char name[1000010];
};

struct Data1 {
    int id;
    float val;
    char* name;
};

void sendWithoutPack(Data1& data, int rank) {
    if (rank == 0) {
        MPI_Send(&data, sizeof(data), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        MPI_Recv(&data, sizeof(data), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void sendWithPack(Data& data, int rank, int dataSize) {
    int bufferSize;
    MPI_Pack_size(1, MPI_INT, MPI_COMM_WORLD, &bufferSize);
    MPI_Pack_size(1, MPI_FLOAT, MPI_COMM_WORLD, &bufferSize);
    MPI_Pack_size(dataSize, MPI_CHAR, MPI_COMM_WORLD, &bufferSize);

    int totalBufferSize = bufferSize * 3;
    char* buffer = new char[totalBufferSize];

    if (rank == 0) {
        int pos = 0;
        MPI_Pack(&data.id, 1, MPI_INT, buffer, totalBufferSize, &pos, MPI_COMM_WORLD);
        MPI_Pack(&data.val, 1, MPI_FLOAT, buffer, totalBufferSize, &pos, MPI_COMM_WORLD);
        MPI_Pack(&data.name, dataSize, MPI_CHAR, buffer, totalBufferSize, &pos, MPI_COMM_WORLD);
        MPI_Send(buffer, totalBufferSize, MPI_BYTE, 1, 1, MPI_COMM_WORLD);
    } else if (rank == 1) {
        MPI_Recv(buffer, totalBufferSize, MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int pos = 0;
        MPI_Unpack(buffer, totalBufferSize, &pos, &data.id, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, totalBufferSize, &pos, &data.val, 1, MPI_FLOAT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, totalBufferSize, &pos, &data.name, dataSize, MPI_CHAR, MPI_COMM_WORLD);
    }

    delete[] buffer;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> sizes = {10, 100, 1000, 10000, 100000, 1000000};

    if (rank == 0) {
        std::cout << "data_size,without_pack_time,with_pack_time" << std::endl;
    }

    for (int dataSize : sizes) {
        Data data;
        Data1 data1;
        data1.name = new char[dataSize + 1];

        data.id = 123;
        data.val = 456.789f;
        std::memset(data.name, 'A', dataSize);
        data.name[dataSize] = '\0';

        data1.id = 123;
        data1.val = 456.789f;
        std::memset(data1.name, 'A', dataSize);
        data1.name[dataSize] = '\0';

        auto start1 = std::chrono::high_resolution_clock::now();
        sendWithoutPack(data1, rank);
        auto end1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration1 = end1 - start1;

        auto start2 = std::chrono::high_resolution_clock::now();
        sendWithPack(data, rank, dataSize);
        auto end2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration2 = end2 - start2;

        if (rank == 0) {
            std::cout << dataSize << "," << duration1.count() << "," << duration2.count() << std::endl;
        }

        delete[] data1.name;
    }

    MPI_Finalize();
    return 0;
}
