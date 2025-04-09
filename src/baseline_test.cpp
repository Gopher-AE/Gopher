#include "../include/pattern.h"
#include "../include/mappings.h"
#include "../include/dag.h"
#include "../include/mining.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <chrono>

#include "../include/schedule.h"


bool is_isomorphic(const Mappings& m1, const Mappings& m2, int size) {
    const int* adj1 = m1.get_adj_mat_ptr();
    const int* adj2 = m2.get_adj_mat_ptr();

    std::vector<int> perm(size);
    for(int i = 0; i < size; ++i) perm[i] = i;

    do {
        bool is_same = true;

        for(int i = 0; i < size && is_same; ++i) {
            for(int j = 0; j < size && is_same; ++j) {
                int val1 = adj1[INDEX(i, j, size)];
                int val2 = adj2[INDEX(perm[i], perm[j], size)];

                if (val1 != val2) {
                    is_same = false;
                }
            }
        }
        if (is_same) return true;
    } while(std::next_permutation(perm.begin(), perm.end()));

    return false;
}

void generate_permutations(const int* adj_mat, int size) {

    std::vector<int> vertices_to_permute;
    for(int i = 2; i < size; ++i) {
        vertices_to_permute.push_back(i);
    }

    std::sort(vertices_to_permute.begin(), vertices_to_permute.end());

    // std::cout << "\nGenerated Schedules:\n";
    int *opr_mat = new int[size * size];


    do {

        for(int i = 0; i < size; ++i)
            for(int j = 0; j < size; ++j)
                opr_mat[INDEX(i, j, size)] = adj_mat[INDEX(i, j, size)];

        // std::cout << "Vertex Searched Order: ";
        int stage = 2;
        for(int v : vertices_to_permute) {
            //[rule1] 
            bool isconnected = false;
            // std::cout << v << " ";
            for (int j = 0; j < stage; j++) {
                if(opr_mat[INDEX(j, v, size)] == 1)
                    isconnected = true;
            }

            if (!isconnected) {
                // std::cout<<"(invalid mining schedule) Rule 1";
                break;
            }

            //[rule 2] 
            if ((size-stage) == 1) {
                // printf("\nGenerated Schedule:\n");
                // for(int i = 0; i < size; ++i) {
                //     for(int j = 0; j < size; ++j)
                //         printf("%d", adj_mat[INDEX(i,j,size)]);
                //     puts("");
                // }
                break;
            }

            int *edge_buffer = new int[size]();
            bool is_valid = true;

            int current_edges = 0;
            for (int curr = 0; curr < stage; ++curr) {
                if(opr_mat[INDEX(v, curr, size)] == 1)
                    current_edges++;
            }


            for(int x = stage; x < size; ++x) {
                if(x == v) continue;  

                int edge_count = 0;
                for (int curr = 0; curr < stage; ++curr) {
                    if(opr_mat[INDEX(x, curr, size)] == 1)
                        edge_count++;
                }

                // 
                if (edge_count > current_edges) {
                    // std::cout << "(invalid mining schedule) Rule 2 ";
                    is_valid = false;
                    break;
                }
            }

            delete[] edge_buffer;
            if (!is_valid) {
                break;
            }



            if (v != stage) {

                for (int j = 0; j < size; j++) {
                    int temp = opr_mat[INDEX(stage, j, size)];
                    opr_mat[INDEX(stage, j, size)] = opr_mat[INDEX(v, j, size)];
                    opr_mat[INDEX(v, j, size)] = temp;
                }

                for (int i = 0; i < size; i++) {
                    int temp = opr_mat[INDEX(i, stage, size)];
                    opr_mat[INDEX(i, stage, size)] = opr_mat[INDEX(i, v, size)];
                    opr_mat[INDEX(i, v, size)] = temp;
                }
            }


            stage++;


        }
        // std::cout << "\n";
    } while(std::next_permutation(vertices_to_permute.begin(), vertices_to_permute.end()));

    delete[] opr_mat;
}


