# Gopher: Efficient Dynamic Graph Pattern Mining via DAG-Driven Execution
## Project Overview

Gopher is an efficient dynamic graph pattern mining system that uses DAG-driven execution to find matches in dynamic graphs.



## Key Components

#### 1. Pattern Management (`pattern.h`, `pattern.cpp`)

- Core class: `Pattern`

- Purpose: Represents and manages graph patterns to be matched

- Key functions:

  - `Constructor`: Takes pattern size and adjacency matrix as input

  - `get_adj_mat_ptr()`: Returns the adjacency matrix pointer

  - `get_size()`: Returns pattern size

#### 2. Mappings (`mappings.h`, `mappings.cpp`)

- Core class: `Mappings`

- Purpose: Generates mapping instances

#### 3. Schedule Management (`schedule.h`, `schedule.cpp`)

- Core class: `Schedule`

- Purpose: Generates schedules for mapping instances

- Key functions:
  - `generate_schedules()`: Generates valid schedules

#### 4. DAG Processing (`dag.h`, `dag.cpp`)

- Core class: `DAG`

- Purpose: Handles Directed Acyclic Graph for pattern matching

- Key functions:

  - `DAG_combination()`: Combines multiple DAGs

  - Graph traversal and processing functions

#### 5. Mining Engine (`mining.h`, `mining.cpp`)

- Core class: `Mining`

- Purpose: Main engine for pattern matching

- Key functions:

  - `initialize()`: Sets up mining environment

  - `run()`: Executes the mining process

    

## How to Use

**1. Building the Project**

```bash
cd Gopher
g++ src/baseline_test.cpp src/pattern.cpp src/mappings.cpp src/schedule.cpp src/dag.cpp src/mining.cpp -o baseline_test
```

**2. Running Pattern Matching**

Basic command format:

```bash
./baseline_test <graph_file> <updates_file> <pattern_size> <pattern_adjacency_matrix>
```

Example:

```bash
./baseline_test dataset/example.txt dataset/updates.txt 5 0111010011100011100001100
```

**3. Input File Formats**

**Graph File Format** (`example.txt`):

```
# Each line represents an edge: <source_vertex> <target_vertex>
...
14285 510
14286 510
2961 510
4276 510
1453 510
1898 510
2773 510
4337 510
32 510
1646 510
14287 510
3257 510
2396 510
111 3060
335 3060
3061 3060
2702 3060
111 14288
111 14289
111 14290
111 14291
...
```

**Updates File Format** (`updates.txt`):

```
# Each line represents a new edge to be added
...
3062 244563
5567 244564
5567 244565
1118 244565
1034 244565
4581 9079
3962 9079
175825 9079
943 9079
27557 9079
244566 9079
11249 9079
2859 9079
1020 9079
52216 9079
2969 9079
1974 244567
1467 244568
6915 244568
1467 244569
6915 244569
1467 244570
...
```

**4. Pattern Specification**

- Pattern size: Number of vertices in the pattern

- Pattern adjacency matrix: A string of 0s and 1s representing the pattern's adjacency matrix

  - Length should be size² (e.g., for size 5, length should be 25)

  - Read row by row

  - 0: No edge

  - 1: Edge exists



## Key Algorithms

**1. Schedule Generation**

- Implemented in `generate_permutations()`

- Follows specific rules for valid mining schedules

- Handles vertex ordering and edge symmetry

**2. Pattern Matching**

- Uses DAG-driven execution

- Processes graph updates incrementally

- Finds new embeddings efficiently
