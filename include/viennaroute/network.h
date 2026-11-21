#ifndef VIENNAROUTE_NETWORK_H
#define VIENNAROUTE_NETWORK_H

#include <map>
#include <string>
#include <vector>

struct Connection {
    int to = 0;
    std::string line;
    int minutes = 0;
};

struct Line {
    std::string name;
    std::string color;
};

class Network {
public:
    static Network load(const std::string& data_dir);

    void add_line(const std::string& name, const std::string& color);
    void add_segment(const std::string& line, const std::string& from, const std::string& to, int minutes);

    int station_count() const { return static_cast<int>(names_.size()); }
    const std::string& station_name(int id) const { return names_.at(id); }
    int station_id(const std::string& exact_name) const;
    const std::vector<Connection>& connections(int id) const { return adjacency_.at(id); }
    std::vector<std::string> lines_at(int id) const;

    const std::map<std::string, Line>& lines() const { return lines_; }
    std::string line_color(const std::string& line) const;

private:
    int get_or_create(const std::string& name);

    std::vector<std::string> names_;
    std::map<std::string, int> ids_;
    std::vector<std::vector<Connection>> adjacency_;
    std::map<std::string, Line> lines_;
};

#endif
