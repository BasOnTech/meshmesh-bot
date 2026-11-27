# Working on MeshMesh

## Purpose and communication

Firmware for the Elecrow ThinkNode M9 and the Heltec V4. This fork builds only these two; upstream MeshMesh also built
for nRF52 and other ESP32 boards. Features, with their main source and document:

- Chats and channels (hashtag, link/QR, key, invitations) — `src/mesh/Channels.cpp`, `docs/channels.md`.
- Regions (default and per channel, search at neighbouring repeaters, companion commands 54/55/63/64) —
  `src/mesh/Regions.cpp`, `src/mesh/RegionSearch.inc`, `docs/regions.md`, check `tools/hw/region_check.py`.
- Radio settings test and previous profile — `radio try|keep|revert|previous`, `renderNet` in `web/index.html`.
- Offline maps on the SD (relief and contour lines from Terrain Tiles during preparation; node map without a base
  layer — `drawMap` in `web/index.html`) — `docs/maps-format.md`.
- Signal radar and Wi-Fi CSI motion sensor — `src/apps/Radar*.cpp`.
- Chess with contacts over MeshCore direct messages: rated ELO games signed by both nodes' keys
  (`src/games/chess/ChessRating.cpp`), Swiss tournaments (`src/games/chess/ChessTour.cpp`), `docs/chess.md`. The site
  page `chess/` plays through a stock MeshCore companion over Web Serial/Bluetooth — `web/chess-companion.js`,
  `tools/site/chess_site.py`.
- Draughts and battleship, unrated — `src/games/duel/` (`Checkers.cpp`, `Sea.cpp`, `Duel.cpp`, `UiDuel.inc`,
  `UiDuelCompact.inc`), `docs/games.md`, check `tests/duel_check.py`. Menu folders ("Games") — `src/core/Apps.cpp`.
- Network pet that feeds on radio traffic — `src/games/pet/`, `docs/pet.md`.
- Dice (DIC3R): RPG rolls and formulas, Warhammer grid, character sheets, D&D and R.E.T. templates, sending a sheet —
  `src/games/dice/`, `docs/dice.md`.
- A message's packet path (repeaters by path hash, SNR/RSSI, repeats of our own messages) — `MeshRadio::echo`,
  `renderMsg` in `web/index.html`.
- Notifications (kinds, mentions, per-chat settings, quiet hours, feed) — `src/ui/Notifications.cpp`, `alertFor` in
  `web/index.html`, `docs/notifications.md`.
- Repeater and room modes (chosen at boot) — `src/mesh/MeshServer.cpp`, `docs/repeater.md`.
- Wardriving (packet log with position, `#wardrive` pings with repeater echoes, CSV/GeoJSON/KML/WiGLE export) —
  `src/apps/Wardrive.cpp`, `docs/wardrive.md`. Record only with a trusted position (`gpsFix` or a `fix` command).
- Backup (settings, node key, channels, contacts, quick send, counters, sheets in one JSON; selective restore) —
  `backup*` in `web/index.html`, `docs/backup.md`.
- Diagnostics, the screen UI (emoji — `src/ui/Emoji.cpp`, `tools/gen/emoji.py`), the device web page and the site.

Communicate with the user in English. Carry agreed changes through to build, installation and verification when the
devices are available. Distinguish between an implemented feature, a successful build, installed firmware and
confirmed hardware operation.

Current results and limitations are in `docs/verification.md`, `artifacts/release-check.json` and
`artifacts/hardware-finish.json`. Do not present old reports or an identical version string as verification of a new
package.

## Repository

