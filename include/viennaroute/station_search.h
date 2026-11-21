#ifndef VIENNAROUTE_STATION_SEARCH_H
#define VIENNAROUTE_STATION_SEARCH_H

#include <string>
#include <vector>

#include "viennaroute/network.h"

struct SearchResult {
    int station = -1;
    std::vector<std::string> suggestions;
};

class StationSearch {
public:
    explicit StationSearch(const Network& network);
    SearchResult find(const std::string& query) const;

private:
    const Network& network_;
    std::vector<std::string> normalized_;
};

#endif
