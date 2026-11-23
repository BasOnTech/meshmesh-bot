#!/usr/bin/env python3
"""Reject stale/mixed site metadata before publishing GitHub Pages."""
import json
import os
import sys
from pathlib import Path
from package import TARGETS
from version import VERSION

root = Path(sys.argv[1])
release = json.loads((root / 'release.json').read_text())
if release['firmware'] != VERSION:
    raise SystemExit('Site versions differ from canonical versions')
if set(release['boards']) != set(TARGETS):
    raise SystemExit('Missing board')
if os.environ.get('GITHUB_ACTIONS') == 'true' and release['revision'] != os.environ['GITHUB_SHA']:
    raise SystemExit('Site does not match this clean commit')
print('PASS complete site, consistent versions/revisions')
