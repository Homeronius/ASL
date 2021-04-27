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
