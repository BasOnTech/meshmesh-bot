// GPS & compass (included by Ui.cpp): position, heading and the compass calibration.
void compassRose(int cx, int cy, int r) {
  auto& d = g();
  d.fillCircle(cx, cy, r, surfaceCard);
  d.drawCircle(cx, cy, r, border);
  d.drawCircle(cx, cy, r - 1, border);
  for(int a = 0; a < 360; a += 30) {
    float rad = a * M_PI / 180;
    int l = a % 90 ? 4 : 7;
    d.drawLine(cx + roundf((r - 2) * sinf(rad)), cy - roundf((r - 2) * cosf(rad)), cx + roundf((r - 2 - l) * sinf(rad)), cy - roundf((r - 2 - l) * cosf(rad)), a % 90 ? graphicSubtle : graphicMuted);
  }
  const char* dirs[] = {tr("N"), tr("E"), tr("S"), tr("W")};
  for(int k = 0; k < 4; k++) {
    float rad = k * M_PI / 2;
    textCenter(cx + roundf((r - 16) * sinf(rad)), cy - roundf((r - 16) * cosf(rad)) + 4, dirs[k], k ? textMuted : bad, bold);
  }
  if(navigation.headingValid) {
    float a = navigation.heading * M_PI / 180;
    int tx = cx + roundf((r - 8) * sinf(a)), ty = cy - roundf((r - 8) * cosf(a));
    d.fillTriangle(tx, ty, cx + roundf(5 * cosf(a)), cy + roundf(5 * sinf(a)), cx - roundf(5 * cosf(a)), cy - roundf(5 * sinf(a)), accent);
    d.fillCircle(cx, cy, 3, graphicPrimary);
  } else textCenter(cx, cy + 4, "--", textSubtle, bold);
}
void drawSensors() {
  compassRose(62, 84, 54);
  String heading = navigation.headingValid ? String(int(navigation.heading + .5f) % 360) + "°" : navigation.calibrated ? t("hold flat")
                                                                                                                       : t("not calibrated");
  textCenter(62, 153, heading, navigation.headingValid ? textPrimary : warn, navigation.headingValid ? bold : small);
  bool fix = hardware.gpsFix();
  int y = 38;
  auto row = [&](const String& name, const String& value, uint16_t color) {text(130,y,name,textMuted,small);textRight(306,y,fit(value,120),color);y+=19; };
  // The GPS switch is the first item: the receiver is turned on and off where its position is used.
  {
    bool focus = selected == 0;
    target(122, 24, 192, 22, 0);
    panel(122, 24, 192, 22, focus ? surfaceFocus : surfaceCard, radiusRowCompact);
    if(focus) ring(122, 24, 192, 22, radiusRowCompact);
    text(130, y, "GPS", textMuted, small);
    g().fillRoundRect(280, 28, 26, 14, radiusPill, config.gps ? ok : border);
    g().fillCircle(config.gps ? 298 : 287, 35, 5, graphicPrimary);
    textRight(274, y, fit(!config.gps ? t("off") : fix                    ? t("fix")
                                                 : hardware.clockConflict ? t("old date")
                                                                          : t("searching"),
                          136),
              !config.gps ? textMuted : fix ? ok
                                            : warn);
    y += 19;
  }
  unsigned view = config.gps ? hardware.gpsInView() : 0;
  row(t("Satellites"), String(hardware.gps.satellites.value()) + (view ? t(" of ") + String(view) + t(" in view") : String()), textPrimary);
  row(t("Latitude"), fix ? String(hardware.gps.location.lat(), 5) : String("-"), textPrimary);
  row(t("Longitude"), fix ? String(hardware.gps.location.lng(), 5) : String("-"), textPrimary);
  row(t("Clock"), (hardware.clockSource == "unset" ? t("not set") : hardware.clockSource == "manual" ? t("manual")
                                                                                                     : hardware.clockSource) +
                    (hardware.clockTrusted ? t(" · trusted") : String()),
      textPrimary);
  row(t("Compass"), navigation.calibrating ? count(navigation.samples, "sample", "samples") : navigation.calibrated ? t("calibrated")
                                                                                                                    : t("needs calibration"),
      navigation.calibrating ? warn : textPrimary);
  String actions[] = {t("Share position"), navigation.calibrating ? t("Finish calibration") : t("Calibrate compass")};
  String hints[] = {fix ? t("Advert with GPS") : t("Needs a fresh GPS fix"), navigation.calibrating ? t("Rotate in all directions") : t("Rotate for 20+ seconds")};
  for(int i = 0; i < 2; i++) {
    int x = 8 + i * 154;
    bool focus = selected == i + 1;
    target(x, 168, 150, 46, i + 1);
    panel(x, 168, 150, 46, focus ? surfaceFocus : surfaceCard, radiusRow);
    if(focus) ring(x, 168, 150, 46, radiusRow);
    text(x + 10, 186, fit(actions[i], 132, bold), textPrimary, bold);
    text(x + 10, 203, fit(hints[i], 132, small), textMuted, small);
  }
  footer({{"<>", t("Select")}, {"OK", selected ? t("Run") : config.gps ? t("GPS off")
                                                                       : t("GPS on")},
          {"BACK", t("Back")}});
}
// Navigation, hold ADV and the screen settings turn the GPS on and off; an open settings draft follows.
void gpsToggle() {
  config.gps = !config.gps;
  config.save();
  hardware.setGps(config.gps);
  draft.gps = config.gps;
  notice("GPS: " + flag(config.gps), config.gps ? ok : graphicMuted);
}
// <> select a row; OK turns the GPS on or off, shares the position or calibrates the compass.
bool sensorsKey(int key) {
  if(key == KeyLeft || key == KeyRight) {
    selected = (selected + (key == KeyLeft ? 2 : 1)) % 3;
    return true;
  }
  if(key != Enter && key != KeyHold) return false;
  if(selected == 0) gpsToggle();
  else if(selected == 1) {
    bool sent = meshRadio.sendPosition();
    notice(sent ? t("Position shared") : t("Waiting for GPS fix"), sent ? ok : warn);
  } else if(navigation.calibrating) {
    bool saved = navigation.finish();
    notice(saved ? t("Calibration saved") : t("Rotate wider, at least 20 seconds"), saved ? ok : warn);
  } else {
    navigation.start();
    notice(t("Rotate in all directions"));
  }
  return true;
}
