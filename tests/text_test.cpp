#include <gtest/gtest.h>

#include "viennaroute/text.h"

TEST(TextTest, NormalizeHandlesUmlautsCaseAndPunctuation) {
    EXPECT_EQ(text::normalize("Südtiroler Platz-Hauptbahnhof"), "suedtirolerplatzhauptbahnhof");
    EXPECT_EQ(text::normalize("Großfeldsiedlung"), "grossfeldsiedlung");
    EXPECT_EQ(text::normalize("Ober St. Veit"), "oberstveit");
    EXPECT_EQ(text::normalize("ROßAUER Lände"), "rossauerlaende");
}

TEST(TextTest, LevenshteinCountsEdits) {
    EXPECT_EQ(text::levenshtein("kitten", "sitting"), 3);
    EXPECT_EQ(text::levenshtein("", "abc"), 3);
    EXPECT_EQ(text::levenshtein("same", "same"), 0);
    EXPECT_EQ(text::levenshtein("stefansplatz", "stephansplatz"), 2);
}

TEST(TextTest, SplitTrimsParts) {
    auto parts = text::split(" U1 , Karlsplatz ,2", ',');
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[1], "Karlsplatz");
}
