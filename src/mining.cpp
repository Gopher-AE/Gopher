#include "../include/mining.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <chrono>


std::unordered_set<int> Mining::neighborhood(int vertex) const {
    if (graph.find(vertex) != graph.end()) {
        return graph.at(vertex);
    }
    return std::unordered_set<int>();
}


void Mining::process(const std::vector<int>& embeddings) {

    ++pattern_count;
}


void Mining::add_node(int node) {
    if (!has_node(node)) {
        graph[node] = std::unordered_set<int>();
    }
}


void Mining::add_edge(int u, int v) {
    add_node(u);
    add_node(v);
    graph[u].insert(v);
    graph[v].insert(u);
}


bool Mining::has_node(int node) const {
    return graph.find(node) != graph.end();
}


bool Mining::has_edge(int u, int v) const {
    if (!has_node(u) || !has_node(v)) return false;
    return graph.at(u).find(v) != graph.at(u).end();
}


size_t Mining::edge_count() const {
    size_t count = 0;
    for (const auto& pair : graph) {
        count += pair.second.size();
    }
    return count / 2;
}


void Mining::mining(const std::pair<int, int>& edge, bool add_to_graph) {
    if (add_to_graph) {
        add_edge(edge.first, edge.second);
    }
    mine_patterns(edge);
}


bool Mining::initialize() {
    if (graph_file_path.empty()) {
        std::cerr << "Graph file path not set!" << std::endl;
        return false;
    }

    clear();

    std::ifstream graph_file(graph_file_path);
    if (!graph_file.is_open()) {
        std::cerr << "Failed to open graph file: " << graph_file_path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(graph_file, line)) {
        std::istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            add_edge(u, v);
        }
    }
    graph_file.close();

    std::cout << "Loaded graph with " << node_count() << " nodes and " 
              << edge_count() << " edges" << std::endl;
    return true;
}

void Mining::mine_patterns(const std::pair<int, int>& edge) {
    auto Nv0 = neighborhood(edge.first);
    auto Nv1 = neighborhood(edge.second);
    

    Nv0.erase(edge.second);
    Nv1.erase(edge.first);

    std::unordered_set<int> v2;
    for (int v : Nv0) {
        if (Nv1.find(v) != Nv1.end()) {
            v2.insert(v);
        }
    }

    for (int node : v2) {
        for (int i : Nv1) {
            if (i != node) {
                auto Nv3 = neighborhood(i);
                Nv3.erase(edge.second);

                std::unordered_set<int> Cv4;
                for (int v : Nv0) {
                    if (v != node && Nv3.find(v) != Nv3.end()) {
                        Cv4.insert(v);
                    }
                }
                
                for (int s : Cv4) {
                    process({edge.first, edge.second, node, i, s});
                }
            }
        }
    }

    for (int node : v2) {
        auto Nv2 = neighborhood(node);
        Nv2.erase(edge.first);
        Nv2.erase(edge.second);
        
        for (int i : Nv0) {
            if (i != node) {
                auto Nv3 = neighborhood(i);
                
                std::unordered_set<int> Cv4;
                for (int v : Nv2) {
                    if (Nv3.find(v) != Nv3.end()) {
                        Cv4.insert(v);
                    }
                }
                
                for (int s : Cv4) {
                    process({node, edge.first, edge.second, i, s});
                }
            }
        }
    }

    for (int node : Nv0) {
        auto Nv3 = neighborhood(node);
        Nv3.erase(edge.first);
        Nv3.erase(edge.second);
        
        std::unordered_set<int> Cv3;
        for (int v : Nv1) {
            if (Nv3.find(v) != Nv3.end()) {
                Cv3.insert(v);
            }
        }
        
        std::unordered_set<int> Cv4;
        for (int v : Nv0) {
            if (Nv3.find(v) != Nv3.end()) {
                Cv4.insert(v);
            }
        }
        
        for (int i : Cv3) {
            for (int s : Cv4) {
                if (i != s) {
                    process({edge.first, edge.second, node, i, s});
                }
            }
        }
    }

    for (int node : Nv0) {
        auto Nv2 = neighborhood(node);
        Nv2.erase(edge.second);
        
        std::unordered_set<int> Cv3;
        for (int v : Nv1) {
            if (Nv2.find(v) != Nv2.end()) {
                Cv3.insert(v);
            }
        }
        
        for (int i : Cv3) {
            auto Nv3 = neighborhood(i);
            
            std::unordered_set<int> Cv4;
            for (int v : Nv2) {
                if (Nv3.find(v) != Nv3.end()) {
                    Cv4.insert(v);
                }
            }
            
            for (int s : Cv4) {
                process({edge.first, node, s, i, edge.second});
            }
        }
    }
}

void Mining::run() {
    if (update_file_path.empty()) {
        std::cerr << "Update file path not set!" << std::endl;
        return;
    }

    std::ifstream update_file(update_file_path);
    if (!update_file.is_open()) {
        std::cerr << "Failed to open update file: " << update_file_path << std::endl;
        return;
    }

    std::vector<std::pair<int, int>> updates;
    std::string line;
    while (std::getline(update_file, line)) {
        std::istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            updates.emplace_back(u, v);
        }
    }
    update_file.close();

    std::cout << "Processing " << updates.size() << " updates..." << std::endl;

    pattern_count = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < updates.size(); ++i) {
        if (i < 1000 && i % 100 == 0 || i % 1000 == 0) {
            std::cout << "Processed updates: " << i << " / " << updates.size() << std::endl;
        }
        mining(updates[i], true);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Mining completed in " << duration.count() << " microseconds" << std::endl;

    std::cout << "\nMining Results:" << std::endl;
    std::cout << "Total matches found: " << pattern_count << std::endl;
}

