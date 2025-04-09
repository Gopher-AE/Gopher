#pragma once

#include "pattern.h"

#include <vector>

class Mappings
{
private:
    /* data */
    int* adj_mat;
    int size;
public:
    Mappings(int _size, char *buffer);
    ~Mappings();
    void add_edge(int x, int y);
    void del_edge(int x, int y);
    void add_update_mapping(int x, int y);
    void print() const;
    inline const int* get_adj_mat_ptr() const {return adj_mat;}
};