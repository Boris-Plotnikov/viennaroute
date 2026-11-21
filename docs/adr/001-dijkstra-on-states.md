# ADR-001: Dijkstra on (station, line) states

- **Status:** Accepted
- **Date:** 2026-11-18

## Context

The fastest route must consider that changing lines costs time: walking between platforms and waiting for the next train.

## Options considered

| Option | Pros | Cons |
|---|---|---|
| BFS on stations | Very simple | Ignores minutes, finds fewest stops, not fastest trip |
| Dijkstra on stations | Uses minutes | Cannot see transfers: a route with 3 changes may look "fastest" |
| Dijkstra on (station, line) | Transfers become edges with a cost | Twice as many states - irrelevant for 100 stations |
| A* | Faster on huge graphs | Needs coordinates for the heuristic; no gain at this size |

## Decision

Dijkstra with states (station, line). Riding is an edge to the next station on the same line; a transfer is an edge to the same station on another line with the transfer cost. Costs are in tenths of a minute, a transfer adds one extra tenth, so equal times prefer fewer transfers.

## Consequences

- Correct handling of transfers with a configurable cost.
- Easy to explain and to test on a small hand-made network.
- Timetables (waiting for a specific train) are not modelled; that would need a time-dependent graph.
