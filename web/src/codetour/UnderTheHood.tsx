import { useEffect, useState } from "react";
import CodeBlock, { languageOf } from "./CodeBlock";
import {
  closeTour, Flow, flowForRequest, flowsForScreen, openTour, RequestEntry, showFlow, Step, useTour,
} from "./store";

export const REPOSITORY_URL = "https://github.com/Boris-Plotnikov/viennaroute";

let cache: Flow[] | null = null;

export function useFlows(): Flow[] | null {
  const [flows, setFlows] = useState<Flow[] | null>(cache);
  useEffect(() => {
    if (cache) return;
    import("../generated/codetour.json").then((m) => {
      cache = (m.default as { flows: Flow[] }).flows;
      setFlows(cache);
    });
  }, []);
  return flows;
}

const LAYER_STYLES: Record<string, string> = {
  Controller: "bg-sky-100 text-sky-800",
  Service: "bg-emerald-100 text-emerald-800",
  Repository: "bg-violet-100 text-violet-800",
  Domain: "bg-amber-100 text-amber-800",
  DTO: "bg-amber-100 text-amber-800",
  AI: "bg-fuchsia-100 text-fuchsia-800",
  "AI client": "bg-fuchsia-100 text-fuchsia-800",
  Test: "bg-lime-100 text-lime-800",
  SQL: "bg-slate-200 text-slate-800",
  Header: "bg-amber-100 text-amber-800",
  Logic: "bg-emerald-100 text-emerald-800",
  WebAssembly: "bg-fuchsia-100 text-fuchsia-800",
};

function LayerPill({ layer }: { layer: string }) {
  return (
    <span className={`rounded px-1.5 py-0.5 text-[10px] font-semibold uppercase tracking-wide ${LAYER_STYLES[layer] ?? "bg-slate-100 text-slate-700"}`}>
      {layer}
    </span>
  );
}

function className(step: Step): string {
  const file = step.file.split("/").pop() ?? "";
  return file.replace(/\.(java|sql|cpp|h)$/, "");
}

function title(step: Step): string {
  if (step.symbol === "file" || step.symbol.startsWith("sql:")) return step.symbol === "file" ? className(step) : `table ${step.name}`;
  if (step.symbol.startsWith("type:") || languageOf(step.file) === "cpp") return `${step.name}${step.symbol.startsWith("method:") ? "()" : ""}`;
  if (step.symbol.startsWith("test:")) return `TEST ${step.name.replace(".", ", ")}`;
  return `${className(step)}.${step.name}${step.symbol.startsWith("method:") ? "()" : ""}`;
}

function StepCard({ step, index, expanded }: { step: Step; index: number; expanded: boolean }) {
  const [open, setOpen] = useState(expanded);
  const link = REPOSITORY_URL ? `${REPOSITORY_URL}/blob/main/${step.path}#L${step.startLine}-L${step.endLine}` : null;
  return (
    <li className="relative pl-8">
      <span className="absolute left-0 top-1 flex h-6 w-6 items-center justify-center rounded-full bg-ink text-xs font-bold text-white">
        {index + 1}
      </span>
      <button onClick={() => setOpen(!open)} className="w-full text-left">
        <div className="flex flex-wrap items-center gap-2">
          <LayerPill layer={step.layer} />
          <span className="font-mono text-sm font-semibold">{title(step)}</span>
          <span className="ml-auto text-xs text-muted">{open ? "Hide" : "Show"} code</span>
        </div>
        {step.note && <p className="mt-1 text-sm text-slate-600">{step.note}</p>}
      </button>
      <div className="mt-1 font-mono text-[11px] text-muted">
        {link ? <a href={link} target="_blank" rel="noreferrer" className="hover:text-brand">{step.path}:{step.startLine}–{step.endLine} ↗</a>
          : <>{step.path}:{step.startLine}–{step.endLine}</>}
      </div>
      {open && (
        <div className="mt-2">
          <CodeBlock code={step.code} startLine={step.startLine} lang={languageOf(step.file)} maxHeight="420px" />
        </div>
      )}
    </li>
  );
}

function FlowDetail({ flow }: { flow: Flow }) {
  return (
    <div>
      <button onClick={() => showFlow(null)} className="mb-3 text-sm text-brand hover:underline">← All flows</button>
      <h3 className="text-xl font-bold">{flow.title}</h3>
      <p className="mt-1 text-sm text-slate-600">{flow.summary}</p>
      {flow.endpoints.length > 0 && (
        <div className="mt-3 flex flex-wrap gap-1.5">
          {flow.endpoints.map((e) => (
            <span key={e} className="rounded bg-slate-100 px-2 py-0.5 font-mono text-[11px] text-slate-700">{e}</span>
          ))}
        </div>
      )}
      <ol className="mt-5 space-y-5 border-l-2 border-slate-100 pl-1">
        {flow.steps.map((s, i) => <StepCard key={flow.id + i} step={s} index={i} expanded={i === 0} />)}
      </ol>
    </div>
  );
}

