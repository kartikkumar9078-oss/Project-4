#include "route_calc.h"
#include <queue>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

RoadGraph::RoadGraph(int n) : numNodes(n), nodes(n), adj(n) {}
void RoadGraph::addEdge(int u, int v, double w) { adj[u].push_back({v,w}); adj[v].push_back({u,w}); }

double RoadGraph::getTrafficFactor(int hour) {
    if ((hour >= 8 && hour <= 10) || (hour >= 17 && hour <= 19)) {
        cout << "  [TRAFFIC] RUSH HOUR! (3x Delay Applied)" << endl;
        return 3.0;
    }
    if (hour >= 22 || hour <= 5) {
        cout << "  [TRAFFIC] NIGHT: Roads Clear (0.8x Speedup)" << endl;
        return 0.8;
    }
    return 1.0;
}

pair<vector<double>, vector<int>> RoadGraph::dijkstra(int src, int hour) {
    double factor = getTrafficFactor(hour);
    vector<double> d(numNodes, ROUTE_INF);
    vector<int> p(numNodes, -1);
    priority_queue<pair<double,int>, vector<pair<double,int>>, greater<pair<double,int>>> pq;
    d[src] = 0; pq.push({0, src});

    while(!pq.empty()){
        int u = pq.top().second; pq.pop();
        for(auto& e : adj[u]) {
            double travelTime = e.weight * factor;
            if(d[u] + travelTime < d[e.to]) {
                d[e.to] = d[u] + travelTime;
                p[e.to] = u;
                pq.push({d[e.to], e.to});
            }
        }
    }
    return {d, p};
}

vector<int> RoadGraph::getPath(vector<int>& p, int s, int d) {
    vector<int> path; if(p[d]==-1 && s!=d) return path;
    for(int c=d; c!=-1; c=p[c]) path.push_back(c);
    reverse(path.begin(), path.end()); return path;
}

void RoadGraph::printPath(const vector<int>& p, double c, const string& a) {
    cout << "  [" << a << "] Path: ";
    for(int i=0; i<(int)p.size(); i++) cout << nodes[p[i]].name << (i+1<(int)p.size() ? " -> ":"");
    cout << " | Travel Time: " << fixed << setprecision(2) << c << " mins" << endl;
}

RoadGraph buildCityGraph() {
    RoadGraph g(9);
    g.nodes = {
        {"FireStation", 37.7800, -122.4300},{"Market", 37.7800, -122.4200},{"School", 37.7800, -122.4100},
        {"Hospital", 37.7700, -122.4300},{"CityCenter", 37.7700, -122.4200},{"Park", 37.7700, -122.4100},
        {"Factory", 37.7600, -122.4300},{"Bridge", 37.7600, -122.4200},{"Airport", 37.7600, -122.4100}
    };
    g.addEdge(0,1,2); g.addEdge(1,2,2); g.addEdge(3,4,2); g.addEdge(4,5,2); g.addEdge(6,7,2); g.addEdge(7,8,2);
    g.addEdge(0,3,2); g.addEdge(1,4,2); g.addEdge(2,5,2); g.addEdge(3,6,2); g.addEdge(4,7,2); g.addEdge(5,8,2);
    return g;
}