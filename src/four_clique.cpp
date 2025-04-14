#include "../include/four_clique.h"
#include <algorithm>
#include <sstream>
#include <filesystem>

FourClique::FourClique() : matches_num_(0), all_time_(0.0) {}

void FourClique::addNode(const std::string& node, int times) {
    if (!hasNode(node)) {
        adjacency_list_[node] = std::set<std::string>();
        node_times_[node] = times;
    }
}

void FourClique::addEdge(const std::string& u, const std::string& v) {
    addNode(u);
    addNode(v);
    adjacency_list_[u].insert(v);
    adjacency_list_[v].insert(u);
}

bool FourClique::hasNode(const std::string& node) const {
    return adjacency_list_.find(node) != adjacency_list_.end();
}

bool FourClique::hasEdge(const std::string& u, const std::string& v) const {
    if (!hasNode(u) || !hasNode(v)) return false;
    return adjacency_list_.at(u).find(v) != adjacency_list_.at(u).end();
}

std::set<std::string> FourClique::neighborhood(const std::string& node) const {
    if (!hasNode(node)) return std::set<std::string>();
    return adjacency_list_.at(node);
}

void FourClique::process(const std::vector<std::string>& nodes) {
    if (nodes.size() == 4) {
        matches_num_++;
    }
}

void FourClique::mining(const std::vector<std::string>& edge, bool add_to_graph) {
    if (edge.size() != 2) return;
    
    if (add_to_graph) {
        addEdge(edge[0], edge[1]);
    }


    std::set<std::string> Nv0 = neighborhood(edge[0]);
    Nv0.erase(edge[1]);
    std::set<std::string> Nv1 = neighborhood(edge[1]);
    Nv1.erase(edge[0]);


    std::set<std::string> Cv2;
    std::set_intersection(Nv0.begin(), Nv0.end(),
                         Nv1.begin(), Nv1.end(),
                         std::inserter(Cv2, Cv2.begin()));

    for (const auto& v2 : Cv2) {
        std::set<std::string> Nv2 = neighborhood(v2);
        std::set<std::string> Cv3;
        std::set_intersection(Nv2.begin(), Nv2.end(),
                            Cv2.begin(), Cv2.end(),
                            std::inserter(Cv3, Cv3.begin()));

        for (const auto& v3 : Cv3) {
            if (v3 > v2) {
                process({edge[0], edge[1], v2, v3});
            }
        }
    }
}

size_t FourClique::getNodeCount() const {
    return adjacency_list_.size();
}

size_t FourClique::getEdgeCount() const {
    size_t count = 0;
    for (const auto& pair : adjacency_list_) {
        count += pair.second.size();
    }
    return count / 2; 
}

void FourClique::readGraphFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string line, u, v;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (iss >> u >> v) {
            if (!hasNode(u)) addNode(u, 0);
            if (!hasNode(v)) addNode(v, 0);
            addEdge(u, v);
        }
    }
}

std::vector<std::vector<std::string>> FourClique::readUpdatesFromFile(const std::string& filepath) {
    std::vector<std::vector<std::string>> updates;
    std::ifstream file(filepath);
    std::string line, u, v;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (iss >> u >> v) {
            if (!hasNode(u)) addNode(u, 0);
            if (!hasNode(v)) addNode(v, 0);
            updates.push_back({u, v});
        }
    }
    return updates;
}

void FourClique::setAllTime(double time) {
    all_time_ += time;
}

double FourClique::getAllTime() const {
    return all_time_;
}

int FourClique::getMatchesNum() const {
    return matches_num_;
} 