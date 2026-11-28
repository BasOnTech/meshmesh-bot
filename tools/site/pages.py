#!/usr/bin/env python3
"""Assemble the GitHub Pages site: site/ plus browser-flashable firmware from tools/build/package.py packages.

tools/site/pages.py OUTDIR ENV...   (run tools/build/package.py ENV for each board first)
Each board gets firmware/<env>/manifest.json for ESP Web Tools and a zip of its package.
The browser installer writes the four components at their offsets, as an update with esptool
does: NVS (key, settings, contacts) and LittleFS (history) stay untouched unless the user
chooses to erase the device.

Device language: manifest-<lang>.json writes partitions-<lang>.bin, the partition table with
"MMLANG:<lang>" in the unused tail of its sector (0x8C00); the firmware applies it once
(src/i18n/I18n.cpp, Config::load).
Screenshots in every language come from tools/site/site_shots.py (artifacts/site-shots) and tools/site/web_shots.py
(artifacts/web-shots).
"""
import json
import hashlib
import re
import shutil
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'tools/build'), str(ROOT / 'tools/gen')]  # package.py, version.py; i18n.py
from package import TARGETS  # noqa: E402
from chess_site import build as build_chess  # noqa: E402
from i18n import CODES  # noqa: E402
from version import VERSION, check  # noqa: E402

LANG_AT = 0xc00  # in the partition table sector; the table itself ends before it


def lang_field(code):
    return f'MMLANG:{code}'.encode().ljust(16, b'\0')


# Boards whose packages are installed and checked on real hardware (docs/verification.md).
VERIFIED = {'m9', 'heltec_v4'}
README = """Murmur {version} — {board}

Update (keeps key, settings, contacts and history):
  python -m esptool --chip {chip} --port PORT write-flash --flash-mode dio --flash-freq {freq} --flash-size {size} \\
    {boot} bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin

Or in Chrome/Edge: https://basontech.github.io/meshcore-murmur-firmware/#install
"""


def main():
    out, envs = Path(sys.argv[1]), sys.argv[2:]
    check()
    version = VERSION
    if out.exists():
        shutil.rmtree(out)
    shutil.copytree(ROOT/'site', out)
    build_chess(out)  # chess/: the chess page for a stock MeshCore companion
    shots = ROOT/'artifacts'/'site-shots'
    if shots.exists():  # screenshots over the committed ones in site/img
        for f in shots.glob('*.png'):
            shutil.copyfile(f, out/'img'/f.name)
        # Every screen a feature card shows must have been made (tools/site/site_shots.py lists them by name).
        cards = (ROOT/'site'/'index.html').read_text().split('const FEATURES=')[1].split('];')[0]
        missing = sorted(n for n in set(re.findall(r"'((?:m9|heltec)-[a-z0-9-]+)'", cards)) if not (out/'img'/f'{n}-en.png').exists())
        if missing:
            raise SystemExit('feature screenshots missing (add them to tools/site/site_shots.py): ' + ' '.join(missing))
    web_shots = ROOT/'artifacts'/'web-shots'
    if web_shots.exists():  # the device page (tools/site/web_shots.py) over the committed ones
        for f in web_shots.glob('*.webp'):
            shutil.copyfile(f, out/'img'/f.name)
    boards = []
    releases = {}
    for env in envs:
        package = ROOT/'artifacts'/f'meshmesh-{TARGETS[env]}-{version}'
        meta = json.loads((package/'manifest.json').read_text())
        if meta['target'] != env:
            raise SystemExit(f'{env}: wrong board package')
        for filename, component in meta.items():
            path = package / filename
            if isinstance(component, dict) and 'sha256' in component and path.is_file():
                if hashlib.sha256(path.read_bytes()).hexdigest() != component['sha256']:
                    raise SystemExit(f'{env}: checksum mismatch for {filename}')
        if meta['version'] != version or not meta.get('revision'):
            raise SystemExit(f'{env}: stale package or missing source revision')
        releases[env] = {'version': version, 'revision': meta['revision'], 'sha256': meta['firmware.bin']['sha256']}
        if len({r['revision'] for r in releases.values()}) != 1:
            raise SystemExit('Packages from different source revisions cannot be published together')
        board_json = json.loads((ROOT/f'boards/meshmesh_{env}.json').read_text())
        chip = 'esp32s3'
        size = board_json['upload']['flash_size']
        boot = 0
        loader = (package/'bootloader.bin').read_bytes()
        # The image header must say DIO (byte 2 == 2): QIO boot-looped these boards.
        if loader[0] != 0xE9 or loader[2] != 2:
            raise SystemExit(f'{env}: bootloader is not a DIO image')
        target = out/'firmware'/env
        target.mkdir(parents=True)
        parts = [('bootloader.bin', boot), ('partitions.bin', 0x8000), ('boot_app0.bin', 0xe000), ('firmware.bin', 0x10000)]
        for name, _ in parts:
            shutil.copyfile(package/name, target/name)
        name = board_json['name'].removeprefix('MeshMesh / ')
        manifest = {'name': f'Murmur — {name}', 'version': version, 'new_install_prompt_erase': True,
                    'builds': [{'chipFamily': 'ESP32-S3',
                                'parts': [{'path': n, 'offset': o} for n, o in parts]}]}
        (target/'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
        table = (package/'partitions.bin').read_bytes()
        if len(table) > LANG_AT or table[LANG_AT - 32:].strip(b'\xff'):
            raise SystemExit(f'{env}: partition table reaches the language field')
        for code in CODES:
            (target/f'partitions-{code}.bin').write_bytes(table.ljust(LANG_AT, b'\xff') + lang_field(code) + b'\xff'*(0x1000 - LANG_AT - 16))
            localized = json.loads(json.dumps(manifest))
            assert localized['builds'][0]['parts'][1]['path'] == 'partitions.bin'
            localized['builds'][0]['parts'][1]['path'] = f'partitions-{code}.bin'
            (target/f'manifest-{code}.json').write_text(json.dumps(localized, indent=2) + '\n')
        archive = out/'firmware'/f'meshmesh-{TARGETS[env]}-{version}.zip'
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
            for f in sorted(package.iterdir()):
                z.write(f, f'{package.name}/{f.name}')
            if not (package/'INSTALL.txt').exists():
                z.writestr(f'{package.name}/README.txt', README.format(
                    version=version, board=name, chip=chip, size=size, boot=hex(boot),
                    freq='80m'))
        boards.append({'env': env, 'name': name, 'chip': manifest['builds'][0]['chipFamily'], 'flash': size,
                       'verified': env in VERIFIED, 'community': False, 'langs': True,
                       'bytes': meta['firmware.bin']['bytes'], 'zip': f'firmware/{archive.name}'})
    (out/'firmware/boards.json').write_text(json.dumps({'version': version, 'boards': boards}, ensure_ascii=False, indent=2) + '\n')
    (out/'release.json').write_text(json.dumps({'firmware': version,
        'revision': next(iter(releases.values()))['revision'], 'boards': releases}, indent=2) + '\n')
    print(out, len(boards), 'boards')


if __name__ == '__main__':
    main()
