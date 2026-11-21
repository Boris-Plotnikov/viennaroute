#include <gtest/gtest.h>

#include "viennaroute/formatter.h"
#include "viennaroute/route_planner.h"

namespace {
Network small_network() {
    Network n;
    n.add_line("L1", "#f00");
    n.add_line("L2", "#00f");
    n.add_segment("L1", "A", "B", 2);
    n.add_segment("L1", "B", "C", 2);
    n.add_segment("L1", "C", "D", 2);
    n.add_segment("L2", "B", "X", 1);
    n.add_segment("L2", "X", "D", 1);
    return n;
}

}

TEST(RoutePlannerTest, TransferPenaltyChangesTheBestRoute) {
    Network n = small_network();
    int a = n.station_id("A");
    int d = n.station_id("D");

    Route with_change = *RoutePlanner(n, 1).find(a, d);
    EXPECT_EQ(with_change.total_minutes(), 5);
    EXPECT_EQ(with_change.transfers(), 1);

    Route direct = *RoutePlanner(n, 5).find(a, d);
    EXPECT_EQ(direct.total_minutes(), 6);
    EXPECT_EQ(direct.transfers(), 0);
    ASSERT_EQ(direct.legs.size(), 1u);
    EXPECT_EQ(direct.legs[0].stops(), 3);
}

TEST(RoutePlannerTest, SameStationIsEmptyRoute) {
    Network n = small_network();
    Route r = *RoutePlanner(n, 4).find(0, 0);
    EXPECT_TRUE(r.legs.empty());
    EXPECT_EQ(r.total_minutes(), 0);
}

TEST(RoutePlannerTest, DisconnectedStationsHaveNoRoute) {
    Network n = small_network();
    n.add_line("L3", "#0f0");
    n.add_segment("L3", "Island1", "Island2", 1);
    EXPECT_FALSE(RoutePlanner(n, 4).find(n.station_id("A"), n.station_id("Island2")).has_value());
}

TEST(RoutePlannerTest, RealNetworkKarlsplatzToSeestadt) {
    Network n = Network::load(VIENNAROUTE_DATA_DIR);
    Route r = *RoutePlanner(n, 4).find(n.station_id("Karlsplatz"), n.station_id("Seestadt"));
    ASSERT_EQ(r.legs.size(), 2u);
    EXPECT_EQ(r.legs[0].line, "U1");
    EXPECT_EQ(n.station_name(r.legs[0].stations.back()), "Praterstern");
    EXPECT_EQ(r.legs[1].line, "U2");
}

TEST(RoutePlannerTest, RealNetworkUsesBothTransfersWhenNeeded) {
    Network n = Network::load(VIENNAROUTE_DATA_DIR);
    Route r = *RoutePlanner(n, 4).find(n.station_id("Ottakring"), n.station_id("Floridsdorf"));
    ASSERT_EQ(r.legs.size(), 2u);
    EXPECT_EQ(r.legs[0].line, "U3");
    EXPECT_EQ(r.legs[1].line, "U6");
}

TEST(FormatterTest, TextAndHtmlContainStations) {
    Network n = small_network();
    Route r = *RoutePlanner(n, 1).find(n.station_id("A"), n.station_id("D"));
    std::string txt = formatter::to_text(r, n);
    EXPECT_NE(txt.find("change at B"), std::string::npos);
    std::string html = formatter::to_html(r, n);
    EXPECT_NE(html.find("<!doctype html>"), std::string::npos);
    EXPECT_NE(html.find("#00f"), std::string::npos);
}
