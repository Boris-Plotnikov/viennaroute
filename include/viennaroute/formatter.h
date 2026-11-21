#ifndef VIENNAROUTE_FORMATTER_H
#define VIENNAROUTE_FORMATTER_H

#include <string>

#include "viennaroute/network.h"
#include "viennaroute/route_planner.h"

namespace formatter {
std::string to_text(const Route& route, const Network& network);

std::string to_html(const Route& route, const Network& network);

}

#endif
