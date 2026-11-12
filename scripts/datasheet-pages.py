"""Register -> page map for the local datasheet copy, read by .vitepress/registers.mts.

    pdftotext -layout public/datasheets/tm4c123gh6pm.pdf /tmp/ds.txt
    python3 scripts/datasheet-pages.py /tmp/ds.txt > .vitepress/datasheet-pages.json

Each register's description starts with a heading like
"Register 4: GPTM Control (GPTMCTL), offset 0x00C"; the first one after the table of contents
(front matter ends at page 44) gives the page. Printed page numbers equal PDF page indexes.
"""
import json
import re
import sys

pages = open(sys.argv[1], encoding="utf-8").read().split("\f")
found = {}
for i, text in enumerate(pages):
    if i < 44:
        continue
    for name in re.findall(r"\(([A-Z][A-Za-z0-9_]+)\),\s+offset\s+0x", text):
        found.setdefault(name, i + 1)
json.dump(found, sys.stdout, indent=0, sort_keys=True)
print()