function FlowList({ flows, empty }: { flows: Flow[]; empty?: string }) {
  if (flows.length === 0) return <p className="text-sm text-muted">{empty}</p>;
  return (
    <div className="space-y-2">
      {flows.map((f) => (
        <button key={f.id} onClick={() => showFlow(f.id)} className="card block w-full p-4 text-left transition hover:border-brand/40">
          <div className="font-semibold">{f.title}</div>
          <p className="mt-1 text-xs leading-relaxed text-muted">{f.summary}</p>
          <div className="mt-2 flex flex-wrap items-center gap-1">
            {[...new Set(f.steps.map((s) => s.layer))].map((l, i) => (
              <span key={l} className="flex items-center gap-1">
                {i > 0 && <span className="text-xs text-slate-300">→</span>}
                <LayerPill layer={l} />
              </span>
            ))}
          </div>
        </button>
      ))}
    </div>
  );
}

function Requests({ requests, flows, empty }: { requests: RequestEntry[]; flows: Flow[]; empty: string }) {
  if (requests.length === 0) {
    return <p className="text-sm text-muted">{empty}</p>;
  }
  return (
    <div className="overflow-hidden rounded-lg border border-slate-200">
      {requests.map((r) => {
        const flow = flowForRequest(flows, r.method, r.path);
        const ok = r.status >= 200 && r.status < 400;
        return (
          <div key={r.id} className="flex items-center gap-2 border-b border-slate-100 px-3 py-2 text-xs last:border-0">
            <span className="w-14 shrink-0 font-mono font-semibold">{r.method}</span>
            <span className="min-w-0 flex-1 truncate font-mono text-slate-700" title={r.path}>{r.path}</span>
            <span className={`shrink-0 font-mono ${ok ? "text-emerald-700" : "text-red-700"}`}>{r.label ?? (r.status || "ERR")}</span>
            <span className="w-14 shrink-0 text-right text-muted">{r.ms} ms</span>
            {flow ? (
              <button onClick={() => showFlow(flow.id)} className="w-16 shrink-0 text-right text-brand hover:underline">code →</button>
            ) : <span className="w-16 shrink-0" />}
          </div>
        );
      })}
    </div>
  );
}

type Tab = "page" | "all" | "requests";

export interface Labels {
  subtitle: string;
  requests: string;
  emptyRequests: string;
}

const API_LABELS: Labels = {
  subtitle: "Real code of this application, taken from the repository at build time.",
  requests: "Live requests",
  emptyRequests: "Use the page: every API call it makes appears here with the code that handles it.",
};

export default function UnderTheHood({ screen, labels = API_LABELS }: { screen: string; labels?: Labels }) {
  const { open, flowId, requests } = useTour();
  const flows = useFlows();
  const [tab, setTab] = useState<Tab>("page");
  const here = flows ? flowsForScreen(flows, screen) : [];

  useEffect(() => {
    if (!open) return;
    const onKey = (e: KeyboardEvent) => e.key === "Escape" && closeTour();
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [open]);

  const flow = flows?.find((f) => f.id === flowId);
  const tabs: [Tab, string][] = [
    ["page", `This page (${here.length})`],
    ["all", `All flows (${flows?.length ?? 0})`],
    ["requests", `${labels.requests} (${requests.length})`],
  ];

  return (
    <>
      <button
        onClick={() => openTour(null)}
        className="fixed bottom-4 right-4 z-40 flex items-center gap-2 rounded-full bg-ink px-4 py-2.5 text-sm font-semibold text-white shadow-lg transition hover:bg-brand-dark"
      >
        <span className="font-mono">&lt;/&gt;</span> Under the hood
      </button>
      {open && (
        <div className="fixed inset-0 z-50 flex justify-end">
          <div className="absolute inset-0 bg-ink/30" onClick={closeTour} />
          <aside className="relative flex h-full w-full flex-col bg-surface shadow-2xl md:w-[760px]">
            <header className="flex items-start gap-3 border-b border-slate-200 bg-white px-5 py-4">
              <div className="flex-1">
                <div className="text-lg font-bold">Under the hood</div>
                <div className="text-xs text-muted">{labels.subtitle}</div>
              </div>
              <button onClick={closeTour} className="rounded-lg px-2 py-1 text-xl leading-none text-muted hover:bg-slate-100" aria-label="Close">×</button>
            </header>
            {!flow && (
              <nav className="flex gap-1 border-b border-slate-200 bg-white px-3">
                {tabs.map(([id, label]) => (
                  <button
                    key={id}
                    onClick={() => setTab(id)}
                    className={`border-b-2 px-3 py-2.5 text-sm font-medium ${tab === id ? "border-brand text-brand-dark" : "border-transparent text-muted hover:text-ink"}`}
                  >
                    {label}
                  </button>
                ))}
              </nav>
            )}
            <div className="flex-1 overflow-y-auto p-5">
              {!flows ? <p className="text-sm text-muted">Loading…</p>
                : flow ? <FlowDetail flow={flow} />
                : tab === "page" ? <FlowList flows={here} empty="No flow is linked to this screen. Open “All flows”." />
                : tab === "all" ? <FlowList flows={flows} />
                : <Requests requests={requests} flows={flows} empty={labels.emptyRequests} />}
            </div>
          </aside>
        </div>
      )}
    </>
  );
}
