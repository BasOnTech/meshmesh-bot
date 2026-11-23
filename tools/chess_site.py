#!/usr/bin/env python3
"""Build the site's chess page for a stock MeshCore companion: OUTDIR/chess/.

tools/chess_site.py OUTDIR   (tools/pages.py calls it for GitHub Pages)
The page is web/index.html, the boards' own page, with web/chess-companion.js and the tab icon
web/chess-icon.svg: the rules, the games
and the companion link run in the browser (docs/chess.md). Web Serial and Web Bluetooth need HTTPS,
so the page works from the site, not from a board's access point.
The page is in English, as the boards' page; a language added to i18n/web comes along as i18n/<lang>.js.
"""
import json
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
def build(out):
    target = Path(out)/'chess'
    target.mkdir(parents=True, exist_ok=True)
    page = (ROOT/'web/index.html').read_text()
    for marker in ('<title>MeshMesh</title>', '</script></body>', 'let companion=null;'):
        if page.count(marker) != 1:
            raise SystemExit(f'web/index.html: expected one {marker!r}')
    page = page.replace('<title>MeshMesh</title>', '<title>MeshMesh Chess</title><link rel="icon" href="icon.svg" type="image/svg+xml">')
    page = page.replace('</script></body>', '</script><script src="companion.js"></script></body>')
    (target/'index.html').write_text(page)
    # The page's own dictionaries (i18n/web), loaded by web/index.html as i18n/<lang>.js.
    (target/'i18n').mkdir(exist_ok=True)
    for f in sorted((ROOT/'i18n/web').glob('*.json')):
        d = {k: v for k, v in json.loads(f.read_text()).items() if (v if isinstance(v, str) else any(v.values()))}
        text = json.dumps(d, ensure_ascii=False, separators=(',', ':')).replace('<', '\\u003c')
        (target/'i18n'/(f.stem + '.js')).write_text(f'globalThis.MeshMeshWebDict={text};\n')
    shutil.copyfile(ROOT/'web/chess-companion.js', target/'companion.js')
    shutil.copyfile(ROOT/'web/chess-icon.svg', target/'icon.svg')
    return target


if __name__ == '__main__':
    print(build(sys.argv[1] if len(sys.argv) > 1 else 'artifacts/site'))
