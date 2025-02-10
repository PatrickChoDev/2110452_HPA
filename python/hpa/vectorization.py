import numpy as np
import time

# Experiment function
def measure_execution_time(size, iterations=10):
    # Python Lists
    a = list(range(size))
    b = list(range(size))
    
    start = time.time()
    for _ in range(iterations):
        for i in range(size):
            a[i] += b[i]
    python_time = (time.time() - start) / iterations

    # NumPy Arrays
    na = np.random.randint(1, 1000, size, dtype=np.int32)
    nb = np.random.randint(1, 1000, size, dtype=np.int32)
    
    start = time.time()
    for _ in range(iterations):
        na += nb
    numpy_time = (time.time() - start) / iterations

    numpy_speedup = python_time / numpy_time

    # Results
    results = {
        "Python Time": python_time,
        "NumPy Time": numpy_time,
        "NumPy Speedup": numpy_speedup,
    }
    return results

# Experiment
size = 6400000  # Array size
iterations = 100
results = measure_execution_time(size, iterations)

# Print Results
print(f"{'Implementation':<15} | {'Time (s)':<10} | {'Speedup':<10}")
print("-" * 40)
print(f"{'Python':<15} | {results['Python Time']:<10.6f} | {'1.00':<10}")
print(f"{'NumPy':<15} | {results['NumPy Time']:<10.6f} | {results['NumPy Speedup']:<10.2f}")