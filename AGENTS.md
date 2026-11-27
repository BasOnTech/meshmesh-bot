# Working on MeshMesh

## Purpose and communication

The project is firmware for the Elecrow ThinkNode M9 and the Heltec V4 (this fork supports only these two; the upstream MeshMesh also built for nRF52 and other ESP32 boards): a standalone interface,
MeshCore chats and channels (hashtag, link/QR, key, invitations; `docs/channels.md`, `src/mesh/Channels.cpp`), MeshCore regions
(default region and per-channel region, search at neighbouring repeaters, companion commands 54/55/63/64; `docs/regions.md`, `src/mesh/Regions.cpp`,
`src/mesh/RegionSearch.inc`, check `tools/hw/region_check.py`), a radio settings test and the previous profile (`radio try|keep|revert|previous`, `renderNet` in `web/index.html`), LoRa, offline maps (on the device SD or in phone storage in the app — `android/.../api/MapStore.kt`; relief and contour lines from Terrain Tiles during preparation; node map without a base layer — `drawMap` in `web/index.html`; `docs/maps-format.md`), Wi-Fi, BLE, a signal radar, a Wi-Fi CSI motion sensor,
chess with contacts (M9 screen and the web page of both boards, MeshCore direct messages,
rated ELO games with a result signed by both nodes' keys — `src/games/chess/ChessRating.cpp`, Swiss-system tournaments — `src/games/chess/ChessTour.cpp`; `docs/chess.md`; draughts and battleship with contacts, unrated — `src/games/duel/Checkers.cpp`, `src/games/duel/Sea.cpp`, `src/games/duel/Duel.cpp`, screens `src/games/duel/UiDuel.inc`, `src/games/duel/UiDuelCompact.inc`, `docs/games.md`, protocol check `tests/duel_check.py`; menu app folders (the "Games" folder) — `src/core/Apps.cpp`; the site page `chess/` plays through a stock MeshCore companion over Web Serial/Bluetooth —
`web/chess-companion.js`, `tools/site/chess_site.py`), a network pet — a tamagotchi that feeds on radio traffic,
grows and can die (`src/games/pet/Pet.cpp`, screens `src/games/pet/UiPet.inc`, `src/games/pet/UiPetCompact.inc`;
`docs/pet.md`), dice — the DIC3R dicer: RPG rolls and formulas, a Warhammer grid, a character sheet of blocks with formulas, D&D and R.E.T. templates, sending a sheet to a contact (`src/games/dice/Dice.cpp`,
screens `src/games/dice/UiDice.inc`, `src/games/dice/UiDiceCompact.inc`; `docs/dice.md`), a message's packet path on the web and in the app (repeaters by path hashes, SNR/RSSI, repeats of our own messages —
`MeshRadio::echo`, `renderMsg` in `web/index.html`), web and app notifications (kinds, mentions, per-chat settings, quiet hours, feed — `alertFor` in `web/index.html`, `android/.../Alerts.kt`), MeshCore repeater and room modes (chosen at boot, `src/mesh/MeshServer.cpp`, `docs/repeater.md`), wardriving — a network coverage map: a log of received packets with position, pings in `#wardrive` with repeater echoes, radar Wi-Fi/BLE on ESP32, CSV/GeoJSON/KML/WiGLE export, phone position from the app (`src/apps/Wardrive.cpp`, `docs/wardrive.md`; recording only with a trusted position, `gpsFix` or a `fix` from the app), backup — settings, node key, channels, contacts, quick send, counters and sheets in one JSON file from the page and the app, selective restore (`backup*` in `web/index.html`, commands `identity`, `identity set`, `contact add`; `docs/backup.md`), diagnostics, the screen interface (emoji on screens — `src/ui/Emoji.cpp`, `tools/gen/emoji.py`), the device web page, the app and the site in 15 languages (section "Languages") and the Android app (`android/`, `docs/android.md`). Communicate with the user
in English. Carry agreed changes through to build, installation and verification
when the devices are available. Distinguish between an implemented feature, a successful build,
installed firmware and confirmed hardware operation.

Current results and limitations are in `docs/verification.md`,
`artifacts/release-check.json` and `artifacts/hardware-finish.json`.
Do not present old reports or an identical version string as verification of a new package.

## Repository

