#include <gtest/gtest.h>

#include <stdexcept>

#include "viennaroute/network.h"

TEST(NetworkTest, LoadsRealDataFiles) {
    Network network = Network::load(VIENNAROUTE_DATA_DIR);
    EXPECT_EQ(network.lines().size(), 5u);
    EXPECT_GT(network.station_count(), 90);
    int karlsplatz = network.station_id("Karlsplatz");
    ASSERT_GE(karlsplatz, 0);
    std::vector<std::string> lines = network.lines_at(karlsplatz);
    EXPECT_EQ(lines, (std::vector<std::string>{"U1", "U4"}));
}

TEST(NetworkTest, SegmentsWorkInBothDirections) {
    Network network;
    network.add_line("U9", "#000");
    network.add_segment("U9", "A", "B", 3);
    int a = network.station_id("A");
    int b = network.station_id("B");
    ASSERT_EQ(network.connections(a).size(), 1u);
    EXPECT_EQ(network.connections(a)[0].to, b);
    EXPECT_EQ(network.connections(b)[0].to, a);
}

TEST(NetworkTest, RejectsInvalidSegments) {
    Network network;
    EXPECT_THROW(network.add_segment("U9", "A", "B", 0), std::invalid_argument);
    EXPECT_THROW(network.add_segment("U9", "A", "A", 2), std::invalid_argument);
}

TEST(NetworkTest, MissingFolderGivesClearError) {
    EXPECT_THROW(Network::load("/no/such/folder"), std::runtime_error);
}
