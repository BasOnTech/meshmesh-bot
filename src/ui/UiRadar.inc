// Signal radar (included by Ui.cpp): the radar, homing on one signal and the Wi-Fi CSI motion sensor.
// Signal radar: Wi-Fi access points, Bluetooth devices and directly heard LoRa nodes. Nearer the centre means a
// stronger signal; the angle only keeps targets apart, because RSSI carries no bearing.
uint64_t scopeId = 0;
RadarTarget::Kind scopeKind = RadarTarget::Wifi;
bool radarSound = true;
uint32_t pingAt = 0, pingedSamples = 0;
void scopeSelect(int i) {
  if(i < 0 || i >= int(radar.count)) return;
  scopeId = radar.targets[i].id;
  scopeKind = radar.targets[i].kind;
}
// Until the user moves it, the selection stays on the strongest signal; then it follows its target
// through re-sorting, and a vanished target passes it back to the strongest.
int scopeSelected() {
  if(!scopeManual && radar.count) {
    scopeSelect(0);
    return 0;
  }
  for(unsigned i = 0; i < radar.count; i++)
    if(radar.targets[i].id == scopeId && radar.targets[i].kind == scopeKind) return i;
  if(!radar.count) return -1;
  scopeSelect(0);
  return 0;
}
float signalLevel(int rssi) { return constrain((rssi + 100) / 65.f, 0.f, 1.f); } // -100 .. -35 dBm
uint16_t mix(uint32_t a, uint32_t b, float f) {
  auto ch = [&](int s) { return int(((a >> s) & 255) * (1 - f) + ((b >> s) & 255) * f); };
  return rgb(ch(16) << 16 | ch(8) << 8 | ch(0));
}
uint32_t targetHue(const RadarTarget& r) { return r.kind == RadarTarget::Lora ? theme->secondary : r.kind == RadarTarget::Ble ? (r.device ? theme->tertiary : theme->graphicMuted)
                                                                                                 : r.open                     ? theme->warn
                                                                                                                              : theme->info; }
