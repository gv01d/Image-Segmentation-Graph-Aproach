#ifndef DISJOINT_H
#define DISJOINT_H

#include <vector>

class Disjoint {
public:
    std::vector<int> component_size; // Stores the number of elements (pixels) in a component.
    std::vector<double> max_internal_edge; // Stores the largest edge weight within a connected component (internal threshold).
    std::vector<int> parent; // Stores the parent of element i

    Disjoint(int element_count);// Disjoint constructor

    int find_set_root(int idx); //Finds the root of idx.

    bool unite_sets(int idx1, int idx2, double edge_weight); //joins the sets with idx1 and idx2
};

#endif