#include "aho_corasick.h"
#include "pattern_verify.h"
#include "preprocessing.h"
#include "route_calc.h"
#include "vehicle_dispatch.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace std;

// Disease Database
map<string, string> diseaseDatabase = {
    {"covid", "Protocol: Respiratory Isolation Required"},
    {"malaria", "Protocol: Anti-malarial & Blood Test Kits"},
    {"cholera", "Protocol: Dehydration Treatment & Saline"},
    {"ebola", "Protocol: Full Biohazard Suit Required"},
    {"pneumonia", "Protocol: Oxygen Support Required"}
};

vector<string> emergencyKeywords = {"fire", "accident", "crash", "flood", "medical", "ambulance", "hurt"};

void showNodeMenu() {
    cout << "\n--- CITY LOCATIONS ---" << endl;
    cout << "0: FireStation | 1: Market     | 2: School" << endl;
    cout << "3: Hospital    | 4: CityCenter | 5: Park" << endl;
    cout << "6: Factory     | 7: Bridge     | 8: Airport" << endl;
}

int main() {
    RoadGraph g = buildCityGraph();
    vector<string> allTerms = emergencyKeywords;
    for(auto const& pair : diseaseDatabase) allTerms.push_back(pair.first);
    AhoCorasick ac(allTerms);
    
    // init vehicles
    vector<Vehicle> fleet = {
        Vehicle(1, "Ambulance", "AMB-001", "Kartik Kumar", "83181xxxxx", 37.7750, -122.4250),
        Vehicle(2, "Ambulance", "AMB-002", "Manasvi Bhasin", "96215xxxxx", 37.7810, -122.4150),
        Vehicle(3, "Ambulance", "AMB-003", "Shivansh Tripathi", "82145xxxxx", 37.7650, -122.4200),
        Vehicle(4, "Ambulance", "AMB-004", "Anshika Lakhera", "63881xxxxx", 37.7720, -122.4180),
        Vehicle(5, "Ambulance", "AMB-005", "Madhur Singh", "99721xxxxx", 37.7680, -122.4280),
        Vehicle(6, "Ambulance", "AMB-006", "Sakiya Tripathi", "78923xxxxx", 37.7790, -122.4080)
    };

    int choice;
    while (true) {
        cout << "\n--- menu ---" << endl;
        cout << "1. View Capabilities" << endl;
        cout << "2. Report Location Emergency (Dijkstra + Traffic)" << endl;
        cout << "3. Report Medical/Disease Outbreak" << endl;
        cout << "4. View Fleet & Fuel Status" << endl;
        cout << "5. EXIT (Use this before Re-Compiling!)" << endl;
        cout << "Choice: ";
        
        if (!(cin >> choice)) break;
        cin.ignore();
        if (choice == 5) break;

        switch(choice) {
            case 1:
                cout << "\n[INFO] Detecting: fire, crash, flood, covid, malaria, ebola, etc." << endl;
                break;
            case 2: {
                int hour, dest;
                cout << "Enter Hour (0-23): "; cin >> hour;
                showNodeMenu();
                cout << "Target Node ID: "; cin >> dest;
                auto res = g.dijkstra(0, hour);
                g.printPath(g.getPath(res.second, 0, dest), res.first[dest], "Traffic-GPS");
                dispatchVehicle(fleet, Emergency(1, "Location Emergency", "General", g.nodes[dest].lat, g.nodes[dest].lng, 1));
                break;
            }
            case 3: {
                string msg;
                cout << "Enter Details: "; getline(cin, msg);
                set<string> found = ac.findKeywords(preprocessMessage(msg));
                for (auto const& pair : diseaseDatabase) {
                    if (msg.find(pair.first) != string::npos) cout << "[ALERT] " << pair.second << endl;
                }
                dispatchVehicle(fleet, Emergency(2, msg, "Medical", g.nodes[3].lat, g.nodes[3].lng, 1));
                break;
            }
            case 4:
                printFleetStatus(fleet);
                break;
        }
    }
    return 0;
}