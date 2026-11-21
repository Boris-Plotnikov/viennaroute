import { FormEvent, useEffect, useMemo, useState } from "react";
import UnderTheHood, { Labels, useFlows } from "./codetour/UnderTheHood";
import { logRequest, openTour } from "./codetour/store";
import { LineInfo, Match, Planner, RouteResult, Station, startPlanner } from "./engine";

const LABELS: Labels = {
  subtitle: "C++ source code of this program, extracted from the repository at build time.",
  requests: "Live calls",
  emptyRequests: "Find a route: every call into the C++ module appears here with the code that handled it.",
};

const EXAMPLES: [string, string][] = [
  ["Hütteldorf", "Seestadt"],
  ["karlspl", "stefansplatz"],
  ["Laengenfeldgasse", "Floridsdorf"],
  ["Schwedenplaz", "Ottakring"],
  ["Schotten", "Oberlaa"],
];

type Tab = "diagram" | "text" | "html";

function Hint({ match }: { match: Match | null }) {
  if (!match || !match.query.trim()) return <div className="mt-1 h-5 text-xs text-muted">Short names, no umlauts and typos are fine.</div>;
  if (match.station) return <div className="mt-1 h-5 text-xs text-emerald-700">→ {match.station}</div>;
  if (match.suggestions.length) {
    return <div className="mt-1 h-5 truncate text-xs text-amber-700">Did you mean: {match.suggestions.slice(0, 4).join(", ")}?</div>;
  }
  return <div className="mt-1 h-5 text-xs text-red-600">No station found</div>;
}

function Badge({ line, color }: { line: string; color: string }) {
  return <span className="rounded px-1.5 py-0.5 text-xs font-bold text-white" style={{ background: color }}>{line}</span>;
}

function Diagram({ result }: { result: RouteResult }) {
  const route = result.route!;
  if (route.legs.length === 0) return <p className="text-sm text-muted">Start and destination are the same station.</p>;
  return (
    <div>
      {route.legs.map((leg, i) => (
        <div key={i}>
          {i > 0 && (
            <div className="my-2 ml-8 text-sm text-muted">⇅ change at {leg.stations[0]}</div>
          )}
          <div className="flex gap-3">
            <div className="w-2.5 shrink-0 rounded-full" style={{ background: leg.color }} />
            <div className="flex-1 py-1">
              <div className="mb-1 flex items-center gap-2">
                <Badge line={leg.line} color={leg.color} />
                <span className="text-xs text-muted">{leg.stations.length - 1} stops · {leg.minutes} min</span>
              </div>
              {leg.stations.map((s, j) => (
                <div key={s + j} className={`py-0.5 text-sm ${j === 0 || j === leg.stations.length - 1 ? "font-semibold" : "text-slate-600"}`}>{s}</div>
              ))}
            </div>
          </div>
        </div>
      ))}
    </div>
  );
}

