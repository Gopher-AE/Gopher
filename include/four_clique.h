#ifndef FOUR_CLIQUE_H
#define FOUR_CLIQUE_H

#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <fstream>

class FourClique {
public:
    FourClique();
    

    void addNode(const std::string& node, int times = 0);
    void addEdge(const std::string& u, const std::string& v);
    bool hasNode(const std::string& node) const;
    bool hasEdge(const std::string& u, const std::string& v) const;
    

    std::set<std::string> neighborhood(const std::string& node) const;
    

    void process(const std::vector<std::string>& nodes);
    

    void mining(const std::vector<std::string>& edge, bool add_to_graph = true);
    

    size_t getNodeCount() const;
    size_t getEdgeCount() const;
    
 
    void readGraphFromFile(const std::string& filepath);
    std::vector<std::vector<std::string>> readUpdatesFromFile(const std::string& filepath);
    
 
    int getMatchesNum() const;
    

    void setAllTime(double time);
    double getAllTime() const;

private:
    std::unordered_map<std::string, std::set<std::string>> adjacency_list_;
    std::unordered_map<std::string, int> node_times_; 
    int matches_num_;
    double all_time_;
};

#endif // FOUR_CLIQUE_H 