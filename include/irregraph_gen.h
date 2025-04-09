#ifndef IRREGRAPH_GEN_H
#define IRREGRAPH_GEN_H

#include <vector>
#include <string>
#include <random>
#include <utility>

class IrreGraphGen {
public:
    IrreGraphGen(int num_vertices = 5000, double edge_probability = 0.2);
    void generateGraph();
    bool saveToFile(const std::string& filename);
    const std::vector<std::pair<int, int>>& getEdges() const;

private:
    int num_vertices_;                    
    double edge_probability_;             
    std::vector<std::pair<int, int>> edges_;  
    std::mt19937 rng_;                 
};

#endif // IRREGRAPH_GEN_H 