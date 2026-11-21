#include "viennaroute/text.h"

#include <algorithm>
#include <cctype>

namespace text {
std::string trim(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : line) {
        if (c == delimiter) {
            parts.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(trim(current));
    return parts;
}

std::string normalize(const std::string& s) {
    struct Replacement {
        const char* from;
        const char* to;
    };
    const Replacement replacements[] = {{"ä", "ae"}, {"ö", "oe"}, {"ü", "ue"}, {"Ä", "ae"},
                                        {"Ö", "oe"}, {"Ü", "ue"}, {"ß", "ss"}};
    std::string result;
    std::size_t i = 0;
    while (i < s.size()) {
        bool replaced = false;
        for (const Replacement& r : replacements) {
            std::string from = r.from;
            if (s.compare(i, from.size(), from) == 0) {
                result += r.to;
                i += from.size();
                replaced = true;
                break;
            }
        }
        if (replaced) continue;
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (std::isalnum(c)) result += static_cast<char>(std::tolower(c));
        ++i;
    }
    return result;
}

int levenshtein(const std::string& a, const std::string& b) {
    std::vector<int> previous(b.size() + 1);
    std::vector<int> current(b.size() + 1);
    for (std::size_t j = 0; j <= b.size(); ++j) previous[j] = static_cast<int>(j);
    for (std::size_t i = 1; i <= a.size(); ++i) {
        current[0] = static_cast<int>(i);
        for (std::size_t j = 1; j <= b.size(); ++j) {
            int cost = a[i - 1] == b[j - 1] ? 0 : 1;
            current[j] = std::min({previous[j] + 1, current[j - 1] + 1, previous[j - 1] + cost});
        }
        std::swap(previous, current);
    }
    return previous[b.size()];
}

}
