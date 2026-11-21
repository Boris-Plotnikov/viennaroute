# Requirements

## Goal

Find the fastest trip between two Vienna U-Bahn stations from the command line and explain the result: lines, transfers, minutes.

## User stories

1. **Fastest trip.** As a passenger I want the fastest route between two stations.
   - Given Karlsplatz and Seestadt, when I run `route`, then I get U1 to Praterstern and U2 to Seestadt with total minutes.
2. **Transfers have a cost.** As a passenger I do not want a route with many changes when a direct one is almost as fast.
   - Given transfer cost 5 min, when a route with one change saves only 1 minute, then the direct route is chosen.
3. **Forgiving input.** As a user I want to type names without umlauts, shortened or with a typo.
   - Given input "Stefansplatz", then Stephansplatz is found. Given "Schotten", then I see Schottenring and Schottentor as suggestions.
4. **Share the route.** As a user I want an HTML diagram of the route with line colours.

## Rules

- Travel times are positive whole minutes. Unknown line in the data file is an error with the line number.
- Same start and end station gives an empty route, not an error.
