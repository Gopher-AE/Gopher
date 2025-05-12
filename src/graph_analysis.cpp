#include "../include/graph_analysis.h"
#include <algorithm>
#include <queue>
#include <limits>
#include <stack>

GraphAnalysis::GraphAnalysis(const DAG& dag) : dag_(dag) {
    buildAdjacencyList();
}

void GraphAnalysis::buildAdjacencyList() {
    int size = dag_.get_size();
    const int* adj_matrix = dag_.get_adj_matrix();
    adjacency_list_.resize(size);
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (adj_matrix[i * size + j] > 0) {
                adjacency_list_[i].push_back(j);
            }
        }
    }
}

int GraphAnalysis::getVertexCount() const {
    return dag_.get_size();
}

int GraphAnalysis::getEdgeCount() const {
    int count = 0;
    const int* adj_matrix = dag_.get_adj_matrix();
    int size = dag_.get_size();
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (adj_matrix[i * size + j] > 0) {
                ++count;
            }
        }
    }
    return count;
}

double GraphAnalysis::getDensity() const {
    int v = getVertexCount();
    int e = getEdgeCount();
    return v > 1 ? (2.0 * e) / (v * (v - 1)) : 0.0;
}

std::vector<int> GraphAnalysis::getDegrees() const {
    int size = dag_.get_size();
    std::vector<int> degrees(size, 0);
    const int* adj_matrix = dag_.get_adj_matrix();
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (adj_matrix[i * size + j] > 0) {
                ++degrees[i];  
                ++degrees[j];  
            }
        }
    }
    return degrees;
}

std::vector<int> GraphAnalysis::findLongestPath() const {
    int size = dag_.get_size();
    std::vector<int> dist(size, std::numeric_limits<int>::min());
    std::vector<int> parent(size, -1);
    std::vector<bool> visited(size, false);
    std::vector<int> order;
    
    for (int i = 0; i < size; ++i) {
        if (!visited[i]) {
            topologicalSort(i, visited, order);
        }
    }
    
    dist[order[0]] = 0;
    for (int u : order) {
        for (int v : adjacency_list_[u]) {
            if (dist[v] < dist[u] + 1) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
            }
        }
    }
    
    std::vector<int> path;
    int max_dist_vertex = std::max_element(dist.begin(), dist.end()) - dist.begin();
    for (int v = max_dist_vertex; v != -1; v = parent[v]) {
        path.push_back(v);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<int> GraphAnalysis::findCriticalPath() const {
    return findLongestPath(); 
}

std::map<int, int> GraphAnalysis::getShortestDistances(int source) const {
    int size = dag_.get_size();
    std::map<int, int> distances;
    std::vector<bool> visited(size, false);
    std::queue<int> q;
    
    for (int i = 0; i < size; ++i) {
        distances[i] = std::numeric_limits<int>::max();
    }
    distances[source] = 0;
    q.push(source);
    
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        visited[u] = true;
        
        for (int v : adjacency_list_[u]) {
            if (!visited[v] && distances[v] > distances[u] + 1) {
                distances[v] = distances[u] + 1;
                q.push(v);
            }
        }
    }
    
    return distances;
}

bool GraphAnalysis::isStronglyConnected() const {
    int size = dag_.get_size();
    std::vector<bool> visited(size, false);
    
    dfs(0, visited, std::vector<int>());
    
    if (std::find(visited.begin(), visited.end(), false) != visited.end()) {
        return false;
    }
    
    std::vector<std::vector<int>> transpose(size);
    for (int i = 0; i < size; ++i) {
        for (int j : adjacency_list_[i]) {
            transpose[j].push_back(i);
        }
    }
    
    std::fill(visited.begin(), visited.end(), false);
    std::vector<int> temp_path;
    dfs(0, visited, temp_path);
    
    return std::find(visited.begin(), visited.end(), false) == visited.end();
}

std::vector<std::set<int>> GraphAnalysis::getStronglyConnectedComponents() const {
    int size = dag_.get_size();
    std::vector<std::set<int>> components;
    std::vector<bool> visited(size, false);
    std::stack<int> stack;
    
    for (int i = 0; i < size; ++i) {
        if (!visited[i]) {
            std::vector<int> temp_path;
            dfs(i, visited, temp_path);
            stack.push(i);
        }
    }
    
    std::vector<std::vector<int>> transpose(size);
    for (int i = 0; i < size; ++i) {
        for (int j : adjacency_list_[i]) {
            transpose[j].push_back(i);
        }
    }
    
    std::fill(visited.begin(), visited.end(), false);
    while (!stack.empty()) {
        int v = stack.top();
        stack.pop();
        
        if (!visited[v]) {
            std::set<int> component;
            std::vector<int> temp_path;
            dfs(v, visited, temp_path);
            for (int u : temp_path) {
                component.insert(u);
            }
            components.push_back(component);
        }
    }
    
    return components;
}

std::vector<std::set<int>> GraphAnalysis::findCycles() const {
    std::vector<std::set<int>> cycles;
    int size = dag_.get_size();
    std::vector<bool> visited(size, false);
    std::vector<bool> in_current_path(size, false);
    std::vector<int> parent(size, -1);
    
    for (int i = 0; i < size; ++i) {
        if (!visited[i]) {
            std::function<void(int)> dfs_cycle = [&](int v) {
                visited[v] = true;
                in_current_path[v] = true;
                
                for (int u : adjacency_list_[v]) {
                    if (!visited[u]) {
                        parent[u] = v;
                        dfs_cycle(u);
                    } else if (in_current_path[u]) {

                        std::set<int> cycle;
                        for (int curr = v; curr != u; curr = parent[curr]) {
                            cycle.insert(curr);
                        }
                        cycle.insert(u);
                        cycles.push_back(cycle);
                    }
                }
                
                in_current_path[v] = false;
            };
            
            dfs_cycle(i);
        }
    }
    
    return cycles;
}

