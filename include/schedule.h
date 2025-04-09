#pragma once

#include "pattern.h"
#include "mappings.h"
#include <vector>

class Schedule
{
public:
    Schedule(std::vector<Mappings> &mappings, std::vector<bool> &is_unique);
    Schedule(const int* _adj_mat, int _size);
    ~Schedule();
    void print_schedule() const;
    void add_edge(int x, int y);
    void del_edge(int x, int y);
    void add_update_mapping(int x, int y);
    void generate_schedules(int *reorderschedule) const;
    
    const int* get_adj_matrix() const { return adj_mat; }
    int get_size() const { return size; }

private:
    int* adj_mat;
    int size;
    void get_full_permutation(std::vector< std::vector<int> >& vec, bool use[], std::vector<int> tmp_vec, int depth) const;
};