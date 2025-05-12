#include "../include/code_generation.h"
#include <sstream>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <thread>

CodeGeneration::CodeGeneration() {
    config.enable_optimization = true;
    config.enable_parallel = true;
    config.enable_caching = true;
    config.max_parallel_blocks = 4;
    config.optimization_level = 2;

    priority_queue = std::priority_queue<std::shared_ptr<CodeBlock>,
        std::vector<std::shared_ptr<CodeBlock>>,
        std::function<bool(const std::shared_ptr<CodeBlock>&,
                         const std::shared_ptr<CodeBlock>&)>>(
        [](const std::shared_ptr<CodeBlock>& a, const std::shared_ptr<CodeBlock>& b) {
            return a->priority < b->priority;
        });
}

std::vector<std::string> CodeGeneration::generateCode(const DAG& dag) {
    std::vector<std::string> code;
    std::vector<std::shared_ptr<CodeBlock>> blocks;
    int size = dag.get_size();

    for (int i = 0; i < size; ++i) {
        auto block = std::make_shared<CodeBlock>();
        block->type = CodeBlockType::SEQUENTIAL;
        block->priority = i;

        std::set<int> neighbors = getNeighbors(dag, i);
        block->code.push_back(generateVertexCode(dag, i, neighbors));

        if (neighbors.size() > 1) {
            processNeighbors(dag, i, neighbors, block);
        }

        blocks.push_back(block);
    }

    if (config.enable_optimization) {
        optimizeCodeBlocks(blocks);
    }

    if (config.enable_parallel) {
        auto parallel_partitions = partitionForParallel(blocks);
        for (const auto& partition : parallel_partitions) {
            if (partition.size() > 1) {
                std::vector<std::string> parallel_code;
                for (const auto& block : partition) {
                    parallel_code.insert(parallel_code.end(),
                                      block->code.begin(),
                                      block->code.end());
                }
                code.push_back(generateParallelBlock(parallel_code));
            } else if (!partition.empty()) {
                code.insert(code.end(),
                          partition[0]->code.begin(),
                          partition[0]->code.end());
            }
        }
    } else {
        for (const auto& block : blocks) {
            code.insert(code.end(), block->code.begin(), block->code.end());
        }
    }

    return code;
}

void CodeGeneration::processNeighbors(const DAG& dag, int vertex,
                                    const std::set<int>& neighbors,
                                    std::shared_ptr<CodeBlock> block) {
    std::set<int> intersection;
    bool first = true;

    if (config.enable_caching) {
        for (int neighbor : neighbors) {
            if (tryGetFromCache(vertex, neighbor, intersection)) {
                continue;
            }
            std::set<int> neighbor_set = getNeighbors(dag, neighbor);
            if (first) {
                intersection = neighbor_set;
                first = false;
            } else {
                intersection = getIntersection(intersection, neighbor_set);
            }
            addToCache(vertex, neighbor, intersection);
        }
    } else {
        for (int neighbor : neighbors) {
            std::set<int> neighbor_set = getNeighbors(dag, neighbor);
            if (first) {
                intersection = neighbor_set;
                first = false;
            } else {
                intersection = getIntersection(intersection, neighbor_set);
            }
        }
    }

    if (!intersection.empty()) {
        std::string result_name = "result_" + std::to_string(vertex);
        block->code.push_back(generateSetOperationCode(SetOperation::INTERSECTION,
                                                     neighbors,
                                                     intersection,
                                                     result_name));
    }
}


std::set<int> CodeGeneration::getDifference(const std::set<int>& set1, const std::set<int>& set2) {
    std::set<int> result;
    std::set_difference(set1.begin(), set1.end(),
                       set2.begin(), set2.end(),
                       std::inserter(result, result.begin()));
    return result;
}

std::set<int> CodeGeneration::getSymmetricDifference(const std::set<int>& set1,
                                                    const std::set<int>& set2) {
    std::set<int> result;
    std::set_symmetric_difference(set1.begin(), set1.end(),
                                set2.begin(), set2.end(),
                                std::inserter(result, result.begin()));
    return result;
}

std::string CodeGeneration::generateParallelBlock(const std::vector<std::string>& code_blocks) {
    std::stringstream ss;
    ss << "#pragma omp parallel sections\n{\n";
    for (const auto& block : code_blocks) {
        ss << "#pragma omp section\n{\n" << block << "}\n";
    }
    ss << "}\n";
    return ss.str();
}

std::string CodeGeneration::generateConditionalBlock(const std::string& condition,
                                                   const std::vector<std::string>& true_block,
                                                   const std::vector<std::string>& false_block) {
    std::stringstream ss;
    ss << "if (" << condition << ") {\n";
    for (const auto& line : true_block) {
        ss << "    " << line << "\n";
    }
    if (!false_block.empty()) {
        ss << "} else {\n";
        for (const auto& line : false_block) {
            ss << "    " << line << "\n";
        }
    }
    ss << "}\n";
    return ss.str();
}

std::string CodeGeneration::generateLoopBlock(const std::string& iterator,
                                            int count,
                                            const std::vector<std::string>& loop_body) {
    std::stringstream ss;
    ss << "for (int " << iterator << " = 0; "
       << iterator << " < " << count << "; ++"
       << iterator << ") {\n";
    for (const auto& line : loop_body) {
        ss << "    " << line << "\n";
    }
    ss << "}\n";
    return ss.str();
}

void CodeGeneration::optimizeCodeBlocks(std::vector<std::shared_ptr<CodeBlock>>& blocks) {
    if (config.optimization_level >= 1) {
        eliminateRedundantOperations(blocks);
    }
    if (config.optimization_level >= 2) {
        mergeSimilarOperations(blocks);
    }
    if (config.optimization_level >= 3) {
        reorderOperations(blocks);
    }
}

