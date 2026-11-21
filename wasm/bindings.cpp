#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <optional>
#include <string>
#include <vector>

#include "viennaroute/formatter.h"
#include "viennaroute/network.h"
#include "viennaroute/route_planner.h"
#include "viennaroute/station_search.h"

using emscripten::val;

namespace {
val to_array(const std::vector<std::string>& items) {
    val result = val::array();
    for (const std::string& s : items) result.call<void>("push", s);
    return result;
}
}

class Planner {
public:
    Planner() : network_(Network::load("/data")), search_(network_) {}

    val stations() const {
        val result = val::array();
        for (int id = 0; id < network_.station_count(); ++id) {
            val row = val::object();
            row.set("name", network_.station_name(id));
            row.set("lines", to_array(network_.lines_at(id)));
            result.call<void>("push", row);
        }
        return result;
    }

    val lines() const {
        val result = val::array();
        for (const auto& pair : network_.lines()) {
            val row = val::object();
            row.set("name", pair.first);
            row.set("color", pair.second.color);
            result.call<void>("push", row);
        }
        return result;
    }

    val search(const std::string& query) const {
        SearchResult found = search_.find(query);
        val result = val::object();
        result.set("query", query);
        result.set("station", found.station >= 0 ? val(network_.station_name(found.station)) : val::null());
        result.set("suggestions", to_array(found.suggestions));
        return result;
    }

    val route(const std::string& from_query, const std::string& to_query, int transfer_minutes) const {
        val result = val::object();
        SearchResult from = search_.find(from_query);
        SearchResult to = search_.find(to_query);
        result.set("from", search(from_query));
        result.set("to", search(to_query));
        if (from.station < 0 || to.station < 0) return result;

        RoutePlanner planner(network_, transfer_minutes);
        std::optional<Route> route = planner.find(from.station, to.station);
        if (!route) return result;

        val legs = val::array();
        for (const Leg& leg : route->legs) {
            std::vector<std::string> names;
            for (int id : leg.stations) names.push_back(network_.station_name(id));
            val row = val::object();
            row.set("line", leg.line);
            row.set("color", network_.line_color(leg.line));
            row.set("minutes", leg.minutes);
            row.set("stations", to_array(names));
            legs.call<void>("push", row);
        }
        val found = val::object();
        found.set("legs", legs);
        found.set("rideMinutes", route->ride_minutes);
        found.set("transferMinutes", route->transfer_minutes);
        found.set("totalMinutes", route->total_minutes());
        found.set("transfers", route->transfers());
        found.set("text", formatter::to_text(*route, network_));
        found.set("html", formatter::to_html(*route, network_));
        result.set("route", found);
        return result;
    }

private:
    Network network_;
    StationSearch search_;
};

EMSCRIPTEN_BINDINGS(viennaroute) {
    emscripten::class_<Planner>("Planner")
        .constructor<>()
        .function("stations", &Planner::stations)
        .function("lines", &Planner::lines)
        .function("search", &Planner::search)
        .function("route", &Planner::route);
}
