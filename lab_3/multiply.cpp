#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <mpi.h>
#include <unistd.h>
using namespace std;

int get_random_number(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

vector<vector<int>> generate_matrix(int size, int minVal = 1, int maxVal = 100) {
    vector<vector<int>> matrix(size, vector<int>(size));
    for (auto& row : matrix) {
        for (auto& elem : row) {
            elem = get_random_number(minVal, maxVal);
        }
    }
    return matrix;
}

void save_matrix(const vector<vector<int>>& matrix, const string& filename) {
    ofstream out(filename);
    for (const auto& row : matrix) {
        for (size_t i = 0; i < row.size(); ++i) {
            out << row[i];
            if (i != row.size() - 1) out << " ";
        }
        out << "\n";
    }
}

vector<vector<int>> multiply_matrices_mpi(const vector<vector<int>>& A, const vector<vector<int>>& B) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = A.size();
    int m = A[0].size();
    int p = B[0].size();
    vector<vector<int>> result(n, vector<int>(p, 0));

    int rows_per_process = n / size;
    int remaining_rows = n % size;
    int start_row = rank * rows_per_process + min(rank, remaining_rows);
    int end_row = start_row + rows_per_process + (rank < remaining_rows ? 1 : 0);

    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < p; ++j) {
            int sum = 0;
            for (int k = 0; k < m; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }

    if (rank == 0) {
        for (int src = 1; src < size; ++src) {
            int src_start_row = src * rows_per_process + min(src, remaining_rows);
            int src_end_row = src_start_row + rows_per_process + (src < remaining_rows ? 1 : 0);

            for (int i = src_start_row; i < src_end_row; ++i) {
                MPI_Recv(result[i].data(), p, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    } else {
        for (int i = start_row; i < end_row; ++i) {
            MPI_Send(const_cast<int*>(result[i].data()), p, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    return result;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, mpi_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    const string results_dir = "results";
    ofstream stats;

    if (rank == 0) {
        stats.open("statistics.txt");
    }

    for (int matrix_size = 100; matrix_size <= 1500; matrix_size += 200) {
        if (rank == 0) {
            cout << "Size: " << matrix_size << "x" << matrix_size << endl;
        }

        vector<vector<int>> matrix1, matrix2;

        if (rank == 0) {
            matrix1 = generate_matrix(matrix_size);
            matrix2 = generate_matrix(matrix_size);

            string file1 = results_dir + "/" + to_string(matrix_size) + "_1.txt";
            string file2 = results_dir + "/" + to_string(matrix_size) + "_2.txt";
            save_matrix(matrix1, file1);
            save_matrix(matrix2, file2);
        }

        if (rank == 0) {
            for (int i = 1; i < mpi_size; ++i) {
                for (const auto& row : matrix2) {
                    MPI_Send(const_cast<int*>(row.data()), matrix_size, MPI_INT, i, 0, MPI_COMM_WORLD);
                }
            }
        } else {
            matrix2.resize(matrix_size, vector<int>(matrix_size));
            for (int i = 0; i < matrix_size; ++i) {
                MPI_Recv(matrix2[i].data(), matrix_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        if (rank == 0) {
            for (int i = 1; i < mpi_size; ++i) {
                int rows_per_process = matrix_size / mpi_size;
                int remaining_rows = matrix_size % mpi_size;
                int start_row = i * rows_per_process + min(i, remaining_rows);
                int end_row = start_row + rows_per_process + (i < remaining_rows ? 1 : 0);

                for (int row = start_row; row < end_row; ++row) {
                    MPI_Send(const_cast<int*>(matrix1[row].data()), matrix_size, MPI_INT, i, 1, MPI_COMM_WORLD);
                }
            }
        }

        int rows_per_process = matrix_size / mpi_size;
        int remaining_rows = matrix_size % mpi_size;
        int start_row = rank * rows_per_process + min(rank, remaining_rows);
        int end_row = start_row + rows_per_process + (rank < remaining_rows ? 1 : 0);

        matrix1.resize(matrix_size, vector<int>(matrix_size));
        if (rank != 0) {
            for (int row = start_row; row < end_row; ++row) {
                MPI_Recv(matrix1[row].data(), matrix_size, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        auto start = chrono::high_resolution_clock::now();

        auto result = multiply_matrices_mpi(matrix1, matrix2);

        MPI_Barrier(MPI_COMM_WORLD);
        auto end = chrono::high_resolution_clock::now();

        if (rank == 0) {
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            stats << matrix_size << "\t" << mpi_size << "\t" << duration.count() << " ms\n";
            cout << "time: " << duration.count() << " ms\n";

            string result_file = results_dir + "/result_" + to_string(matrix_size) + "_" +
                               to_string(mpi_size) + ".txt";
            save_matrix(result, result_file);
        }
    }

    if (rank == 0) {
        stats.close();
        cout << "check statistics.txt\n";
    }
    MPI_Finalize();
    return 0;
}