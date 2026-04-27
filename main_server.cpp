// 
//  SERS - Smart Emergency Response System
//  HTTP Server + UI Integration
//  Build: g++ -std=c++17 -o sers_server main_server.cpp
//         aho_corasick.cpp preprocessing.cpp pattern_verify.cpp
//         route_calc.cpp vehicle_dispatch.cpp -lws2_32  (Windows)
//  OR:    g++ -std=c++17 -o sers_server main_server.cpp
//         aho_corasick.cpp preprocessing.cpp pattern_verify.cpp
//         route_calc.cpp vehicle_dispatch.cpp             (Linux/Mac)
// 

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cmath>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define closesocket close
#endif

#include "aho_corasick.h"
#include "preprocessing.h"
#include "pattern_verify.h"
#include "route_calc.h"
#include "vehicle_dispatch.h"

using namespace std;

// Global State
static int emergencyCounter = 1;

struct LogEntry {
    int id;
    string timestamp;
    string message;
    string type;
    string keywords;
    string vehicle;
    string path;
    double travelTime;
    int priority;
};

vector<LogEntry> emergencyLog;
vector<Vehicle> fleet = {
    Vehicle(1, "Ambulance", "AMB-001", "Kartik Kumar", "83181xxxxx", 37.7750, -122.4250),
    Vehicle(2, "Ambulance", "AMB-002", "Manasvi Bhasin", "96215xxxxx", 37.7810, -122.4150),
    Vehicle(3, "Ambulance", "AMB-003", "Shivansh Tripathi", "82145xxxxx", 37.7650, -122.4200),
    Vehicle(4, "Ambulance", "AMB-004", "Anshika Lakhera", "63881xxxxx", 37.7720, -122.4180),
    Vehicle(5, "Ambulance", "AMB-005", "Madhur Singh", "99721xxxxx", 37.7680, -122.4280),
    Vehicle(6, "Ambulance", "AMB-006", "Sakiya Tripathi", "78923xxxxx", 37.7790, -122.4080)
};

RoadGraph cityGraph = buildCityGraph();

// Emergency keywords mapped to type
map<string,string> keywordTypes = {
    {"fire","Fire"},{"burning","Fire"},{"smoke","Fire"},{"flames","Fire"},
    {"accident","Medical"},{"injured","Medical"},{"hurt","Medical"},{"ambulance","Medical"},{"heart","Medical"},
    {"robbery","Police"},{"theft","Police"},{"crime","Police"},{"assault","Police"},{"gun","Police"},
    {"flood","Disaster"},{"earthquake","Disaster"},{"disaster","Disaster"},{"collapse","Disaster"},
};

vector<string> allKeywords = {
    "fire","burning","smoke","flames",
    "accident","injured","hurt","ambulance","heart",
    "robbery","theft","crime","assault","gun",
    "flood","earthquake","disaster","collapse"
};

// Utility
string getCurrentTimestamp() {
    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

int getCurrentHour() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    return t->tm_hour;
}

