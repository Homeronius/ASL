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
