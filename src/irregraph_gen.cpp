#include "../include/irregraph_gen.h"
#include <fstream>
#include <chrono>

IrreGraphGen::IrreGraphGen(int num_vertices, double edge_probability)
    : num_vertices_(num_vertices)
    , edge_probability_(edge_probability)
    , rng_(std::chrono::steady_clock::now().time_since_epoch().count()) {
}

void IrreGraphGen::generateGraph() {
    edges_.clear();
    
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (int i = 0; i < num_vertices_; ++i) {
        for (int j = i + 1; j < num_vertices_; ++j) {
            if (dist(rng_) < edge_probability_) {
                edges_.emplace_back(i, j);
            }
        }
    }
}

bool IrreGraphGen::saveToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        return false;
    }
    
    outFile << num_vertices_ << " " << edges_.size() << "\n";

    for (const auto& edge : edges_) {
        outFile << edge.first << " " << edge.second << "\n";
    }
    
    outFile.close();
    return true;
}

const std::vector<std::pair<int, int>>& IrreGraphGen::getEdges() const {
    return edges_;
} 