String deviceText(const RadarTarget& r) {
  switch(r.device) {
  case RadarTarget::Phone: return t("Phone");
  case RadarTarget::Watch: return t("Watch");
  case RadarTarget::Audio: return t("Headphones");
  case RadarTarget::Personal: return t("Phone/watch");
  default: return t("BLE device");
  }
}
String vendorText(uint16_t v) {
  switch(v) {
  case 0x004c: return "Apple";
  case 0x0075: return "Samsung";
  case 0x00e0: return "Google";
  case 0x027d: return "Huawei";
  case 0x038f: return "Xiaomi";
  case 0x0087: return "Garmin";
  case 0x0006: return "Microsoft";
  }
  return "";
}
String targetName(const RadarTarget& r) {
  if(r.name[0]) return r.name;
  if(r.kind == RadarTarget::Ble) {
    String v = vendorText(r.vendor);
    return deviceText(r) + (v.length() ? " " + v : String());
  }
  return r.kind == RadarTarget::Wifi ? t("Hidden network") : meshRadio.idText(r.id);
}
String macText(uint64_t id) {
  char b[18];
  snprintf(b, sizeof b, "%02X:%02X:%02X:%02X:%02X:%02X", unsigned(id >> 40 & 255), unsigned(id >> 32 & 255), unsigned(id >> 24 & 255), unsigned(id >> 16 & 255), unsigned(id >> 8 & 255), unsigned(id & 255));
  return b;
}
bool radarFresh() { return radar.fresh(); }
String bleState() {
  switch(radar.ble) {
  case Radar::BleBusy: return t("Bluetooth busy: probe");
  case Radar::BleFailed: return t("Bluetooth error");
  default: return "";
  }
}
String wifiState() {
  switch(radar.wifi) {
  case Radar::WifiPortal: return t("Wi-Fi busy: access point");
  case Radar::WifiBusy: return t("Wi-Fi busy: probe");
  case Radar::WifiFailed: return t("Wi-Fi error");
  default: return radar.sweeps ? "" : t("Scanning Wi-Fi...");
  }
}
void drawScope() {
  auto& d = g();
  const int cx = 96, cy = 116, R = 86;
  const uint32_t cardRgb = theme->surfaceCard;
  uint32_t now = millis();
  d.fillCircle(cx, cy, R, surfaceCard);
  for(int k = 1; k <= 4; k++) d.drawCircle(cx, cy, R * k / 4, k == 4 ? border : gridLine);
  d.drawFastHLine(cx - R, cy, 2 * R + 1, gridLine);
  d.drawFastVLine(cx, cy - R, 2 * R + 1, gridLine);
  float sweep = (now % 3000) * 2 * M_PI / 3000; // one turn per 3 s, fading trail behind the beam
  for(int k = 24; k >= 0; k--) {
    float a = sweep - k * .03f;
    d.drawLine(cx, cy, cx + roundf(R * sinf(a)), cy - roundf(R * cosf(a)), mix(theme->accent, cardRgb, .25f + k * .03f));
  }
  int sel = scopeSelected();
  for(int i = radar.count - 1; i >= 0; i--) {
    const RadarTarget& r = radar.targets[i];
    uint32_t h = uint32_t(r.id ^ (r.id >> 29)) * 2654435761u;
    float a = (h >> 8) % 360 * M_PI / 180;
    float dist = R * (.1f + .84f * (1 - signalLevel(r.rssi)));
    int x = cx + roundf(dist * sinf(a)), y = cy - roundf(dist * cosf(a));
    float since = fmodf(sweep - a + 4 * M_PI, 2 * M_PI) / (2 * M_PI); // blips glow as the beam passes
    float faded = r.kind == RadarTarget::Wifi ? 0 : constrain((now - r.seen) / 30000.f, 0.f, .5f); // quiet targets fade
    d.fillCircle(x, y, i == sel ? 4 : 3, mix(targetHue(r), cardRgb, min(.85f, .15f + .6f * since + faded)));
    if(i == sel) {
      d.drawCircle(x, y, 7, graphicPrimary);
      d.drawCircle(x, y, 8, accent);
    }
  }
  d.fillCircle(cx, cy, 3, graphicPrimary);
  textCenter(cx, 216, t("closer to centre = stronger"), textSubtle, small);
  text(194, 34, fit("Wi-Fi " + String(radar.counted(RadarTarget::Wifi)) + "·BLE " + String(radar.counted(RadarTarget::Ble)) + "·LoRa " + String(radar.counted(RadarTarget::Lora)), 120, small), textMuted, small);
  int first = max(0, sel - 6);
  for(int i = first; i < int(radar.count) && i < first + 7; i++) {
    const RadarTarget& r = radar.targets[i];
    int y = 40 + (i - first) * 22;
    bool focus = i == sel;
    if(focus) panel(190, y, 124, 20, surfaceFocus, radiusRowCompact);
    d.fillCircle(199, y + 10, 3, rgb(targetHue(r)));
    text(206, y + 14, fit(targetName(r), 76), focus ? textPrimary : textMuted);
    textRight(310, y + 14, String(int(r.rssi)), focus ? textPrimary : textMuted, bold);
  }
  String state = wifiState();
  if(!state.length()) state = bleState();
  unsigned people = radar.personal(30000);
  if(!state.length() && people) state = count(people, "phone/watch nearby", "phones/watches nearby");
  if(!radar.count) textCenter(252, 110, state.length() ? state : t("No signals yet"), radar.wifi == Radar::WifiReady ? textMuted : warn, small);
  else if(state.length()) text(194, 212, fit(state, 120, small), radar.wifi == Radar::WifiReady && radar.ble != Radar::BleFailed && radar.ble != Radar::BleBusy ? textMuted : warn, small);
  footer({{"OK", t("Home in")}, {"<>", t("Motion")}, {"^v", t("Select")}, {"BACK", t("Menu")}});
}
void drawHoming() {
  auto& d = g();
  const RadarTarget& f = radar.focus;
  uint16_t hue = rgb(targetHue(f));
  bool fresh = radarFresh(), lora = f.kind == RadarTarget::Lora, bt = f.kind == RadarTarget::Ble;
  panel(8, 26, 304, 34, surfaceCard, radiusCard);
  d.fillRoundRect(14, 31, 24, 24, radiusWell, surfaceWell);
  icon(lora ? IcTower : bt ? (f.device ? IcPerson : IcBle)
                           : IcWifi,
       26, 43, 7, hue, surfaceWell);
  text(46, 40, fit(targetName(f), 258, bold), textPrimary, bold);
  String vendor = vendorText(f.vendor);
  text(46, 54, fit(lora ? "LoRa · " + meshRadio.idText(f.id) + t(" · direct packets only") : bt ? "Bluetooth · " + deviceText(f) + (vendor.length() ? " · " + vendor : String()) + t(" · address changes over time")
                                                                                                : "Wi-Fi · " + t("channel ") + String(f.channel) + " · " + macText(f.id),
                   258, small),
       textMuted, small);
  int x = text(14, 112, radar.samples ? String(int(lroundf(radar.fast))) : String("--"), fresh ? textPrimary : textSubtle, digits);
  text(x + 4, 112, "dBm", textMuted, small);
  // Homing indicator: the fast average against the slow one, as in the RSSI tracker.
  int trend = radar.trend();
  panel(186, 66, 126, 46, surfaceCard, radiusCard);
  if(f.kind == RadarTarget::Wifi && radar.wifi != Radar::WifiReady) text(196, 93, fit(wifiState(), 108, small), warn, small);
  else if(bt && radar.ble != Radar::BleReady) text(196, 93, fit(bleState(), 108, small), warn, small);
  else if(!fresh) text(196, 93, radar.samples > 1 ? t("Lost") : t("No signal"), warn, bold);
  else {
    uint16_t c = trend > 0 ? ok : trend < 0 ? bad
                                            : iconMuted;
    if(trend) tri(206, 89, trend > 0 ? 0 : 2, 10, c);
    else {
      d.fillRect(197, 83, 18, 3, c);
      d.fillRect(197, 91, 18, 3, c);
    }
    text(224, 86, trend > 0 ? t("Stronger") : trend < 0 ? t("Weaker")
                                                        : t("Steady"),
         c, bold);
    text(224, 101, t("keep moving"), textSubtle, small);
  }
  // Strength meter with the peak marker.
  float level = signalLevel(lroundf(radar.fast));
  d.fillRoundRect(14, 120, 292, 8, radiusBar, border);
  if(radar.samples) d.fillRoundRect(14, 120, max(3, int(292 * level)), 8, radiusBar, !fresh ? graphicSubtle : level < .3f ? bad
                                                                                                    : level < .6f ? warn
                                                                                                                  : ok);
  if(radar.samples) {
    int px = 14 + int(291 * signalLevel(radar.peak));
    d.fillRect(px - 1, 116, 3, 16, graphicPrimary);
  }
  text(14, 142, "-100", textSubtle, small);
  textRight(306, 142, "-35 dBm", textSubtle, small);
  textCenter(160, 142, t("peak ") + (radar.samples ? String(int(lroundf(radar.peak))) + " dBm (" + String(radar.fast - radar.peak, 0) + ")" : String("-")), textMuted, small);
  // History: the last 120 samples, oldest on the left.
  panel(8, 148, 304, 58, surfaceCard, radiusCard);
  for(int dbm : {-90, -70, -50}) {
    int y = 202 - int(50 * signalLevel(dbm));
    d.drawFastHLine(12, y, 296, gridLine);
  }
  unsigned n = radar.historyCount();
  int px = 0, py = 0;
  for(unsigned i = 0; i < n; i++) {
    int x = 12 + int(i * 296 / (Radar::HistorySize - 1)), y = 202 - int(50 * signalLevel(radar.sample(i)));
    if(i) d.drawLine(px, py, x, y, accent);
    else d.drawPixel(x, y, accent);
    px = x;
    py = y;
  }
  if(!n) textCenter(160, 181, t("Waiting for samples"), textSubtle, small);
  uint32_t age = radar.samples ? millis() - radar.lastSample : 0;
  String stats = lora ? count(radar.samples, "packet", "packets") : String(radar.rate) + (bt ? t(" adverts/s") : t(" beacons/s"));
  int sx = text(12, 216, stats, textMuted, small);
  // Sample age: green up to 0.5 s, yellow up to 1.5 s, then red (LoRa counts in minutes).
  if(radar.samples) text(sx, 216, " · " + (age < 60000 ? String(age / 1000.f, 1) + t(" s ago") : ago(age)), lora ? textMuted : age < 500  ? ok
                                                                                                                             : age < 1500 ? warn
                                                                                                                                          : bad,
                         small);
  footer({{"OK", radarSound ? t("Mute") : t("Sound")}, {"<>", t("Reset peak")}, {"BACK", t("List")}});
}
// CSI motion: one board is the beacon, this one the sensor (or the other way round).
uint32_t motionBeepAt = 0;
bool wasMoving = false;
void drawMotion() {
  auto& d = g();
  bool beacon = radar.csi == Radar::CsiBeacon, heard = radar.beaconHeard();
  uint32_t now = millis();
  panel(8, 26, 304, 34, surfaceCard, radiusCard);
  d.fillRoundRect(14, 31, 24, 24, radiusWell, surfaceWell);
  icon(beacon ? IcRadio : IcPulse, 26, 43, 7, beacon ? accent : heard ? ok
                                                                      : warn,
       surfaceWell);
  String blocked = radar.wifi == Radar::WifiBusy ? t("Wi-Fi busy: probe") : radar.wifi == Radar::WifiFailed              ? t("Wi-Fi error")
                                                                          : (!beacon && radar.wifi == Radar::WifiPortal) ? t("Turn the access point off")
                                                                                                                         : "";
  text(46, 40, beacon ? t("Beacon for another board") : t("Sensor"), textPrimary, bold);
  text(46, 54, fit(blocked.length() ? blocked : beacon ? String(radar.csiRate) + t(" frames/s on channel 1")
                                              : heard  ? t("beacon ") + String(radar.csiRate) + t(" frames/s · ") + String(int(radar.csiRssi)) + " dBm"
                                                       : t("waiting: set the other board to CSI beacon"),
                   258, small),
       blocked.length() ? warn : textMuted, small);
  String state;
  uint16_t color;
  if(blocked.length()) {
    state = t("NO WI-FI");
    color = warn;
  } else if(beacon) {
    state = t("BEACON ON");
    color = accent;
  } else if(!heard) {
    state = t("NO BEACON");
    color = warn;
  } else if(radar.csiStale) {
    state = t("CSI FROZEN");
    color = warn;
  } else if(radar.calibrateUntil) {
    state = t("CALIBRATING ") + String((radar.calibrateUntil - now + 999) / 1000) + t(" s");
    color = info;
  } else if(radar.moving) {
    state = t("MOTION");
    color = bad;
  } else {
    state = t("STILL");
    color = ok;
  }
  textCenter(160, 92, state, color, big);
  if(!beacon) {
    float threshold = radar.motionThreshold(), scale = max(threshold * 2.5f, radar.activity * 1.1f);
    d.fillRoundRect(14, 104, 292, 8, radiusBar, border);
    if(heard) d.fillRoundRect(14, 104, max(3, int(292 * min(1.f, radar.activity / scale))), 8, radiusBar, radar.moving ? bad : ok);
    int tx = 14 + int(291 * threshold / scale);
    d.fillRect(tx - 1, 100, 3, 16, graphicPrimary);
    text(14, 128, t("activity ") + String(radar.activity * 1000, 1), textMuted, small);
    textRight(306, 128, t("threshold ") + String(threshold * 1000, 1) + (radar.baseline > 0 ? String() : t(" (default)")), radar.baseline > 0 ? textMuted : warn, small);
    // History: one point per 0.5 s, threshold dotted.
    panel(8, 136, 304, 70, surfaceCard, radiusCard);
    unsigned n = radar.motionCount();
    float top = threshold * 2.5f;
    for(unsigned i = 0; i < n; i++) top = max(top, radar.motion(i));
    int ty = 202 - int(62 * threshold / top);
    for(int x = 12; x < 308; x += 6) d.drawFastHLine(x, ty, 3, graphicSubtle);
    int px = 0, py = 0;
    for(unsigned i = 0; i < n; i++) {
      int x = 12 + int(i * 296 / (Radar::MotionHistory - 1)), y = 202 - int(62 * radar.motion(i) / top);
      if(i) d.drawLine(px, py, x, y, radar.motion(i) > threshold ? bad : accent);
      px = x;
      py = y;
    }
    if(!n) textCenter(160, 175, t("Walk between the boards to see the trace"), textSubtle, small);
    text(12, 216, fit(t("Calibrate with the area empty and still (10 s)"), 296, small), textSubtle, small);
  } else text(12, 130, fit(t("Place the boards 2-5 m apart; motion between them is sensed"), 296, small), textMuted, small);
  if(beacon) footer({{"<>", t("Signals")}, {"B", t("Sensor")}, {"BACK", t("Menu")}});
  else footer({{"OK", t("Calibrate")}, {"<>", t("Signals")}, {"B", t("Beacon")}, {"BACK", t("Menu")}});
}
// The radar pages: ^v pick a signal and <> switch to the motion sensor (and back), B swaps its role; on homing <>
// resets the peak. OK homes on the signal, calibrates the motion sensor or turns the homing ping on and off.
bool radarKey(int key) {
  if(page == Scope && (key == KeyUp || key == KeyDown)) {
    int i = scopeSelected();
    scopeManual = true;
    if(radar.count) scopeSelect((i + (key == KeyUp ? -1 : 1) + radar.count) % radar.count);
    return true;
  }
  if(page == Scope && (key == KeyLeft || key == KeyRight)) {
    change(Motion);
    return true;
  }
  if(page == Motion && (key == KeyLeft || key == KeyRight)) {
    change(Scope);
    return true;
  }
  if(page == Motion && (key == 'b' || key == 'B')) {
    csiBeaconRole = !csiBeaconRole;
    radar.setCsi(csiBeaconRole ? Radar::CsiBeacon : Radar::CsiSensor);
    notice(csiBeaconRole ? t("This board is now the beacon") : t("This board is now the sensor"));
    return true;
  }
  if(page == Homing && (key == KeyLeft || key == KeyRight)) {
    radar.resetPeak();
    notice(t("Peak reset"), graphicMuted);
    return true;
  }
  if(key != Enter && key != KeyHold) return false;
  if(page == Scope) {
    int i = scopeSelected();
    if(i >= 0 && radar.track(i)) {
      scopeManual = true;
      pingedSamples = radar.samples;
      change(Homing);
    } else notice(t("No signal selected"), warn);
  } else if(page == Motion) {
    if(radar.csi == Radar::CsiSensor && radar.beaconHeard()) {
      radar.calibrate();
      notice(t("Calibrating: keep the area still for 10 s"), info);
    } else notice(t("Needs a heard beacon"), warn);
  } else {
    radarSound = !radarSound;
    notice(radarSound ? (config.sound ? t("Ping on") : t("Device sound is off in settings")) : t("Ping off"), radarSound && !config.sound ? warn : graphicMuted);
  }
  return true;
}
