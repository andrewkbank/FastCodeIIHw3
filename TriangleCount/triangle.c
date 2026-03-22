/**
 * Main function copied from my HW1 implementation
 * The 3 lines of code added in count_triangles were added by consulting with Gemini 3 Flash
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "rdtsc.h"
#include "omp.h"

unsigned int *IA;
unsigned int *JA;

/*
This function reads the input data. It populates the IA and JA arrays used to store the adjacency matrix of the graph in CSR (compressed sparse row) format.

The CSR is a sparse matrix format can be described with 3 arrays (A, IA, JA). In this assignment, you do not need the A array.

0. The length of IA is the number of vertices in the graph + 1.
1. IA is an array that splits the JA array into multiple subarrays. Specifically, the (IA[k+1] - IA[k]) is the length of the k^{th} subarray of JA.
2. The k^{th} subarray of JA is the neighbor list for the k^{th} vertex.

*/
void input()
{
  unsigned int num_IA = N + 1;
  unsigned int num_JA = NUM_A;

  IA = (unsigned int *)malloc((N + 1) * sizeof(unsigned int));
  JA = (unsigned int *)malloc(NUM_A * sizeof(unsigned int));

  for (unsigned int i = 0; i < num_IA; i++)
    scanf("%d", &IA[i]);
  for (unsigned int i = 0; i < num_JA; i++)
    scanf("%d", &JA[i]);
}

/*
This function computes the number of triangles in the graph

Description of algorithm:

1. A triangle is created with 3 vertices (x, y, z)
2. We assume that the vertices are numbered from 0 to N-1
3. The algorithm iterates over the vertices, and for each vertex (y) with a number i,
      the algorithm checks if there is a triangle (x, y, z) where
        x has a smaller number than i, and z has a larger number than i.

Note: OpenMP date is 201511 meaning it is OpenMP 4.5
Unfortunately, this version doesn't support reduction
*/
unsigned int count_triangles(int num_threads)
{
  unsigned int delta = 0;

// 1. Create a parallel region
#pragma omp parallel num_threads(num_threads)
  {
// 2. Use 'single' so only one thread generates the tasks
// 3. Use 'taskgroup' with reduction to safely aggregate delta
#pragma omp single
#pragma omp taskgroup
    {
      for (unsigned int i = 1; i < N - 1; i++)
      {
// 4. Each iteration of the outer loop becomes a task
#pragma omp task firstprivate(i)
        {
          unsigned int local_delta = 0; // Local counter to minimize atomic pressure

          unsigned int *curr_row_x = IA + i;
          unsigned int *curr_row_A = IA + i + 1;
          unsigned int num_nnz_curr_row_x = *curr_row_A - *curr_row_x;
          unsigned int *x_col_begin = (JA + *curr_row_x);
          unsigned int *x_col_end = x_col_begin;
          unsigned int *row_bound = x_col_begin + num_nnz_curr_row_x;
          unsigned int col_x_min = 0;
          unsigned int col_x_max = i - 1;

          while (x_col_end < row_bound && *x_col_end < col_x_max)
            ++x_col_end;
          x_col_end -= (*x_col_end > col_x_max || x_col_end == row_bound);

          unsigned int *y_col_begin = x_col_end + 1;
          unsigned int *y_col_end = row_bound - 1;
          unsigned int num_nnz_y = (y_col_end - y_col_begin) + 1;
          unsigned int num_nnz_x = (x_col_end - x_col_begin) + 1;

          unsigned int y_col_first = i + 1;
          unsigned int x_col_first = 0;
          unsigned int *y_col = y_col_begin;

          for (unsigned int j = 0; j < num_nnz_y; ++j, ++y_col)
          {
            unsigned int row_index_A = *y_col - y_col_first;
            unsigned int *x_col = x_col_begin;
            unsigned int num_nnz_A = *(curr_row_A + row_index_A + 1) - *(curr_row_A + row_index_A);
            unsigned int *A_col = (JA + *(curr_row_A + row_index_A));
            unsigned int *A_col_max = A_col + num_nnz_A;

            for (unsigned int k = 0; k < num_nnz_x && *A_col <= col_x_max; ++k)
            {
              unsigned int row_index_x = *x_col - x_col_first;
              while ((*A_col < *x_col) && (A_col < A_col_max))
                ++A_col;

              if (*A_col == row_index_x)
              {
                local_delta++;
              }
              ++x_col;
            }
          }
          // Unfortunately, we are forced to use atomic due to no task reduction in our OpenMP version
          #pragma omp atomic
          delta += local_delta;
        }
      }
    }
  }

  return delta;
}

/*
This main function takes as input the number of runs. It reads the data through the input function and then computes the number of triangles. It prints at the end the average execution time for computing the number of triangles given a data set.
*/

int main(int argc, char **argv)
{
  int runs = atoi(argv[1]);
  tsc_counter t0, t1;

  input();

  int thread_counts[] = {1, 2, 4, 8, 16};
  int num_tests = 6;

  for (int t = 0; t < num_tests; ++t)
  {
    int threads = thread_counts[t];
    long long sum1 = 0;

    for (unsigned int i = 0; i != runs; ++i)
    {
      unsigned int x = 0;

      RDTSC(t0);
      x = count_triangles(threads);
      RDTSC(t1);
      // printf("%u\n", x);

      sum1 += COUNTER_DIFF(t1, t0, CYCLES);
    }

    printf("Threads: %d, Avg cycles: %lf\n",
           threads,
           (double)(sum1 / (double)runs));
  }

  free(IA);
  free(JA);
}
