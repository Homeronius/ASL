# HDBSCAN (Hierarchical density based clustering)

![main algo](doc/main-algo.png "Main algo outline")

## One-to-one meeting 07.05.20

### Presentation of current progress

- Algorithm and its constituents (Tom)
- Codebase / organisation (Alex)
- Performance benchmark results -> visualization with Hotspot (Martin)
- Possible optimizations (Tobia):
  - Intrinsics
  - Memory alignment
  - Ball-trees for distance calculations
  - Dual-tree Boruvka

### Open questions

- Should we only focus on the main bottleneck (i.e.  MST)? Or should we also consider cluster extraction (~4% of runtime)
- Should algorithmic optimizations (e.g. memory complexity) be taken into consideration, or is the main focus on applying / testing optimizations shown in the lecture (e.g. vector intrinsics, loop unrolling, blocking, ...)?

## Interfaces:

### Input (Tobia)
N datapoints in a d-dimensional euclidean space. \
`n x d` dimensional Matrix. Stored as c-array.

### Distances (Martin)
Mutual reachability distance matrix: \
`n x n` dimensional matrix. Stored as c-array.

### MST Creation (Alex)
Minimum Spanning Tree: \
`n x n` dimensional adjacency matrix. Stored as c-array.

### Cluster Selection (Tom)
Cluster allocation: \
`n` dimensional array containing the cluster id of each point. 0 = no cluster / noise. Stored as c-array.

## Setting this up

```bash
mkdir build && cd build
cmake -G Ninja ..
ninja        # building binaries
ninja check  # running tests
```

## CMake arguments
Note that all of these arguments are cached by CMake (i.e. need to be explicitly set to 0 again after having them used).
* `-DHDBSCAN_VERBOSE`: enable or disable (default) verbose mode by setting to 1 or 0; do not enable when benchmarking!
* `-DHDBSCAN_DATA_DIR`: absolute path to folder with generated input files. Defaults to <project-dir>/data
* `-DHDBSCAN_INSTRUMENT`: enable or disable (default) cost analysis instrumentation. Do not enable when benchmarking as it adds extra operations!
* `-BENCHMARK_AMD`: enable when using an AMD system for benchmarking (not fully verified correctness yet). See the comments in `benchmark_util.h`

## Benchmarking

You might need to install the `perf` tool to run the benchmarks.
Benchmarking is currently set up for the Intel Skylake architecture and AMD processor family 17h.
Please see and modify the event and unmask values in `benchmark_util.h` if needed.

To run the benchmarks: `ninja benchmark`

## References

[R. J. G. B. Campello, D. Moulavi, A. Zimek, and J. Sander, “Hierarchical density estimates for data clustering, visualization, and outlier detection”](https://dl.acm.org/doi/pdf/10.1145/2733381)

[L. McInnes and J. Healy, “Accelerated hierarchical density based clustering”](https://arxiv.org/pdf/1705.07321.pdf)

[https://github.com/ojmakhura/hdbscan](https://github.com/ojmakhura/hdbscan)

[https://github.com/rohanmohapatra/hdbscan-cpp](https://github.com/rohanmohapatra/hdbscan-cpp)


# Team 35
© Tobia Claglüna, Martin Erhart, Alexander Hägele, Tom Lausberg
