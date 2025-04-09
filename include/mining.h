#ifndef MINING_H
#define MINING_H

#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "dag.h"

class Mining {
private:
    std::unordered_map<int, std::unordered_set<int>> graph;
    std::string graph_file_path;    
    std::string update_file_path;   
    std::unique_ptr<DAG> dag;
    size_t pattern_count;          
    
    std::unordered_set<int> neighborhood(int vertex) const;
    void process(const std::vector<int>& pattern);
    
    void mine_patterns(const std::pair<int, int>& edge);

public:
    Mining(const std::string& graph_path, const std::string& update_path, std::unique_ptr<DAG>& input_dag) 
        : graph_file_path(graph_path), update_file_path(update_path), dag(std::move(input_dag)) {}
    
    Mining() : pattern_count(0) {}
    
    void set_graph_file(const std::string& path) { graph_file_path = path; }
    void set_update_file(const std::string& path) { update_file_path = path; }
    
    const std::string& get_graph_file() const { return graph_file_path; }
    const std::string& get_update_file() const { return update_file_path; }
    
    void add_node(int node);
    void add_edge(int u, int v);
    bool has_node(int node) const;
    bool has_edge(int u, int v) const;
    
    void mining(const std::pair<int, int>& edge, bool add_to_graph = true);
    
    bool initialize(); 
    void run();       
    
    size_t node_count() const { return graph.size(); }
    size_t edge_count() const;
    
    void clear() { graph.clear(); }

    size_t get_pattern_count() const { return pattern_count; }
    
    void reset_count() { pattern_count = 0; }
};

#endif // MINING_H