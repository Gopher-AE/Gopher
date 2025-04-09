# Gopher
### Gopher Implementation

How to run on a single machine

```
$ cd Gopher
$ g++ src/baseline_test.cpp src/pattern.cpp src/mappings.cpp src/schedule.cpp src/dag.cpp src/mining.cpp -o baseline_test  
$ ./baseline_test dataset/example.txt dataset/updates.txt 5 0111010011100011100001100
```

In `dataset/example.txt`, we provide an initial input graph with 7 vertices and 9 edges.

```
# example.txt
1 3
1 5
2 3
2 4
3 4
4 5
5 6
5 7
6 7
```

In `dataset/updates.txt`, there are 3 inserted edges.

```
# updates.txt
1 2
1 4
4 6
```

After applying 3 updates to the initial input graph, 12 new embeddings will be found for matching the House Pattern.