string escapeJson(const string& s) {
    string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

// Core Logic
string processEmergency(const string& rawMsg, int srcNode, int hourOverride = -1) {
    // Preprocess
    string cleaned = preprocessMessage(rawMsg);

    // Aho-Corasick keyword detection
    AhoCorasick ac(allKeywords);
    set<string> found = ac.findKeywords(cleaned);

    // Determine type & priority
    string emType = "Unknown";
    int priority = 3;
    for (auto& kw : found) {
        if (keywordTypes.count(kw)) {
            string t = keywordTypes[kw];
            if (t == "Fire")    { emType = t; priority = 1; break; }
            if (t == "Medical") { emType = t; priority = min(priority,2); }
            if (t == "Police")  { emType = t; priority = min(priority,2); }
            if (t == "Disaster"){ emType = t; priority = min(priority,1); }
        }
    }

    // Build keyword string
    string kwStr;
    for (auto& k : found) kwStr += (kwStr.empty()?"":", ") + k;

    // Route calc
    int hour = (hourOverride == -1) ? getCurrentHour() : hourOverride;
    pair<vector<double>, vector<int>> dijkResult = cityGraph.dijkstra(srcNode, hour);
    vector<double>& dists   = dijkResult.first;
    vector<int>&    parents = dijkResult.second;

    // Find dispatch station node (FireStation=0, Hospital=3, FireStation=0 for police)
    int destNode = 0;
    if (emType == "Medical") destNode = 3; // Hospital
    if (emType == "Police")  destNode = 4; // CityCenter

    vector<int> pathNodes = cityGraph.getPath(parents, srcNode, destNode);
    string pathStr;
    for (int i = 0; i < (int)pathNodes.size(); i++)
        pathStr += cityGraph.nodes[pathNodes[i]].name + (i+1<(int)pathNodes.size()?" -> ":"");

    double travelTime = dists[destNode];

    // dispatch unit
    Emergency em(emergencyCounter, rawMsg, emType, 
                 cityGraph.nodes[srcNode].lat, cityGraph.nodes[srcNode].lng, priority);
    
    int bestVehicle = -1; double minDist = 1e18;
    for (int i=0; i<(int)fleet.size(); i++) {
        if (fleet[i].available) {
            double d = sqrt(pow(fleet[i].lat-em.lat,2)+pow(fleet[i].lng-em.lng,2));
            if (d < minDist) { minDist = d; bestVehicle = i; }
        }
    }
    string vehicleStr = "None Available";
    if (bestVehicle != -1) {
        fleet[bestVehicle].available = false;
        vehicleStr = "#" + to_string(fleet[bestVehicle].id) + " " + fleet[bestVehicle].type;
    }

    // Log
    LogEntry entry;
    entry.id = emergencyCounter++;
    entry.timestamp = getCurrentTimestamp();
    entry.message = rawMsg;
    entry.type = emType;
    entry.keywords = kwStr;
    entry.vehicle = vehicleStr;
    entry.path = pathStr.empty() ? "No route found" : pathStr;
    entry.travelTime = travelTime > 1e17 ? -1 : travelTime;
    entry.priority = priority;
    emergencyLog.push_back(entry);

    // build json
    ostringstream json;
    json << "{"
         << "\"id\":" << entry.id << ","
         << "\"timestamp\":\"" << escapeJson(entry.timestamp) << "\","
         << "\"message\":\"" << escapeJson(rawMsg) << "\","
         << "\"cleaned\":\"" << escapeJson(cleaned) << "\","
         << "\"type\":\"" << escapeJson(emType) << "\","
         << "\"keywords\":\"" << escapeJson(kwStr) << "\","
         << "\"vehicle\":\"" << escapeJson(vehicleStr) << "\","
         << "\"path\":\"" << escapeJson(entry.path) << "\","
         << "\"travelTime\":" << (entry.travelTime < 0 ? -1 : entry.travelTime) << ","
         << "\"priority\":" << priority << ","
         << "\"hour\":" << hour
         << "}";
    return json.str();
}

string getFleetJson() {
    ostringstream j;
    j << "[";
    for (int i=0; i<(int)fleet.size(); i++) {
        j << "{\"id\":" << fleet[i].id
          << ",\"type\":\"" << fleet[i].type << "\""
          << ",\"vehicleNumber\":\"" << escapeJson(fleet[i].vehicleNumber) << "\""
          << ",\"driverName\":\"" << escapeJson(fleet[i].driverName) << "\""
          << ",\"phoneNumber\":\"" << escapeJson(fleet[i].phoneNumber) << "\""
          << ",\"available\":" << (fleet[i].available?"true":"false") << "}";
        if (i+1<(int)fleet.size()) j << ",";
    }
    j << "]";
    return j.str();
}

string getLogJson() {
    ostringstream j;
    j << "[";
    for (int i=0; i<(int)emergencyLog.size(); i++) {
        auto& e = emergencyLog[i];
        j << "{\"id\":" << e.id
          << ",\"timestamp\":\"" << escapeJson(e.timestamp) << "\""
          << ",\"message\":\"" << escapeJson(e.message) << "\""
          << ",\"type\":\"" << escapeJson(e.type) << "\""
          << ",\"keywords\":\"" << escapeJson(e.keywords) << "\""
          << ",\"vehicle\":\"" << escapeJson(e.vehicle) << "\""
          << ",\"path\":\"" << escapeJson(e.path) << "\""
          << ",\"travelTime\":" << e.travelTime
          << ",\"priority\":" << e.priority << "}";
        if (i+1<(int)emergencyLog.size()) j << ",";
    }
    j << "]";
    return j.str();
}

string resetFleet() {
    for (auto& v : fleet) v.available = true;
    return "{\"status\":\"Fleet reset\"}";
}

// HTTP Server
string readBody(const string& req) {
    size_t pos = req.find("\r\n\r\n");
    if (pos == string::npos) return "";
    return req.substr(pos + 4);
}

string getParam(const string& body, const string& key) {
    // simple JSON value extractor for string fields: "key":"value"
    string search = "\"" + key + "\":\"";
    size_t start = body.find(search);
    if (start == string::npos) {
        // try numeric
        string search2 = "\"" + key + "\":";
        start = body.find(search2);
        if (start == string::npos) return "";
        start += search2.size();
        size_t end = body.find_first_of(",}", start);
        return body.substr(start, end - start);
    }
    start += search.size();
    size_t end = body.find("\"", start);
    return body.substr(start, end - start);
}

string unescape(const string& s) {
    string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i]=='\\' && i+1<s.size()) {
            if (s[i+1]=='"') { out+='"'; i++; }
            else if (s[i+1]=='n') { out+='\n'; i++; }
            else out += s[i];
        } else out += s[i];
    }
    return out;
}

