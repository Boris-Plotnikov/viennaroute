#include "viennaroute/network.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <stdexcept>

#include "viennaroute/text.h"

namespace {
std::ifstream open_csv(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open " + path);
    return file;
}

}

Network Network::load(const std::string& data_dir) {
    Network network;

    std::string lines_path = data_dir + "/lines.csv";
    std::ifstream lines_file = open_csv(lines_path);
    std::string row;
    int line_number = 0;
    while (std::getline(lines_file, row)) {
        ++line_number;
        if (line_number == 1 || text::trim(row).empty()) continue;
        std::vector<std::string> parts = text::split(row, ',');
        if (parts.size() < 2) throw std::runtime_error(lines_path + ", line " + std::to_string(line_number));
        network.add_line(parts[0], parts[1]);
    }

    std::string segments_path = data_dir + "/segments.csv";
    std::ifstream segments_file = open_csv(segments_path);
    line_number = 0;
    while (std::getline(segments_file, row)) {
        ++line_number;
        if (line_number == 1 || text::trim(row).empty()) continue;
        std::vector<std::string> parts = text::split(row, ',');
        std::string where = segments_path + ", line " + std::to_string(line_number) + ": ";
        if (parts.size() != 4) throw std::runtime_error(where + "expected 4 columns");
        int minutes = 0;
        try {
            minutes = std::stoi(parts[3]);
        } catch (const std::exception&) {
            throw std::runtime_error(where + "minutes is not a number");
        }
        if (network.lines_.count(parts[0]) == 0) throw std::runtime_error(where + "unknown line " + parts[0]);
        network.add_segment(parts[0], parts[1], parts[2], minutes);
    }
    return network;
}

void Network::add_line(const std::string& name, const std::string& color) { lines_[name] = Line{name, color}; }

void Network::add_segment(const std::string& line, const std::string& from, const std::string& to, int minutes) {
    if (minutes <= 0) throw std::invalid_argument("Travel time must be positive: " + from + " - " + to);
    if (from == to) throw std::invalid_argument("A segment needs two different stations: " + from);
    int a = get_or_create(from);
    int b = get_or_create(to);
    adjacency_[a].push_back(Connection{b, line, minutes});
    adjacency_[b].push_back(Connection{a, line, minutes});
}

int Network::get_or_create(const std::string& name) {
    auto it = ids_.find(name);
    if (it != ids_.end()) return it->second;
    int id = static_cast<int>(names_.size());
    names_.push_back(name);
    ids_[name] = id;
    adjacency_.emplace_back();
    return id;
}

int Network::station_id(const std::string& exact_name) const {
    auto it = ids_.find(exact_name);
    return it == ids_.end() ? -1 : it->second;
}

std::vector<std::string> Network::lines_at(int id) const {
    std::set<std::string> unique;
    for (const Connection& c : adjacency_.at(id)) unique.insert(c.line);
    return std::vector<std::string>(unique.begin(), unique.end());
}

std::string Network::line_color(const std::string& line) const {
    auto it = lines_.find(line);
    return it == lines_.end() ? "#666666" : it->second.color;
}
