#include <stdio.h>
#include <omp.h>

int main(void)
{
  int *a = (int *)malloc(10000000 * sizeof(int));
  double start_time, end_time, serial_time, parallel_time;

  // Serial Execution
  start_time = omp_get_wtime();
  for (int i = 0; i < 10000000; i++)
  {
    a[i] = 2 * i + i;
  }
  end_time = omp_get_wtime();
  serial_time = end_time - start_time;
  printf("S execution time: %f seconds\n", serial_time);

  // Parallel Execution
  start_time = omp_get_wtime();
#pragma omp parallel for
  for (int i = 0; i < 10000000; i++)
  {
    a[i] = 2 * i + i;
  }
  end_time = omp_get_wtime();
  parallel_time = end_time - start_time;
  printf("P execution time: %f seconds\n", parallel_time);

  // Speedup Calculation
  printf("Speedup: %.2fx\n", serial_time / parallel_time);

  return 0;
}