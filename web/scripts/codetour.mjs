import { mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { dirname, join, relative } from "node:path";
import { fileURLToPath } from "node:url";

const here = dirname(fileURLToPath(import.meta.url));
const projectRoot = join(here, "..", "..");
const config = JSON.parse(readFileSync(join(here, "..", "codetour.json"), "utf8"));
const sourceRoot = join(here, "..", config.root);
const output = join(here, "..", "src", "generated", "codetour.json");

function skipString(text, i) {
  if (text.startsWith('"""', i)) return text.indexOf('"""', i + 3) + 3;
  const quote = text[i];
  let j = i + 1;
  while (j < text.length && text[j] !== quote && text[j] !== "\n") j += text[j] === "\\" ? 2 : 1;
  return j + 1;
}

function matchingBrace(text, open) {
  let depth = 0;
  for (let i = open; i < text.length; ) {
    const c = text[i];
    if (c === '"' || c === "'") {
      i = skipString(text, i);
      continue;
    }
    if (c === "{") depth++;
    if (c === "}" && --depth === 0) return i;
    i++;
  }
  throw new Error("Unbalanced braces");
}

function firstBrace(text, from) {
  for (let i = from; i < text.length; ) {
    const c = text[i];
    if (c === '"' || c === "'") {
      i = skipString(text, i);
      continue;
    }
    if (c === "{") return i;
    if (c === ";") return -1;
    i++;
  }
  return -1;
}

function withAnnotations(text, start) {
  let lineStart = text.lastIndexOf("\n", start - 1) + 1;
  for (;;) {
    const prevEnd = lineStart - 1;
    if (prevEnd < 0) return lineStart;
    const prevStart = text.lastIndexOf("\n", prevEnd - 1) + 1;
    if (!text.slice(prevStart, prevEnd).trim().startsWith("@")) return lineStart;
    lineStart = prevStart;
  }
}

function blockRange(text, pattern, name) {
  const regex = new RegExp(pattern, "gm");
  for (let m; (m = regex.exec(text)); ) {
    const open = firstBrace(text, m.index + m[0].length);
    if (open < 0) continue;
    return [withAnnotations(text, m.index), matchingBrace(text, open) + 1];
  }
  throw new Error(`${name} not found`);
}

const escape = (s) => s.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");

function find(text, symbol) {
  if (symbol === "file") return [0, text.length];
  const kind = symbol.slice(0, symbol.indexOf(":"));
  const name = escape(symbol.slice(kind.length + 1));
  if (kind === "method") {
    const pattern = `^[ \\t]*(?:(?:public|protected|private|static|final|synchronized|default|inline)\\s+)*(?:<[^>]+>\\s+)?[\\w<>\\[\\],.?:&* ]+\\s+${name}\\s*\\(`;
    return blockRange(text, pattern, symbol);
  }
  if (kind === "type") {
    return blockRange(text, `^[ \\t]*(?:(?:public|protected|private|static|final|abstract)\\s+)*(?:class|record|interface|enum|struct)\\s+${name}\\b`, symbol);
  }
  if (kind === "test") {
    const [suite, test] = name.split("\\.");
    return blockRange(text, `^TEST(?:_F)?\\(\\s*${suite}\\s*,\\s*${test}\\s*\\)`, symbol);
  }
  if (kind === "field") {
    const m = new RegExp(`^[ \\t]*(?:(?:public|protected|private|static|final)\\s+)+[\\w<>]+\\s+${name}\\s*=`, "m").exec(text);
    if (!m) throw new Error(`${symbol} not found`);
    let i = m.index + m[0].length;
    while (text[i] !== ";") i = text[i] === '"' ? skipString(text, i) : i + 1;
    return [m.index, i + 1];
  }
  if (kind === "sql") {
    const start = text.search(new RegExp(`^CREATE TABLE ${name}\\b`, "m"));
    if (start < 0) throw new Error(`${symbol} not found`);
    return [start, text.indexOf(");", start) + 2];
  }
  throw new Error(`Unknown symbol kind: ${symbol}`);
}

function dedent(code) {
  const lines = code.replace(/\s+$/, "").split("\n");
  const indent = Math.min(...lines.filter((l) => l.trim()).map((l) => l.match(/^ */)[0].length));
  return lines.map((l) => l.slice(indent)).join("\n");
}

const lineOf = (text, index) => text.slice(0, index).split("\n").length;
const sources = new Map();
const read = (file) => {
  if (!sources.has(file)) sources.set(file, readFileSync(join(sourceRoot, file), "utf8").replace(/\r\n/g, "\n"));
  return sources.get(file);
};

const flows = config.flows.map((flow) => ({
  ...flow,
  steps: flow.steps.map((step) => {
    const text = read(step.file);
    const [start, end] = find(text, step.symbol);
    const name = step.symbol === "file" ? step.file.split("/").pop() : step.symbol.slice(step.symbol.indexOf(":") + 1);
    return {
      ...step,
      name,
      path: relative(projectRoot, join(sourceRoot, step.file)).split("\\").join("/"),
      startLine: lineOf(text, start),
      endLine: lineOf(text, end),
      code: dedent(text.slice(start, end)),
    };
  }),
}));

mkdirSync(dirname(output), { recursive: true });
writeFileSync(output, JSON.stringify({ flows }));
console.log(`codetour: ${flows.length} flows, ${flows.reduce((n, f) => n + f.steps.length, 0)} snippets`);
