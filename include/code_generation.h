#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include "dag.h"
#include <vector>
#include <string>
#include <set>
#include <map>
#include <memory>
#include <queue>
#include <functional>


enum class SetOperation {
    INTERSECTION,
    DIFFERENCE,
    SYMMETRIC_DIFFERENCE
};


enum class CodeBlockType {
    SEQUENTIAL,
    PARALLEL,
    CONDITIONAL,
    LOOP
};


struct CodeBlock {
    CodeBlockType type;
    std::vector<std::string> code;
    std::vector<std::shared_ptr<CodeBlock>> children;
    std::string condition;  
    int loop_count;        
    int priority;          
};

class CodeGeneration {
public:
    CodeGeneration();
    ~CodeGeneration() = default;

    struct Config {
        bool enable_optimization;   
        bool enable_parallel;        
        bool enable_caching;        
        int max_parallel_blocks;    
        int optimization_level;    
    };


    void setConfig(const Config& config) { this->config = config; }
    
    std::vector<std::string> generateCode(const DAG& dag);

    std::vector<std::string> generateOptimizedCode(const DAG& dag);

private:
    Config config;
    std::map<std::pair<int, int>, std::set<int>> cache;  
    std::priority_queue<std::shared_ptr<CodeBlock>, 
                       std::vector<std::shared_ptr<CodeBlock>>,
                       std::function<bool(const std::shared_ptr<CodeBlock>&, 
                                        const std::shared_ptr<CodeBlock>&)>> priority_queue;


    std::set<int> getNeighbors(const DAG& dag, int vertex);
    std::set<int> getIntersection(const std::set<int>& set1, const std::set<int>& set2);
    std::set<int> getDifference(const std::set<int>& set1, const std::set<int>& set2);
    std::set<int> getSymmetricDifference(const std::set<int>& set1, const std::set<int>& set2);


    std::string generateVertexCode(const DAG& dag, int vertex, const std::set<int>& neighbors);
    std::string generateSetOperationCode(SetOperation op, const std::set<int>& set1, 
                                       const std::set<int>& set2, const std::string& result_name);
    std::string generateParallelBlock(const std::vector<std::string>& code_blocks);
    std::string generateConditionalBlock(const std::string& condition, 
                                       const std::vector<std::string>& true_block,
                                       const std::vector<std::string>& false_block);
    std::string generateLoopBlock(const std::string& iterator, int count,
                                const std::vector<std::string>& loop_body);


    void optimizeCodeBlocks(std::vector<std::shared_ptr<CodeBlock>>& blocks);
    void mergeSimilarOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks);
    void eliminateRedundantOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks);
    void reorderOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks);


    bool tryGetFromCache(int vertex1, int vertex2, std::set<int>& result);
    void addToCache(int vertex1, int vertex2, const std::set<int>& result);
    void clearCache();


    std::vector<std::vector<std::shared_ptr<CodeBlock>>> partitionForParallel(
        const std::vector<std::shared_ptr<CodeBlock>>& blocks);
    bool canExecuteInParallel(const std::shared_ptr<CodeBlock>& block1,
                            const std::shared_ptr<CodeBlock>& block2);
};

#endif // CODE_GENERATION_H 