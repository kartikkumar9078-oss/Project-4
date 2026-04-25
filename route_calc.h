#ifndef ROUTE_CALC_H
#define ROUTE_CALC_H
#include <string>
#include <vector>

const double ROUTE_INF = 1e18;

struct RoadNode { std::string name; double lat, lng; };
struct RoadEdge { int to; double weight; };

class RoadGraph {
public:
    int numNodes;
    std::vector<RoadNode> nodes;
    std::vector<std::vector<RoadEdge>> adj;
    RoadGraph(int n);
    void addEdge(int u, int v, double weight);
    double getTrafficFactor(int hour);
    std::pair<std::vector<double>, std::vector<int>> dijkstra(int src, int hour);
    std::vector<int> getPath(std::vector<int>& parent, int src, int dst);
    void printPath(const std::vector<int>& path, double cost, const std::string& algo);
};

RoadGraph buildCityGraph();
#endif