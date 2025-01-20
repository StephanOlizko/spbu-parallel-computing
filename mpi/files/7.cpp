#include <mpi.h>
#include <iostream>
#include <vector>
#include <unistd.h> // Для usleep
#include <chrono>   // Для замера времени

// Функция для моделирования вычислений
void do_computations(int delay_time_us) {
    usleep(delay_time_us); // Задержка в микросекундах
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Определение ранга процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Определение числа процессов

    // Массивы для параметров задачи
    std::vector<int> computation_delays = {1000, 10000, 100000, 1000000}; // Время вычислений (в мкс)
    std::vector<int> message_sizes = {1024, 10240, 102400, 1048576};      // Размер сообщений (в байтах)
    std::vector<int> num_transfers = {1, 10, 100};                        // Количество пересылок

    // Заголовок таблицы (только на процессе 0)
    if (rank == 0) {
        std::cout << "Объем вычислений (мкс),Размер сообщения (байт),Количество пересылок,Время выполнения (сек)\n";
    }

    // Перебираем комбинации параметров
    for (int delay_time : computation_delays) {
        for (int message_size : message_sizes) {
            for (int transfers : num_transfers) {

                double start_time = MPI_Wtime(); // Засекаем время выполнения

                // Создаем буферы для отправки и приёма
                std::vector<char> send_buffer(message_size, rank); // Буфер отправки
                std::vector<char> recv_buffer(message_size, 0);    // Буфер приёма

                MPI_Request request; // Структура для хранения информации о неблокирующей операции

                if (rank == 0) {
                    // Процесс 0 отправляет данные всем остальным
                    for (int i = 1; i < size; ++i) {
                        MPI_Isend(send_buffer.data(), message_size, MPI_CHAR, i, 0, MPI_COMM_WORLD, &request);
                        MPI_Wait(&request, MPI_STATUS_IGNORE); // Убеждаемся, что операция завершена
                    }

                    // В цикле выполняем пересылки и совмещаем их с вычислениями
                    for (int t = 0; t < transfers; ++t) {
                        for (int i = 1; i < size; ++i) {
                            MPI_Irecv(recv_buffer.data(), message_size, MPI_CHAR, i, 0, MPI_COMM_WORLD, &request);
                            MPI_Wait(&request, MPI_STATUS_IGNORE); // Ждём завершения приёма
                        }
                    }
                } else {
                    // Остальные процессы получают данные от процесса 0
                    MPI_Irecv(recv_buffer.data(), message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &request);
                    do_computations(delay_time / size); // Моделируем вычисления
                    MPI_Wait(&request, MPI_STATUS_IGNORE); // Ждём завершения приёма

                    // Отправляем данные обратно процессу 0
                    for (int t = 0; t < transfers; ++t) {
                        MPI_Isend(send_buffer.data(), message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &request);
                        MPI_Wait(&request, MPI_STATUS_IGNORE); // Ждём завершения отправки
                    }

                }

                double end_time = MPI_Wtime(); // Конец измерения времени

                // Процесс 0 выводит результаты
                if (rank == 0) {
                    std::cout << delay_time << ","
                              << message_size << ","
                              << transfers << ","
                              << end_time - start_time << "\n";
                }
            }
        }
    }

    MPI_Finalize(); // Завершаем работу MPI
    return 0;
}
