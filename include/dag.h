#ifndef DAG_H
#define DAG_H

#include "schedule.h"
#include <vector>
#include <memory>

class DAG {
private:
    int size;                   
    int* adj_matrix;           
    std::vector<Schedule> schedules; 

public:
    DAG(const std::vector<Schedule>& scheds);
    
    DAG(const DAG& other);

    ~DAG();
    
    int get_size() const { return size; }
    

    const int* get_adj_matrix() const { return adj_matrix; }
    const std::vector<Schedule>& get_schedules() const { return schedules; }
    
    void print() const;

    void build_from_schedules();

    static std::unique_ptr<DAG> DAG_combination(const std::vector<DAG>& dags);

    bool has_overlap(const DAG& other) const;

    std::vector<std::pair<int, int>> get_overlap_vertices(const DAG& other) const;

private:
    bool is_vertex_similar(int v1, int v2, const DAG& other, int other_v1, int other_v2) const;
};

#endif // DAG_H