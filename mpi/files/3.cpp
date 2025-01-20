#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstring> // Для memset

int main(int argc, char** argv) {
    // Инициализация MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получаем ранг текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получаем общее количество процессов

    // Массив размеров сообщений для тестирования (в байтах)
    std::vector<int> message_sizes = {1, 10, 100, 1000, 10000, 100000, 1000000};
    // Массив количества обменов для тестирования
    std::vector<int> exchange_counts = {10, 100, 1000, 10000};

    // Заголовок таблицы
    if (rank == 0) {
        std::cout << "Размер сообщения (байт),Кол-во обменов,Среднее время обмена (сек)\n";
    }

    // Перебор всех комбинаций размера сообщения и количества обменов
    for (int message_size : message_sizes) {
        for (int num_exchanges : exchange_counts) {
            // Выделяем буферы для отправки и получения сообщений
            char* send_buffer = new char[message_size];
            char* recv_buffer = new char[message_size];
            std::memset(send_buffer, 0, message_size); // Инициализация буфера для отправки
            std::memset(recv_buffer, 0, message_size); // Инициализация буфера для получения

            // Синхронизация всех процессов перед замером времени
            MPI_Barrier(MPI_COMM_WORLD);

            // Засекаем время перед выполнением обменов
            double start_time = MPI_Wtime();

            // Многократный обмен сообщениями между процессами
            for (int i = 0; i < num_exchanges; ++i) {
                if (rank == 0) {
                    // Процесс 0 отправляет сообщение процессу 1 и получает ответ от него
                    MPI_Send(send_buffer, message_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                    MPI_Recv(recv_buffer, message_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                } else if (rank == 1) {
                    // Процесс 1 получает сообщение от процесса 0 и отправляет ответ обратно
                    MPI_Recv(recv_buffer, message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Send(send_buffer, message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                }
            }

            // Засекаем время после выполнения обменов
            double end_time = MPI_Wtime();
            double total_time = end_time - start_time;

            // Освобождаем выделенную память
            delete[] send_buffer;
            delete[] recv_buffer;

            // Выводим результаты на процесс 0
            if (rank == 0) {
                // Выводим размер сообщения, количество обменов и среднее время одного обмена
                std::cout << message_size << "," 
                          << num_exchanges << ","
                          << total_time / num_exchanges << "\n";
            }
        }
    }

    // Завершаем работу с MPI
    MPI_Finalize();
    return 0;
}
