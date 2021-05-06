# HDBSCAN (Hierarchical density based clustering)

![main algo](doc/main-algo.png "Main algo outline")

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

* `-DHDBSCAN_VERBOSE`: enable or disable (default) verbose mode by setting to 1 or 0; do not enable when benchmarking!
* `-DHDBSCAN_DATA_DIR`: absolute path to folder with generated input files. Defaults to <project-dir>/data

## Benchmarking

You might need to install the `perf` tool to run the benchmarks.
If you have a processor that is not based on the Intel Skylake architecture, please modify the event and unmask values in `benchmark_util.h`.

To run the benchmarks: `ninja benchmark`

## References

[R. J. G. B. Campello, D. Moulavi, A. Zimek, and J. Sander, “Hierarchical density estimates for data clustering, visualization, and outlier detection”](https://dl.acm.org/doi/pdf/10.1145/2733381)

[L. McInnes and J. Healy, “Accelerated hierarchical density based clustering”](https://arxiv.org/pdf/1705.07321.pdf)

[https://github.com/ojmakhura/hdbscan](https://github.com/ojmakhura/hdbscan)

[https://github.com/rohanmohapatra/hdbscan-cpp](https://github.com/rohanmohapatra/hdbscan-cpp)


# Team 35
© Tobia Claglüna, Martin Erhart, Alexander Hägele, Tom Lausberg