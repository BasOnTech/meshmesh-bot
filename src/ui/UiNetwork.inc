// Connections (included by Ui.cpp): the access point with its QR codes, BLE, and Internet over Wi-Fi.
void toggle(int x, int y, bool on) {
  panel(x, y, 30, 16, on ? accent : border, radiusPill);
  g().fillCircle(on ? x + 22 : x + 8, y + 8, 6, on ? onFill : graphicMuted);
}
// The access point as QR codes, so a phone needs no typing: 0 - joining it (the WIFI: text of camera apps), 1 - the
// device page with the password after "#" (it stays in the browser). Beside the code the network and the password,
// in groups of four, for typing it. -1: closed.
int wifiQr = -1;
String wifiSsid() { return "MM-" + meshRadio.idText(meshRadio.nodeId).substring(6); }
String groupedPassword() {
  String p = portalPassword(), r;
  for(unsigned i = 0; i < p.length(); i += 4) {
    if(i) r += ' ';
    r += p.substring(i, i + 4);
  }
  return r;
}
void drawWifiQr() {
  auto& d = g();
  static uint8_t modules[channels::QrBytes];
  static String coded;
  static int size = 0;
  String pw = portalPassword();
  String code = wifiQr == 0 ? "WIFI:T:WPA;S:" + wifiSsid() + ";P:" + pw + ";;" : "http://192.168.4.1/#pw=" + pw;
  if(coded != code) {
    size = channels::qr(code, modules, true);
    coded = code;
  }
  // Level L keeps both codes at version 3 (29 modules): 5 px each with the full 4-module quiet zone of the standard,
  // which phone cameras want.
  int s = size ? constrain(188 / (size + 8), 2, 6) : 0, side = (size + 8) * s, x0 = 8, y0 = 27;
  if(size) {
    d.fillRect(x0, y0, side, side, 0xffff);
    for(int y = 0; y < size; y++)
      for(int x = 0; x < size; x++)
        if(channels::qrDark(modules, size, x, y)) d.fillRect(x0 + (x + 4) * s, y0 + (y + 4) * s, s, s, 0);
  }
  int x = x0 + side + 10, w = 312 - x, y = 40;
  String rows[2];
  unsigned n = wrap(wifiQr == 0 ? t("1. Join the Wi-Fi") : t("2. Open the page"), rows, 2, w, bold);
  for(unsigned i = 0; i < n; i++) {
    text(x, y, rows[i], textPrimary, bold);
    y += 14;
  }
  n = wrap(wifiQr == 0 ? t("Scan with the phone camera") : t("Logged in, after step 1"), rows, 2, w, small);
  for(unsigned i = 0; i < n; i++) {
    text(x, y, rows[i], textMuted, small);
    y += 10;
  }
  y = max(y + 6, wifiQr == 0 ? 80 : 76);
  text(x, y, t("Network"), textMuted, small);
  text(x, y + 14, fit(wifiSsid(), w, bold), accent, bold);
  text(x, y + 30, t("Password"), textMuted, small);
  for(unsigned i = 0; i < pw.length(); i += 4) text(x, y + 50 + int(i / 4) * 22, pw.substring(i, i + 4), textPrimary, big);
  if(wifiQr == 1) text(x, y + 126, "192.168.4.1", accent, bold);
  footer({{"<>", t("Other code")}, {"BACK", t("Back")}});
}
// The QR codes take every key while open; they close by themselves when the access point goes off.
bool networkKey(int key) {
  if(wifiQr < 0) {
    if((key == KeyLeft || key == KeyRight) && selected == 0 && portalActive()) {
      wifiQr = 0;
      return true;
    }
    return false;
  }
  if(key == KeyLeft || key == KeyRight) {
    wifiQr = 1 - wifiQr;
    return true;
  }
  if(key == KeyBack || key == Enter || key == KeyHold || key == KeyUp || key == KeyDown) {
    wifiQr = -1;
    return true;
  }
  wifiQr = -1;
  return false; // HOME, MSG...: leave as usual
}
void drawNetwork() {
  if(wifiQr >= 0 && !portalActive()) wifiQr = -1;
  if(wifiQr >= 0) {
    drawWifiQr();
    return;
  }
  auto& d = g();
  bool wifi = portalActive(), ble = bleActive(), web = internet.enabled;
  int y = 25;
  auto cardAt = [&](int i, int h, Icon ic, const String& name, const String& state, bool on) {bool focus=selected==i;target(8,y,304,h,i);panel(8,y,304,h,focus?surfaceFocus:surfaceCard,radiusCard);if(focus)ring(8,y,304,h,radiusCard);d.fillRoundRect(16,y+7,24,24,radiusWell,surfaceWell);icon(ic,28,y+19,7,on?info:iconSubtle,surfaceWell);text(48,y+17,name,textPrimary,bold);text(48,y+30,fit(state,220,small),textMuted,small); };
  int wh = wifi ? 56 : 38;
  cardAt(0, wh, IcWifi, t("Wi-Fi access point"), wifi ? "192.168.4.1 · " + String(t("Web chat and map upload")) : t("Off"), wifi);
  toggle(274, y + 11, wifi);
  // The network and its password, whole, as on the home network card below (the address is in the line above).
  if(wifi) {
    text(48, y + 48, wifiSsid(), accent, bold);
    int pw = measure(groupedPassword(), bold);
    textRight(302, y + 48, groupedPassword(), textPrimary, bold);
    icon(IcKey, 291 - pw, y + 44, 6, iconMuted, selected == 0 ? surfaceFocus : surfaceCard);
  }
  // On the home network the device page answers at the client's address, with the same password.
  y += wh + 6;
  int ih = internet.online() ? 56 : 38;
  cardAt(1, ih, IcWifi, t("Internet over Wi-Fi"), internet.stateText(), internet.online());
  toggle(274, y + 11, web);
  if(internet.online()) {
    text(48, y + 48, "http://" + internet.address(), accent, bold);
    int pw = measure(groupedPassword(), bold);
    textRight(302, y + 48, groupedPassword(), textPrimary, bold);
    icon(IcKey, 291 - pw, y + 44, 6, iconMuted, selected == 1 ? surfaceFocus : surfaceCard);
  } // the password, whole
  y += ih + 6;
  cardAt(2, 38, IcBle, "Bluetooth LE", ble ? bleName() : t("Off"), ble);
  toggle(274, y + 11, ble);
  if(ble) textRight(266, y + 17, "PIN " + String(blePin()), accent, bold);
  y += 44;
  if(y + 38 <= 216) cardAt(3, 38, IcKey, t("MeshCore identity"), meshRadio.publicKeyText().substring(0, 24) + "...", true);
  if(selected == 0 && wifi) footer({{"OK", t("Toggle")}, {"<>", t("QR code")}, {"BACK", t("Back")}}); // no room for ^v in every language
  else footer({{"OK", selected == 1 ? t("Networks") : t("Toggle")}, {"^v", t("Select")}, {"BACK", t("Back")}});
}
// Internet over Wi-Fi: the client switch, networks in range, then saved networks out of range.
String pendingSsid;
struct NetRow {
  String ssid;
  int rssi;
  bool open, saved, visible;
};
unsigned netRows(NetRow* rows, unsigned max) {
  unsigned n = 0;
  for(unsigned i = 0; i < internet.seenCount && n < max; i++) {
    auto& v = internet.seen[i];
    rows[n++] = {v.ssid, v.rssi, v.open, v.saved, true};
  }
  for(unsigned i = 0; i < Internet::MaxSaved && n < max; i++) {
    String name = internet.savedName(i);
    if(!name.length()) continue;
    bool seen = false;
    for(unsigned k = 0; k < internet.seenCount; k++)
      if(name == internet.seen[k].ssid) seen = true;
    if(!seen) rows[n++] = {name, -127, false, true, false};
  }
  return n;
}
// Internet errors come in English from Internet.cpp (also shown over USB).
String netError() {
  String e = internet.error;
  if(config.lang == LangEn) return e;
  const char* map[] = {N_("Wrong password or refused: "), N_("No answer from "), N_("No saved network in range"), N_("Connection lost"), N_("Wi-Fi scan failed"), N_("Wi-Fi scan timed out"), N_("Clock not set"), N_("SSID 1-32 bytes, password empty or 8-63"), N_("Tile is not 256 or 512 px"), N_("Not a PNG image"), N_("No location"), N_("SD full: web tiles not cached")};
  for(auto m : map)
    if(e.startsWith(m)) return String(trText(m)) + e.substring(strlen(m));
  return e;
}
unsigned netCount() {
  NetRow rows[Internet::MaxSeen + Internet::MaxSaved];
  return netRows(rows, Internet::MaxSeen + Internet::MaxSaved);
}
uint32_t netScanAt = 0;
void netListEnter() {
  if(selected == 0) {
    internet.setEnabled(!internet.enabled);
    notice(internet.enabled ? t("Wi-Fi client on") : t("Wi-Fi client off"), internet.enabled ? ok : graphicMuted);
    return;
  }
  NetRow rows[Internet::MaxSeen + Internet::MaxSaved];
  unsigned total = netRows(rows, Internet::MaxSeen + Internet::MaxSaved);
  if(unsigned(selected - 1) >= total) return;
  auto& r = rows[selected - 1];
  if(r.saved || r.open) {
    if(!r.saved && !internet.save(r.ssid, "")) {
      notice(netError(), bad);
      return;
    }
    internet.connectTo(r.ssid);
    notice(t("Connecting to ") + r.ssid);
    return;
  }
  pendingSsid = r.ssid;
  edit = "";
  editing = true;
}
void netListErase() {
  NetRow rows[Internet::MaxSeen + Internet::MaxSaved];
  unsigned total = netRows(rows, Internet::MaxSeen + Internet::MaxSaved);
  if(selected > 0 && unsigned(selected - 1) < total && rows[selected - 1].saved) {
    String name = rows[selected - 1].ssid;
    if(internet.forget(name)) notice(t("Forgotten: ") + name, graphicMuted);
    selected = min(selected, int(netCount()));
    return;
  }
  internet.rescan();
  notice(t("Searching..."), graphicMuted);
}
void drawNetList() {
  auto& d = g();
  bool focus = selected == 0;
  target(8, 25, 304, 40, 0);
  panel(8, 25, 304, 40, focus ? surfaceFocus : surfaceCard, radiusCard);
  if(focus) ring(8, 25, 304, 40, radiusCard);
  d.fillRoundRect(16, 33, 24, 24, radiusWell, surfaceWell);
  icon(IcWifi, 28, 45, 7, internet.online() ? ok : internet.enabled ? info
                                                                    : iconSubtle,
       surfaceWell);
  // The last failure (wrong password, no answer) stays visible until the next attempt starts.
  bool failed = internet.enabled && !internet.online() && internet.error.length() && internet.state != Internet::Connecting && internet.state != Internet::Scanning;
  text(48, 42, t("Wi-Fi client"), textPrimary, bold);
  text(48, 56, fit(failed ? netError() : internet.stateText(), 220, small), internet.online() ? ok : failed ? warn
                                                                                                            : textMuted,
       small);
  toggle(274, 37, internet.enabled);
  NetRow rows[Internet::MaxSeen + Internet::MaxSaved];
  unsigned total = netRows(rows, Internet::MaxSeen + Internet::MaxSaved);
  if(!internet.enabled) textCenter(160, 120, t("Turn the client on to find networks"), textMuted);
  else if(!total) textCenter(160, 120, internet.state == Internet::Paused ? internet.stateText() : t("Searching for networks..."), textMuted);
  int first = max(0, selected - 1 - 5);
  for(unsigned i = first; i < total && i < unsigned(first + 6); i++) {
    int y = 70 + (i - first) * 24;
    auto& r = rows[i];
    bool f = selected == int(i) + 1;
    listRow(y, 22, f);
    target(8, y, 304, 22, i + 1);
    bool current = internet.online() && internet.ssid == r.ssid;
    if(r.visible) bars(18, y + 16, r.rssi >= -55 ? 4 : r.rssi >= -67 ? 3
                                                     : r.rssi >= -78 ? 2
                                                                     : 1,
                       current ? ok : f ? graphicPrimary
                                        : graphicMuted);
    else text(18, y + 15, "--", textSubtle, small);
    text(40, y + 15, fit(r.ssid, 150), current ? ok : textPrimary, f ? bold : body);
    String tag = current ? t("connected") : r.saved ? (r.visible ? t("saved") : t("saved, away"))
                                          : r.open  ? t("open")
                                                    : "";
    if(!r.open && !current) icon(IcLock, 297, y + 11, 6, iconMuted);
    if(tag.length()) textRight(286, y + 15, tag, current ? ok : r.saved ? accent
                                                                        : textMuted,
                               small);
  }
  scrollbar(first, 6, total, 70, 144);
  bool saved = selected > 0 && unsigned(selected - 1) < total && rows[selected - 1].saved;
  footer({{"OK", selected == 0 ? t("On/off") : t("Connect")}, {"DEL", saved ? t("Forget") : t("Rescan")}, {"BACK", t("Back")}});
}
// OK on a Connections row: the access point, Internet over Wi-Fi, BLE, the public key.
void networkEnter() {
  if(selected == 0) portalToggle();
  if(selected == 1) {
    if(config.role != RoleNormal) notice(t("Internet over Wi-Fi is off in this mode"), warn);
    else change(NetList);
  }
  if(selected == 2) bleToggle();
  if(selected == 3) notice(t("Public key: ") + meshRadio.publicKeyText().substring(0, 16));
}
