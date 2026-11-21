#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "viennaroute/formatter.h"
#include "viennaroute/network.h"
#include "viennaroute/route_planner.h"
#include "viennaroute/station_search.h"
#include "viennaroute/text.h"

#ifndef VIENNAROUTE_DATA_DIR
#define VIENNAROUTE_DATA_DIR "data"
#endif

namespace {
void print_usage() {
    std::cout << "ViennaRoute - fastest way through the Vienna U-Bahn\n\n"
                 "  viennaroute route <from> <to> [--transfer N] [--html file.html]\n"
                 "  viennaroute stations [filter]\n"
                 "  viennaroute lines\n"
                 "  viennaroute                      (interactive mode)\n\n"
                 "Station names may be short or have typos: viennaroute route karlspl \"stefansplatz\"\n"
                 "Options: --transfer N  minutes for one change of line (default 4)\n"
                 "         --data DIR    folder with lines.csv and segments.csv\n";
}

int resolve(const StationSearch& search, const Network& network, const std::string& query) {
    SearchResult result = search.find(query);
    if (result.station >= 0) return result.station;
    if (result.suggestions.empty()) {
        std::cout << "No station found for '" << query << "'. Try: viennaroute stations " << query.substr(0, 3)
                  << '\n';
    } else {
        std::cout << "'" << query << "' is ambiguous. Did you mean:\n";
        for (const std::string& s : result.suggestions) std::cout << "  " << s << '\n';
    }
    (void)network;
    return -1;
}

int run_route(const Network& network, const std::string& from_query, const std::string& to_query, int transfer,
              const std::string& html_path) {
    StationSearch search(network);
    int from = resolve(search, network, from_query);
    int to = resolve(search, network, to_query);
    if (from < 0 || to < 0) return 2;

    RoutePlanner planner(network, transfer);
    std::optional<Route> route = planner.find(from, to);
    if (!route) {
        std::cout << "No connection between these stations.\n";
        return 3;
    }
    std::cout << formatter::to_text(*route, network);
    if (!html_path.empty()) {
        std::ofstream file(html_path);
        if (!file) {
            std::cerr << "Cannot write " << html_path << '\n';
            return 1;
        }
        file << formatter::to_html(*route, network);
        std::cout << "Saved diagram to " << html_path << '\n';
    }
    return 0;
}

void list_stations(const Network& network, const std::string& filter) {
    std::string f = text::normalize(filter);
    std::vector<std::string> rows;
    for (int id = 0; id < network.station_count(); ++id) {
        if (!f.empty() && text::normalize(network.station_name(id)).find(f) == std::string::npos) continue;
        std::string row = network.station_name(id) + "  [";
        std::vector<std::string> lines = network.lines_at(id);
        for (std::size_t i = 0; i < lines.size(); ++i) row += (i ? " " : "") + lines[i];
        rows.push_back(row + "]");
    }
    std::sort(rows.begin(), rows.end());
    for (const std::string& row : rows) std::cout << row << '\n';
    std::cout << rows.size() << " stations\n";
}

void interactive(const Network& network, int transfer) {
    std::cout << "ViennaRoute - " << network.station_count() << " stations, " << network.lines().size()
              << " lines. Empty line to exit.\n";
    while (true) {
        std::string from;
        std::string to;
        std::cout << "\nFrom: " << std::flush;
        if (!std::getline(std::cin, from) || text::trim(from).empty()) break;
        std::cout << "To:   " << std::flush;
        if (!std::getline(std::cin, to) || text::trim(to).empty()) break;
        std::cout << '\n';
        run_route(network, from, to, transfer, "");
    }
}

}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string data_dir = VIENNAROUTE_DATA_DIR;
    std::string html_path;
    int transfer = 4;
    std::vector<std::string> positional;
    try {
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--transfer" && i + 1 < args.size()) transfer = std::stoi(args[++i]);
            else if (args[i] == "--html" && i + 1 < args.size()) html_path = args[++i];
            else if (args[i] == "--data" && i + 1 < args.size()) data_dir = args[++i];
            else if (args[i] == "--help" || args[i] == "-h") {
                print_usage();
                return 0;
            } else positional.push_back(args[i]);
        }
    } catch (const std::exception&) {
        std::cerr << "--transfer needs a number of minutes\n";
        return 1;
    }

    Network network;
    try {
        network = Network::load(data_dir);
    } catch (const std::exception& e) {
        std::cerr << "Cannot load network: " << e.what() << '\n';
        return 1;
    }

    if (positional.empty()) {
        interactive(network, transfer);
        return 0;
    }
    const std::string& command = positional[0];
    if (command == "route" && positional.size() == 3) {
        return run_route(network, positional[1], positional[2], transfer, html_path);
    }
    if (command == "stations") {
        list_stations(network, positional.size() > 1 ? positional[1] : "");
        return 0;
    }
    if (command == "lines") {
        for (const auto& pair : network.lines()) std::cout << pair.first << "  " << pair.second.color << '\n';
        return 0;
    }
    print_usage();
    return 1;
}
