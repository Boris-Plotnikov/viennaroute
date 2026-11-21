import { useSyncExternalStore } from "react";

export interface Step {
  layer: string;
  file: string;
  path: string;
  symbol: string;
  name: string;
  note: string;
  startLine: number;
  endLine: number;
  code: string;
}

export interface Flow {
  id: string;
  title: string;
  summary: string;
  routes: string[];
  endpoints: string[];
  steps: Step[];
}

export interface RequestEntry {
  id: number;
  method: string;
  path: string;
  status: number;
  ms: number;
  at: number;
  label?: string;
}

interface State {
  open: boolean;
  flowId: string | null;
  requests: RequestEntry[];
}

let state: State = { open: false, flowId: null, requests: [] };
let nextId = 1;
const listeners = new Set<() => void>();

function set(patch: Partial<State>) {
  state = { ...state, ...patch };
  listeners.forEach((l) => l());
}

function subscribe(listener: () => void) {
  listeners.add(listener);
  return () => listeners.delete(listener);
}

export function useTour(): State {
  return useSyncExternalStore(subscribe, () => state);
}

export function openTour(flowId: string | null = null) {
  set({ open: true, flowId });
}

export function closeTour() {
  set({ open: false });
}

export function showFlow(flowId: string | null) {
  set({ flowId });
}

export function logRequest(entry: Omit<RequestEntry, "id" | "at">) {
  const requests = [{ ...entry, id: nextId++, at: Date.now() }, ...state.requests].slice(0, 50);
  set({ requests });
}

export function endpointMatches(endpoint: string, method: string, path: string): boolean {
  const [m, pattern] = endpoint.split(" ");
  if (m !== method) return false;
  const regex = new RegExp("^" + pattern.replace(/\{[^}]+\}/g, "[^/]+") + "$");
  return regex.test(path.split("?")[0].split(" ")[0]);
}

export function flowForRequest(flows: Flow[], method: string, path: string): Flow | undefined {
  return flows.find((f) => f.endpoints.some((e) => endpointMatches(e, method, path)));
}

export function flowsForScreen(flows: Flow[], screen: string): Flow[] {
  return flows.filter((f) => f.routes.some((r) => screen === r || screen.startsWith(r + "/")));
}