void CodeGeneration::mergeSimilarOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks) {
    for (size_t i = 0; i < blocks.size(); ++i) {
        for (size_t j = i + 1; j < blocks.size(); ++j) {
            if (blocks[i]->type == blocks[j]->type &&
                blocks[i]->type == CodeBlockType::SEQUENTIAL) {
                // 合并相似的操作
                if (blocks[i]->code.size() == blocks[j]->code.size()) {
                    bool similar = true;
                    for (size_t k = 0; k < blocks[i]->code.size(); ++k) {
                        if (blocks[i]->code[k].find("neighbors_") != std::string::npos &&
                            blocks[j]->code[k].find("neighbors_") != std::string::npos) {
                            continue;
                        }
                        if (blocks[i]->code[k] != blocks[j]->code[k]) {
                            similar = false;
                            break;
                        }
                    }
                    if (similar) {
                        auto loop_block = std::make_shared<CodeBlock>();
                        loop_block->type = CodeBlockType::LOOP;
                        loop_block->code = blocks[i]->code;
                        loop_block->loop_count = 2;
                        blocks[i] = loop_block;
                        blocks.erase(blocks.begin() + j);
                        --j;
                    }
                }
            }
        }
    }
}

void CodeGeneration::eliminateRedundantOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks) {
    std::unordered_set<std::string> seen_operations;
    for (auto it = blocks.begin(); it != blocks.end(); ) {
        bool redundant = false;
        for (const auto& code_line : (*it)->code) {
            if (seen_operations.find(code_line) != seen_operations.end()) {
                redundant = true;
                break;
            }
            seen_operations.insert(code_line);
        }
        if (redundant) {
            it = blocks.erase(it);
        } else {
            ++it;
        }
    }
}

void CodeGeneration::reorderOperations(std::vector<std::shared_ptr<CodeBlock>>& blocks) {
    std::sort(blocks.begin(), blocks.end(),
              [](const std::shared_ptr<CodeBlock>& a,
                 const std::shared_ptr<CodeBlock>& b) {
                  return a->priority > b->priority;
              });
}

std::vector<std::vector<std::shared_ptr<CodeBlock>>> CodeGeneration::partitionForParallel(
    const std::vector<std::shared_ptr<CodeBlock>>& blocks) {
    std::vector<std::vector<std::shared_ptr<CodeBlock>>> partitions;
    std::vector<std::shared_ptr<CodeBlock>> current_partition;

    for (const auto& block : blocks) {
        bool can_add_to_current = true;
        if (!current_partition.empty()) {
            for (const auto& existing_block : current_partition) {
                if (!canExecuteInParallel(existing_block, block)) {
                    can_add_to_current = false;
                    break;
                }
            }
        }

        if (can_add_to_current && 
            current_partition.size() < static_cast<size_t>(config.max_parallel_blocks)) {
            current_partition.push_back(block);
        } else {
            if (!current_partition.empty()) {
                partitions.push_back(current_partition);
                current_partition.clear();
            }
            current_partition.push_back(block);
        }
    }

    if (!current_partition.empty()) {
        partitions.push_back(current_partition);
    }

    return partitions;
}

bool CodeGeneration::canExecuteInParallel(const std::shared_ptr<CodeBlock>& block1,
                                        const std::shared_ptr<CodeBlock>& block2) {
    for (const auto& code1 : block1->code) {
        for (const auto& code2 : block2->code) {
            if (code1.find("neighbors_") != std::string::npos &&
                code2.find("neighbors_") != std::string::npos) {
                std::string var1 = code1.substr(code1.find("neighbors_"),
                                              code1.find(" =") - code1.find("neighbors_"));
                std::string var2 = code2.substr(code2.find("neighbors_"),
                                              code2.find(" =") - code2.find("neighbors_"));
                if (var1 == var2) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool CodeGeneration::tryGetFromCache(int vertex1, int vertex2, std::set<int>& result) {
    auto key = std::make_pair(std::min(vertex1, vertex2), std::max(vertex1, vertex2));
    auto it = cache.find(key);
    if (it != cache.end()) {
        result = it->second;
        return true;
    }
    return false;
}

void CodeGeneration::addToCache(int vertex1, int vertex2, const std::set<int>& result) {
    auto key = std::make_pair(std::min(vertex1, vertex2), std::max(vertex1, vertex2));
    cache[key] = result;
}

void CodeGeneration::clearCache() {
    cache.clear();
}

std::string CodeGeneration::generateSetOperationCode(SetOperation op,
                                                   const std::set<int>& set1,
                                                   const std::set<int>& set2,
                                                   const std::string& result_name) {
    std::stringstream ss;
    ss << "std::set<int> " << result_name << " = {";
    
    std::set<int> result;
    switch (op) {
        case SetOperation::INTERSECTION:
            result = getIntersection(set1, set2);
            ss << "// Intersection operation\n";
            break;
        case SetOperation::UNION:
            result = getUnion(set1, set2);
            ss << "// Union operation\n";
            break;
        case SetOperation::DIFFERENCE:
            result = getDifference(set1, set2);
            ss << "// Difference operation\n";
            break;
        case SetOperation::SYMMETRIC_DIFFERENCE:
            result = getSymmetricDifference(set1, set2);
            ss << "// Symmetric difference operation\n";
            break;
    }
    
    bool first = true;
    for (int value : result) {
        if (!first) {
            ss << ", ";
        }
        ss << value;
        first = false;
    }
    ss << "};\n";
    
    return ss.str();
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