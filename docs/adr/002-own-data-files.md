# ADR-002: Own CSV model of the network

- **Status:** Accepted
- **Date:** 2026-11-18

## Context

Official open data (GTFS) contains every tram and bus with full timetables - tens of megabytes. The project needs only the metro and average travel times.

## Decision

Two small CSV files compiled by hand from public network maps: lines with colours and segments with minutes. The loader reports bad lines with file name and line number.

## Consequences

- The repository stays small, the data is readable and easy to correct.
- Times are approximate. Importing GTFS would be the next step for exact times and trams.
