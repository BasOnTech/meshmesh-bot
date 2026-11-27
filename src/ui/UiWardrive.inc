// Wardrive (included by Ui.cpp): the coverage logger page and its keys.
// Wardrive (Wardrive.h): what it logs, the position it uses, the last ping and the repeaters heard.
String wardriveMode() {
  if(!wardrive::running()) return t("Off");
  String s;
  if(wardrive::settings.passive) s = t("Listening");
  if(wardrive::settings.ping) s += String(s.length() ? " + " : "") + t("pings");
  if(wardrive::settings.nets) s += String(s.length() ? " + " : "") + "Wi-Fi/BLE";
  return s;
}
void drawWardrive() {
  int32_t lat, lon;
  bool phone, fix = wardrive::position(lat, lon, phone), on = wardrive::running();
  panel(8, 26, 304, 44, surfaceCard, radiusCard);
  icon(IcTower, 26, 48, 9, on ? ok : iconSubtle, surfaceCard);
  text(44, 44, fit(wardriveMode(), 256, bold), on ? textPrimary : textMuted, bold);
  text(44, 61, fit(fix ? (phone ? t("Phone: ") : "GPS: ") + String(lat / 1e6, 5) + ", " + String(lon / 1e6, 5) : t("No position: GPS fix or phone app"), 256, small), fix ? textMuted : warn, small);
  panel(8, 74, 304, 42, surfaceCard, radiusCard);
  String counts = count(wardrive::points(), "point", "points");
  if(wardrive::settings.nets || wardrive::nets()) counts += " · Wi-Fi/BLE " + String(wardrive::nets());
  text(18, 90, fit(counts, 288, small), textPrimary, small);
  const auto& r = wardrive::last();
  String last = wardrive::pinging() ? t("Ping sent, listening for repeaters...") : !r.done ? t("No pings yet: P sends one")
                                                                                 : r.count ? t("Last ping: ") + count(r.count, "repeater", "repeaters") + ", " + hashText(r.hash, r.size) + " " + snrText(r.snr) + " · " + ago(millis() - r.at)
                                                                                           : t("Last ping: no repeater heard it") + String(" · ") + ago(millis() - r.at);
  text(18, 107, fit(last, 288, small), wardrive::pinging() ? accent : r.done && !r.count ? bad
                                                                                         : textMuted,
       small);
  const wardrive::Repeater* list;
  unsigned n = wardrive::repeaters(list), order[24];
  for(unsigned i = 0; i < n; i++) order[i] = i;
  std::sort(order, order + n, [&](unsigned a, unsigned b) {unsigned x=list[a].echoes*4+list[a].rx,y=list[b].echoes*4+list[b].rx;return x!=y?x>y:list[a].snr>list[b].snr; });
  if(!n) textCenter(160, 150, on ? t("No repeaters heard yet") : t("OK starts listening and pings"), textSubtle, small);
  for(unsigned i = 0; i < n && i < 5; i++) {
    auto& e = list[order[i]];
    int y = 132 + i * 17;
    text(18, y, hashText(e.hash, e.size), textPrimary, bold);
    text(80, y, snrText(e.snr), snrColor(e.snr), small);
    text(150, y, t("rx ") + String(e.rx) + t(" · echo ") + String(e.echoes), textMuted, small);
    textRight(306, y, ago(millis() - e.at), textSubtle, small);
  }
  footer({{"OK", on ? t("Stop") : t("Start")}, {"P", t("Ping")}, {"N", "Wi-Fi/BLE"}, {"BACK", t("Back")}});
}
// OK starts (listening and pings) or stops; P pings now; N switches the Wi-Fi/BLE log.
bool wardriveKey(int key) {
  StaticJsonDocument<128> d;
  d["action"] = "settings";
  if(key == Enter) {
    if(wardrive::running()) {
      d["action"] = "stop";
    } else {
      d["passive"] = true;
      d["ping"] = true;
    }
    String r = wardrive::command(d.as<JsonObjectConst>());
    if(r.startsWith("ERR")) notice(r, bad);
    else notice(wardrive::running() ? t("Wardrive on") : t("Wardrive off"), wardrive::running() ? ok : graphicMuted);
    return true;
  }
  if(key == 'p' || key == 'P') {
    int32_t lat, lon;
    bool phone;
    notice(wardrive::ping() ? t("Ping queued") : !wardrive::position(lat, lon, phone) ? t("No position yet")
                                                                                      : t("Wait: a ping is under way"),
           wardrive::pinging() ? accent : warn);
    return true;
  }
  if(key == 'n' || key == 'N') {
    d["nets"] = !wardrive::settings.nets;
    String r = wardrive::command(d.as<JsonObjectConst>());
    notice(r.startsWith("ERR") ? r : "Wi-Fi/BLE: " + flag(wardrive::settings.nets), r.startsWith("ERR") ? bad : ok);
    return true;
  }
  return false;
}
