# Translations

MeshMesh speaks English (`en`); this fork removed Russian and the 13 other upstream languages.
A new language (Dutch is planned) is added to the end of the language lists with its own files below.

| What | Source | Translations |
|---|---|---|
| Device screens (firmware) | `t("English")`, `tr("English")`, `count(n,"one","many")`, `N_("...")` in tables, in `src/` | `i18n/firmware/<lang>.json` |
| Website | `site/i18n/en.json` (and `ru.json`) | `site/i18n/<lang>.json` |
| Device web page and the site's chess page | the English text of `web/index.html` (`L(...)`, `plural(...)`, markup) | `i18n/web/<lang>.json` |
| Overview and detailed documents | `README.md`, `docs/*.md` | — (English only) |

The website's chess page (`tools/site/chess_site.py`) is the device page with `web/chess-companion.js`; it uses the
page's dictionaries.

```sh
python3 tools/gen/i18n.py sync        # after changing strings in src/: new ones appear empty, removed ones go
python3 tools/gen/i18n.py todo LANG   # what a language still lacks, with the code line it comes from
python3 tools/gen/i18n.py check       # everything below; the build of the site fails without it
.venv/bin/python tools/gen/i18n.py generate   # src/i18n/I18nTable.h and the glyph subsets in src/i18n/I18nFonts.h
```

`generate` needs the BDF fonts and u8g2 `bdfconv` in `upstream/fonts` (see `tools/gen/i18n_fonts.py`).
The glyph subsets come from Fusion Pixel Font (SIL OFL 1.1, licenses in `i18n/fonts/fusion-pixel/`),
ClearlyU 12 (`i18n/fonts/clearlyu-cu12.txt`) and the public-domain X11 misc-fixed fonts.
It also renders the anti-aliased Inter faces of the 320x240 screens (M9, T-Deck) into `src/ui/AaFonts.h`
(`tools/gen/aa_fonts.py`, Inter v4.1 TTFs in `upstream/fonts/inter`, SIL OFL 1.1 in `i18n/fonts/inter/OFL.txt`).
Run it after every translation change, then build the boards and look at the screens:
`.venv/bin/python tools/site/site_shots.py OUTDIR de` renders them on a computer.

## Firmware strings

Each entry of `i18n/firmware/<lang>.json` carries the English source and the translation:

```json
"5b0c2a7e": {"en": "Settings", "text": "Instellingen"}
```

The key is a hash of the English text, so editing it in the code makes a new entry
(and drops the old translation). `i18n/firmware/source.json` tells where each string is used and its
width: `cells` is the English text in 6-pixel columns.

- **Fit the screen.** The M9 screen is 320×240, the Heltec OLED 128×64 (21 columns).
  Keep a translation within `cells`; abbreviate when needed.
  `check` reports translations more than 30% longer.
- **Keep the edges.** Leading and trailing spaces, line breaks and placeholders (`%d`, `%s`,
  `{weekday}`, `{month}`, `{day}`) stay exactly as in English: the code glues these strings together.
- **Keep the names.** MeshMesh, MeshCore, LoRa, Wi-Fi, Bluetooth, BLE, GPS, USB, SD, SNR, RSSI, dBm, ACK, UTC,
  PIN, NTP, CSI, ESP-NOW, OSM, board names, and the device keys written on them (OK, BACK, DEL, HOME, MSG,
  MAP, CTRL, ADV, MIC, Enter, Sym, `@`) are not translated.
- **Plurals.** `forms` has the categories of the language (CLDR), e.g. `one`/`other` for Dutch (add them to
  `PLURALS` in `tools/gen/i18n.py` and a case in `category()` in `src/i18n/I18n.cpp` for other rules). Write `{n}`
  where the number goes: `"{n} partijen"`.
- **Dates.** `{weekday}, {month} {day}` is the order of the date line; month names are in the form that
  order needs.
- **Glyphs.** The firmware keeps only the Latin Extended glyphs the translations use, so `generate`
  must run after every change.
- **Tone.** Short, neutral interface language: infinitives or imperatives for actions, no exclamation marks.
  Address the user informally (Dutch: je/jij) unless a sentence needs otherwise.

## Device page and app

The page picks its language from `?lang=`, then the one chosen on the page (`mm-lang`), then the browser's,
and loads `i18n/<lang>.js` (boards: `/i18n/<lang>.js` from the firmware). English is the source and needs
none. `i18n/web/<lang>.json` maps the English text to the translation:

```json
"Delete channel “{0}”?": "Kanaal “{0}” verwijderen?",
"{n} message|{n} messages": {"one": "{n} bericht", "other": "{n} berichten"},
"_pieces": "K D T L P"
```

- **Placeholders.** `{0}`, `{1}`… are names, numbers or pieces of markup; keep every one, in any order.
  Plural forms (keys `one|other` from `plural(n,…)`) are objects with the categories of the language, as for
  the firmware, and `{n}` where the number goes.
- **Markup and edges.** Keep tags (`<b>`, `<small>`, `<code>`…) and entities, and the leading and trailing
  spaces or punctuation of a key (` · password saved` is glued to another text).
- **Words.** Use the terms of the device screens of the same language (`i18n/firmware/<lang>.json`) for
  apps, settings and actions. Names and keys stay as for the firmware. Short labels (buttons, tabs, compass
  points N E S W) stay short. `_pieces` are the piece letters K Q R B N of the chess notation.
- After changing page or app texts: `node tools/gen/web_i18n.cjs sync`, translate the new entries
  (`node tools/gen/web_i18n.cjs todo LANG`), then `node tools/gen/web_i18n.cjs check`. The tool needs
  `npm ci --prefix tools` once.

## Website

`site/i18n/<lang>.json` has the keys of `en.json`. Values may hold `<b>`, `<em>` and entities; keep the
same tags. The steps of the installer name buttons of the page (`btn.install`, `nrf.continue`), so use
the same words; the “Erase device” checkbox belongs to ESP Web Tools, which is English only, so keep its
English name. The page sets `dir="rtl"` for Arabic.

