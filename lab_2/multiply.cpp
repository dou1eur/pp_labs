#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <omp.h>

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

vector<vector<int>> multiply_matrices(const vector<vector<int>>& A, const vector<vector<int>>& B, int num_threads) {
    int n = A.size();
    int m = A[0].size();
    int p = B[0].size();
    vector<vector<int>> result(n, vector<int>(p, 0));
    
    omp_set_num_threads(num_threads);
    #pragma omp parallel for shared(A, B, result) schedule(static)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < p; ++j) {
            int sum = 0;
            for (int k = 0; k < m; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

int main() {
    const string results_dir = "C:/Users/Yana/Documents/parallel_prog/pp_labs/lab_2/results";
    ofstream stats("statistics.txt");
    vector<int> thread_counts = {1, 2, 4, 8};
    for (int size = 100; size <= 1500; size += 200) {
        cout << "Size: " << size << "x" << size << endl;
        
        auto matrix1 = generate_matrix(size);
        auto matrix2 = generate_matrix(size);
        
        string file1 = results_dir + "/" + to_string(size) + "_1.txt";
        string file2 = results_dir + "/" + to_string(size) + "_2.txt";
        save_matrix(matrix1, file1);
        save_matrix(matrix2, file2);
        
        for (int threads : thread_counts) {
            cout << "  threads: " << threads << endl;
            auto start = chrono::high_resolution_clock::now();
            auto result = multiply_matrices(matrix1, matrix2, threads);
            auto end = chrono::high_resolution_clock::now();
    
            string result_file = results_dir + "/result_" + to_string(size) + "_" + to_string(threads) + ".txt";
            save_matrix(result, result_file);
            
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            stats << size << "\t" << threads << "\t" << duration.count() << " ms\n";
            
            cout << "time: " << duration.count() << " ms\n";
        }
    }
    stats.close();
    cout << "check statistics.txt\n";
    return 0;
}