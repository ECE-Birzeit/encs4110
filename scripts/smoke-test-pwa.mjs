// Smoke test: catches the exact regression fixed on 2026-08-29, where the
// service worker's update path was silently removed and new deploys stopped
// activating for visitors who already had the PWA installed. Run this after
// `npm run docs:build` -- it inspects the built output, not source files, so
// it verifies what actually gets deployed.

import { readFileSync, existsSync } from "node:fs";
import { fileURLToPath } from "node:url";
import path from "node:path";

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const distDir = path.join(repoRoot, "docs/.vitepress/dist");

let failures = [];

function fail(message) {
  failures.push(message);
}

function requireFile(relPath) {
  const full = path.join(distDir, relPath);
  if (!existsSync(full)) {
    fail(`Missing ${relPath} -- did you run "npm run docs:build" first?`);
    return null;
  }
  return readFileSync(full, "utf8");
}

// ---- Service worker: must self-activate, never wait on a message that's never sent ----
const swSource = requireFile("sw.js");
if (swSource) {
  if (!swSource.includes("self.skipWaiting()")) {
    fail(
      "sw.js never calls self.skipWaiting() -- a new service worker version " +
        "will sit in the \"waiting\" state forever instead of activating. " +
        "Check that registerType: \"autoUpdate\" is set in docs/.vitepress/config.mts."
    );
  }
  if (!swSource.includes("clientsClaim")) {
    fail(
      "sw.js never calls clientsClaim() -- an activated service worker won't " +
        "take control of tabs that are already open."
    );
  }
  if (swSource.includes("SKIP_WAITING")) {
    fail(
      "sw.js is back to gating activation behind a \"SKIP_WAITING\" postMessage " +
        "-- this is the exact broken pattern from commit 6420c87: it only " +
        "activates if something sends that message, and nothing in this app " +
        "does. registerType has likely reverted away from \"autoUpdate\"."
    );
  }
}

// ---- Manifest: must be valid, installable JSON ----
const manifestSource = requireFile("manifest.webmanifest");
if (manifestSource) {
  try {
    const manifest = JSON.parse(manifestSource);
    for (const field of ["name", "start_url", "display"]) {
      if (!manifest[field]) fail(`manifest.webmanifest is missing "${field}"`);
    }
    if (!Array.isArray(manifest.icons) || manifest.icons.length === 0) {
      fail("manifest.webmanifest has no icons");
    }
  } catch (e) {
    fail(`manifest.webmanifest is not valid JSON: ${e.message}`);
  }
}

// ---- Source-level guard: catch the regression before it even reaches a build ----
const configSource = readFileSync(path.join(repoRoot, "docs/.vitepress/config.mts"), "utf8");
if (!/registerType:\s*["']autoUpdate["']/.test(configSource)) {
  fail('docs/.vitepress/config.mts no longer sets registerType: "autoUpdate"');
}

const registerSWSource = readFileSync(
  path.join(repoRoot, "docs/.vitepress/theme/components/RegisterSW.vue"),
  "utf8"
);
if (!registerSWSource.includes("onNeedRefresh")) {
  fail(
    "RegisterSW.vue no longer defines onNeedRefresh -- this is the exact " +
      "callback commit 6420c87 removed, which is what broke auto-update."
  );
}

// ---- Report ----
if (failures.length > 0) {
  console.error(`PWA smoke test FAILED (${failures.length} issue${failures.length > 1 ? "s" : ""}):\n`);
  for (const f of failures) console.error(`  - ${f}`);
  process.exit(1);
}

console.log("PWA smoke test passed: service worker self-activates, manifest is valid.");
