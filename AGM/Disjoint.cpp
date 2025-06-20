#include "Disjoint.h"

// Constructor: Initializes 'element_count' elements
Disjoint::Disjoint(int element_count) {
    parent.resize(element_count);
    component_size.resize(element_count);
    max_internal_edge.resize(element_count);

    for (int i = 0; i < element_count; i++) {
        parent[i] = i; // Each element is its own parent
        max_internal_edge[i] = 0.0; // internal difference is 0 at the start
        component_size[i] = 1; // Each set initially contains one element
    }
}

//unites the sets containing 'idx1' and 'idx2'
bool Disjoint::unite_sets(int root1, int root2, double edge_weight) {

    // Union by size
    if (component_size[root1] < component_size[root2]) {
        std::swap(root1, root2);
    }

    parent[root2] = root1; // Make root1 the new root
    component_size[root1] += component_size[root2]; // Update the size of the new component
    // The new max_internal_edge is the weight of the edge that caused the merge
    max_internal_edge[root1] = edge_weight;
    return true;// Sets were merged
}

// Finds the root of the set containing 'idx' with path compression.
int Disjoint::find_set_root(int idx) {
    if (parent[idx] == idx) { // If 'idx' is its own parent
        return idx;
    }

    return parent[idx] = find_set_root(parent[idx]);
}