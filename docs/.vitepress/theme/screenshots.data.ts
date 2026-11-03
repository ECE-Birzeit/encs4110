// Lists the screenshots that actually exist under public/screenshots, so the
// <Screenshot> component can show the real image when it is there and a
// labelled placeholder when it is not. Resolved at build time (and re-run in
// dev when a file is added), so there is no broken-image flash at runtime.
import { relative } from "node:path";
import { fileURLToPath } from "node:url";

const root = fileURLToPath(new URL("../../public/screenshots/", import.meta.url));

export default {
  watch: ["../../public/screenshots/**/*.{png,jpg,jpeg,webp}"],
  load(files: string[]): string[] {
    return files.map((f) => relative(root, f).split("\\").join("/"));
  },
};
