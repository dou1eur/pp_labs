import os
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

def check_multiply(size):
    base_path = Path('pp_labs') / 'lab_1' / 'results'
    
    m1 = np.loadtxt(base_path / f'{size}_1.txt')
    m2 = np.loadtxt(base_path / f'{size}_2.txt')
    cpp_result = np.loadtxt(base_path / f'result_{size}.txt')
    
    py_result = np.dot(m1, m2)
    
    if np.allclose(cpp_result, py_result, atol=1e-6):
        print(f'Size {size}x{size}: results match')
        return True
    else:
        print(f'Size {size}x{size}: results do not match')
        return False

def plot_performance():
    sizes = []
    times = []
    
    with open(Path('pp_labs') / 'lab_1' / 'statistics.txt', 'r') as f:
        for line in f:
            size, t = line.strip().split('\t')
            sizes.append(int(size))
            times.append(float(t.split()[0]))
    
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, times, 'o-', label='Время выполнения')
    
    plt.title('Зависимость времени умножения матриц от их размера')
    plt.xlabel('Размер матрицы (N x N)')
    plt.ylabel('Время (мс)')
    plt.grid(True)
    plt.legend()
    
    output_path = Path('pp_labs') / 'lab_1' / 'results' / 'visual.png'
    plt.savefig(output_path)
    plt.show()

if __name__ == '__main__':
    all_correct = True
    for size in range(100, 2101, 200):
        if not check_multiply(size):
            all_correct = False
    
    if all_correct:
        print("\nAll results match")
    else:
        print("\nSome results differ")
    
    plot_performance()