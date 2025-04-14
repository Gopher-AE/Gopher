#include "../include/five_clique.h"
#include <chrono>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc,char *argv[]) {
    if(argc < 3) {
        printf("Usage: %s dataset_name graph_file pattern_size pattern_adjacency_matrix\n", argv[0]);
        printf("Example(Triangle counting on dataset WikiVote) : \n");
        printf("%s dataset/example.txt dataset/updates.txt\n", argv[0]);
        return 0;
    }

    auto start1 = std::chrono::high_resolution_clock::now();
    
    FiveClique G;
    
    std::string graph_file_path = argv[1];
    G.readGraphFromFile(graph_file_path);
    
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff1 = end1 - start1;
    
    std::cout << "Read/generated graph in " << diff1.count() << " seconds" << std::endl;
    std::cout << "Graph has " << G.getNodeCount() << " vertices and " 
              << G.getEdgeCount() << " edges" << std::endl;
    
    std::string update_file_path = argv[2];
    std::vector<std::vector<std::string>> updates = G.readUpdatesFromFile(update_file_path);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < updates.size(); ++i) {
        if (i < 1000 && i % 100 == 0 || i % 1000 == 0) {
            std::cout << "Processed updates: " << i << " / " << updates.size() << std::endl;
        }
        G.mining(updates[i], true);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    
    std::cout << "Execution Time: " << diff.count() << " seconds" << std::endl;
    std::cout << "Total matches found: " << G.getMatchesNum() << std::endl;
    
    return 0;
} 