GitHub: this fork, `BasOnTech/meshmesh-bot`, of the public `IKrasnodymov/meshmesh` (MIT; `README.md` — the overview,
`docs/` — the detailed documents, all in English), main branch `main`. The site is GitHub Pages from `site/`
(https://basontech.github.io/meshmesh-bot/): `.github/workflows/pages.yml` builds all boards and `tools/site/pages.py`
(ESP Web Tools browser-install manifests, writing at offsets without erasing NVS). The link preview image
`site/img/og.png` — `tools/site/og_image.py` (from `site/img/m9-home-en.png`; run by hand, not in CI). Screen
screenshots — `tools/ui_preview`; the interactive simulator in an SDL2 window — `tools/ui_preview/sim.sh [m9] [LANG]`
(`SIM=1` in `build.sh`, code in `tools/ui_preview/live.inc`; rebuilds via fswatch).

Keep sources, vendored MeshCore with its licences, tools and documentation in Git. `backups/`, `artifacts/`, `logs/`,
`vendor-cache/`, the Python environment, build outputs and `.env` stay local and are excluded via `.gitignore`.
The repository is public: do not add flash/NVS dumps, chat history, private keys, Wi-Fi passwords, BLE PINs, real node
addresses or coordinates. Check the staged files before committing.

## Commits

Commit messages follow [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/):
`<type>(<optional scope>): <summary>`, e.g. `fix(ui): keep the cursor on the selected channel`.
Types: `feat`, `fix`, `refactor`, `perf`, `build`, `ci`, `docs`, `test`, `style`, `chore`; a breaking change
(storage format, protocol, NVS keys) gets `!` after the type and a `BREAKING CHANGE:` footer.
Keep the summary imperative and under ~72 characters; explain the why in the body. Split unrelated
changes into separate commits (e.g. build flags apart from the code fixes they need).

## Devices

- M9: ESP32-S3, 16 MB flash, 8 MB PSRAM, LR1110, 320×240 screen, keyboard, SD, RTC, GPS, IMU, magnetometer.
  PlatformIO target `m9`.
- Heltec V4: ESP32-S3, 16 MB flash, 2 MB PSRAM, SX1262, 128×64 OLED and the PRG button. Target `heltec_v4`.
- The `heltec_v4_r8` target is for another revision with 8 MB PSRAM; a successful build does not mean that revision
  was checked on hardware.
- Shared code distinguishes the M9 from the Heltec by the family (`MM_COMPACT`, set together with `MM_HELTEC_V4`),
  the transceiver (`MM_RADIO_*`), native USB (`MM_NATIVE_USB`) and one button (`MM_ONE_BUTTON`). Pins —
  `src/hw/BoardPins.h`, board properties — `src/hw/Board.h`; the Heltec — `src/hw/HardwareHeltec.cpp` with
  `src/ui/UiHeltec.cpp`, the M9 — `src/hw/Hardware.cpp` with `src/ui/Ui.cpp`.
- Find the ports with `tools/hw/ports.py` (by USB bridge). Check that the devices are present and are the expected
  ones; port names change after reconnecting.
- Preserve the user's radio profile and per-board power, hops and relay settings on update; checks compare only the
  profile between boards.

## Protocol and compatibility

MeshMesh uses MeshCore from 0.3.0 onwards (`docs/meshcore-migration.md`, `lib/MeshCore`). The archived 0.2.0 used
MM/1 (`lib/MeshProtocol/Mm1Packet.h`). Meshtastic is not supported.
Since 0.9.0 the board implements the MeshCore companion protocol itself (`src/mesh/Companion.inc`,
`docs/companion.md`): BLE Nordic UART and the name `MeshCore-<name> XXXX` (4 characters of the key) always (our own
service still answers; the advertisement carries the `MM` tag of company 0xFFFF), USB frames `<`/`>`; login to
repeaters and rooms, CLI, status, telemetry, room posts, tracing and path discovery — also on the screen, the web page
and over USB (`remote`, `src/mesh/Remote.h`, `src/mesh/Remote.inc`, M9 — `src/ui/UiRemote.inc`); no TCP. Requests to a
server need a clock later than the previous request (replay protection): set the board's clock in checks (`set_time`).
Matching frequency and other radio parameters does not make the packet, advert, address, key, message and
acknowledgement formats compatible.

WadaMesh uses the MeshCore core. Do not present WadaMesh as a separate protocol incompatible with MeshCore. Keep
original headers and source attributions. Our changes to the vendored MeshCore are listed in
`lib/MeshCore/PATCHES.md`.

When adding compatibility, keep our UI, maps and hardware drivers; change the network layer deliberately. Verify new
compatibility with a real third-party node: adverts both ways, addresses/contacts, messages, keys and ACKs.
Detecting a packet or radio signal is not proof that messages can be exchanged. Do not promise simultaneous discovery
of all networks: each device has one LoRa transceiver, different radio profiles require switching.

## Code and build

- `src/` — the application and drivers, each header next to its source, grouped by feature: `core/` (app, config,
  storage, power, clock, version), `i18n/`, `hw/` (boards, pins, displays), `mesh/` (radio, server roles, channels,
  regions, companion, remote, history), `ui/` (screens, icons, navigation, notifications, emoji), `net/` (Wi-Fi,
  portal, diagnostics), `apps/` (maps, radar, wardrive, counter), `games/<game>/` (chess, dice, duel, pet, solitaire:
  logic and its `Ui*.inc` screens together). Includes are written from `src/`: `#include "games/chess/Chess.h"`.
- The M9 screen: `src/ui/Ui.cpp` keeps what the pages share (theme, text, drawing primitives, footer and touch
  targets, read marks and drafts, `change()`, the status bar) and the dispatch (`draw()`, `uiKey()`, `uiTouch()`,
  `uiTick()`). Each page's drawing and its keys are in `src/ui/Ui<Page>.inc`: `UiHome`, `UiChat`, `UiMap`, `UiNodes`,
  `UiSensors`, `UiRadar`, `UiSettings`, `UiWardrive`, `UiQuick`, `UiNetwork`, `UiSystem` (diagnostics, help, lock
  screen), `UiServer`, `UiChannels`, `UiRemote`; the games' screens are next to the games. Ui.cpp includes them inside
  its anonymous namespace, so they share its state; the order matters (a page can use what comes before it; for later
  code it forward-declares, as `UiNodes.inc` does for `openPick`). `uiKey()` keeps the order in which keys are tried
  and calls the page's handler (`chatKey`, `nodesKey`, `settingsEnter`...): add a page's keys there, not inline.
- The Heltec one-button UI (`src/ui/UiHeltec.cpp`): screens where a click does something (quick send, the counter,
  dice) are closed while paging through — a click goes to the next screen, a hold opens it (`appOpen`); add new such
  screens to `appPage()`. With `config.doubleBack` (NVS `dbl_back`, off by default) a click waits 0.4 s and a second
  click is `stepBack()`: add new nested states to `stepBack()`.
- `web/index.html` — the source of the web interface. The firmware serves it and the dictionaries compressed (gzip):
  `tools/build/pio_web.py` generates `PortalWeb.h` in the build folder at build time; it is not in Git. The page is
  English; a further language's dictionary is packed with its keys once (`/i18n/keys.js`) and `/i18n/<lang>.js`
  carrying the translations in order (`MeshMeshWebDict`). Page text — `L('…')`/`` L`…${x}` `` and `plural(n,…)`
  (section "Languages"). Do not rebuild sections redrawn on a timer under a focused input field. Demo without a
  board — `tools/site/web_demo.py`.
- `boards/`, `platformio.ini`, `partitions.csv` — hardware targets and build.
- `tools/` — by purpose: `build/` (version, PlatformIO scripts, packaging), `gen/` (generated headers and
  dictionaries: i18n, fonts, emoji, pet sprites), `site/` (GitHub Pages, screenshots, web demo), `hw/` (USB and BLE
  access, flashing, bridges, maps upload and every check on real boards; they import each other), `ui_preview/` (host
  UI and simulator). A tool that imports from another folder adds it to `sys.path`
  (e.g. `tools/hw/flash.py` → `tools/build/version.py`). Run them from the repository root.
- `tests/` — checks that run on the computer alone, against the real sources: `protocol/` (MM/1 packets), `chess/`
  (rules, rated results, the companion page against `Chess.cpp`), `duel_check.py` (checkers and sea battle), `web/`
  (the device page's chat filter and map codec, on data taken from a board). `sh tests/run.sh` runs all of them but
  `web/` (needs `pio run -e m9` first for the libraries).
- Dependencies are pinned in `platformio.ini`; change them only when necessary.

```sh
.venv/bin/pio run -e m9 -e heltec_v4 -e heltec_v4_r8
.venv/bin/python tools/build/package.py m9
.venv/bin/python tools/build/package.py heltec_v4
.venv/bin/python tools/build/package.py heltec_v4_r8
```

Build the affected boards. Repeat and extend checks for the errors and changes found; do not add tests that merely
duplicate the implementation.

## Colours and themes

- The colour screen UI (`src/ui/Ui.cpp` and the `Ui*.inc` it includes) takes its colours from a theme:
  `src/ui/Theme.h` declares the `Theme` struct and `themes[]` (the first is the default), each theme is a file in
  `src/ui/themes/` (Midnight is the only one so far). `applyTheme()` in `Ui.cpp` converts the chosen theme to the
  RGB565 globals the UI draws with; `theme->…` keeps the `0xRRGGBB` values for `mix()`. The Heltec mono UI
  (`UiHeltec.cpp`, `*Compact.inc`) does not use themes.
- Colours are named by role, never by hue or a vague word (no `violet`, `pink`, `ink`, `dim`, `card`). Hue names live
  only in a theme file's own palette namespace (`midnight::violet`), and the theme assigns them to roles.
- The roles: surfaces `surfacePage`, `surfaceBar` (status and key bars), `surfaceSubbar` (a page's own bars around
  scrolling content: a chat's subtitle and composer), `surfaceCard`, `surfaceFocus` (focused row), `surfaceWell`
  (inset: icon squares, input fields), `border`, `gridLine`; foreground per kind, each with `Primary`/`Muted`/`Subtle`
  — `text*` for text, `icon*` for `icon()` and arrow glyphs, `graphic*` for drawn shapes (lines, knobs, bars, chart
  strokes, the toast's colour bar); `onFill` for text and glyphs on a coloured fill (badges, buttons, toggle knob,
  checkmarks); `accent`, `secondary`, `tertiary` for categories; state `info`, `ok`, `warn`, `alert` (between warn and
  bad), `bad`; `*Deep` for the dark tint behind a role's icon or text; `avatarHues[]`.
- Pick the role by what is drawn and on what: text on a panel — `text*`; an icon's hole is the colour behind it
  (`surfacePage` on the page, `surfaceWell` on a well); content on an `accent` or state fill — `onFill`.
- No `rgb(0x…)` for UI chrome: use a role, or add one to `Theme.h`, to every theme file (designated initializers in
  the struct's order) and to `applyTheme()`. Game art (chess board and pieces, cards, dice and pet colours, emoji)
  keeps its own colours.
- Check a colour refactor (or any refactor meant to leave the screens unchanged) on screenshots: render before and
  after with `tools/site/site_shots.py` or `tools/ui_preview/build.sh m9 OUTDIR` to two folders and compare the pixels.
  Mask out the clock first: the status bar (rows 0–20 of the 320×240 screen) shows the time on every screen, so a
  plain comparison reports every screen as changed. Even then, screens with message times, the lock-screen clock or
  game ages (chats, threads, chess list, locked) differ between two runs of the same code: render the "before" twice
  to see which screens differ by themselves, and check that the "after" differs only in those places.

## Versions and publishing

The public firmware version is SemVer; the single source is `versions.properties`. After a change:
`python tools/build/version.py --generate`, then `--check`; `src/core/Version.h` is generated and committed. Details —
`docs/releases.md`. The firmware reports `version`, `revision` (Git SHA + `-dirty` locally), `build_sha256`; packages
and the site include the revision and hashes. Do not treat an identical SemVer as proof of an identical image.
A push to `main` runs `.github/workflows/pages.yml`: version and translation checks (screens, page, chess), all boards
and language packages, screenshots, tests, the site, then Pages. Any required failure blocks publishing. After
pushing, check that Actions finished and check the public `release.json`. The build/deploy does not flash boards.
Update the documentation and these instructions by hand from actual results before committing.

## Languages

Firmware screens — English (`src/i18n/I18n.h`; the language number is stored in NVS, do not change the order;
upstream Russian and 13 other languages were removed, their stored numbers fall back to English). English is written
in the code: `t("English")`, `tr("English")`, `count(n,"one","many")`; strings of a table are marked with `N_("...")`
and read with `trText()`. A further language (Dutch is planned) goes to the end of `Lang` with
`i18n/firmware/<lang>.json` following `i18n/README.md`: `tools/gen/i18n.py sync`, translate,
`.venv/bin/python tools/gen/i18n.py generate` (the table; the Latin Extended font subsets need `vendor-cache/fonts`),
`tools/gen/i18n.py check`, build, and review the screens with `tools/site/site_shots.py OUTDIR <lang>`.

Site — `site/i18n/en.json`; the detailed documents in `docs/` are English. The device web page — English: the English
text in the code is the dictionary key of `i18n/web/<lang>.json` for a further language; in `web/index.html` write
`L('…')`, `` L`…${x}` `` (do not glue a phrase together from pieces), `plural(n,'{n} game','{n} games')`. Then
`node tools/gen/web_i18n.cjs sync`, `node tools/gen/web_i18n.cjs check` (needs `npm ci --prefix tools`). The page
language — `?lang=`, chosen on the login screen/in "Settings" (`mm-lang`), otherwise the browser language. Page
screenshots for the site — `tools/site/web_shots.py` (demo data `tools/site/web_demo.py`, in CI); `site/img` holds the
fallbacks. The site's chess page (`chess/`) is the same page in English and gets the same dictionaries.
The site installs the firmware with the chosen language: `partitions-<lang>.bin` with a tag at 0x8C00; the tag is
applied once. `src/core/Modules.h` (`MM_CHESS`, `MM_PET`, `MM_DICE`, `MM_WARDRIVE`, `MM_CHECKERS`, `MM_SEA`): both
boards build every module. Cyrillic fonts stay in all images: Cyrillic messages arrive over the radio.

## Installation and data safety

Both original flashes are saved and verified in `backups/` together with metadata. Before writing, verify the full
backup, the model, MAC and component digests with `tools/hw/flash.py`. Use DIO, 80 MHz, 16 MB and the intended
offsets of the four components. QIO previously caused a boot loop on these boards.

Preserve history, the network key, NVS, calibration and SD cards. NVS is 20 KB and is shared with Wi-Fi and BLE: keep
large and frequently changing data (contacts `/meshmesh/contacts.bin`, read markers `/meshmesh/read.bin`, history) in
LittleFS; the headroom is visible in `status` (`nvs_free`, `contacts_saved`). Do not format the SD or reset settings
as part of a normal update. Keep backups, files with keys, passwords, PINs and histories with 0600 permissions; do not
print secrets to a shared log or report. Authorisation for the necessary updates holds within the agreed task; do not
ask for it again without a new reason.

```sh
.venv/bin/python tools/hw/finish_on_hardware.py --check
.venv/bin/python tools/hw/finish_on_hardware.py
```

The first mode checks the packages without USB. The second installs and verifies both boards in sequence, stopping
on an error. Run the full pass (20–30 min) for the final package, not after every edit: while debugging, flash the
affected board with `tools/hw/flash.py` and do targeted checks, stating explicitly in the report that the full pass
was not run. Individual steps are written to `logs/finish-*.log`, previous reports are archived. Report the cause of
an error and the actual state of the installed devices precisely. If the environment forbids USB
(`Operation not permitted`), do not bypass the access restrictions.

## Operation and hardware verification

The radio keeps being serviced during maps, USB downloads and the lock screen. Split long operations; watch memory,
lengths, CRC, UTF-8, timeouts and atomic saving. Show the DELIVERED state only after a matching ACK; do not treat a
send left unfinished across a restart as delivered; the absence of a repeat is not proof of non-delivery. The input
language is independent of the interface language.

For an STA client with BLE enabled, keep Wi-Fi modem sleep enabled: `WiFi.setSleep(true)`. Disabling it caused a
confirmed `abort()` in the ESP32-S3 driver (`logs/heltec-wifi-reset-trace.log`). Do not mask the problem by disabling
BLE in the interface coexistence check.

One process owns each USB port. Other sessions may be working in the same working tree: coordinate USB ports and
shared files with them (especially `src/ui/Ui.cpp`) and do not build packages from someone else's unfinished edits.
Do not start several reads or a flash on top of an active check of the same port. Use `tools/hw/device.py`: it
handles DTR/RTS and the accumulation of partial native-USB replies on the Heltec.

LoRa and BLE checks must confirm a real radio exchange, growth of RX/TX, no restarts, no USB injections and a single
copy of the message. Boot numbers in reports must match the installation being checked. USB key events check the
production UI, but not the physical buttons. Node JavaScript checks do not replace a visual check in the browser.

## Power saving

The shared code waits up to 20 ms for the LoRa IRQ notification with the screen off and no active send, radar/CSI or
probes, in all roles. After a USB/BLE command or a button press the fast loop is held for 1 s. `idle_waits`,
`idle_wait_ms`, `idle_radio_events` measure the task's waiting, not CPU sleep or current. Check —
`tools/hw/power_check.py --port PORT --peer PORT`; when checking a direct message, do not poll the receiver's USB until
the ACK. After a message the screen may turn off again on its timer — a late read of `screen_off` does not prove there
was no wake-up. `docs/power.md`; RXPS and cyclic GPS power are not implemented yet.
Battery voltage on the Heltec V4: 2.5 dB attenuation and a 4.9 × 1.045 divider. Check new multipliers against the
owner's voltmeter, not against another firmware; not yet checked on hardware.

`src/core/Power.cpp`: 80 MHz with the screen off and no Wi-Fi/radar, light sleep in repeater and room modes
(`docs/repeater.md`). The transceiver's IRQ line has a RadioLib edge interrupt: set the level type for wake-up only
inside a critical section and restore `GPIO_INTR_POSEDGE` after sleep — otherwise an interrupt storm (interrupt
watchdog on the M9) or lost reception. Bytes that woke the board over USB-UART are lost: send a CR before a command
(`tools/hw/device.py`). Do not change the frequency while the Wi-Fi driver or the Bluetooth controller is running:
after CSI this caused an M9 panic while loading a map.

Power results in `docs/verification.md` are tied to particular images: do not carry them over to other images by
version number alone. History is serialised one entry at a time; running out of memory returns an explicit error, and
`status.history_count` lets you check completeness. After switching from repeater to normal mode, the hardware test
waits for an advert of type CHAT, not just the key.

## Radar and Wi-Fi CSI

`src/apps/Radar*.cpp`: a Wi-Fi/BLE/LoRa sweep, RSSI direction finding and a CSI motion sensor (ESP-NOW beacon ↔
receiver). Rules found on hardware:
- The radar uses Wi-Fi only while it is open and only with the access point and the Wi-Fi probe off;
  `portalToggle()` calls `radar.release()`.
- Do not deinitialise the Bluetooth controller after the first start (`NimBLEDevice::deinit`): deinitialising right
  after stopping the scan caused an M9 panic in the NimBLE task, and deinitialising with a restart "froze" CSI data
  until reboot. "BLE off" means no advertising and no connections. The message
  `wifi:timeout when WiFi un-init, type=4` when turning Wi-Fi off with a live controller caused no failures.
- Wi-Fi scanning goes through `esp_wifi_scan_start` with a zeroed configuration: Arduino 2.0.17's `scanNetworks()`
  does not fill `home_chan_dwell_time`.
- The CSI beacon: ESP-NOW, OFDM 6 Mbit/s, channel 1 (MCS0 frames did not go on air); when stopping the beacon restore
  the default rate, otherwise CSI on reception on the same board "freezes".
- USB `radar` does not output other people's network names or Bluetooth addresses; `radar web` outputs the names,
  like the web page, and holds the radar for 10 s, like `/api/radar`.
- Checks: `tools/hw/radar_check.py`, `tools/hw/csi_check.py` (real radio reception). Motion detection is checked only
  with a person walking between the boards; the automated check confirms only the stream and the change in CSI.

RSSI gives neither direction nor distance; phones' BLE addresses change, the device count is approximate. CSI between
two boards detects motion in the zone between them, but not where people are. The firmware does not measure
breathing or heart rate; locating people without devices would require three or more nodes and training on a grid of
points.

## Maps, GPS and the limits of verification

Map packages — `artifacts/maps/*.mmmap`. Keep the map, the catalogue and the previous intact tiles on corrupted or
interrupted downloads. The M9 access point does not provide internet. The M9 as a Wi-Fi client (`src/net/Internet.cpp`,
"Network → Internet over Wi-Fi") downloads missing OSM tiles, NTP time and IP-based position itself, caching tiles on
the SD. The client yields Wi-Fi to the access point, the radar and the Wi-Fi probe (`internet.yieldRadio()` before a
mode change) and reconnects after them. `tools/hw/maps.py` does not draw relief. Overpass cycles through mirrors: the
main server often answers 504/429.

GPS can receive a spoofed signal (a multi-satellite fix with a wrong date has been seen). GPS/RTC time earlier than
the build date is rejected (`src/core/Clock.cpp`, `gps_conflict`, the position is not used); GPS does not overwrite
time from the phone/NTP; such a date must not be taken as a current position, used for following, sending coordinates
or overwriting trusted time. ESP32 time survives a restart (not a power-off) and is saved only with a known source;
messages recorded before the clock was set get their time when it is set (`MeshRadio::stampLate`). A saved compass
calibration does not prove accuracy or axis orientation.

Repeater and room modes (`src/mesh/MeshServer.cpp`, `docs/repeater.md`) are checked from the side of the official
MeshCore companion: `tools/hw/repeater_check.py`, `tools/hw/room_check.py`. For this the Heltec is temporarily
flashed with the stock companion: MeshCore `companion-v1.17.1`, built as `meshmesh_stock_interop` for our flash layout
(`tools/hw/stock_roundtrip.py` expects the checkout in `vendor-cache/meshcore-stock`). Order: JSON snapshots and a full
flash copy in `backups/` with `verify-flash`, the checks, then the copy is written back and verified. The stock
firmware formats our LittleFS, so do not install it without a copy. `tools/hw/stock_roundtrip.py` is part of the full
pass: stock protocol/sources v1.17.1, DIO/dio_qspi, a matching bootloader and partition table (only the LittleFS label
is replaced with SPIFFS, the geometry is the same). The first start can take more than 45 s; the tool waits for USB to
be ready. Restoring the full 16 MB copy is verified byte by byte before starting. Restoring from the bootloader uses a
watchdog reset. While the board is in server mode, its normal-mode contacts are not updated.

What is and is not confirmed on hardware (GPS under open sky, compass accuracy, LoRa range, radar direction finding,
CSI threshold, region forwarding, language install from the site, …) is tracked in `docs/verification.md`. Update it
and this file from actual results, not assumptions.

## Feature notes

Rules from earlier releases that still apply to the M9 and Heltec:

- Notifications and quick send (`src/ui/Notifications.cpp`, `src/mesh/QuickSend.cpp`, `src/mesh/ChannelPolicy.cpp`,
  `src/mesh/History.cpp`, `src/mesh/HistoryStorage.cpp`): an independent event queue, 10 templates with a recipient by
  ID; the recipient is a separate tile at the top of the list. Batched `Preferences` writes apply only inside
  `Config::save`; replay-protection timestamps are saved synchronously.
- Quick counter (`people::Tally` in `src/apps/People.h`, file `/meshmesh/tally`, M9 — `src/apps/UiTally.inc`, web —
  `renderPeople`/`tallyAct`): up to 8 cards, the open card is shared by the screen and the page. The app id and
  commands stay `people`. The default order is `uiApps`; never reorder a user's saved `apps`. `path_hash` defaults to
  2 (boards with 1 switch once, NVS `path_hash2`); the repeater and room take the same setting.
- Character sheet (`src/games/dice/Dice.h`, `drawDiceBlock` in `src/games/dice/UiDice.inc`): blocks (stat, gauge,
  marks, ability, note, header; up to 64) with formulas referencing block codes (`@dex`, `@dex!`); one
  `/meshmesh/sheetN.bin` per character, only the current one in memory; `dice.bin` version 3 (version 2 is read). The
  sheet string `MMS1~…` is sent in parts `🎲S ID K/N` as direct messages with ACK. BLE writes are at most 255 bytes:
  long strings use `dice sheet put K/N`. Browser check: `tools/ui_preview/build.sh` with `KEEP=PATH` keeps the
  program, `PATH --dice` answers `dice` commands from stdin (`loopback` returns the sent parts as received). The
  builder is on the page (`sheetBuild`, commands `dice sheet set/add/move/del`).
- Radio test (`Config::previous`, NVS `radio_prev`; `Config::trial`: `Config::save` writes the saved profile, a
  restart brings it back): only frequency, BW, SF and CR change.
- Emoji: `tools/gen/emoji.py` builds `src/ui/EmojiData.h` from Twemoji 15.1 (CC-BY 4.0, attribution in the README and
  the header; cached in `vendor-cache/twemoji`). `src/ui/Emoji.cpp` — lookup and decoder, `src/ui/Emoji.h` —
  modifiers, flags as letters, a frame for emoji without a picture, `mono()` for monochrome screens. Drawn by `glyph()`
  in `src/ui/Ui.cpp` and `emojiGlyph()`/`glyph()` in `src/ui/UiHeltec.cpp`; width — `glyphCells()` (2). New emoji go
  into the generator's `EMOJI` list.
- Backup (`renderBackup`/`backupSave`/`backupRestore` in `web/index.html`): assembled with ordinary commands, writes
  back only what is ticked, the node key last (`identity set` restarts the board after 1.5 s; `restartTick` calls
  `meshRadio.flush()` first). Commands `identity` (128 hex), `identity set`, `contact add {JSON}`
  (`MeshRadio::privateKeyText/restoreKey/learnContact`); over Wi-Fi the key is returned only via the access point
  (`src/net/Portal.cpp`). Settings are written in chunks of up to 200 bytes; a contact's path is not part of the backup.
  The backup has a single public key (`node.public_key`).
- Duels and folders: games of both kinds in `duels` (up to 6, files `/meshmesh/duels.0|1`), USB `duel …`,
  `GET /api/duel`, direct messages `🏁…`/`🚢…` with ACK and resend as in chess. The battleship winner sends their
  layout, the loser checks it against the answers. The `apps` setting — `{NAME id …}` folders, `-{…}` hidden, `games`
  — "Games" in the screen language; the saved order moved the games into the folder once (NVS `apps3`). Over BLE `set`
  is at most 255 bytes: the page does not save an app order longer than 240 bytes. Protocol check —
  `tests/duel_check.py` (in CI); screens — the `tools/ui_preview/scenario_games_*.inc` scenarios.
