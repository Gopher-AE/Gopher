 #include "../include/dag.h"
#include <iostream>
#include <cstring>
#include <map>
#include <set>
#include <algorithm>


DAG::DAG(const std::vector<Schedule>& scheds) : schedules(scheds) {
    if (schedules.empty()) {
        size = 0;
        adj_matrix = nullptr;
        return;
    }
    
    size = schedules[0].get_size();
    
    adj_matrix = new int[size * size];
    memset(adj_matrix, 0, size * size * sizeof(int));

    build_from_schedules();
}

DAG::~DAG() {
    delete[] adj_matrix;
}

void DAG::print() const {
    std::cout << "DAG Size: " << size << "\n";
    std::cout << "Adjacency Matrix:\n";
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            std::cout << adj_matrix[i * size + j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "Number of Schedules: " << schedules.size() << "\n";
}

void DAG::build_from_schedules() {

    for (const Schedule& sched : schedules) {
        const int* curr_matrix = sched.get_adj_matrix();
        
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (curr_matrix[i * size + j] > 0) {
                    adj_matrix[i * size + j] = curr_matrix[i * size + j];
                }
            }
        }
    }
}



DAG::DAG(const DAG& other) : size(other.size), schedules(other.schedules) {
    adj_matrix = new int[size * size];
    memcpy(adj_matrix, other.adj_matrix, size * size * sizeof(int));
}

bool DAG::is_vertex_similar(int v1, int v2, const DAG& other, int other_v1, int other_v2) const {

    for (int i = 0; i < size; ++i) {
        if (i != v1 && i != v2) {
            for (int j = 0; j < other.size; ++j) {
                if (j != other_v1 && j != other_v2) {
                    if (adj_matrix[v1 * size + i] != other.adj_matrix[other_v1 * other.size + j] ||
                        adj_matrix[v2 * size + i] != other.adj_matrix[other_v2 * other.size + j]) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}


bool DAG::has_overlap(const DAG& other) const {
    for (int i = 0; i < size - 1; ++i) {
        for (int j = i + 1; j < size; ++j) {
            for (int k = 0; k < other.size - 1; ++k) {
                for (int l = k + 1; l < other.size; ++l) {
                    if (is_vertex_similar(i, j, other, k, l)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


std::vector<std::pair<int, int>> DAG::get_overlap_vertices(const DAG& other) const {
    std::vector<std::pair<int, int>> overlaps;
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < other.size; ++j) {
            bool is_similar = true;

            for (int k = 0; k < size; ++k) {
                if (k == i) continue;
                bool found_match = false;
                for (int l = 0; l < other.size; ++l) {
                    if (l == j) continue;
                    if (adj_matrix[i * size + k] == other.adj_matrix[j * other.size + l]) {
                        found_match = true;
                        break;
                    }
                }
                if (!found_match) {
                    is_similar = false;
                    break;
                }
            }
            if (is_similar) {
                overlaps.push_back({i, j});
            }
        }
    }
    return overlaps;
}


std::unique_ptr<DAG> DAG::DAG_combination(const std::vector<DAG>& dags) {
    if (dags.empty()) {
        return nullptr;
    }

    if (dags.size() == 1) {
        return std::make_unique<DAG>(dags[0]);
    }

    std::map<std::pair<int, int>, int> vertex_mapping;  // <<dag_idx, vertex>, new_vertex>
    std::set<int> used_vertices;
    int next_vertex = 0;

    for (size_t i = 0; i < dags.size() - 1; ++i) {
        for (size_t j = i + 1; j < dags.size(); ++j) {
            auto overlaps = dags[i].get_overlap_vertices(dags[j]);
            for (const auto& overlap : overlaps) {
                int v1 = overlap.first;
                int v2 = overlap.second;

                auto key1 = std::make_pair(i, v1);
                auto key2 = std::make_pair(j, v2);
                
                if (vertex_mapping.find(key1) == vertex_mapping.end() &&
                    vertex_mapping.find(key2) == vertex_mapping.end()) {
                    vertex_mapping[key1] = next_vertex;
                    vertex_mapping[key2] = next_vertex;
                    used_vertices.insert(next_vertex);
                    next_vertex++;
                } else if (vertex_mapping.find(key1) != vertex_mapping.end()) {
                    vertex_mapping[key2] = vertex_mapping[key1];
                } else {
                    vertex_mapping[key1] = vertex_mapping[key2];
                }
            }
        }
    }

    for (size_t i = 0; i < dags.size(); ++i) {
        for (int v = 0; v < dags[i].size; ++v) {
            auto key = std::make_pair(i, v);
            if (vertex_mapping.find(key) == vertex_mapping.end()) {
                while (used_vertices.find(next_vertex) != used_vertices.end()) {
                    next_vertex++;
                }
                vertex_mapping[key] = next_vertex;
                used_vertices.insert(next_vertex);
                next_vertex++;
            }
        }
    }

    int new_size = next_vertex;
    std::vector<Schedule> combined_schedules;
    int* new_adj_matrix = new int[new_size * new_size];
    memset(new_adj_matrix, 0, new_size * new_size * sizeof(int));

    for (size_t i = 0; i < dags.size(); ++i) {
        const DAG& curr_dag = dags[i];
        for (int v1 = 0; v1 < curr_dag.size; ++v1) {
            for (int v2 = 0; v2 < curr_dag.size; ++v2) {
                if (curr_dag.adj_matrix[v1 * curr_dag.size + v2] > 0) {
                    int new_v1 = vertex_mapping[std::make_pair(i, v1)];
                    int new_v2 = vertex_mapping[std::make_pair(i, v2)];
                    new_adj_matrix[new_v1 * new_size + new_v2] = 
                        curr_dag.adj_matrix[v1 * curr_dag.size + v2];
                }
            }
        }
    }

    Schedule combined_schedule(new_adj_matrix, new_size);
    combined_schedules.push_back(combined_schedule);

    auto result = std::make_unique<DAG>(combined_schedules);
    delete[] new_adj_matrix;
    return result;
}