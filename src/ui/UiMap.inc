// Map (included by Ui.cpp): the map with its HUD and the library of saved maps.
// Map: north-up tiles with HUD chips, own position, node markers and a scale bar.
void drawMap() {
  auto& d = g();
  maps.draw(0, 21, 320, 199);
  bool online = internet.online();
  if(!maps.haveCenter || (!maps.available && !online)) {
    panel(30, 70, 260, 86, surfaceCard, radiusDialog);
    icon(IcPin, 160, 96, 12, iconSubtle, surfaceCard);
    textCenter(160, 128, online ? t("Finding location by IP...") : !maps.available ? t("Needs Wi-Fi internet or an SD card")
                                                                                   : t("Connect Wi-Fi or load a map"),
               textPrimary);
    textCenter(160, 144, online ? t("GPS will take over after a fix") : t("Connections > Internet over Wi-Fi"), textMuted, small);
  } else {
    double scale = 256.0 * (1UL << maps.zoom);
    auto yy = [&](double l) {double r=l*M_PI/180;return (1-log(tan(r)+1/cos(r))/M_PI)/2*scale; };
    auto point = [&](double lat, double lon, int& x, int& y) {if(!isfinite(lat)||!isfinite(lon)||fabs(lat)>85.05112878)return false;x=160+int((lon-maps.longitude)/360*scale);y=120+int(yy(lat)-yy(maps.latitude));return x>=6&&x<314&&y>=26&&y<216; };
    {
      const wardrive::Recent* pts;
      unsigned n = wardrive::recent(pts);
      for(unsigned i = 0; i < n; i++) {
        int x, y;
        if(!point(pts[i].lat / 1e6, pts[i].lon / 1e6, x, y)) continue; // the wardrive log: pings larger
        if(pts[i].kind == wardrive::Ping) {
          d.fillCircle(x, y, 5, surfaceBar);
          d.fillCircle(x, y, 3, snrColor(pts[i].snr));
        } else d.fillCircle(x, y, 2, snrColor(pts[i].snr));
      }
    }
    for(unsigned i = 0; i < meshRadio.peerCount; i++) {
      auto& p = meshRadio.peers[i];
      int x, y;
      if(!p.position || !point(p.latitude, p.longitude, x, y)) continue;
      d.fillCircle(x, y, 6, surfaceBar);
      d.fillCircle(x, y, 4, typeColor(p.type));
      String name = fit(p.name, 80, small);
      int w = measure(name, small) + 6, lx = constrain(x + 8, 2, 316 - w);
      d.fillRoundRect(lx, y - 6, w, 11, radiusOverlay, surfaceBar);
      text(lx + 3, y + 2, name, textPrimary, small);
    }
    if(hardware.gpsFix()) {
      int x, y;
      if(point(hardware.gps.location.lat(), hardware.gps.location.lng(), x, y)) {
        if(navigation.headingValid) {
          float a = navigation.heading * M_PI / 180;
          d.fillTriangle(x + roundf(18 * sinf(a)), y - roundf(18 * cosf(a)), x + roundf(8 * sinf(a - 0.6f)), y - roundf(8 * cosf(a - 0.6f)), x + roundf(8 * sinf(a + 0.6f)), y - roundf(8 * cosf(a + 0.6f)), info);
        }
        d.fillCircle(x, y, 7, graphicPrimary);
        d.fillCircle(x, y, 5, info);
      }
    }
    if(!maps.follow) {
      d.fillRect(151, 119, 19, 3, surfaceBar);
      d.fillRect(159, 111, 3, 19, surfaceBar);
      d.drawFastHLine(152, 120, 17, graphicPrimary);
      d.drawFastVLine(160, 112, 17, graphicPrimary);
      d.drawCircle(160, 120, 5, surfaceBar);
    }
    if(!maps.visibleTiles) {
      panel(30, 96, 260, 44, surfaceCard, radiusCard);
      textCenter(160, 114, maps.fetching ? t("Downloading map...") : maps.waiting ? t("Reading map from SD...")
                                                                   : online       ? t("Tiles unavailable")
                                                                                  : t("This zoom is not downloaded"),
                 textPrimary);
      if(!maps.waiting) textCenter(160, 130, online ? fit(netError(), 240, small) : t("+/- zoom · Wi-Fi internet loads maps"), textMuted, small);
    }
    double metresPerPx = cos(maps.latitude * M_PI / 180) * 40075016.686 / scale;
    int nice[] = {5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000, 2000000}, dist = nice[0];
    for(int v : nice)
      if(v / metresPerPx <= 90) dist = v;
    int px = max(8, int(dist / metresPerPx));
    String scaleText = dist >= 1000 ? String(dist / 1000) + t(" km") : String(dist) + t(" m");
    int sw = max(px, measure(scaleText, small)) + 12;
    d.fillRoundRect(312 - sw, 196, sw, 19, radiusOverlay, surfaceBar);
    d.fillRect(306 - px, 209, px, 2, graphicPrimary);
    d.drawFastVLine(306 - px, 206, 5, graphicPrimary);
    d.drawFastVLine(305, 206, 5, graphicPrimary);
    textRight(306, 204, scaleText, textPrimary, small);
    d.fillRoundRect(4, 203, 82, 12, radiusOverlay, surfaceBar);
    text(8, 212, "© OpenStreetMap", textMuted, small);
  }
  String area = fit(maps.available && maps.inArea() ? maps.title : online ? "OpenStreetMap"
                                                                          : t("Offline map"),
                    120, small) +
                "  z" + String(maps.zoom) + (maps.fetching ? t(" · loading") : "");
  int aw = measure(area, small) + 22;
  d.fillRoundRect(6, 26, aw, 16, radiusOverlay, surfaceBar);
  icon(IcPin, 14, 34, 5, ok, surfaceBar);
  text(22, 37, area, textPrimary, small);
  bool fix = hardware.gpsFix();
  String gps = !config.gps ? t("GPS off") : fix                    ? "GPS " + String(hardware.gps.satellites.value())
                                          : hardware.clockConflict ? t("GPS: old date")
                                                                   : t("GPS: searching");
  int gw = measure(gps, small) + 10;
  d.fillRoundRect(6, 45, gw, 15, radiusOverlay, surfaceBar);
  text(11, 56, gps, !config.gps ? textMuted : fix ? ok
                                                  : warn,
       small);
  if(navigation.headingValid) {
    int cx = 298, cy = 42;
    d.fillCircle(cx, cy, 15, surfaceBar);
    d.drawCircle(cx, cy, 15, border);
    float a = navigation.heading * M_PI / 180;
    d.fillTriangle(cx + roundf(11 * sinf(a)), cy - roundf(11 * cosf(a)), cx + roundf(4 * cosf(a)), cy + roundf(4 * sinf(a)), cx - roundf(4 * cosf(a)), cy - roundf(4 * sinf(a)), bad);
    d.fillTriangle(cx - roundf(11 * sinf(a)), cy + roundf(11 * cosf(a)), cx + roundf(4 * cosf(a)), cy + roundf(4 * sinf(a)), cx - roundf(4 * cosf(a)), cy - roundf(4 * sinf(a)), graphicMuted);
    String deg = String(int(navigation.heading + .5f) % 360) + "°";
    d.fillRoundRect(cx - 15, 60, 30, 12, radiusOverlay, surfaceBar);
    textCenter(cx, 69, deg, textPrimary, small);
  }
  footer({{"<>", t("Pan")}, {"+/-", t("Zoom")}, {"OK", maps.follow ? t("Following") : t("To GPS")}, {"L", t("Maps")}});
}
void drawLibrary() {
  unsigned total = library.size();
  if(!total) {
    icon(IcPin, 160, 90, 14, iconSubtle);
    textCenter(160, 128, t("No saved maps"), textPrimary);
    textCenter(160, 146, t("Upload a map via Wi-Fi or USB"), textMuted, small);
  }
  int first = max(0, selected - 3);
  for(unsigned i = first; i < total && i < unsigned(first + 4); i++) {
    int y = 24 + (i - first) * 48;
    bool focus = selected == int(i);
    listRow(y, 45, focus);
    target(8, y, 304, 45, i);
    g().fillRoundRect(18, y + 8, 30, 30, radiusWell, surfaceCard);
    icon(IcPin, 33, y + 22, 9, ok, surfaceCard);
    String name = library[i]["name"].as<String>();
    text(58, y + 19, fit(name, 200, bold), textPrimary, bold);
    text(58, y + 36, count(library[i]["tiles"].as<unsigned>(), "tile on SD", "tiles on SD"), textMuted, small);
    if(name == maps.title) {
      g().drawLine(284, y + 22, 288, y + 26, accent);
      g().drawLine(288, y + 26, 296, y + 17, accent);
      g().drawLine(284, y + 23, 288, y + 27, accent);
      g().drawLine(288, y + 27, 296, y + 18, accent);
    }
  }
  scrollbar(first, 4, total, 24, 190);
  footer({{"OK", t("Open")}, {"^v", t("Select")}, {"BACK", t("Map")}});
}
// The map takes every key that reaches it: L the saved maps, arrows pan, +/- zoom, OK follows the GPS.
void mapKey(int key) {
  if(key == 'l' || key == 'L') {
    change(Library);
    return;
  }
  if(key == KeyLeft) maps.pan(-70, 0);
  if(key == KeyRight) maps.pan(70, 0);
  if(key == KeyUp) maps.pan(0, -70);
  if(key == KeyDown) maps.pan(0, 70);
  if(key == '+' || key == '=') maps.changeZoom(1);
  if(key == '-' || key == '_') maps.changeZoom(-1);
  if(key == Enter) {
    maps.follow = true;
    if(hardware.gpsFix()) maps.center(hardware.gps.location.lat(), hardware.gps.location.lng());
    else notice(t("Waiting for GPS fix"), warn);
  }
}
// OK on a saved map opens it.
void libraryEnter() {
  if(!library.size()) return;
  if(maps.selectArea(library[selected]["id"].as<String>())) change(Map);
  else notice(t("Map unavailable"), bad);
}
