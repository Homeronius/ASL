# Optimizations for different parts of the algorithm

# Optimization Overview



### Distance Calculation:

---

##### Distance Matrix:

- Default implementation

##### Triangular Distance Matrix:

- Only computes/ saves pairwise distance once

##### Blocked Distance Matrix:

- Cache optimized distance computation

##### Inplace Distances:

- Doesn't store computed distances.
- More operations, lower memory requirement

---

##### Vectorized Distance Matrix:

- Default implementation with intrinsics

---

##### Standard Euclidian Distance:

- Default implmentation

##### Manhattan Distance:

- No sqrt


--- 

##### Vecorized Euclidian Distance:

- Intrinsics

##### 2d Optimized Euclidian Distance:

- Intrinsics, for 2d

##### 4d Optimized Euclidian Distance:

- Intrinsics, for 4d


##### 2d Optimized Manhattan Distance:

- No sqrt, for 2d

##### 4d Optimized Manhattan Distance:

- No sqrt, for 2d




---

##### Quickselect

##### Iterative Quickselect

- Quickselect without recursion

---

##### Vectorized Partition Function

---


### Prim

##### Prim Naive

- Uses n x n distance matrix



##### Prim Upper Triangular

- Uses upper triangular distance matrix

##### Prim Advanced


----

##### Prim Naive Vectorized

##### Prim Advanced Vectorized



# Additional analyses
 * Different dimensions $$d$$
 * Compare different compiler flags and options
 * AMD vs. Intel?
 * Hand vectorized vs. compiler
 

 ## Notes
  * Memory alignment: perf events?
  * 
