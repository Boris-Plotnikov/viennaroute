#include "viennaroute/formatter.h"

#include <sstream>

namespace {
std::string escape_html(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out += c;
        }
    }
    return out;
}

std::string plural(int n, const std::string& word) { return std::to_string(n) + " " + word + (n == 1 ? "" : "s"); }

}

namespace formatter {
std::string to_text(const Route& route, const Network& network) {
    if (route.legs.empty()) return "You are already there.\n";
    std::ostringstream out;
    const std::string& start = network.station_name(route.legs.front().stations.front());
    const std::string& end = network.station_name(route.legs.back().stations.back());
    out << start << " -> " << end << ": " << route.total_minutes() << " min, "
        << plural(route.transfers(), "transfer") << "\n\n";
    for (std::size_t i = 0; i < route.legs.size(); ++i) {
        const Leg& leg = route.legs[i];
        if (i > 0) out << "   change at " << network.station_name(leg.stations.front()) << '\n';
        out << "  " << leg.line << "  " << network.station_name(leg.stations.front()) << " -> "
            << network.station_name(leg.stations.back()) << "  (" << plural(leg.stops(), "stop") << ", "
            << leg.minutes << " min)\n";
    }
    out << "\nRiding " << route.ride_minutes << " min + changing " << route.transfer_minutes << " min.\n";
    return out.str();
}

std::string to_html(const Route& route, const Network& network) {
    std::ostringstream out;
    out << "<!doctype html>\n<html lang=\"en\"><head><meta charset=\"utf-8\">"
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
           "<title>ViennaRoute</title><style>"
           "body{font-family:system-ui,sans-serif;margin:24px;color:#1b1b1b;background:#fafafa}"
           "h1{font-size:20px;margin:0 0 4px}.sum{color:#555;margin-bottom:20px}"
           ".leg{display:flex;gap:14px}.bar{width:10px;border-radius:5px;margin:6px 0}"
           ".st{padding:3px 0;font-size:15px}.st.end{font-weight:700}"
           ".badge{display:inline-block;color:#fff;border-radius:4px;padding:1px 7px;font-weight:700;font-size:13px;"
           "margin-right:6px}.change{margin:8px 0 8px 24px;color:#777;font-size:14px}"
           "</style></head><body>\n";
    if (route.legs.empty()) {
        out << "<h1>You are already there.</h1></body></html>\n";
        return out.str();
    }
    out << "<h1>" << escape_html(network.station_name(route.legs.front().stations.front())) << " &rarr; "
        << escape_html(network.station_name(route.legs.back().stations.back())) << "</h1>\n";
    out << "<div class=\"sum\">" << route.total_minutes() << " min &middot; "
        << plural(route.transfers(), "transfer") << "</div>\n";
    for (std::size_t i = 0; i < route.legs.size(); ++i) {
        const Leg& leg = route.legs[i];
        std::string color = network.line_color(leg.line);
        if (i > 0) out << "<div class=\"change\">&#8645; change, about " << route.transfer_minutes / route.transfers()
                       << " min</div>\n";
        out << "<div class=\"leg\"><div class=\"bar\" style=\"background:" << color << "\"></div><div>\n";
        for (std::size_t s = 0; s < leg.stations.size(); ++s) {
            bool end = s == 0 || s + 1 == leg.stations.size();
            out << "<div class=\"st" << (end ? " end" : "") << "\">";
            if (s == 0) {
                out << "<span class=\"badge\" style=\"background:" << color << "\">" << escape_html(leg.line)
                    << "</span>";
            }
            out << escape_html(network.station_name(leg.stations[s])) << "</div>\n";
        }
        out << "</div></div>\n";
    }
    out << "<p class=\"sum\">Travel times are approximate. Data: own simplified model of the Vienna U-Bahn.</p>\n"
        << "</body></html>\n";
    return out.str();
}

}