void test_pattern(const std::string &graphfile, const std::string &udpatefile, const Pattern &p) {
    // p.print();
    // std::set< std::set<int> > pattern_edge;
    // p.count_all_isomorphism(pattern_edge);
    // for (const std::set<int>& edge_set : pattern_edge) {
    //     for (int vs : edge_set) {
    //         std::cout << vs << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::string adj_str = std::to_string(p.get_adj_mat_ptr());
    // Mappings mps(p.get_size(),adj_str);

    std::vector<Mappings> mappingses;
    const int size = p.get_size();
    int* adj_mat = new int[size*size];
    memset(adj_mat, 0, size * size * sizeof(int));
    const int* pattern_adj_mat = p.get_adj_mat_ptr();

    for(int i = 0; i < size; ++i) {
        for(int j = i + 1; j < size; ++j) {
            // if(pattern_adj_mat[i][j] == 1)
            if (pattern_adj_mat[INDEX(i, j, size)] == 1) {
                for(int x = 0; x < size; ++x)
                    for(int y = 0; y < size; ++y)
                        adj_mat[INDEX(x, y, size)] = pattern_adj_mat[INDEX(x, y, size)];

                adj_mat[INDEX(i, j, size)] = 2;
                adj_mat[INDEX(j, i, size)] = 2;

                char* adj_str = new char[size * size + 1];
                for(int x = 0; x < size * size; ++x) {
                    adj_str[x] = adj_mat[x] + '0';
                }
                adj_str[size * size] = '\0';

                Mappings initial_mapping(size, adj_str);
                mappingses.push_back(initial_mapping);
            }
        }
    }


    for(size_t i = 0; i < mappingses.size(); ++i) {
        // std::cout << "\nMapping #" << i + 1 << ":\n";
        const int* curr_adj_mat = mappingses[i].get_adj_mat_ptr()   ;

        // for(int x = 0; x < size; ++x) {
        //     for(int y = 0; y < size; ++y) {
        //         std::cout << curr_adj_mat[INDEX(x, y, size)] << " ";
        //     }
        //     std::cout << "\n";
        // }
        // std::cout << "------------------------\n";
    }

    delete[] adj_mat;

    std::vector<bool> is_unique(mappingses.size(), true);  
    std::vector<std::pair<int, int>> update_edges;  

    for(size_t i = 0; i < mappingses.size(); ++i) {
        if (!is_unique[i]) continue;

        const int* curr_adj = mappingses[i].get_adj_mat_ptr();
        std::pair<int, int> edge;
        for(int x = 0; x < size; ++x) {
            for(int y = x + 1; y < size; ++y) {
                if(curr_adj[INDEX(x, y, size)] == 2) {
                    edge = {x, y};
                    break;
                }
            }
        }
        update_edges.push_back(edge);

        for(size_t j = i + 1; j < mappingses.size(); ++j) {
            if (!is_unique[j]) continue;

            if (is_isomorphic(mappingses[i], mappingses[j], size)) {
                is_unique[j] = false;
            }
        }
    }

    int unique_count = std::count(is_unique.begin(), is_unique.end(), true);
    // std::cout << "\nAfter breaking edge symmetrys, there remains " << unique_count << " mappings\n";

    // std::cout << "\nFinal Results:\n";
    int idx = 0;
    for(size_t i = 0; i < mappingses.size(); ++i) {
        if (!is_unique[i]) continue;

        int a = update_edges[idx].first;
        int b = update_edges[idx].second;

        std::vector<int> colors1(size, 0);  
        colors1[a] = 1;  
        colors1[b] = 2; 

        std::vector<int> colors2(size, 0);  
        colors2[a] = 2;  
        colors2[b] = 1;  

        bool is_color_symmetric = false;
        std::vector<int> perm(size);
        for(int i = 0; i < size; ++i) perm[i] = i;

        do {
            bool matches = true;
            for(int i = 0; i < size; ++i) {
                if(colors1[i] != colors2[perm[i]]) {
                    matches = false;
                    break;
                }
            }

            if(matches) {
                const int* adj = mappingses[i].get_adj_mat_ptr();
                for(int x = 0; x < size && matches; ++x) {
                    for(int y = 0; y < size && matches; ++y) {
                        if(adj[INDEX(x, y, size)] != adj[INDEX(perm[x], perm[y], size)]) {
                            matches = false;
                        }
                    }
                }
            }
            if(matches) {
                is_color_symmetric = true;
                break;
            }
        } while(std::next_permutation(perm.begin(), perm.end()));

        // std::cout << "\nMapping #" << i + 1 << ":\n";
        // if (is_color_symmetric) {
        //     std::cout << "Vertex pair: (" << a << "," << b << ")\n";
        // } else {
        //     std::cout << "Vertex pair: (" << a << "," << b << "), (" << b << "," << a << ")\n";
        // }

        // const int* curr_adj_mat = mappingses[i].get_adj_mat_ptr();
        // for(int x = 0; x < size; ++x) {
        //     for(int y = 0; y < size; ++y) {
        //         std::cout << curr_adj_mat[INDEX(x, y, size)] << " ";
        //     }
        //     std::cout << "\n";
        // }

        idx++;
    }


    std::vector<Schedule> schedules;

    for(size_t i = 0; i < mappingses.size(); ++i) {
        if (!is_unique[i]) continue;
        Schedule sche(mappingses[i].get_adj_mat_ptr(), size);
        int *reorder = new int[size * size];
        sche.generate_schedules(reorder);

        generate_permutations(reorder, size);

        Schedule nsche(reorder, size);
        // nsche.print_schedule();
        schedules.push_back(nsche);

    }

    //
    std::vector<DAG> all_dags;  
    auto combined_dag = DAG::DAG_combination(all_dags); 

    Mining mining(graphfile, 
                     udpatefile, combined_dag);
    
    if (mining.initialize()) {
        std::cout << "\nStarting mining process...\n";
        mining.run();
    }

}



int main(int argc,char *argv[]) {

    if(argc < 5) {
        printf("Usage: %s dataset_name graph_file pattern_size pattern_adjacency_matrix\n", argv[0]);
        printf("Example(Triangle counting on dataset WikiVote) : \n");
        printf("%s dataset/example.txt dataset/updates.txt 5 0111010011100011100001100\n", argv[0]);
        return 0;
    }


    const std::string type = argv[1];
    const std::string path = argv[2];

    int size = atoi(argv[3]);
    char* adj_mat = argv[4];

    Pattern p(size, adj_mat);

    auto start = std::chrono::high_resolution_clock::now();
    test_pattern(type, path, p);
    auto end = std::chrono::high_resolution_clock::now();

    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "\nExecution Time: " << duration.count() << " us" << std::endl;

}