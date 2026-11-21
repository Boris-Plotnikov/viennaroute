#include "viennaroute/route_planner.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <stdexcept>
#include <tuple>

namespace {
struct State {
    int station;
    std::string line;
    bool operator<(const State& other) const {
        return std::tie(station, line) < std::tie(other.station, other.line);
    }
};

struct QueueEntry {
    int cost;
    State state;
    bool operator>(const QueueEntry& other) const { return cost > other.cost; }
};

}

RoutePlanner::RoutePlanner(const Network& network, int transfer_minutes)
    : network_(network), transfer_minutes_(transfer_minutes) {
    if (transfer_minutes < 0) throw std::invalid_argument("Transfer time must not be negative");
}

std::optional<Route> RoutePlanner::find(int from, int to) const {
    if (from < 0 || to < 0 || from >= network_.station_count() || to >= network_.station_count()) {
        throw std::out_of_range("Unknown station id");
    }
    if (from == to) return Route{};

    std::map<State, int> best_cost;
    std::map<State, State> came_from;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> queue;

    for (const std::string& line : network_.lines_at(from)) {
        State start{from, line};
        best_cost[start] = 0;
        queue.push({0, start});
    }

    std::optional<State> finish;
    while (!queue.empty()) {
        QueueEntry current = queue.top();
        queue.pop();
        if (current.cost > best_cost[current.state]) continue;
        if (current.state.station == to) {
            finish = current.state;
            break;
        }

        auto relax = [&](const State& next, int cost) {
            auto it = best_cost.find(next);
            if (it == best_cost.end() || cost < it->second) {
                best_cost[next] = cost;
                came_from[next] = current.state;
                queue.push({cost, next});
            }
        };

        for (const Connection& c : network_.connections(current.state.station)) {
            if (c.line == current.state.line) relax(State{c.to, c.line}, current.cost + c.minutes * 10);
        }
        for (const std::string& line : network_.lines_at(current.state.station)) {
            if (line != current.state.line) {
                relax(State{current.state.station, line}, current.cost + transfer_minutes_ * 10 + 1);
            }
        }
    }

    if (!finish) return std::nullopt;

    std::vector<State> path;
    State step = *finish;
    path.push_back(step);
    while (came_from.count(step)) {
        step = came_from.at(step);
        path.push_back(step);
    }
    std::reverse(path.begin(), path.end());

    Route route;
    for (std::size_t i = 0; i < path.size(); ++i) {
        const State& s = path[i];
        if (route.legs.empty() || route.legs.back().line != s.line) {
            if (!route.legs.empty()) route.transfer_minutes += transfer_minutes_;
            route.legs.push_back(Leg{s.line, {s.station}, 0});
            continue;
        }
        Leg& leg = route.legs.back();
        int previous = leg.stations.back();
        for (const Connection& c : network_.connections(previous)) {
            if (c.to == s.station && c.line == s.line) {
                leg.minutes += c.minutes;
                break;
            }
        }
        leg.stations.push_back(s.station);
    }
    while (!route.legs.empty() && route.legs.front().stops() == 0) {
        route.legs.erase(route.legs.begin());
        route.transfer_minutes -= transfer_minutes_;
    }
    for (const Leg& leg : route.legs) route.ride_minutes += leg.minutes;
    return route;
}
