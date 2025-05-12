#include "../include/code_generation.h"
#include <sstream>
#include <algorithm>

std::vector<std::string> CodeGeneration::generateCode(const DAG& dag) {
    std::vector<std::string> code;
    int size = dag.get_size();
    const int* adj_matrix = dag.get_adj_matrix();

    // 遍历每个顶点
    for (int i = 0; i < size; ++i) {
        // 获取当前顶点的邻居
        std::set<int> neighbors = getNeighbors(dag, i);
        
        // 生成当前顶点的代码
        std::string vertex_code = generateVertexCode(dag, i, neighbors);
        code.push_back(vertex_code);

        // 如果有多个邻居，生成交集操作
        if (neighbors.size() > 1) {
            std::set<int> intersection;
            bool first = true;
            
            for (int neighbor : neighbors) {
                std::set<int> neighbor_set = getNeighbors(dag, neighbor);
                if (first) {
                    intersection = neighbor_set;
                    first = false;
                } else {
                    intersection = getIntersection(intersection, neighbor_set);
                }
            }
            
            if (!intersection.empty()) {
                std::string intersection_code = generateSetOperationCode("intersection", intersection);
                code.push_back(intersection_code);
            }
        }
    }

    return code;
}

std::set<int> CodeGeneration::getNeighbors(const DAG& dag, int vertex) {
    std::set<int> neighbors;
    int size = dag.get_size();
    const int* adj_matrix = dag.get_adj_matrix();

    for (int i = 0; i < size; ++i) {
        if (adj_matrix[vertex * size + i] > 0) {
            neighbors.insert(i);
        }
    }

    return neighbors;
}

std::set<int> CodeGeneration::getIntersection(const std::set<int>& set1, const std::set<int>& set2) {
    std::set<int> result;
    std::set_intersection(set1.begin(), set1.end(),
                         set2.begin(), set2.end(),
                         std::inserter(result, result.begin()));
    return result;
}

std::string CodeGeneration::generateVertexCode(const DAG& dag, int vertex, const std::set<int>& neighbors) {
    std::stringstream ss;
    ss << "// Process vertex " << vertex << "\n";
    ss << "std::set<int> neighbors_" << vertex << " = {";
    
    bool first = true;
    for (int neighbor : neighbors) {
        if (!first) {
            ss << ", ";
        }
        ss << neighbor;
        first = false;
    }
    ss << "};\n";
    
    return ss.str();
}

std::string CodeGeneration::generateSetOperationCode(const std::string& operation, const std::set<int>& set) {
    std::stringstream ss;
    ss << "// " << operation << " operation\n";
    ss << "std::set<int> " << operation << "_result = {";
    
    bool first = true;
    for (int value : set) {
        if (!first) {
            ss << ", ";
        }
        ss << value;
        first = false;
    }
    ss << "};\n";
    
    return ss.str();
} 