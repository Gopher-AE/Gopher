 #ifndef INSTANCE_RESTORATION_H
#define INSTANCE_RESTORATION_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "pattern.h"


class InstanceRestoration {
public:

    InstanceRestoration(const std::shared_ptr<Graph>& graph);
    ~InstanceRestoration() = default;
    std::vector<std::vector<int>> mapSubstructureToEmbeddings(
        const std::shared_ptr<Pattern>& pattern,
        const std::vector<int>& substructure
    );

    std::vector<std::vector<int>> getAllPossibleEmbeddings(
        const std::shared_ptr<Pattern>& pattern
    );

    bool isValidEmbedding(
        const std::vector<int>& embedding,
        const std::shared_ptr<Pattern>& pattern
    );

    double getEmbeddingScore(const std::vector<int>& embedding);

    void setMaxEmbeddings(int max_count);

    int getMaxEmbeddings() const;

private:

    void dfsGenerateEmbeddings(
        const std::shared_ptr<Pattern>& pattern,
        std::vector<int>& current_embedding,
        std::vector<bool>& visited,
        std::vector<std::vector<int>>& embeddings
    );

    bool hasEdge(int u, int v) const;

    bool satisfiesPatternConstraints(
        const std::vector<int>& embedding,
        const std::shared_ptr<Pattern>& pattern
    );
    
    double calculateEmbeddingScore(const std::vector<int>& embedding);

    std::shared_ptr<Graph> graph_;

    int max_embeddings_;

    std::unordered_map<std::string, std::vector<std::vector<int>>> embedding_cache_;

    std::unordered_map<std::string, double> score_cache_;
};

#endif // INSTANCE_RESTORATION_H