# Cache Simulation

The goal of this assignment was to develop a complete memory hierarchy. To achieve this, we were tasked with completing the base code provided by the faculty (found in the folder `Base_Code`) by implementing the L1 cache and L2 cache from scratch, and then integrating these components to form a complete memory hierarchy.

### Directly-Mapped L1 Cache

The files `L1Cache.h` and `L1Cache.c` implement a memory hierarchy with an L1 direct mapped cache with multiple lines with the parameters provided in the constant file.

### Directly-Mapped L2 Cache

The files `L2Cache1W.h` and `L2Cache1W.c` implement a direct mapped L2 cache with the parameters specified in the constant file. It uses the Directly-Mapped L1 Cache developed in the previous task in the memory hierarchy.

### 2-Way L2 Cache

The files `L2Cache2W.h` and `L2Cache2W.c` implement a change to the L2 cache developed in the previous task, modifying it into a two way set-associate cache. Note that the other parameters remain the same, in particular the _L2Size_ value. It uses the Directly-Mapped L1 Cache developed previously in the memory hierarchy.

## Report

A two page report explaining our implementation can be found in `G031_report.pdf`.

## Running Instructions and Testing

To test each cache, simply run the `make` command followed by which cache you wish to test (`L1`, `L2-1` or `L2-2`), and then do `./SimpleCache`. The folder `tests_alunos` was provided by the faculty for testing, and it contains the desired results for each cache implementation.