GitHub: this fork, `BasOnTech/meshmesh-bot`, of the public `IKrasnodymov/meshmesh` (MIT; `README.md` — the overview, `docs/` — the detailed documents, all in English), main branch `main`. The site is
GitHub Pages from `site/` (https://basontech.github.io/meshmesh-bot/): `.github/workflows/pages.yml`
builds all boards and `tools/site/pages.py` (ESP Web Tools browser-install manifests,
writing at offsets without erasing NVS). Screen screenshots — `tools/ui_preview`; the interactive simulator in an SDL2 window —
`tools/ui_preview/sim.sh [m9] [LANG]` (`SIM=1` in `build.sh`, code in `tools/ui_preview/live.inc`; rebuilds via fswatch).
Keep sources, vendored MeshCore with its licences, tools and
documentation in Git. `backups/`, `artifacts/`, `logs/`, `vendor-cache/`, the Python environment,
build outputs and `.env` stay local and are excluded via `.gitignore`.
The repository is public: do not add flash/NVS dumps, chat history, private keys,
Wi-Fi passwords, BLE PINs, real node addresses or coordinates. Check the
staged files before committing.

## Commits

Commit messages follow [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/):
`<type>(<optional scope>): <summary>`, e.g. `fix(ui): keep the cursor on the selected channel`.
Types: `feat`, `fix`, `refactor`, `perf`, `build`, `ci`, `docs`, `test`, `style`, `chore`; a breaking change
(storage format, protocol, NVS keys) gets `!` after the type and a `BREAKING CHANGE:` footer.
Keep the summary imperative and under ~72 characters; explain the why in the body. Split unrelated
changes into separate commits (e.g. build flags apart from the code fixes they need).

## Devices

- M9: ESP32-S3, 16 MB flash, 8 MB PSRAM, LR1110, 320×240 screen,
  keyboard, SD, RTC, GPS, IMU, magnetometer. PlatformIO target `m9`.
- The connected Heltec V4: ESP32-S3, 16 MB flash, 2 MB PSRAM, SX1262,
  128×64 OLED and the PRG button. Target `heltec_v4`.
- The `heltec_v4_r8` target is for another revision with 8 MB PSRAM;
  a successful build does not mean that revision was checked on hardware.
- Shared code distinguishes the M9 from the Heltec by the family (`MM_COMPACT`, set together with `MM_HELTEC_V4`),
  the transceiver (`MM_RADIO_*`) and native USB (`MM_NATIVE_USB`). Pins — `src/hw/BoardPins.h`, board properties —
  `src/hw/Board.h`; the Heltec — `src/hw/HardwareHeltec.cpp` with `src/ui/UiHeltec.cpp`, the M9 — `src/hw/Hardware.cpp` with `src/ui/Ui.cpp`.
- Last known ports: M9 `/dev/cu.wchusbserial110`,
  Heltec `/dev/cu.usbmodem101`; `tools/hw/ports.py`
  finds them by USB bridge.
  Check that devices are present and are the expected ones; port names change after
  reconnecting.
- User parameters: 868.731 MHz, BW 62.5 kHz, SF8, CR4/6 (the network's shared profile);
  power, hops and relay are per board (1 October: M9 — 14 dBm, hops 3, relay
  off; Heltec — 10 dBm, hops 3). Preserve these settings on update; checks compare
  only the profile between boards.

## Protocol and compatibility

MeshMesh uses MeshCore from 0.3.0 onwards (`docs/meshcore-migration.md`,
`lib/MeshCore`). The archived 0.2.0 uses MM/1 (`docs/protocol.md`,
`lib/MeshProtocol/Mm1Packet.h`). Meshtastic is not supported.
The 0.3.0 compatibility concerns the radio. Since 0.9.0 the board implements the MeshCore companion protocol itself
(`src/mesh/Companion.inc`, `docs/companion.md`): BLE Nordic UART and the name `MeshCore-<name> XXXX` (4 characters of the key) always (the MeshMesh app looks for the `MM` tag
of company 0xFFFF in the advertisement, our own service still answers), USB frames `<`/`>`;
since 0.10.0 — login to repeaters and rooms, CLI, status, telemetry,
room posts, tracing and path discovery; since 0.13.0 login, status, CLI, tracing and room posts are also on the screen,
the web page and over USB (`remote`, `src/mesh/Remote.h`, `src/mesh/Remote.inc`, M9 — `src/ui/UiRemote.inc`); no TCP. Requests to a server need a clock later than
the previous request (replay protection): set the board's clock in checks (`set_time`).
Matching frequency and other radio parameters does not make the packet, advert, address, key, message and
acknowledgement formats compatible.

WadaMesh uses the MeshCore core. Do not present WadaMesh as a separate protocol incompatible
with MeshCore. Keep original headers and source attributions.

When adding compatibility, keep our UI, maps and hardware drivers;
change the network layer deliberately. Verify new compatibility with a real third-party
node: adverts both ways, addresses/contacts, messages, keys and ACKs.
Detecting a packet or radio signal is not proof that messages can be exchanged.
Do not promise simultaneous discovery of all networks: each
device has one LoRa transceiver, different radio profiles require switching.

## Code and build

- `src/` — the application and drivers, each header next to its source, grouped by feature: `core/` (app, config,
storage, power, clock, version), `i18n/`, `hw/` (boards, pins, displays), `mesh/` (radio, server roles, channels,
regions, companion, remote, history), `ui/` (screens, icons, navigation, notifications, emoji), `net/` (Wi-Fi, portal,
diagnostics), `apps/` (maps, radar, wardrive, counter), `games/<game>/` (chess, dice, duel, pet, solitaire: logic and
its `Ui*.inc` screens together). Includes are written from `src/`: `#include "games/chess/Chess.h"`.
- The M9 screen: `src/ui/Ui.cpp` keeps what the pages share (theme, text, drawing primitives, footer and touch targets,
read marks and drafts, `change()`, the status bar) and the dispatch (`draw()`, `uiKey()`, `uiTouch()`, `uiTick()`).
Each page's drawing and its keys are in `src/ui/Ui<Page>.inc`: `UiHome`, `UiChat`, `UiMap`, `UiNodes`, `UiSensors`,
`UiRadar`, `UiSettings`, `UiWardrive`, `UiQuick`, `UiNetwork`, `UiSystem` (diagnostics, help, lock screen),
`UiServer`, `UiChannels`, `UiRemote`; the games' screens are next to the games. Ui.cpp includes them inside its
anonymous namespace, so they share its state; the order matters (a page can use what comes before it; for later
code it forward-declares, as `UiNodes.inc` does for `openPick`). `uiKey()` keeps the order in which keys are tried
and calls the page's handler (`chatKey`, `nodesKey`, `settingsEnter`...): add a page's keys there, not inline.
`lib/MeshProtocol/` — MM/1.
- `web/index.html` — the source of the web interface. ESP32 firmware serves it and the dictionaries compressed (gzip):
  `tools/build/pio_web.py` generates `PortalWeb.h` in the build folder at build time; it is not in Git. The page is English; a further language's dictionary is packed with its keys once (`/i18n/keys.js`) and `/i18n/<lang>.js` carrying the translations in order (`MeshMeshWebDict`). Page text —
  `L('…')`/`` L`…${x}` `` and `plural(n,…)` (section "Languages"). Demo without a board — `tools/site/web_demo.py`.
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

Build the affected boards. Repeat and extend checks for the errors and changes
found; do not add tests that merely duplicate the implementation.

## Colours and themes

- The colour screen UI (`src/ui/Ui.cpp` and the `Ui*.inc` it includes) takes its colours from a theme:
  `src/ui/Theme.h` declares the `Theme` struct and `themes[]` (the first is the default), each theme is a file in
  `src/ui/themes/` (Midnight is the only one so far). `applyTheme()` in `Ui.cpp` converts the chosen theme to the
  RGB565 globals the UI draws with; `theme->…` keeps the `0xRRGGBB` values for `mix()`. The Heltec mono UI
  (`UiHeltec.cpp`, `*Compact.inc`) does not use themes.
- Colours are named by role, never by hue or a vague word (no `violet`, `pink`, `ink`, `dim`, `card`). Hue names live
  only in a theme file's own palette namespace (`midnight::violet`), and the theme assigns them to roles.
- The roles: surfaces `surfacePage`, `surfaceBar` (status and key bars), `surfaceCard`, `surfaceFocus` (focused row),
  `surfaceWell` (inset: icon squares, input fields), `border`, `gridLine`; foreground per kind, each with
  `Primary`/`Muted`/`Subtle` — `text*` for text, `icon*` for `icon()` and arrow glyphs, `graphic*` for drawn shapes
  (lines, knobs, bars, chart strokes, the toast's colour bar); `onFill` for text and glyphs on a coloured fill
  (badges, buttons, toggle knob, checkmarks); `accent`, `secondary`, `tertiary` for categories; state `info`, `ok`,
  `warn`, `alert` (between warn and bad), `bad`; `*Deep` for the dark tint behind a role's icon or text;
  `outBubble`, `inBubble`, `avatarHues[]`.
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

The public firmware version is SemVer; the single source is
`versions.properties`. After a change: `python tools/build/version.py --generate`, then `--check`;
`src/core/Version.h` is generated and committed. Details — `docs/releases.md`.
The firmware reports `version`, `revision` (Git SHA + `-dirty` locally), `build_sha256`; packages and the site
include the revision and hashes. Do not treat an identical SemVer as proof of an identical image.
A push to `main` runs `.github/workflows/pages.yml`: version and translation checks (screens, page, chess),
all boards and language packages, screenshots, tests, the site, then Pages. Any
required failure blocks publishing. After pushing, check that Actions finished and check the public `release.json`.
The build/deploy does not flash boards.
Update the documentation and these instructions by hand from actual results before committing.

## Languages

Firmware screens — English (`src/i18n/I18n.h`; the language number is stored in NVS, do not change the
order; upstream Russian and 13 other languages were removed, their stored numbers fall back to English). English is
written in the code: `t("English")`, `tr("English")`, `count(n,"one","many")`; strings of a table are marked with
`N_("...")` and read with `trText()`. A further language (Dutch is planned) goes to the end of `Lang`
with `i18n/firmware/<lang>.json` following `i18n/README.md`: `tools/gen/i18n.py sync`, translate,
`.venv/bin/python tools/gen/i18n.py generate` (the table; the Latin Extended font subsets need `vendor-cache/fonts`),
`tools/gen/i18n.py check`, build, and review the screens with `tools/site/site_shots.py OUTDIR <lang>`.
Site — `site/i18n/en.json`; the detailed documents in `docs/` are English. The device web page — English: the English text in the
code is the dictionary key of `i18n/web/<lang>.json` for a further language; in `web/index.html` write `L('…')`,
`` L`…${x}` `` (do not glue a phrase together from pieces), `plural(n,'{n} game','{n} games')`. Then
`node tools/gen/web_i18n.cjs sync`, `node tools/gen/web_i18n.cjs check` (needs `npm ci --prefix tools`). The page language —
`?lang=`, chosen on the login screen/in "Settings" (`mm-lang`), otherwise the browser language. Page screenshots for
the site — `tools/site/web_shots.py` (demo data `tools/site/web_demo.py`, in CI); `site/img` holds the fallbacks.
The site's chess page (`chess/`) is the same page in English and gets the same dictionaries.
The site installs the firmware with the chosen language: `partitions-<lang>.bin` with a tag at 0x8C00; the tag is applied once.
`src/core/Modules.h` (`MM_CHESS`, `MM_PET`, `MM_DICE`, `MM_WARDRIVE`, `MM_CHECKERS`, `MM_SEA`) comes from the upstream
nRF52 images: both boards build everything. Cyrillic fonts stay in all images: Cyrillic messages arrive over the radio.

## Installation and data safety

Both original flashes are saved and verified in `backups/` together with metadata.
Before writing, verify the full backup, the model, MAC and component digests with
`tools/hw/flash.py`. Use DIO, 80 MHz, 16 MB and the intended offsets of the
four components. QIO previously caused a boot loop on these boards.

Preserve history, the network key, NVS, calibration and SD cards. NVS is 20 KB on all ESP32 boards and is shared with Wi-Fi and BLE: keep large and frequently
changing data (contacts `/meshmesh/contacts.bin`, read markers `/meshmesh/read.bin`, history)
in LittleFS; the headroom is visible in `status` (`nvs_free`, `contacts_saved`). Do not format the SD
or reset settings as part of a normal update. Keep backups,
files with keys, passwords, PINs and histories with 0600 permissions;
do not print secrets to a shared log or report.

The order is already agreed with the user: finish the M9 first, then the Heltec.
Authorisation for the necessary updates holds within the agreed
task; do not ask for it again without a new reason.

```sh
.venv/bin/python tools/hw/finish_on_hardware.py --check
.venv/bin/python tools/hw/finish_on_hardware.py
```

The first mode checks the packages without USB. The second installs and verifies
both boards in sequence, stopping on an error. Run the full pass
(20–30 min) for the final package, not after every edit: while
debugging, flash the affected board with `tools/hw/flash.py` and do targeted checks, stating explicitly
in the report that the full pass was not run. Individual steps
are written to `logs/finish-*.log`, previous reports are archived. Report the cause of an error
and the actual state of the installed devices precisely. If the environment
forbids USB (`Operation not permitted`), do not bypass the access restrictions.

## Operation and hardware verification

The radio keeps being serviced during maps, USB downloads and the lock screen.
Split long operations; watch memory, lengths, CRC, UTF-8,
timeouts and atomic saving. Show the DELIVERED state only
after a matching ACK; do not treat a send left unfinished across a restart
as delivered. The input language is independent of the interface language.

For an STA client with BLE enabled, keep Wi-Fi modem sleep enabled:
`WiFi.setSleep(true)`. Disabling it caused a confirmed `abort()` in the
ESP32-S3 driver (`logs/heltec-wifi-reset-trace.log`). Do not mask the problem
by disabling BLE in the interface coexistence check.

One process owns each USB port. Other sessions may be working in the same
working tree: coordinate USB ports and shared files with them (especially `src/ui/Ui.cpp`)
and do not build packages from someone else's unfinished edits. Do not start several reads or
a flash on top of an active check of the same port. Use `tools/hw/device.py`:
it handles DTR/RTS and the accumulation of partial native-USB replies on the Heltec.

LoRa and BLE checks must confirm a real radio exchange, growth of RX/TX,
no restarts, no USB injections and a single copy of the message.
Boot numbers in reports must match the installation being checked.
USB key events check the production UI, but not the physical buttons.
Node JavaScript checks do not replace a visual check in the browser.

## Power saving

Since 0.4.0 the shared code on all boards waits up to 20 ms for the LoRa IRQ notification with the screen off and
no active send, radar/CSI or probes. This works in all roles, including ordinary chats.
After a USB/BLE command or a button press the fast loop is held for 1 s (10 s blocked the wait with
Android's background polling every 8 s). `idle_waits`, `idle_wait_ms`, `idle_radio_events` measure
the task's waiting, not CPU sleep or current. Check — `tools/hw/power_check.py --port PORT --peer PORT`;
when checking a direct message, do not poll the receiver's USB until the ACK. After a message the screen may
turn off again on its timer — a late read of `screen_off` does not prove there was no wake-up.
`docs/power.md`; RXPS and cyclic GPS power are not implemented yet.
Battery voltage (0.30.3): Heltec V4 — 2.5 dB attenuation and a 4.9 × 1.045 divider. Check new multipliers against the owner's voltmeter, not
against another firmware; not yet checked on hardware.

`src/core/Power.cpp`: 80 MHz with the screen off and no Wi-Fi/radar, light sleep in repeater and room modes
(`docs/repeater.md`). The transceiver's IRQ line has a RadioLib edge interrupt: set the level type for
wake-up only inside a critical section and restore `GPIO_INTR_POSEDGE` after sleep —
otherwise an interrupt storm (interrupt watchdog on the M9) or lost reception. Bytes that woke the board over
USB-UART are lost: send a CR before a command (`tools/hw/device.py`, `UsbLink.kt`). Do not change the frequency while
the Wi-Fi driver or the Bluetooth controller is running: after CSI this caused an M9 panic while loading a map.

Power results in `docs/verification.md` are tied to particular images: do not carry them over to other images by
version number alone.
Since 0.4.1 history is serialised one entry at a time instead of an extra 32 KB JSON pool;
running out of memory returns an explicit error, and `status.history_count` lets you check completeness.
After switching from repeater to normal mode, the hardware test waits for an advert of type CHAT, not just the key.

## Radar and Wi-Fi CSI

`src/Radar*.cpp`: a Wi-Fi/BLE/LoRa sweep, RSSI direction finding and a CSI motion sensor
(ESP-NOW beacon ↔ receiver). Rules found on hardware:
- The radar uses Wi-Fi only while it is open and only with the access point
  and the Wi-Fi probe off; `portalToggle()` calls `radar.release()`.
- Do not deinitialise the Bluetooth controller after the first start
  (`NimBLEDevice::deinit`): deinitialising right after stopping the scan
  caused an M9 panic in the NimBLE task, and deinitialising with a restart
  "froze" CSI data until reboot. "BLE off" means no advertising and
  no connections. The message `wifi:timeout when WiFi un-init, type=4` when turning
  Wi-Fi off with a live controller caused no failures.
- Wi-Fi scanning goes through `esp_wifi_scan_start` with a zeroed configuration:
  Arduino 2.0.17's `scanNetworks()` does not fill `home_chan_dwell_time`.
- The CSI beacon: ESP-NOW, OFDM 6 Mbit/s, channel 1 (MCS0 frames did not go on air);
  when stopping the beacon restore the default rate, otherwise CSI on reception
  on the same board "freezes".
- USB `radar` does not output other people's network names or Bluetooth addresses; `radar web` (for the app)
  outputs the names, like the web page, and holds the radar for 10 s, like `/api/radar`.
- Checks: `tools/hw/radar_check.py`, `tools/hw/csi_check.py` (real radio reception).
  Motion detection is checked only with a person walking between
  the boards; the automated check confirms only the stream and the change in CSI.

RSSI gives neither direction nor distance; phones' BLE addresses change, the device
count is approximate. CSI between two boards detects motion in the zone
between them, but not where people are. Breathing and heart rate via CSI are described in
research (a motionless person, a model trained on reference data); the firmware has none of this;
locating a heartbeat with a single ESP32 antenna is impossible. Locating people without
devices requires three or more nodes around the zone and training on a grid of points.

## Maps, GPS and the limits of verification

The SD holds Kazan (1022 tiles, z12–14) and a detailed area of about 4×4 km
(218 tiles, z15–16). Packages — `artifacts/maps/*.mmmap`. The detailed zoom
does not cover the whole city. Keep the map, the catalogue and the previous intact tiles
on corrupted or interrupted downloads. The M9 access point does not provide
internet. The M9 as a Wi-Fi client (`src/net/Internet.cpp`, "Network → Internet over Wi-Fi")
downloads missing OSM tiles, NTP time and IP-based position itself, caching tiles
on the SD. The client yields Wi-Fi to the access point, the radar and the Wi-Fi probe
(`internet.yieldRadio()` before a mode change) and reconnects after them.

The GPS modules of the M9 and Heltec V4 receive a spoofed signal: a 10-satellite fix dated 8 July 2026.
GPS/RTC time earlier than the build date is rejected (`src/core/Clock.cpp`, `gps_conflict`, the position is not used);
GPS does not overwrite time from the phone/NTP; such a date must not be taken as a current position,
used for following, sending coordinates or overwriting trusted time. ESP32 time
survives a restart (not a power-off) and is saved only with a known source; messages
recorded before the clock was set get their time when it is set (`MeshRadio::stampLate`).
A saved compass calibration does not prove accuracy or axis orientation.

Repeater and room modes (`src/mesh/MeshServer.cpp`, `docs/repeater.md`) are checked from the side of the
official MeshCore companion: `tools/hw/repeater_check.py`, `tools/hw/room_check.py`. For this the Heltec is
temporarily flashed with the stock companion: MeshCore `companion-v1.17.1`, built as
`meshmesh_stock_interop` for our flash layout (`tools/hw/stock_roundtrip.py` expects the checkout in
`vendor-cache/meshcore-stock`). Order: JSON snapshots and a full flash
copy in `backups/` with `verify-flash`, the checks, then the copy is written back and verified.
The stock firmware formats our LittleFS, so do not install it without a copy.
`tools/hw/stock_roundtrip.py` is part of the full pass: stock protocol/sources v1.17.1, DIO/dio_qspi,
a matching bootloader and partition table (only the LittleFS label is replaced with SPIFFS, the geometry is the same).
The first start can take more than 45 s; the tool waits for USB to be ready. Restoring the full
16 MB copy is verified byte by byte before starting. Restoring from the bootloader uses a watchdog reset.
While the board is in server mode, its normal-mode contacts are not updated.

Radio compatibility with stock MeshCore was checked on the final M9 ELF;
the user checks their separate MeshCore device themselves.
A fresh GPS fix under open sky, compass accuracy, LoRa range, forwarding
through a third repeater and a visual check of the new web design on a phone remain separate
unconfirmed checks, as do radar direction finding while moving outdoors,
chess on the `chess/` page over a BLE companion (Web Bluetooth) and in Chrome on Android,
the accuracy of BLE device classification and the CSI threshold after calibration (the threshold was chosen
from a single walking experiment). The Android app was checked in the emulator with boards through a computer; on a phone (Nubia
NX729J, Android 15) BLE pairing with a PIN and reading M9 data are confirmed; USB OTG and logging
in to the access point on a real phone are not checked. Voice, route planning
and OTA are not there yet.
Maps in the app (0.23.0 / Android 0.16.0): areas are stored on the phone (`MapStore`, app folders: internal
storage or the phone's SD card), the page takes a tile from the phone first (`/phone-maps/tile`), then from the board's SD; the node
map is drawn even without a base layer. Relief: Terrain Tiles elevations (AWS, CORS open) during preparation in the browser/app,
stepped hillshading (long RLE runs) and contour lines with an interval based on the area's steepness. Overpass cycles through mirrors:
the main server often answers 504/429. Checked in the Android emulator with the M9 via `usb_tcp_bridge.py`; on a phone,
over BLE and copying a phone area to a real board's SD — not. `tools/hw/maps.py` does not draw relief.
Also not checked: a repeater forwarding and refusing to forward a packet with a region (region search at a real
repeater and region codes in frames are confirmed, `docs/regions.md`), the MeshCore app's region menu, installing with a chosen language from the site in Chrome, and translations by native
speakers; 12 px Arabic on OLED overlaps neighbouring lines (there is no smaller pixel font).
Update this information from actual results, not assumptions.

## Release history (upstream)

The sections below are the upstream project's release notes. They also cover boards this fork no longer builds
(T-Deck, GAT562, T114 and other nRF52 boards; `UiHires.inc`, `src/nrf52/`, `tools/nrf52.py`): ignore those parts,
and do not size-check against the Chinese T114 image any more.

## Notifications, quick replies and the counter (0.19.0)

`docs/notifications.md`, `src/ui/Notifications.cpp`, `src/mesh/QuickSend.cpp`, `src/mesh/ChannelPolicy.cpp`,
`src/mesh/History.cpp` and `src/mesh/HistoryStorage.cpp`: independent notifications and an event queue,
10 templates with a recipient by ID, channel priority and depth. `src/apps/People.cpp` and `src/apps/PeopleWifi.cpp` —
a separate manual count and an approximate number of BLE/Wi-Fi devices; the counter does not depend on `MM_DICE`.
Android 0.12.0 keeps a bounded archive per node by public key (`api/ArchiveApi.kt`).
Compact ESP32 boards use the shared Wi-Fi client and optional periodic NTP; nRF52 has no Wi-Fi.
The LED colour depends on the hardware; the absence of a repeat is not proof of non-delivery.
On nRF52 `messages` over USB/BLE returns an immutable snapshot in chunks (`src/mesh/HistoryReply.cpp`), without a second
large contiguous JSON buffer. Batched `Preferences` writes apply only inside `Config::save`;
replay-protection timestamps are saved synchronously. The final targeted checks of 0.19.0 —
`docs/verification.md` and the local `artifacts/wishlist/`: the full M9/V4 pass succeeded on images
189E200C9C56… / BC6A4D591373… and boots 242/251. Quick Send and the manual count were checked on both;
the original settings and keys were preserved, M9 Wi-Fi/BLE off, V4 Wi-Fi off/BLE on.
The early targeted V4/T114 checks apply only to 4F071425B1BF… / E798B587599A…;
physical buttons, Android on a phone and a successful NTP sync with an available network are not yet confirmed.

## Quick Send recipient (0.19.1)

Choosing the recipient is a separate tile at the top of the list, visible while scrolling the templates.
Final targeted M9 check: `50B447386207…`, boot 244; V4 as peer —
`BC6A4D591373…`, boot 251, on 0.19.0. Report `artifacts/quick-recipient-check.json`,
packages — `artifacts/quick-recipient-release/`. Templates and settings were restored;
the full 0.19.1 pass was not run, the previous full reports apply to 0.19.0.

The recipient tile was also moved to the top in the shared compact interface and on the T114; on OLED it is highlighted with
a frame/inversion, on the T114 with colour. All 15 targets were built, the native scenarios of five interfaces
passed in ru/en/zh/ar; for the T-Deck, taps on the tile and on the edge templates were checked. The full Chinese
GAT562/T114 images are 671032/702124 bytes; the variants without chess/pet/dice were also built and packaged.
Build report — `artifacts/quick-recipient-all-check.json`. These new images were not installed on the boards:
USB was taken by the Wardrive session. The M9 result 50B447386207… above applies only to the previous package.

## Closed screens on one-button boards (0.20.1)

On one-button boards (`UiHeltec.cpp`, not the joystick) screens where a click does something (quick send, the manual counter,
dice) are closed while paging through: a click goes to the next screen, a hold opens it (`appOpen`, `app_open` in `ui`);
`showPage` closes them again. Add new such screens to `appPage()`. Do not rebuild web sections redrawn on a timer
under a focused input field (otherwise the keyboard closes on Android). Checked on the T114 via USB keys
(`3AC3F556FAAB…`, boot 16); M9/V4 0.20.1 were not installed.

## Quick counter, app order and path hash (0.24.0)

The "People counter" became the "Quick counter": up to 8 cards (`people::Tally` in `src/apps/People.h`, file `/meshmesh/tally`;
the manual count of earlier versions moves into the first card), the open card is shared by the screen, the page and the app.
M9/T-Deck — `src/apps/UiTally.inc` (left/right page through, after the last one — "New counter"), compact — a menu in `UiHeltec.cpp`,
T114 — `UiHires.inc`, web — the `renderPeople`/`tallyAct` carousel in `web/index.html`. The app identifier and commands
stayed `people`. Default order (`uiApps`): messages, places, tools, games, device; the user's saved `apps`
is not changed. Icons `IcSend`/`IcTally` (`UiIcons.cpp`, on the page `send`/`tally`). `path_hash` defaults to 2;
boards with the value 1 switch to 2 once (NVS key `path_hash2`), the repeater and room take the same setting.

## Character sheet (0.25.0)

The third dice mode is a sheet of blocks (`src/games/dice/Dice.h`: stat, gauge, marks, ability, note, header; up to 64,
width 1/6…the whole row of 12 columns), replacing the earlier counters: a 0.24 `dice.bin` is converted on first start
(counters → gauges, saved rolls → abilities; confirmed on the M9, "Pupig" 10/19/23). Formulas reference block codes
(`@dex`, `@dex!`); stat rules are a plain number, the D&D modifier, a d20 bonus, the R.E.T. die by level (`rN`,
a success adds the next level's die, a one is a failure with d4). D&D 5e and R.E.T. templates are in the firmware (names in the screen
language, codes in Latin). Each character's sheet is `/meshmesh/sheetN.bin`, only the current one is in memory. The sheet string
`MMS1~…` is for export, import and transfer: in parts `🎲S ID K/N` as direct messages with ACK, the recipient sees an offer.
From a phone, long strings use `dice sheet put K/N` (a BLE write is 255 bytes). The builder is on the page and in the app
(`sheetBuild`, commands `dice sheet set/add/move/del`). Checking with a browser: `tools/ui_preview/build.sh` with `KEEP=PATH`
keeps the program, `PATH --dice` answers `dice` commands from stdin (`loopback` returns the sent parts as
received). For the Chinese T114 image to fit, the nRF52 libraries are built with `-Os` — check the radio exchange with this change
on the T114/GAT562.
Since 0.26.0 blocks can also be edited on the M9/T-Deck screen (`E` — block, `A` — new, fields as rows, `src/games/dice/UiDice.inc` `drawDiceBlock`),
and the board adds the characters "D&D 5e" and "R.E.T." once (`State::flags`, `dice.bin` version 3; version 2 is read).

## Radio test and previous profile (0.27.0)

The board remembers the profile before the last change of frequency, BW, SF or CR (`Config::previous`, NVS `radio_prev`)
and a test (`Config::trial`: `Config::save` writes the saved profile, a restart brings it back): `radio try|keep|revert|previous`,
the row "Previous radio"/"Radio test" (M9), "Previous"/"Test" (Heltec), the Keep/Revert/Check block on the page (`renderNet`).
Only frequency, BW, SF and CR change. The upstream city network catalogue and the on-air network search built on it (0.29.0)
were removed: every place in it was Russian.
Since 0.27.0 nRF52 does not start without USB with the battery below 3.4 V and shuts down after 30 s below 3.3 V (`src/core/Power.cpp`,
`docs/power.md`): a reset on an empty battery in the middle of rewriting the first storage page probably erased the T114's storage twice.
Triggering on a discharged battery is not checked; the Chinese T114 image is 712108 bytes (`Platform.cpp`, `Power.cpp` with `-Os`).

## Emoji on screens and T114 backlight (0.32.0)

`tools/gen/emoji.py` builds `src/ui/EmojiData.h` from Twemoji 15.1 (CC-BY 4.0, attribution in the README and the header): ~300 emoji
12×12, a shared 31-colour palette, bit packing (local palette, "same as the pixel above", gamma codes; ~10 KB). The pictures
are cached in `vendor-cache/twemoji`. `src/ui/Emoji.cpp` — lookup and decoder, `src/ui/Emoji.h` — modifiers (FE0F, skin tones,
ZWJ — zero width), flags as letters, a frame for emoji without a picture, `mono()` for monochrome screens. Drawn by `glyph()`
in `src/ui/Ui.cpp` (M9/T-Deck, RGB565), `emojiGlyph()`/`glyph()` in `src/ui/UiHeltec.cpp` (palette colours `ColEmoji0…` on ST7735,
a silhouette on OLED/e-ink), `hiGlyph()` (T114, 16 px); the width in columns — `glyphCells()` (2). For the Chinese T114 to fit,
the T114 (`MM_HIRES`) does not take the 8 px CJK fonts (`fallbackFonts` in `src/i18n/I18n.cpp`): small text there is drawn with 12–13 px
fonts anyway. New emoji go into the generator's `EMOJI` list; check the size against the Chinese T114.
T114/T096 backlight: after `analogWrite` the LEDA pin belongs to the PWM module, `digitalWrite` does not turn it off — `light()`
first detaches the pin from PWM (`HwPWMx[i]->removePin`). On ESP32 the LEDC backlight has no such problem.

## Double click — back (0.31.0)

One-button boards (`MM_ONE_BUTTON` in `src/hw/Board.h`: compact boards without a joystick): `config.doubleBack` (NVS `dbl_back`,
`double_back` in `/api/config` only on them; off by default). `uiKey` in `src/ui/UiHeltec.cpp` holds a click for 0.4 s (`clickAt`,
`click_wait` in `ui`), a second click is `stepBack()`, another key first releases the pending click; the counter and button counting in dice
(`fastClicks`) do not wait. Add new nested states of the one-button interface to `stepBack()`. Check — a
`tools/ui_preview` scenario (heltec/t114); not checked on hardware.

## Backup (0.30.0 / Android 0.21.0)

`docs/backup.md`: the page ("Settings → Backup", `renderBackup`/`backupSave`/`backupRestore` in `web/index.html`)
assembles the backup with ordinary commands and writes back only what is ticked, the node key last (`identity set` restarts the board
after 1.5 s; `restartTick` now calls `meshRadio.flush()` first). New firmware commands — `identity` (the MeshCore private
key, 128 hex), `identity set`, `contact add {JSON}` (`MeshRadio::privateKeyText/restoreKey/learnContact`); over Wi-Fi the key
is returned only via the access point (`Portal.cpp`). BLE commands are at most 255 bytes: settings are written in chunks of up to 200 bytes,
a contact's path is not part of the backup. For the Chinese T114 to fit, `Dispatcher::calcRxDelay` and `MeshServer` compute the delay
via `powf` (the double `pow` is no longer linked, ~3.5 KB; `lib/MeshCore/PATCHES.md`): the Chinese T114 is 709672 bytes.
Checked: the build of all boards, saving and restoring on the page with `tools/site/web_demo.py` in the browser. On hardware (the commands
`identity`/`contact add`, moving the key to another board, the app over BLE) not checked: the boards were not connected.
0.30.1 / Android 0.21.1: the backup has a single public key (`node.public_key`), the file name uses local time; without storage
(`status.storage=false`, nRF52 after Meshtastic/MeshCore) the page shows a "Storage not created" card with `fsformat`
(`storageCard` in `web/index.html`, demo — `tools/site/web_demo.py --no-storage`), the site — step `uf2.step4`.
0.30.2: nRF52 creates the storage by itself if there is no file system at the start of the 0xD4000 area (neither `littlefs` nor a
littlefs v1 block header) — that is where the factory Meshtastic T114 code lives (`foreignCode` in `src/nrf52/Platform.cpp`); a foreign littlefs (MeshCore) and
our own torn one — still only via `fsformat`. Not checked on hardware.


## Draughts, battleship and folders (0.33.0 / Android 0.22.0)

`docs/games.md`: Russian draughts (`src/games/duel/Checkers.cpp`) and battleship (`src/games/duel/Sea.cpp`) with contacts via direct messages
`🏁…`/`🚢…` with ACK and resend as in chess, unrated; games of both kinds — `src/games/duel/Duel.cpp` (`duels`, up to 6, file
`/meshmesh/duels.0|1`), USB `duel …`, `GET /api/duel`. The battleship winner sends their layout, the loser checks it
against the answers. Screens: M9/T-Deck — `src/games/duel/UiDuel.inc`, compact and T114 — `src/games/duel/UiDuelCompact.inc`, `UiHires.inc`; page —
sections `checkers`, `sea`, `duel`, the "Games" tile (`games`) and the folder editor in `renderApps` (`web/index.html`), app —
`DuelNews.kt`. Folders: the `apps` setting — `{NAME id …}`, `-{…}` hidden, `games` — "Games" in the screen language; on the first start of
0.33.0 the saved order moves the games into the folder once (`appsAddGames`, NVS `apps3`). Protocol check —
`tests/duel_check.py` (two instances of the code via `tools/ui_preview` `--duel`, message loss; in CI), screens —
`tools/ui_preview/scenario_games_*.inc` scenarios. For the Chinese T114 to fit with folders: `quickSend::state` and the radio draft
in zero-initialised memory (not in `.data`), the large T114 font without Russian — `inr24_mr`. Over BLE `set` is at most 255 bytes: the page
does not save an app order longer than 240 bytes. Not checked on hardware: there were no boards on USB.
