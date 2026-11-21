#include <gtest/gtest.h>

#include "viennaroute/station_search.h"

class StationSearchTest : public ::testing::Test {
protected:
    Network network = Network::load(VIENNAROUTE_DATA_DIR);
    StationSearch search{network};
    std::string found(const std::string& query) {
        SearchResult r = search.find(query);
        return r.station >= 0 ? network.station_name(r.station) : "";
    }
};

TEST_F(StationSearchTest, FindsExactNameIgnoringCaseAndUmlauts) {
    EXPECT_EQ(found("karlsplatz"), "Karlsplatz");
    EXPECT_EQ(found("Laengenfeldgasse"), "Längenfeldgasse");
}

TEST_F(StationSearchTest, FindsUniquePrefixAndSubstring) {
    EXPECT_EQ(found("karlspl"), "Karlsplatz");
    EXPECT_EQ(found("hauptbahnhof"), "Südtiroler Platz-Hauptbahnhof");
}

TEST_F(StationSearchTest, ToleratesTypos) {
    EXPECT_EQ(found("Stefansplatz"), "Stephansplatz");
    EXPECT_EQ(found("Schwedenpaltz"), "Schwedenplatz");
}

TEST_F(StationSearchTest, AmbiguousInputGivesSuggestions) {
    SearchResult r = search.find("Schotten");
    EXPECT_EQ(r.station, -1);
    EXPECT_EQ(r.suggestions, (std::vector<std::string>{"Schottenring", "Schottentor"}));
}

TEST_F(StationSearchTest, NonsenseFindsNothing) {
    SearchResult r = search.find("xyzxyzxyz");
    EXPECT_EQ(r.station, -1);
    EXPECT_TRUE(r.suggestions.empty());
}
