#include "vehicle_dispatch.h"
#include <iostream>
#include <cmath>

using namespace std;

void dispatchVehicle(vector<Vehicle>& fleet, const Emergency& em) {
    int best = -1; double minDist = 1e18;
    for (int i = 0; i < (int)fleet.size(); i++) {
        if (fleet[i].available) {
            double d = sqrt(pow(fleet[i].lat-em.lat,2)+pow(fleet[i].lng-em.lng,2));
            if (d < minDist) { minDist = d; best = i; }
        }
    }
    if (best != -1) {
        fleet[best].available = false;
        cout << "  [DISPATCH] Emergency #" << em.id << " -> Vehicle #" << fleet[best].id << " (" << fleet[best].type << ")" << endl;
    }
}

void printFleetStatus(const vector<Vehicle>& fleet) {
    cout << "  Fleet Status: ";
    for (const auto& v : fleet) cout << "[" << v.id << ":" << (v.available ? "Ready" : "Busy") << "] ";
    cout << endl;
}