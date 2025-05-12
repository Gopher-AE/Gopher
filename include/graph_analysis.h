#ifndef GRAPH_ANALYSIS_H
#define GRAPH_ANALYSIS_H

#include "dag.h"
#include <vector>
#include <map>
#include <set>

class GraphAnalysis {
public:
    GraphAnalysis(const DAG& dag);
    ~GraphAnalysis() = default;

    int getVertexCount() const;
    int getEdgeCount() const;
    double getDensity() const;
    std::vector<int> getDegrees() const;
    
    std::vector<int> findLongestPath() const;
    std::vector<int> findCriticalPath() const;
    std::map<int, int> getShortestDistances(int source) const;
    
    bool isStronglyConnected() const;
    std::vector<std::set<int>> getStronglyConnectedComponents() const;
    std::vector<std::set<int>> findCycles() const;
    
    struct OptimizationSuggestion {
        std::string type;
        std::string description;
        double impact_score;
        std::vector<int> affected_vertices;
    };
    
    std::vector<OptimizationSuggestion> analyzeForOptimization() const;
    
    std::map<int, std::set<int>> getDependencyMap() const;
    std::vector<std::vector<int>> getExecutionLevels() const;
    
    struct PerformanceMetrics {
        double estimated_execution_time;
        double parallelization_potential;
        int critical_path_length;
        std::vector<std::pair<int, int>> bottlenecks;
    };
    
    PerformanceMetrics estimatePerformance() const;

private:
    const DAG& dag_;
    std::vector<std::vector<int>> adjacency_list_;
    
    void buildAdjacencyList();
    void dfs(int vertex, std::vector<bool>& visited, std::vector<int>& path) const;
    void topologicalSort(int vertex, std::vector<bool>& visited, 
                        std::vector<int>& order) const;
    bool hasCycle() const;
    double calculateNodeComplexity(int vertex) const;
};

#endif // GRAPH_ANALYSIS_H 