#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include "dag.h"
#include <vector>
#include <string>
#include <set>
#include <map>

class CodeGeneration {
public:
    CodeGeneration() = default;
    ~CodeGeneration() = default;

    std::vector<std::string> generateCode(const DAG& dag);

private:
    std::set<int> getNeighbors(const DAG& dag, int vertex);
    
    std::set<int> getIntersection(const std::set<int>& set1, const std::set<int>& set2);
    
    std::string generateVertexCode(const DAG& dag, int vertex, const std::set<int>& neighbors);
    
    std::string generateSetOperationCode(const std::string& operation, const std::set<int>& set);
};

#endif // CODE_GENERATION_H 