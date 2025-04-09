#include "../include/mappings.h"

#include <assert.h>
#include <cstring>
#include <set>
#include <vector>
#include <cstdio>
#include <algorithm>


Mappings::Mappings(int _size, char *buffer)
{
    size = _size;
    adj_mat = new int[size * size];
    memset(adj_mat, 0, size * size * sizeof(int));

    for(int i = 0; i < size; ++i)
        for(int j = 0; j < size; ++j)
            if(buffer[INDEX(i,j,size)] == '1')
                add_edge(i, j);
            else if (buffer[INDEX(i,j,size)] == '2')
                add_update_mapping(i, j);


}


Mappings::~Mappings()
{
    // delete[] adj_mat;
}

void Mappings::add_edge(int x, int y)
{
    adj_mat[INDEX(x, y, size)] = 1;
    adj_mat[INDEX(y, x, size)] = 1;
}

void Mappings::del_edge(int x, int y)
{
    adj_mat[INDEX(x, y, size)] = 0;
    adj_mat[INDEX(y, x, size)] = 0;
}

void Mappings::add_update_mapping(int x, int y) {
    adj_mat[INDEX(x, y, size)] = 2;
    adj_mat[INDEX(y, x, size)] = 2;
}

void Mappings::print() const
{
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (adj_mat[INDEX(i, j, size)] != 0)
                printf("(%d,%d) ", i, j);
    printf("\n");
}