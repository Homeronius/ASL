# HDBSCAN (Hierarchical density based clustering)

![main algo](doc/main-algo.png "Main algo outline")

### Meeting debrief 28.05.20
#### What has to be analyzed

* Reason why were able to get much better speedup for the basic variant
* Compare Perf. and Cycles plots next to each other (taking into account). Reason about common features
* Some comments about our findings explaining the discrepancy of using different bit shifting / extraction intrinsics on AMD vs. Intel
* State assumptions on why with large 'd' performance increases significantly (memory bottlenecks?)



#### What not to forget to mention in the report

* How the data was generated (not on-the-fly during the benchmark but beforehand)
* Specifications of AMD and Intel processor used as well as mentioning the different clock-rates they have
* Comment on that data variance is to be expected (due to data and quickselect)
* State all assumptions we made over the course of the project

## TODOs until next meeting:

* Roofline plot of containing the OI's of all possible optimizations considered
* Explore impact of altering structure of the datasets on performance measurements
* Prepare reference implementation and compute performance thereof
* Setting up preliminary structure of presentation slides
* Do cache miss-rate analysis of the optimizations (using cachegrind). LL cache might not be enough to reason about the miss-rate. Check out lower level caches as well. Check out how fast each processor can load / store into each cache (what might be the bottleneck)
* Change plot title for cycles plots
* Make plots more readable (e.g. put legend outside of plot might be a good idea)
* Create one plot of the best approaches comparing GNU gcc and clang

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
* `-DHDBSCAN_PRECOMPUTE_DIST_TRIANG`: enable or disable upper triangular version of precomputation of the pairwise distance matrix. If set to 0, the `hdbscan_basic` uses the full distance matrix of size `n*n`. Set to 1, a special triangular matrix laid out linear in memory with size `n*(n+1)/2` is used.

### Print CMake arguments at runtime
Including the header file `config.h` (generated by CMake) grants access to CMake variables through preprocessor directives at runtime (see `main.cpp`).
You can add further CMake variables in `config.h.in`. Custom CMake variables have to be added with `#cmakedefine ...`.

### Ninja targets

* `all`: builds `hdbscan` and `hdbscan_vec` (default)
* `benchmark`: build and run non-vectorized benchmark
* `benchmark_vec`: build and run vectorized benchmark
* `build_bench`: build only non-vectorized benchmark but do not run it
* `build_bench_vec`: build only vectorized benchmark but do not run it
* `check`: build and run unit tests

## Benchmarking

You might need to install the `perf` tool to run the benchmarks.
Benchmarking is currently set up for the Intel Skylake architecture and AMD processor family 17h.
Please see and modify the event and unmask values in `benchmark_util.h` if needed. To run the benchmark with the current build: `ninja benchmark`

Note! If all flop counts stay at zero, it might be that your user does not have the permission to see kernel PMU events. To fix this, try
```
sudo sysctl -w kernel.perf_event_paranoid=-1
```
This gives the user access to (almost) all events.
Reference [here](https://www.kernel.org/doc/Documentation/sysctl/kernel.txt).

### Scripts
We have prepared the following scripts to run multiple benchmarks and plot the performance measurements:
  * `benchmarks.sh` is the entrypoint to a specific comparison as well as **all** experiments. It should generate data, build the project with different flags, execute benchmarks and save plotted results (i.e. calls all scripts outlined below).
    * Usage: `./benchmarks.sh [intel|amd] [baseline_flags|...|all]` (add new comparisons here).
  * ```run_perf_measurements.sh``` is the script that is called by the above `benchmarks.sh`. It executes a specific binary with different csv inputs.
    * Usage: `run_perf_measurements.sh <benchm_name> <binary_name> <filebase> <n>`. 
  * `helper_scripts/generate_clusters.py` generates and stores data as `data/perf_data_d<x>_<n>.csv`, for example `perf_data_d2_7.csv` where `d` is number of feature dimensions. The sizes of inputs are roughly evenly spaced from 32 to 14436.
    * Usage: `generate_clusters.py <relative_data_folder_path> <n_clusters> <d_dimensions>`
  * `helper_scripts/plot_performance_alt.py` creates a nice performance plot close to what was given in the lecture guidelines on benchmarking.
    * Arguments: 
      * `--system [intel|amd]`
      * `--data-path <data/timings/>`
      * `--files <x1.csv> ... <xn.csv>`
      * `--save-path <file>`
      * `--metric ['fp/c', 'cycles', 'time']`
      * `--x-scale ['log', 'linear']`
  * Other useful scripts, e.g. plotting clusters: See [`helper_scripts/`](helper_scripts).

Please see the outline and TODOs in `benchmarks.sh`.
## References

[R. J. G. B. Campello, D. Moulavi, A. Zimek, and J. Sander, “Hierarchical density estimates for data clustering, visualization, and outlier detection”](https://dl.acm.org/doi/pdf/10.1145/2733381)

[L. McInnes and J. Healy, “Accelerated hierarchical density based clustering”](https://arxiv.org/pdf/1705.07321.pdf)

[https://github.com/ojmakhura/hdbscan](https://github.com/ojmakhura/hdbscan)

[https://github.com/rohanmohapatra/hdbscan-cpp](https://github.com/rohanmohapatra/hdbscan-cpp)


# Team 35
© Tobia Claglüna, Martin Erhart, Alexander Hägele, Tom Lausberg
