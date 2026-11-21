#ifndef VIENNAROUTE_TEXT_H
#define VIENNAROUTE_TEXT_H

#include <string>
#include <vector>

namespace text {
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& line, char delimiter);

std::string normalize(const std::string& s);

int levenshtein(const std::string& a, const std::string& b);

}

#endif
