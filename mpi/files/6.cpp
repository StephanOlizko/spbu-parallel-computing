int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> vector_sizes = {10000, 100000, 1000000};
    std::vector<std::string> modes = {"synchronous", "ready", "buffered"};

    if (rank == 0) {
        std::cout << "vector_size,mode,num_processes,execution_time,correctness\n";
    }

    for (int N : vector_sizes) {
        for (const auto& mode : modes) {
            std::vector<int> A(N), B(N);

            if (rank == 0) {
                std::srand(static_cast<unsigned>(std::time(0)));
                for (int i = 0; i < N; ++i) {
                    A[i] = std::rand() % 10;
                    B[i] = std::rand() % 10;
                }
            }

            // Последовательное вычисление
            int seq_result = 0;
            if (rank == 0) {
                seq_result = dot_product_simple(A, B, N);
            }

            // Параллельное вычисление
            auto start_time = std::chrono::high_resolution_clock::now();
            int parallel_result = dot_product_parallel(A, B, N, rank, size, mode);
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> parallel_duration = end_time - start_time;
            double parallel_time = parallel_duration.count();

            if (rank == 0) {
                bool correct = verify_result(seq_result, parallel_result);

                std::cout << N << "," << mode << "," << size << "," << parallel_time << "," << (correct ? "Yes" : "No") << "\n";
            }
        }
    }

    MPI_Finalize();
    return 0;
}
