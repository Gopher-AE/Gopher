 #include "../include/instance_restoration.h"
#include <algorithm>
#include <cmath>
#include <sstream>

InstanceRestoration::InstanceRestoration(const std::shared_ptr<Graph>& graph)
    : graph_(graph), max_embeddings_(1000) {}

std::vector<std::vector<int>> InstanceRestoration::mapSubstructureToEmbeddings(
    const std::shared_ptr<Pattern>& pattern,
    const std::vector<int>& substructure) {
    
    std::vector<std::vector<int>> embeddings;
    std::vector<int> current_embedding;
    std::vector<bool> visited(graph_->getVertexCount(), false);
    

    dfsGenerateEmbeddings(pattern, current_embedding, visited, embeddings);

    if (embeddings.size() > max_embeddings_) {
        std::sort(embeddings.begin(), embeddings.end(),
            [this](const std::vector<int>& a, const std::vector<int>& b) {
                return getEmbeddingScore(a) > getEmbeddingScore(b);
            });
        embeddings.resize(max_embeddings_);
    }
    
    return embeddings;
}

std::vector<std::vector<int>> InstanceRestoration::getAllPossibleEmbeddings(
    const std::shared_ptr<Pattern>& pattern) {
    
    std::string pattern_key = pattern->getPatternKey();
    if (embedding_cache_.find(pattern_key) != embedding_cache_.end()) {
        return embedding_cache_[pattern_key];
    }
    
    std::vector<std::vector<int>> embeddings;
    std::vector<int> current_embedding;
    std::vector<bool> visited(graph_->getVertexCount(), false);
    
    dfsGenerateEmbeddings(pattern, current_embedding, visited, embeddings);
    
    embedding_cache_[pattern_key] = embeddings;
    return embeddings;
}

bool InstanceRestoration::isValidEmbedding(
    const std::vector<int>& embedding,
    const std::shared_ptr<Pattern>& pattern) {
    
    if (embedding.size() != pattern->getVertexCount()) {
        return false;
    }
    

    std::vector<int> sorted_embedding = embedding;
    std::sort(sorted_embedding.begin(), sorted_embedding.end());
    if (std::adjacent_find(sorted_embedding.begin(), sorted_embedding.end()) != sorted_embedding.end()) {
        return false;
    }
    

    return satisfiesPatternConstraints(embedding, pattern);
}

double InstanceRestoration::getEmbeddingScore(const std::vector<int>& embedding) {
    std::stringstream ss;
    for (int v : embedding) {
        ss << v << ",";
    }
    std::string key = ss.str();
    
    if (score_cache_.find(key) != score_cache_.end()) {
        return score_cache_[key];
    }
    
    double score = calculateEmbeddingScore(embedding);
    score_cache_[key] = score;
    return score;
}

void InstanceRestoration::setMaxEmbeddings(int max_count) {
    max_embeddings_ = max_count;
}

int InstanceRestoration::getMaxEmbeddings() const {
    return max_embeddings_;
}

void InstanceRestoration::dfsGenerateEmbeddings(
    const std::shared_ptr<Pattern>& pattern,
    std::vector<int>& current_embedding,
    std::vector<bool>& visited,
    std::vector<std::vector<int>>& embeddings) {
    
    if (current_embedding.size() == pattern->getVertexCount()) {
        if (isValidEmbedding(current_embedding, pattern)) {
            embeddings.push_back(current_embedding);
        }
        return;
    }
    
    int current_vertex = pattern->getVertex(current_embedding.size());
    for (int v = 0; v < graph_->getVertexCount(); ++v) {
        if (!visited[v]) {
            bool valid = true;
            for (int i = 0; i < current_embedding.size(); ++i) {
                int u = current_embedding[i];
                if (pattern->hasEdge(i, current_embedding.size()) != hasEdge(u, v)) {
                    valid = false;
                    break;
                }
            }
            
            if (valid) {
                visited[v] = true;
                current_embedding.push_back(v);
                dfsGenerateEmbeddings(pattern, current_embedding, visited, embeddings);
                current_embedding.pop_back();
                visited[v] = false;
            }
        }
    }
}

bool InstanceRestoration::hasEdge(int u, int v) const {
    return graph_->hasEdge(u, v);
}

bool InstanceRestoration::satisfiesPatternConstraints(
    const std::vector<int>& embedding,
    const std::shared_ptr<Pattern>& pattern) {
    
    for (int i = 0; i < embedding.size(); ++i) {
        for (int j = i + 1; j < embedding.size(); ++j) {
            if (pattern->hasEdge(i, j) != hasEdge(embedding[i], embedding[j])) {
                return false;
            }
        }
    }
    return true;
}

double InstanceRestoration::calculateEmbeddingScore(const std::vector<int>& embedding) {
    double score = 0.0;
    for (int v : embedding) {
        score += graph_->getVertexDegree(v);
    }
    return score;
}