std::vector<GraphAnalysis::OptimizationSuggestion> GraphAnalysis::analyzeForOptimization() const {
    std::vector<OptimizationSuggestion> suggestions;
    
    auto levels = getExecutionLevels();
    if (levels.size() > 1) {
        for (size_t i = 0; i < levels.size(); ++i) {
            if (levels[i].size() > 1) {
                OptimizationSuggestion suggestion;
                suggestion.type = "Parallelization";
                suggestion.description = "Level " + std::to_string(i) + " can be parallelized";
                suggestion.impact_score = levels[i].size() * 0.1;
                suggestion.affected_vertices = levels[i];
                suggestions.push_back(suggestion);
            }
        }
    }
    
    auto degrees = getDegrees();
    for (int i = 0; i < getVertexCount(); ++i) {
        if (degrees[i] > getVertexCount() / 2) {
            OptimizationSuggestion suggestion;
            suggestion.type = "Bottleneck";
            suggestion.description = "Vertex " + std::to_string(i) + " is a potential bottleneck";
            suggestion.impact_score = 0.8;
            suggestion.affected_vertices = {i};
            suggestions.push_back(suggestion);
        }
    }
    
    return suggestions;
}

std::map<int, std::set<int>> GraphAnalysis::getDependencyMap() const {
    std::map<int, std::set<int>> dependencies;
    int size = dag_.get_size();
    
    for (int i = 0; i < size; ++i) {
        dependencies[i] = std::set<int>();
        for (int j : adjacency_list_[i]) {
            dependencies[i].insert(j);
        }
    }
    
    return dependencies;
}

std::vector<std::vector<int>> GraphAnalysis::getExecutionLevels() const {
    int size = dag_.get_size();
    std::vector<std::vector<int>> levels;
    std::vector<int> in_degree(size, 0);
    std::queue<int> q;
    
    for (int i = 0; i < size; ++i) {
        for (int j : adjacency_list_[i]) {
            ++in_degree[j];
        }
    }
    
    for (int i = 0; i < size; ++i) {
        if (in_degree[i] == 0) {
            q.push(i);
        }
    }
    
    while (!q.empty()) {
        int level_size = q.size();
        std::vector<int> current_level;
        
        for (int i = 0; i < level_size; ++i) {
            int v = q.front();
            q.pop();
            current_level.push_back(v);
            
            for (int u : adjacency_list_[v]) {
                if (--in_degree[u] == 0) {
                    q.push(u);
                }
            }
        }
        
        levels.push_back(current_level);
    }
    
    return levels;
}

GraphAnalysis::PerformanceMetrics GraphAnalysis::estimatePerformance() const {
    PerformanceMetrics metrics;
    
    auto critical_path = findCriticalPath();
    metrics.critical_path_length = critical_path.size();
    
    metrics.estimated_execution_time = critical_path.size() * calculateNodeComplexity(critical_path[0]);
    
    auto levels = getExecutionLevels();
    double max_parallel = 0;
    for (const auto& level : levels) {
        max_parallel = std::max(max_parallel, static_cast<double>(level.size()));
    }
    metrics.parallelization_potential = max_parallel / getVertexCount();
    
    auto degrees = getDegrees();
    for (int i = 0; i < getVertexCount(); ++i) {
        if (degrees[i] > getVertexCount() / 2) {
            metrics.bottlenecks.push_back({i, degrees[i]});
        }
    }
    
    return metrics;
}

void GraphAnalysis::dfs(int vertex, std::vector<bool>& visited, std::vector<int>& path) const {
    visited[vertex] = true;
    path.push_back(vertex);
    
    for (int neighbor : adjacency_list_[vertex]) {
        if (!visited[neighbor]) {
            dfs(neighbor, visited, path);
        }
    }
}

void GraphAnalysis::topologicalSort(int vertex, std::vector<bool>& visited, 
                                  std::vector<int>& order) const {
    visited[vertex] = true;
    
    for (int neighbor : adjacency_list_[vertex]) {
        if (!visited[neighbor]) {
            topologicalSort(neighbor, visited, order);
        }
    }
    
    order.push_back(vertex);
}

bool GraphAnalysis::hasCycle() const {
    int size = dag_.get_size();
    std::vector<bool> visited(size, false);
    std::vector<bool> rec_stack(size, false);
    
    std::function<bool(int)> dfs_cycle = [&](int v) {
        visited[v] = true;
        rec_stack[v] = true;
        
        for (int neighbor : adjacency_list_[v]) {
            if (!visited[neighbor] && dfs_cycle(neighbor)) {
                return true;
            } else if (rec_stack[neighbor]) {
                return true;
            }
        }
        
        rec_stack[v] = false;
        return false;
    };
    
    for (int i = 0; i < size; ++i) {
        if (!visited[i] && dfs_cycle(i)) {
            return true;
        }
    }
    
    return false;
}

double GraphAnalysis::calculateNodeComplexity(int vertex) const {
    double complexity = 1.0;
    int out_degree = adjacency_list_[vertex].size();
    int in_degree = 0;
    
    for (const auto& adj : adjacency_list_) {
        if (std::find(adj.begin(), adj.end(), vertex) != adj.end()) {
            ++in_degree;
        }
    }
    
    complexity *= (1 + 0.1 * (in_degree + out_degree));
    return complexity;
} 