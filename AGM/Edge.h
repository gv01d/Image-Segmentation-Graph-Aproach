#ifndef EDGE_H
#define EDGE_H

struct Edge {
    int u, v;     // Indices of the two pixels connected by this edge
    double weight; // The weight of the edge (e.g., color difference)
};

#endif // EDGE_H