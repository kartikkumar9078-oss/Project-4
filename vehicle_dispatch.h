#ifndef VEHICLE_DISPATCH_H
#define VEHICLE_DISPATCH_H
#include <string>
#include <vector>

struct Vehicle { int id; std::string type; std::string vehicleNumber; std::string driverName; std::string phoneNumber; double lat, lng; bool available; Vehicle(int i, std::string t, std::string vn, std::string dn, std::string pn, double _lat, double _lng) : id(i), type(t), vehicleNumber(vn), driverName(dn), phoneNumber(pn), lat(_lat), lng(_lng), available(true) {} };
struct Emergency { int id; std::string description, type; double lat, lng; int priority; Emergency(int i, std::string d, std::string t, double _lat, double _lng, int p) : id(i), description(d), type(t), lat(_lat), lng(_lng), priority(p) {} };

void dispatchVehicle(std::vector<Vehicle>& fleet, const Emergency& em);
void printFleetStatus(const std::vector<Vehicle>& fleet);
#endif