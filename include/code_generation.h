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

    // 生成代码的主要函数
    std::vector<std::string> generateCode(const DAG& dag);

private:
    // 获取顶点的邻居节点
    std::set<int> getNeighbors(const DAG& dag, int vertex);
    
    // 获取两个顶点集合的交集
    std::set<int> getIntersection(const std::set<int>& set1, const std::set<int>& set2);
    
    // 生成单个顶点的操作代码
    std::string generateVertexCode(const DAG& dag, int vertex, const std::set<int>& neighbors);
    
    // 生成集合操作的代码
    std::string generateSetOperationCode(const std::string& operation, const std::set<int>& set);
};

#endif // CODE_GENERATION_H 