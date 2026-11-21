#ifndef VIENNAROUTE_ROUTE_PLANNER_H
#define VIENNAROUTE_ROUTE_PLANNER_H

#include <optional>
#include <string>
#include <vector>

#include "viennaroute/network.h"

struct Leg {
    std::string line;
    std::vector<int> stations;
    int minutes = 0;
    int stops() const { return static_cast<int>(stations.size()) - 1; }
};

struct Route {
    std::vector<Leg> legs;
    int ride_minutes = 0;
    int transfer_minutes = 0;
    int total_minutes() const { return ride_minutes + transfer_minutes; }
    int transfers() const { return legs.empty() ? 0 : static_cast<int>(legs.size()) - 1; }
};

class RoutePlanner {
public:
    RoutePlanner(const Network& network, int transfer_minutes);
    std::optional<Route> find(int from, int to) const;

private:
    const Network& network_;
    int transfer_minutes_;
};

#endif
