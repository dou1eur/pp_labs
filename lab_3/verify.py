import numpy as np
import os

def check_results():
    sizes = [100, 300, 500, 700, 900, 1100, 1300, 1500]
    print(os.getcwd())
    for size in sizes:
        try:
            m1 = np.loadtxt(f"pp_labs/lab_3/results/{size}_1.txt")
            m2 = np.loadtxt(f"pp_labs/lab_3/results/{size}_2.txt")
            cpp_res = np.loadtxt(f"pp_labs/lab_3/results/result_{size}_4.txt")

            py_res = np.dot(m1, m2)

            if np.array_equal(cpp_res, py_res):
                print(f"{size}x{size}: OK")
            else:
                print(f"{size}x{size}: Error")

        except Exception as e:
            print(f"{size}x{size}: Failed - {str(e)}")

if __name__ == "__main__":
    check_results()