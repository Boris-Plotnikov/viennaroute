#include "viennaroute/station_search.h"

#include <algorithm>

#include "viennaroute/text.h"

StationSearch::StationSearch(const Network& network) : network_(network) {
    for (int id = 0; id < network.station_count(); ++id) normalized_.push_back(text::normalize(network.station_name(id)));
}

SearchResult StationSearch::find(const std::string& query) const {
    SearchResult result;
    std::string q = text::normalize(query);
    if (q.empty()) return result;

    std::vector<int> prefix_matches;
    std::vector<int> substring_matches;
    for (int id = 0; id < static_cast<int>(normalized_.size()); ++id) {
        const std::string& name = normalized_[id];
        if (name == q) {
            result.station = id;
            return result;
        }
        if (name.compare(0, q.size(), q) == 0) prefix_matches.push_back(id);
        else if (name.find(q) != std::string::npos) substring_matches.push_back(id);
    }

    for (const std::vector<int>* matches : {&prefix_matches, &substring_matches}) {
        if (matches->size() == 1) {
            result.station = matches->front();
            return result;
        }
        if (matches->size() > 1) {
            for (int id : *matches) result.suggestions.push_back(network_.station_name(id));
            std::sort(result.suggestions.begin(), result.suggestions.end());
            return result;
        }
    }

    int best = 1000;
    std::vector<int> closest;
    for (int id = 0; id < static_cast<int>(normalized_.size()); ++id) {
        int distance = text::levenshtein(q, normalized_[id]);
        if (distance < best) {
            best = distance;
            closest = {id};
        } else if (distance == best) {
            closest.push_back(id);
        }
    }
    int allowed = std::max(1, static_cast<int>(q.size()) / 4);
    if (best <= allowed) {
        if (closest.size() == 1) {
            result.station = closest.front();
        } else {
            for (int id : closest) result.suggestions.push_back(network_.station_name(id));
        }
    }
    return result;
}
