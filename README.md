**How to run my implementations:**

**TriangleCount**

Every Makefile command will run 100 iterations of 1, 2, 4, 8, and 16 threads. It will output the average number of cycles each threadnum takes in the following format:

Threads: 1, Avg cycles: 4789556.320000

Threads: 2, Avg cycles: 9269000.480000

Threads: 4, Avg cycles: 9669956.320000

Threads: 8, Avg cycles: 13391926.080000

Threads: 16, Avg cycles: 14259841.920000

make run_small

make run_medium

make run_large

**TaskGraph**

make all will just compile all 3 files (main.c, main_v1.c, and main_v2.c) to .x files.
