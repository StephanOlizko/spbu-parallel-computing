#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <ctime>

struct MyData {
    int id;
    float value;
    char name[1000010]; // Увеличиваем размер строки для эксперимента
};

struct MyData1 {
    int id;
    float value;
    char* name; // Строка будет динамической
};

// Функция для передачи данных без упаковки
void send_without_pack(MyData1& data, int rank) {
    if (rank == 0) {
        MPI_Send(&data, sizeof(data), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        MPI_Recv(&data, sizeof(data), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

// Функция для передачи данных с упаковкой
void send_with_pack(MyData& data, int rank, int data_size) {
    int buffer_size;

    // Определение размера буфера для передачи одного целого числа (MPI_INT)
    MPI_Pack_size(1, MPI_INT, MPI_COMM_WORLD, &buffer_size);
    // Определение размера буфера для передачи одного числа с плавающей запятой (MPI_FLOAT)
    MPI_Pack_size(1, MPI_FLOAT, MPI_COMM_WORLD, &buffer_size);
    // Определение размера буфера для передачи строки из data_size символов (MPI_CHAR)
    MPI_Pack_size(data_size, MPI_CHAR, MPI_COMM_WORLD, &buffer_size);

    // Общий размер буфера, необходимый для упаковки всех данных
    int total_buffer_size = buffer_size * 3;
    // Выделение памяти под буфер
    char* buffer = new char[total_buffer_size];

    if (rank == 0) {
        int position = 0;

        // Упаковка данных из структуры 'data' в буфер
        MPI_Pack(&data.id, 1, MPI_INT, buffer, total_buffer_size, &position, MPI_COMM_WORLD);
        MPI_Pack(&data.value, 1, MPI_FLOAT, buffer, total_buffer_size, &position, MPI_COMM_WORLD);
        MPI_Pack(&data.name, data_size, MPI_CHAR, buffer, total_buffer_size, &position, MPI_COMM_WORLD);

        // Отправка упакованных данных процессу с рангом 1
        MPI_Send(buffer, total_buffer_size, MPI_BYTE, 1, 1, MPI_COMM_WORLD);
    } else if (rank == 1) {
        // Получение данных из буфера, отправленных процессом с рангом 0
        MPI_Recv(buffer, total_buffer_size, MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int position = 0;

        // Распаковка данных из буфера в структуру 'data'
        MPI_Unpack(buffer, total_buffer_size, &position, &data.id, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, total_buffer_size, &position, &data.value, 1, MPI_FLOAT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, total_buffer_size, &position, &data.name, data_size, MPI_CHAR, MPI_COMM_WORLD);
    }

    // Освобождение выделенной памяти для буфера
    delete[] buffer;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        std::cerr << "Программа должна быть запущена с 2 процессами!" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    std::vector<int> sizes = {10, 100, 1000, 10000, 100000, 1000000}; // Размеры данных для эксперимента

    if (rank == 0) {
        std::cout << "data_size,without_pack_time,with_pack_time" << std::endl;
    }

    for (int data_size : sizes) {
        MyData data;
        MyData received_data;

        // Заполняем данные с разным размером строки
        data.id = 123;
        data.value = 456.789f;
        std::memset(data.name, 'A', data_size); // Заполняем строку символами 'A'
        data.name[data_size] = '\0'; // Завершаем строку

        MyData1 data1;
        MyData1 received_data1;

        // Выделяем память для строки на основе размера
        data1.name = new char[data_size + 1];

        // Заполняем данные с разным размером строки
        data1.id = 123;
        data1.value = 456.789f;
        std::memset(data1.name, 'A', data_size); // Заполняем строку символами 'A'
        data1.name[data_size] = '\0'; // Завершаем строку

        // Время передачи без упаковки
        double start_time1 = MPI_Wtime();
        send_without_pack(data1, rank);
        double end_time1 = MPI_Wtime();

        // Время передачи с упаковкой
        double start_time = MPI_Wtime();
        send_with_pack(data, rank, data_size);
        double end_time = MPI_Wtime();

        if (rank == 0) {
            std::cout << data_size << "," << end_time1 - start_time1 << "," << end_time - start_time << std::endl;
        }

        delete[] data1.name;
    }

    MPI_Finalize();
    return 0;
}
