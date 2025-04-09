#include "../include/schedule.h"
#include "../include/mappings.h"
#include <assert.h>
#include <cstring>
#include <set>
#include <vector>
#include <cstdio>
#include <iostream>


Schedule::Schedule(std::vector<Mappings> &mappings, std::vector<bool> &is_unique) {
    for(size_t i = 0; i < mappings.size(); ++i) {
        if (!is_unique[i]) continue;
    }
}

Schedule::Schedule(const int* _adj_mat, int _size) {
    size = _size;
    adj_mat = new int[size * size];
    memset(adj_mat, 0, size * size * sizeof(int));

    for(int i = 0; i < size; ++i)
        for(int j = 0; j < size; ++j)
            if(_adj_mat[INDEX(i,j,size)] == 1)
                add_edge(i, j);
            else if (_adj_mat[INDEX(i,j,size)] == 2)
                add_update_mapping(i, j);
}

Schedule::~Schedule()
{
    // delete[] adj_mat;
}

void Schedule::generate_schedules(int *reorderschedule) const{
    const int* curr_adj = adj_mat;

    // 找到包含边值2的两行
    int row1 = -1, row2 = -1;
    for(int x = 0; x < size; ++x) {
        for(int y = 0; y < size; ++y) {
            if(curr_adj[INDEX(x, y, size)] == 2) {
                if(row1 == -1) {
                    row1 = x;
                } else {
                    row2 = x;
                    break;
                }
            }
        }
        if(row2 != -1) break;
    }

    int* new_adj = new int[size * size];
    memset(new_adj, 0, size * size * sizeof(int));

    if(row1 != 0 || row2 != 1) {

        std::vector<int> row_map(size);
        for(int i = 0; i < size; ++i) row_map[i] = i;

        if(row1 != 0) {
            std::swap(row_map[0], row_map[row1]);
        }
        if(row2 != 1) {
            std::swap(row_map[1], row_map[row2]);
        }

        for(int x = 0; x < size; ++x) {
            for(int y = 0; y < size; ++y) {
                new_adj[INDEX(row_map[x], row_map[y], size)] =
                    curr_adj[INDEX(x, y, size)];
            }
        }
        memcpy(reorderschedule, new_adj, size * size * sizeof(int));
    } else {
        memcpy(reorderschedule, curr_adj, size * size * sizeof(int));
    }

    // printf("\nCanonical Mapping:\n");
    // for(int i = 0; i < size; ++i) {
    //     for(int j = 0; j < size; ++j)
    //         printf("%d", reorderschedule[INDEX(i,j,size)]);
    //     puts("");
    // }
}

void Schedule::add_edge(int x, int y)
{
    adj_mat[INDEX(x, y, size)] = 1;
    adj_mat[INDEX(y, x, size)] = 1;
}

void Schedule::del_edge(int x, int y)
{
    adj_mat[INDEX(x, y, size)] = 0;
    adj_mat[INDEX(y, x, size)] = 0;
}

void Schedule::add_update_mapping(int x, int y) {
    adj_mat[INDEX(x, y, size)] = 2;
    adj_mat[INDEX(y, x, size)] = 2;
}

void Schedule::print_schedule() const{
    printf("Schedule:\n");
    for(int i = 0; i < size; ++i) {
        for(int j = 0; j < size; ++j)
            printf("%d", adj_mat[INDEX(i,j,size)]);
        puts("");
    }
}


