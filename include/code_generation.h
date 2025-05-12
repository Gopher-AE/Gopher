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

// 定义集合操作类型
enum class SetOperation {
    INTERSECTION,
    UNION,
    DIFFERENCE,
    SYMMETRIC_DIFFERENCE
};

// 定义代码块类型
enum class CodeBlockType {
    SEQUENTIAL,
    PARALLEL,
    CONDITIONAL,
    LOOP
};

// 代码块结构
struct CodeBlock {
    CodeBlockType type;
    std::vector<std::string> code;
    std::vector<std::shared_ptr<CodeBlock>> children;
    std::string condition;  // 用于条件块
    int loop_count;        // 用于循环块
    int priority;          // 执行优先级
};

class CodeGeneration {
public:
    CodeGeneration();
    ~CodeGeneration() = default;

    // 配置选项
    struct Config {
        bool enable_optimization;     // 是否启用优化
        bool enable_parallel;         // 是否启用并行执行
        bool enable_caching;         // 是否启用结果缓存
        int max_parallel_blocks;     // 最大并行块数
        int optimization_level;      // 优化级别
    };

    // 设置配置
    void setConfig(const Config& config) { this->config = config; }
    
    // 主要的代码生成函数
    std::vector<std::string> generateCode(const DAG& dag);

    // 生成优化后的代码
    std::vector<std::string> generateOptimizedCode(const DAG& dag);

private:
    Config config;
    std::map<std::pair<int, int>, std::set<int>> cache;  // 缓存计算结果
    std::priority_queue<std::shared_ptr<CodeBlock>, 
                       std::vector<std::shared_ptr<CodeBlock>>,
                       std::function<bool(const std::shared_ptr<CodeBlock>&, 
                                        const std::shared_ptr<CodeBlock>&)>> priority_queue;

    // 基本操作函数
    std::set<int> getNeighbors(const DAG& dag, int vertex);
    std::set<int> getIntersection(const std::set<int>& set1, const std::set<int>& set2);
    std::set<int> getUnion(const std::set<int>& set1, const std::set<int>& set2);
    std::set<int> getDifference(const std::set<int>& set1, const std::set<int>& set2);
    std::set<int> getSymmetricDifference(const std::set<int>& set1, const std::set<int>& set2);

    // 代码生成辅助函数
    std::string generateVertexCode(const DAG& dag, int vertex, const std::set<int>& neighbors);
    std::string generateSetOperationCode(SetOperation op, const std::set<int>& set1, 
                                       const std::set<int>& set2, const std::string& result_name);
    std::string generateParallelBlock(const std::vector<std::string>& code_blocks);
    std::string generateConditionalBlock(const std::string& condition, 
                                       const std::vector<std::string>& true_block,
                                       const std::vector<std::string>& false_block);
    std::string generateLoopBlock(const std::string& iterator, int count,
                                const std::vector<std::string>& loop_body);

    // 优化相关函数
    void optimizeCodeBlocks(std::vector<std::shared_ptr<CodeBlock>>& blocks);
    void mergeSimilarOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks);
    void eliminateRedundantOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks);
    void reorderOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks);

    // 缓存相关函数
    bool tryGetFromCache(int vertex1, int vertex2, std::set<int>& result);
    void addToCache(int vertex1, int vertex2, const std::set<int>& result);
    void clearCache();

    // 并行化相关函数
    std::vector<std::vector<std::shared_ptr<CodeBlock>>> partitionForParallel(
        const std::vector<std::shared_ptr<CodeBlock>>& blocks);
    bool canExecuteInParallel(const std::shared_ptr<CodeBlock>& block1,
                            const std::shared_ptr<CodeBlock>& block2);
};

#endif // CODE_GENERATION_H 