#include <mpi.h>
#include <iostream>
#include <unistd.h> // Для sleep
#include <vector>
#include <chrono>   // Для замера времени

// Функция для моделирования вычислений с задержкой
void do_computations(int delay_time_us) {
    usleep(delay_time_us);  // Задержка в микросекундах
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Ранг процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Количество процессов

    // Массив различных объемов вычислений (в микросекундах)
    std::vector<int> computation_delays = {1000, 10000, 100000, 1000000}; // Разные задержки

    // Массив размеров сообщений для коммуникации (размеры в байтах)
    std::vector<int> message_sizes = {1024, 10240, 102400, 1048576}; // Разные размеры сообщений

    // Массив различных значений количества пересылок
    std::vector<int> num_transfers = {1, 10, 100};  // Разные количества пересылок

    // Вывод заголовка таблицы в формате CSV
    if (rank == 0) {
        std::cout << "computation_delay_us,message_size_bytes,num_transfers,execution_time_sec\n";
    }

    // Перебор различных комбинаций объемов вычислений, размеров сообщений и количества пересылок
    for (int delay_time : computation_delays) {
        for (int message_size : message_sizes) {
            for (int transfers : num_transfers) {

                double start_time = MPI_Wtime();  // Начало измерения времени

                // Создаем буфер для отправки и получения данных
                std::vector<char> send_buffer(message_size, rank); // Буфер с размером message_size
                std::vector<char> recv_buffer(message_size, 0);    // Буфер для получения данных

                // Процесс 0 рассылает всем остальным их номера
                if (rank == 0) {
                    for (int i = 1; i < size; i++) {
                        MPI_Send(send_buffer.data(), message_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);  // Отправляем данные
                    }

                    // Количество пересылок сообщений
                    for (int t = 0; t < transfers; t++) {
                        for (int i = 1; i < size; i++) {
                            MPI_Recv(recv_buffer.data(), message_size, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        }
                    }
                } else {
                    // Все остальные процессы получают данные
                    MPI_Recv(recv_buffer.data(), message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    // Моделируем вычисления с задержкой
                    do_computations(delay_time / size); // Делим на количество процессов

                    // Количество пересылок сообщений
                    for (int t = 0; t < transfers; t++) {
                        MPI_Send(send_buffer.data(), message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD); // Отправляем данные обратно
                    }
                }

                double end_time = MPI_Wtime();  // Конец измерения времени

                // Вывод времени выполнения на процесс 0 в формате CSV
                if (rank == 0) {
                    std::cout << delay_time << ","
                              << message_size << ","
                              << transfers << ","
                              << end_time - start_time << "\n";
                }
            }
        }
    }

    MPI_Finalize();
    return 0;
}
