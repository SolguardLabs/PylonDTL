import { readFileSync, readdirSync, statSync } from "node:fs";
import { join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const root = resolve(fileURLToPath(new URL("..", import.meta.url)));
const ignored = new Set([".git", "dist", "build", "node_modules", "coverage"]);
const allowed = new Set([
  ".cpp",
  ".hpp",
  ".js",
  ".json",
  ".md",
  ".sh",
  ".yml",
  ".yaml",
  ".gitignore",
  ".prettierignore",
  ".prettierrc",
  ".editorconfig",
  "Makefile",
]);

function extOf(name) {
  if (allowed.has(name)) {
    return name;
  }
  const index = name.lastIndexOf(".");
  return index === -1 ? "" : name.slice(index);
}

function walk(dir, out = []) {
  for (const name of readdirSync(dir)) {
    if (ignored.has(name)) {
      continue;
    }
    const path = join(dir, name);
    const stat = statSync(path);
    if (stat.isDirectory()) {
      walk(path, out);
    } else if (allowed.has(extOf(name))) {
      out.push(path);
    }
  }
  return out;
}

let failed = false;
for (const file of walk(root)) {
  const text = readFileSync(file, "utf8");
  if (text.includes("\r\n")) {
    console.error(`CRLF line endings: ${file}`);
    failed = true;
  }
  if (!text.endsWith("\n")) {
    console.error(`Missing final newline: ${file}`);
    failed = true;
  }
  const lines = text.split("\n");
  lines.forEach((line, index) => {
    if (/[ \t]+$/.test(line)) {
      console.error(`Trailing whitespace: ${file}:${index + 1}`);
      failed = true;
    }
  });
}

if (failed) {
  process.exit(1);
}
