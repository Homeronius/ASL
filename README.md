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

## TODOs until next meeting:

### Tobia
Optimize Prim with vector intrinsics. \
Small fixes in Python / shell scripts.

### Martin
Create table of optimizations implemented so far, including their approximate resulting speedup. \
Find cause of inconsistencies when using perf and Hotspot.

### Alex
Memory optimization in distance matrix computation (only compute upper diagonal).

### Tom
Take care of memory alignment (32 bytes) when reading in dataset. \
Fix small bug in cluster extraction.

## Setting this up

```bash
mkdir build && cd build
cmake -G Ninja ..
ninja        # building binaries
ninja check  # running tests
```

## CMake arguments
Note that all of these arguments are cached by CMake (i.e. need to be explicitly set to 0 again after having them used).
### General arguments
* `-DHDBSCAN_VERBOSE`: enable or disable (default) verbose mode by setting to 1 or 0; do not enable when benchmarking!
* `-DHDBSCAN_DATA_DIR`: absolute path to folder with generated input files. Defaults to <project-dir>/data
* `-DHDBSCAN_INSTRUMENT`: enable or disable (default) cost analysis instrumentation. Do not enable when benchmarking as it adds extra operations!
* `-DCMAKE_CXX_COMPILER`: Choose your compiler (i.e. `g++`, `clang++`). Defaults to `g++`
* `-DOPT_LEVEL`: Choose your compiler optimization level (i.e. `O0`, `O1`). Defaults to `O3`
* `-BENCHMARK_AMD`: enable when using an AMD system for benchmarking (not fully verified correctness yet). See the comments in `benchmark_util.h`

### Arguments to determine algorithm version
* `-DHDBSCAN_PRECOMPUTE_DIST`: enable or disable full computation of the pairwise distance matrix. If set to 0, only the `core_distances`
are precomputed and the prim algorithm implicitly assumes the mutual reachability graph.

### Print CMake arguments at runtime
Including the header file `config.h` (generated by CMake) grants access to CMake variables through preprocessor directives at runtime (see `main.cpp`).
You can add further CMake variables in `config.h.in`. Custom CMake variables have to be added with `#cmakedefine ...`.

## Benchmarking

You might need to install the `perf` tool to run the benchmarks.
Benchmarking is currently set up for the Intel Skylake architecture and AMD processor family 17h.
Please see and modify the event and unmask values in `benchmark_util.h` if needed.

To run the benchmarks: `ninja benchmark`

Note! If all flop counts stay at zero, it might be that your user does not have the permission to see kernel PMU events. To fix this, try
```
sudo sysctl -w kernel.perf_event_paranoid=-1
```
This gives the user access to (almost) all events.
Reference [here](https://www.kernel.org/doc/Documentation/sysctl/kernel.txt).
## References

[R. J. G. B. Campello, D. Moulavi, A. Zimek, and J. Sander, “Hierarchical density estimates for data clustering, visualization, and outlier detection”](https://dl.acm.org/doi/pdf/10.1145/2733381)

[L. McInnes and J. Healy, “Accelerated hierarchical density based clustering”](https://arxiv.org/pdf/1705.07321.pdf)

[https://github.com/ojmakhura/hdbscan](https://github.com/ojmakhura/hdbscan)

[https://github.com/rohanmohapatra/hdbscan-cpp](https://github.com/rohanmohapatra/hdbscan-cpp)


# Team 35
© Tobia Claglüna, Martin Erhart, Alexander Hägele, Tom Lausberg
