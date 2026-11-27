// Home and app folders (included by Ui.cpp): the menu rows of the apps, a folder's apps, opening an app.
// Home: identity strip and a list of destinations; six rows are visible, the rest scroll.
// The tiles follow config.apps: its order and folders, without the hidden ones (Apps.cpp).
const int tileTop = 48, tileRowH = 28, tileRowsShown = 6;
struct AppTile {
  Icon ic;
  uint16_t hue;
  String name, detail;
  unsigned badge;
};
// The tile of every app ID, in uiApps order.
void appTiles(AppTile* out) {
  unsigned unreadCount = unreadTotal(), near = 0, total = meshRadio.peerCount;
  for(unsigned i = 0; i < total; i++)
    if(meshRadio.peers[i].heard && millis() - meshRadio.peers[i].seen < 1800000) near++;
  String links = portalActive() ? "Wi-Fi" : internet.online() ? t("Internet")
                                                              : "";
  if(bleActive()) links += links.length() ? " + BLE" : "Bluetooth";
  if(!links.length()) links = t("All off");
  unsigned faults = moduleStates(nullptr);
  AppTile tiles[] = {
    {IcChat, accent, t("Chats"), unreadCount ? String() : count(meshRadio.historyCount, "message", "messages"), unreadCount},
    {IcSend, info, t("Quick send"), quickSend::targetName(), 0},
    {IcMesh, secondary, t("Nodes"), String(near) + t(" of ") + String(total) + t(" nearby"), 0},
    {IcCompass, warn, t("Navigation"), hardware.gpsFix() ? "GPS: " + count(hardware.gps.satellites.value(), "sat", "sats") : config.gps ? t("GPS: searching")
                                                                                                                                        : t("GPS off"),
     0},
    {IcPin, ok, t("Map"), maps.title.length() && maps.available ? maps.title : internet.online() ? t("Online map")
                                                                             : !maps.available   ? t("No SD card")
                                                                                                 : t("No maps yet"),
     0},
    {IcRadar, accent, t("Radar"), "Wi-Fi, BLE, LoRa", 0},
    {IcTower, wardrive::running() ? ok : iconMuted, t("Wardrive"), wardrive::running() ? count(wardrive::points(), "point", "points") : t("Off"), 0},
    {IcTally, tallyHue(people::tally.current), t("Quick counter"), tallyTileDetail(), 0},
    {IcChess, iconPrimary, t("Chess"), chessTileDetail(), chessNet.waiting()},
    {IcCheckers, rgb(0xf0d9b5), t("Checkers"), duelTileDetail(Duel::Checkers), duels.waiting(Duel::Checkers)},
    {IcShip, info, t("Sea battle"), duelTileDetail(Duel::Sea), duels.waiting(Duel::Sea)},
    {IcPaw, creature.needsCare() ? warn : creature.has() ? tertiary
                                                         : iconMuted,
     t("Pet"), petTileDetail(), 0},
    {IcDice, dicer.last() ? warn : iconMuted, t("Dice"), diceTileDetail(), 0},
    {IcCards, warn, t("Solitaire"), gameTileDetail(), 0},
    {IcWifi, info, t("Connect"), links, 0},
    {IcPulse, faults ? bad : ok, t("Module health"), faults ? count(faults, "fault", "faults") : t("All OK"), 0},
    {IcGear, iconMuted, t("Settings"), t("Radio, screen"), 0}};
  static_assert(sizeof tiles / sizeof *tiles == sizeof uiApps / sizeof *uiApps, "a tile for every app ID");
  for(unsigned i = 0; i < uiAppCount; i++) out[i] = std::move(tiles[i]);
}
// A folder's tile: the names of its shown apps and the sum of their badges (games waiting, new messages).
AppTile folderTile(uint8_t f, const AppTile* tiles) {
  uint8_t in[AppEntriesMax];
  unsigned n = appsShown(in, f);
  AppTile t{IcFolder, warn, appsFolderName(f), String(), 0};
  for(unsigned i = 0; i < n; i++) {
    t.badge += tiles[in[i]].badge;
    if(i < 4) t.detail += String(i ? ", " : "") + tiles[in[i]].name;
  }
  return t;
}
// Single-line rows from y 48 (home and a folder): icon, name, the detail on the right and the badge; a folder's row has the folder icon.
// The first shown row keeps one row above the selection while it can.
int menuFirst(int shown, int visible) { return max(0, min(selected - 1, shown - visible)); }
// A menu row (home, a folder, settings): row i at y, the icon, the name, the detail on the right and the badge;
// the highlight alone marks the selection.
void menuRow(int y, int i, Icon ic, uint16_t hue, const String& name, const String& detail, unsigned badge = 0) {
  auto& d = g();
  const int h = tileRowH - 1;
  bool focus = selected == i;
  listRow(y, h, focus);
  target(8, y, 304, h, i);
  icon(ic, 20, y + h / 2, 7, hue, focus ? surfaceFocus : surfacePage);
  int right = 306;
  if(badge) {
    String b = badge > 99 ? "99+" : String(badge);
    int w = max(20, measure(b, bold) + 10);
    d.fillRoundRect(right - w, y + (h - 19) / 2, w, 19, radiusPill, bad); // odd, so the 9 px digits sit 5 px from both edges
    textCenter(right - w / 2, y + (h - 19) / 2 + 14, b, textPrimary, bold);
    right -= w + 6;
  }
  int nameW = min(measure(name, card), right - 33 - 40);
  text(33, y + h / 2 + 5, fit(name, nameW, card), textPrimary, card);
  int detailW = right - 33 - nameW - 10;
  if(detailW > 20) textRight(right, y + h / 2 + 5, fit(detail, detailW), textMuted);
}
void drawTiles(const uint8_t* order, int shown, const AppTile* tiles) {
  selected = max(0, min(selected, shown - 1));
  int first = menuFirst(shown, tileRowsShown);
  for(int i = first; i < shown && i < first + tileRowsShown; i++) {
    bool folder = order[i] & AppFolder;
    AppTile folderRow;
    const AppTile& tile = folder ? (folderRow = folderTile(order[i] & ~AppFolder, tiles)) : tiles[order[i]];
    menuRow(tileTop + (i - first) * tileRowH, i, tile.ic, tile.hue, tile.name, tile.detail, tile.badge);
  }
  scrollbar(first, tileRowsShown, shown, tileTop, tileRowsShown * tileRowH - 1);
}
// The one-line strip above the home and folder rows: icon, name in bold, then the detail.
void strip(Icon ic, uint16_t hue, const String& name, int nameW) {
  panel(8, 25, 304, 20, surfaceCard, radiusRowCompact);
  icon(ic, 20, 35, 6, hue, surfaceCard);
  text(33, 39, fit(name, nameW, bold), textPrimary, bold);
}
void drawHome() {
  strip(IcRadio, meshRadio.ready ? accent : bad, config.name, 110);
  int x = 33 + min(measure(config.name, bold), 110) + 8, r = 306;
  auto counter = [&](unsigned n, int dir, uint16_t c) {String v(n);textRight(r,39,v,textPrimary,small);r-=measure(v,small)+7;tri(r,36,dir,3,c);r-=10; };
  counter(meshRadio.rxCount, 2, ok);
  counter(meshRadio.txCount, 0, accent); // sent, then received, on the right
  char radio[64]; // one snprintf instead of six growing concatenations on the most drawn page
  if(meshRadio.ready) snprintf(radio, sizeof radio, "%.3f%s%d · BW %.1f · CR 4/%d", config.frequency, tr(" MHz · SF"), int(config.sf), config.bandwidth, int(config.cr));
  else snprintf(radio, sizeof radio, "%s%d", tr("Radio error "), int(meshRadio.radioError));
  text(x, 39, fit(radio, r - x, small), meshRadio.ready ? textMuted : bad, small);
  AppTile tiles[sizeof uiApps / sizeof *uiApps];
  appTiles(tiles);
  uint8_t order[AppEntriesMax];
  drawTiles(order, appsShown(order), tiles);
  footer({{"OK", t("Open")}, {"MSG", t("Chats")}, {"MAP", t("Map")}, {hardware.micKey ? "MIC" : tr("hold OK"), t("Lock")}});
}
// A folder: its apps in the same grid; folders are arranged on the web page or in the app.
void drawFolder() {
  AppTile tiles[sizeof uiApps / sizeof *uiApps];
  appTiles(tiles);
  uint8_t order[AppEntriesMax];
  int shown = appsShown(order, folderOpen);
  unsigned badge = 0;
  for(int i = 0; i < shown; i++) badge += tiles[order[i]].badge;
  String head = count(shown, "app", "apps") + (badge ? " · " + String(badge) + t(" waiting") : String());
  strip(IcFolder, warn, head, 150);
  int x = 33 + min(measure(head, bold), 150) + 8;
  text(x, 39, fit(t("Folders and order: web page or app"), 306 - x, small), textMuted, small);
  if(!shown) {
    textCenter(160, 130, t("No apps in this folder"), textMuted);
    footer({{"BACK", t("Menu")}});
    return;
  }
  drawTiles(order, shown, tiles);
  footer({{"OK", t("Open")}, {"<>", t("Select")}, {"BACK", t("Menu")}});
}
// Opens an app of the menu (an index into uiApps) from Home or from a folder.
void openApp(uint8_t app, int folder) {
  Page pages[] = {Threads, QuickView, Nodes, Sensors, Map, Scope, WardriveView, PeopleView, ChessList, CheckersList, SeaList, PetView, DiceView, Game, Network, Diagnostics, Settings}; // as uiApps
  static_assert(sizeof pages / sizeof *pages == sizeof uiApps / sizeof *uiApps, "a page for every app ID");
  if(app >= uiAppCount) return;
  if(pages[app] == QuickView) {
    quickReplyTo = 0;
    quickTargets = false;
  }
  if(pages[app] == CheckersList || pages[app] == SeaList) duelKind = pages[app] == SeaList ? Duel::Sea : Duel::Checkers;
  change(pages[app]);
  appFolder = folder;
}
// Home and a folder are lists: left leaves a folder, right opens the row as OK does, up the previous row, down the next.
bool homeKey(int key) {
  if(key == KeyLeft) {
    if(page == Folder) change(Home, true);
    return true;
  }
  if(key == KeyUp || key == KeyDown) {
    uint8_t order[AppEntriesMax];
    int tileCount = page == Folder ? appsShown(order, folderOpen) : appsShown(order);
    if(!tileCount) return true;
    selected = constrain(selected, 0, tileCount - 1);
    selected = (selected + (key == KeyUp ? tileCount - 1 : 1)) % tileCount;
    return true;
  }
  if(key != KeyRight && key != Enter && key != KeyHold) return false;
  uint8_t order[AppEntriesMax];
  int n = page == Folder ? appsShown(order, folderOpen) : appsShown(order);
  if(selected < 0 || selected >= n) return true;
  uint8_t e = order[selected];
  if(e & AppFolder) {
    folderOpen = e & ~AppFolder;
    change(Folder);
  } else openApp(e, page == Folder ? folderOpen : -1);
  return true;
}