void handleClient(SOCKET clientSock) {
    char buf[8192] = {};
    recv(clientSock, buf, sizeof(buf)-1, 0);
    string request(buf);

    string method, path;
    istringstream ss(request);
    ss >> method >> path;

    string responseBody, contentType = "application/json";
    int status = 200;

    if (path == "/" || path == "/index.html") {
        // Serve the UI HTML file
        ifstream f("ui.html");
        if (f) {
            responseBody = string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
            contentType = "text/html";
        } else {
            responseBody = "<h1>ui.html not found. Place ui.html next to the server binary.</h1>";
            contentType = "text/html";
            status = 404;
        }
    }
    else if (method == "POST" && path == "/api/dispatch") {
        string body = readBody(request);
        string msg = unescape(getParam(body, "message"));
        string nodeStr = getParam(body, "node");
        string hourStr = getParam(body, "hour");
        int node = nodeStr.empty() ? 0 : stoi(nodeStr);
        int hour = hourStr.empty() ? -1 : stoi(hourStr);
        if (msg.empty()) {
            responseBody = "{\"error\":\"Empty message\"}";
            status = 400;
        } else {
            responseBody = processEmergency(msg, node, hour);
        }
    }
    else if (method == "GET" && path == "/api/fleet") {
        responseBody = getFleetJson();
    }
    else if (method == "GET" && path == "/api/log") {
        responseBody = getLogJson();
    }
    else if (method == "POST" && path == "/api/reset") {
        responseBody = resetFleet();
    }
    else if (method == "GET" && path == "/api/nodes") {
        ostringstream j;
        j << "[";
        for (int i=0; i<(int)cityGraph.nodes.size(); i++) {
            j << "{\"id\":" << i << ",\"name\":\"" << cityGraph.nodes[i].name << "\"}";
            if (i+1<(int)cityGraph.nodes.size()) j << ",";
        }
        j << "]";
        responseBody = j.str();
    }
    else {
        responseBody = "{\"error\":\"Not found\"}";
        status = 404;
    }

    ostringstream resp;
    resp << "HTTP/1.1 " << status << " OK\r\n"
         << "Content-Type: " << contentType << "; charset=utf-8\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
         << "Access-Control-Allow-Headers: Content-Type\r\n"
         << "Content-Length: " << responseBody.size() << "\r\n"
         << "Connection: close\r\n\r\n"
         << responseBody;

    string respStr = resp.str();
    send(clientSock, respStr.c_str(), (int)respStr.size(), 0);
    closesocket(clientSock);
}

int main() {
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    const int PORT = 8080;
    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        cerr << "Failed to create socket\n"; return 1;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(serverSock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Bind failed on port " << PORT << "\n"; return 1;
    }
    listen(serverSock, 10);

    cout << "\n========================================\n";
    cout << "  SERS - Smart Emergency Response System\n";
    cout << "========================================\n";
    cout << "  Server running at: http://localhost:" << PORT << "\n";
    cout << "  Open your browser and go to the URL above.\n";
    cout << "  Press Ctrl+C to stop.\n";
    cout << "========================================\n\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientLen);
        if (clientSock == INVALID_SOCKET) continue;
        handleClient(clientSock);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}