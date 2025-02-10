#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include "scalar_add.h"
#include "neon_add.h"

struct BenchmarkResult {
    int size;
    double scalar_time;
    double neon_time;
    double speedup;
};

BenchmarkResult benchmark_add(int size, int iterations) {
    // Allocate memory for arrays
    int *a = new int[size];
    int *b = new int[size];

    // Random data generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 100);
    for (int i = 0; i < size; i++) {
        a[i] = dist(gen);
        b[i] = dist(gen);
    }

    // Scalar implementation timing
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        add_scalar(size, a, b);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double scalar_time = std::chrono::duration<double>(end - start).count() / iterations;

    // Reset arrays for fair comparison
    for (int i = 0; i < size; i++) {
        a[i] = dist(gen);
        b[i] = dist(gen);
    }

    // NEON implementation timing
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        add_neon(size, a, b);
    }
    end = std::chrono::high_resolution_clock::now();
    double neon_time = std::chrono::duration<double>(end - start).count() / iterations;

    // Compute speedup
    double speedup = scalar_time / neon_time;

    // Clean up
    delete[] a;
    delete[] b;

    // Return results
    return {size, scalar_time, neon_time, speedup};
}

void print_table_header() {
    std::cout << std::setw(10) << "Size"
              << " | " << std::setw(15) << "Scalar Time (s)"
              << " | " << std::setw(15) << "NEON Time (s)"
              << " | " << std::setw(10) << "Speedup"
              << std::endl;
    std::cout << std::string(60, '-') << std::endl;
}

void print_table_row(const BenchmarkResult &result) {
    std::cout << std::setw(10) << result.size
              << " | " << std::setw(15) << std::scientific << result.scalar_time
              << " | " << std::setw(15) << std::scientific << result.neon_time
              << " | " << std::setw(10) << std::fixed << std::setprecision(2) << result.speedup
              << std::endl;
}

int main() {
    // Array sizes to test
    const int sizes[] = {4, 8, 16, 32, 64, 128, 512, 1024, 4096, 16384, 65536, 262144, 1048576};
    const int num_tests = 10000;

    // Print table header
    print_table_header();

    // Benchmark each size and print the result
    for (int size : sizes) {
        BenchmarkResult result = benchmark_add(size, num_tests);
        print_table_row(result);
    }

    return 0;
}