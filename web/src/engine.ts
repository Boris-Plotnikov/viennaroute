import create from "./wasm/viennaroute_web.mjs";

export interface Station {
  name: string;
  lines: string[];
}

export interface LineInfo {
  name: string;
  color: string;
}

export interface Match {
  query: string;
  station: string | null;
  suggestions: string[];
}

export interface Leg {
  line: string;
  color: string;
  minutes: number;
  stations: string[];
}

export interface Route {
  legs: Leg[];
  rideMinutes: number;
  transferMinutes: number;
  totalMinutes: number;
  transfers: number;
  text: string;
  html: string;
}

export interface RouteResult {
  from: Match;
  to: Match;
  route?: Route;
}

export interface Planner {
  stations(): Station[];
  lines(): LineInfo[];
  search(query: string): Match;
  route(from: string, to: string, transferMinutes: number): RouteResult;
}

export async function startPlanner(): Promise<Planner> {
  const module = (await create()) as { Planner: new () => Planner };
  return new module.Planner();
}