export default function App() {
  const [planner, setPlanner] = useState<Planner | null>(null);
  const [loadMs, setLoadMs] = useState(0);
  const [failed, setFailed] = useState(false);
  const [stations, setStations] = useState<Station[]>([]);
  const [lines, setLines] = useState<LineInfo[]>([]);
  const [from, setFrom] = useState("Hütteldorf");
  const [to, setTo] = useState("Seestadt");
  const [transfer, setTransfer] = useState(4);
  const [result, setResult] = useState<RouteResult | null>(null);
  const [ms, setMs] = useState(0);
  const [tab, setTab] = useState<Tab>("diagram");
  const flows = useFlows();

  useEffect(() => {
    const started = performance.now();
    startPlanner()
      .then((p) => {
        setLoadMs(Math.round(performance.now() - started));
        setPlanner(p);
        setStations(p.stations());
        setLines(p.lines());
      })
      .catch(() => setFailed(true));
  }, []);

  const fromMatch = useMemo(() => (planner ? planner.search(from) : null), [planner, from]);
  const toMatch = useMemo(() => (planner ? planner.search(to) : null), [planner, to]);

  const run = (a: string, b: string, minutes: number) => {
    if (!planner) return;
    const started = performance.now();
    const r = planner.route(a, b, minutes);
    const elapsed = Math.round((performance.now() - started) * 10) / 10;
    setMs(elapsed);
    setResult(r);
    const ok = Boolean(r.route);
    logRequest({ method: "CALL", path: `route ${a} → ${b}, transfer ${minutes} min`, status: ok ? 200 : 404, ms: elapsed, label: ok ? `${r.route!.totalMinutes} min` : "no route" });
  };

  useEffect(() => {
    if (planner) run(from, to, transfer);
  }, [planner]);

  useEffect(() => {
    if (planner && result?.route) run(from, to, transfer);
  }, [transfer]);

  const submit = (e: FormEvent) => {
    e.preventDefault();
    run(from, to, transfer);
  };

  const example = ([a, b]: [string, string]) => {
    setFrom(a);
    setTo(b);
    run(a, b, transfer);
  };

  const route = result?.route;

  return (
    <div className="min-h-screen">
      <header className="border-b border-slate-200 bg-white">
        <div className="mx-auto flex max-w-6xl items-center gap-3 px-4 py-3">
          <img src="./favicon.svg" alt="" className="h-7 w-7" />
          <div className="font-bold">ViennaRoute</div>
          <span className="hidden text-sm text-muted sm:inline">C++17 · Dijkstra · CMake · GoogleTest</span>
          <a href="../" className="ml-auto text-sm text-muted hover:text-ink">← Portfolio</a>
        </div>
      </header>

      <main className="mx-auto max-w-6xl px-4 py-8">
        <h1 className="text-3xl font-bold tracking-tight">Fastest trip through the Vienna U-Bahn</h1>
        <p className="mt-2 max-w-3xl text-slate-600">
          Route planner in C++17. The network is a graph, the fastest trip is found with Dijkstra's algorithm, a change of line costs time.
          The library used by the command-line version and its 18 unit tests is compiled to WebAssembly and runs in the browser.
        </p>
        <div className="mt-3 flex flex-wrap gap-2 text-xs">
          <span className="rounded-full bg-emerald-100 px-2.5 py-1 font-semibold text-emerald-800">{planner ? `● C++ module loaded in ${loadMs} ms` : failed ? "Could not load the module" : "Loading the C++ module…"}</span>
          {planner && <span className="rounded-full bg-slate-100 px-2.5 py-1 text-slate-700">{stations.length} stations · {lines.length} lines</span>}
          {lines.map((l) => <Badge key={l.name} line={l.name} color={l.color} />)}
        </div>

        <div className="mt-6 grid gap-6 lg:grid-cols-[380px_1fr]">
          <div>
            <form onSubmit={submit} className="card space-y-3 p-4">
              <div>
                <label className="mb-1 block text-xs font-semibold uppercase tracking-wide text-muted" htmlFor="from">From</label>
                <input id="from" list="stations" className="input" value={from} onChange={(e) => setFrom(e.target.value)} />
                <Hint match={fromMatch} />
              </div>
              <div>
                <label className="mb-1 block text-xs font-semibold uppercase tracking-wide text-muted" htmlFor="to">To</label>
                <input id="to" list="stations" className="input" value={to} onChange={(e) => setTo(e.target.value)} />
                <Hint match={toMatch} />
              </div>
              <div>
                <label className="mb-1 flex justify-between text-xs font-semibold uppercase tracking-wide text-muted" htmlFor="transfer">
                  <span>One change of line costs</span><span className="text-ink">{transfer} min</span>
                </label>
                <input id="transfer" type="range" min={0} max={15} value={transfer} onChange={(e) => setTransfer(Number(e.target.value))} className="w-full accent-[rgb(var(--brand))]" />
                <p className="text-xs text-muted">A higher value leads to routes with fewer changes.</p>
              </div>
              <button disabled={!planner} className="w-full rounded-lg bg-brand py-2 text-sm font-semibold text-white transition hover:bg-brand-dark disabled:opacity-50">Find route</button>
              <datalist id="stations">
                {stations.map((s) => <option key={s.name} value={s.name} />)}
              </datalist>
            </form>
            <div className="mt-3">
              <div className="mb-1.5 text-xs font-semibold uppercase tracking-wide text-muted">Try</div>
              <div className="flex flex-wrap gap-1.5">
                {EXAMPLES.map((e) => <button key={e.join()} onClick={() => example(e)} className="chip">{e[0]} → {e[1]}</button>)}
              </div>
            </div>
          </div>

          <div className="card p-5">
            {!result ? <p className="text-sm text-muted">{failed ? "The C++ module could not be loaded." : "Loading…"}</p>
              : !route ? (
                <div className="text-sm">
                  <p className="font-semibold">No route.</p>
                  {[result.from, result.to].map((m) => !m.station && (
                    <p key={m.query} className="mt-2 text-slate-600">
                      {m.suggestions.length ? <>“{m.query}” is ambiguous. Did you mean: <b>{m.suggestions.join(", ")}</b>?</> : <>No station found for “{m.query}”.</>}
                    </p>
                  ))}
                </div>
              ) : (
                <>
                  <div className="flex flex-wrap items-end gap-x-6 gap-y-2">
                    <div>
                      <div className="text-xs text-muted">{result.from.station} → {result.to.station}</div>
                      <div className="text-3xl font-bold">{route.totalMinutes} min</div>
                    </div>
                    <div className="text-sm text-slate-600">{route.transfers} {route.transfers === 1 ? "change" : "changes"} · riding {route.rideMinutes} min + changing {route.transferMinutes} min</div>
                    <div className="ml-auto font-mono text-xs text-muted">Dijkstra: {ms} ms</div>
                  </div>
                  <nav className="mb-4 mt-4 flex gap-1 border-b border-slate-200">
                    {([["diagram", "Diagram"], ["text", "Terminal output"], ["html", "HTML export"]] as [Tab, string][]).map(([id, label]) => (
                      <button key={id} onClick={() => setTab(id)}
                        className={`border-b-2 px-3 py-2 text-sm font-medium ${tab === id ? "border-brand text-brand-dark" : "border-transparent text-muted hover:text-ink"}`}>
                        {label}
                      </button>
                    ))}
                  </nav>
                  <div className="max-h-[460px] overflow-y-auto">
                    {tab === "diagram" && <Diagram result={result} />}
                    {tab === "text" && <pre className="code-block rounded-lg p-4 text-[12.5px]">{route.text}</pre>}
                    {tab === "html" && (
                      <div>
                        <p className="mb-2 text-xs text-muted">What <span className="font-mono">viennaroute route … --html route.html</span> writes, produced by formatter::to_html.</p>
                        <iframe title="HTML export" srcDoc={route.html} sandbox="" className="h-[400px] w-full rounded-lg border border-slate-200" />
                      </div>
                    )}
                  </div>
                </>
              )}
            <button onClick={() => openTour("dijkstra")} className="mt-4 font-mono text-xs font-semibold text-brand hover:underline">&lt;/&gt; C++ code of the route search</button>
          </div>
        </div>

        <section className="mt-12">
          <h2 className="text-xl font-bold">How it is built</h2>
          <p className="mt-1 text-sm text-slate-600">Graph, station search, output and tests.</p>
          <div className="mt-4 grid gap-3 sm:grid-cols-2 lg:grid-cols-3">
            {(flows ?? []).map((f) => (
              <button key={f.id} onClick={() => openTour(f.id)} className="card p-4 text-left transition hover:border-brand/40">
                <div className="font-semibold">{f.title}</div>
                <p className="mt-1 text-xs leading-relaxed text-muted">{f.summary}</p>
                <div className="mt-2 font-mono text-xs text-brand">&lt;/&gt; {f.steps.length} snippets →</div>
              </button>
            ))}
          </div>
        </section>

        <footer className="mt-12 border-t border-slate-200 pt-6 text-sm text-muted">
          ViennaRoute · a portfolio project by Boris Plotnikov · network data typed by hand from the public line maps · <a href="../" className="hover:text-ink">boris-plotnikov.click</a>
        </footer>
      </main>
      <UnderTheHood screen="/" labels={LABELS} />
    </div>
  );
}
