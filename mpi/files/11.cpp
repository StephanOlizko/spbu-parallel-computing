#include <mpi.h>
#include <iostream>
#include <cmath>
#include <cstdlib>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Ранг процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Число процессов

    // Проверка, что количество процессов — квадратное число
    int grid_size = static_cast<int>(std::sqrt(size));
    if (grid_size * grid_size != size) {
        if (rank == 0) {
            std::cerr << "Количество процессов должно быть квадратом (например, 4, 9, 16, 25, ...)!" << std::endl;
        }
        MPI_Finalize();
        return -1;
    }

    // Вычисление номера строки и столбца для текущего процесса
    int row = rank / grid_size;
    int col = rank % grid_size;

    // Создание коммуникатора для строк
    MPI_Comm row_comm;
    MPI_Comm_split(MPI_COMM_WORLD, row, rank, &row_comm);

    // Создание коммуникатора для столбцов
    MPI_Comm col_comm;
    MPI_Comm_split(MPI_COMM_WORLD, col, rank, &col_comm);

    // Количество измерений
    const int num_measurements = 10;

    // Переменные для накопления времени
    double total_all_time = 0.0;
    double total_row_time = 0.0;
    double total_col_time = 0.0;

    // Выполнение 10 измерений
    for (int i = 0; i < num_measurements; ++i) {
        // Коллективная операция для всех процессов (Reduce)
        int value = rank; // Каждый процесс имеет свое значение
        int global_sum = 0;
        double start_time = MPI_Wtime();
        MPI_Reduce(&value, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); // Суммирование всех процессов
        double end_time = MPI_Wtime();
        total_all_time += end_time - start_time;

        // Коллективная операция для строк (Reduce)
        int row_value = rank;
        int row_sum = 0;
        start_time = MPI_Wtime();
        MPI_Reduce(&row_value, &row_sum, 1, MPI_INT, MPI_SUM, 0, row_comm); // Суммирование в строках
        end_time = MPI_Wtime();
        total_row_time += end_time - start_time;

        // Коллективная операция для столбцов (Reduce)
        int col_value = rank;
        int col_sum = 0;
        start_time = MPI_Wtime();
        MPI_Reduce(&col_value, &col_sum, 1, MPI_INT, MPI_SUM, 0, col_comm); // Суммирование в столбцах
        end_time = MPI_Wtime();
        total_col_time += end_time - start_time;
    }

    // Усреднение времени
    double avg_all_time = total_all_time / num_measurements;
    double avg_row_time = total_row_time / num_measurements;
    double avg_col_time = total_col_time / num_measurements;

    // Вывод результатов в формате CSV
    if (rank == 0) {
        std::cout << "Grid Size,Avg All Time,Avg Row Time,Avg Col Time" << std::endl;
        std::cout << grid_size << "x" << grid_size << "," << avg_all_time << "," << avg_row_time << "," << avg_col_time << std::endl;
    }

    // Завершаем работу с коммуникаторами
    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);

    MPI_Finalize();
    return 0;
}
