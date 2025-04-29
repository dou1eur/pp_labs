import os
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

def check_multiply(size, threads=1):
    base_path = Path('pp_labs') / 'lab_2' / 'results'
    
    m1 = np.loadtxt(base_path / f'{size}_1.txt')
    m2 = np.loadtxt(base_path / f'{size}_2.txt')
    cpp_result = np.loadtxt(base_path / f'result_{size}_{threads}.txt')
    
    py_result = np.dot(m1, m2)
    
    if np.allclose(cpp_result, py_result, atol=1e-6):
        print(f'Size {size}x{size}, threads {threads}: results match')
        return True
    else:
        print(f'Size {size}x{size}, threads {threads}: results do not match')
        return False

def plot_performance():
    thread_counts = [1, 2, 4, 8]
    size_data = {}
    with open(Path('pp_labs') / 'lab_2' / 'statistics.txt', 'r') as f:
        for line in f:
            if not line.strip():
                continue
            parts = line.strip().split('\t')
            size = int(parts[0])
            threads = int(parts[1])
            time = float(parts[2].split()[0])
            
            if size not in size_data:
                size_data[size] = {}
            size_data[size][threads] = time
    plt.figure(figsize=(12, 8))
    
    for threads in thread_counts:
        sizes = sorted(size_data.keys())
        times = [size_data[size].get(threads, 0) for size in sizes]
        plt.plot(sizes, times, 'o-', label=f'{threads} потоков')
    
    plt.title('Производительность умножения матриц')
    plt.xlabel('Размер матрицы (N x N)')
    plt.ylabel('Время выполнения (мс)')
    plt.grid(True)
    plt.legend()
    plt.savefig(Path('pp_labs') / 'lab_2' / 'results' / 'visual.png')
    plt.show()

if __name__ == '__main__':
    all_correct = True
    thread_counts = [1, 2, 4, 8]
    
    for size in range(100, 1501, 200):
        for threads in thread_counts:
            try:
                if not check_multiply(size, threads):
                    all_correct = False
            except Exception as e:
                print(f"fata for size not found, {e}")
                continue
    
    if all_correct:
        print("\nAll results match")
    else:
        print("\nSome results differ")
    
    plot_performance()