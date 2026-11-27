// Settings (included by Ui.cpp): the settings menu, the Radio and Screen & device editors and turning off.
const int settingsCount = 9;
// Power off (Power.cpp): the M9 has no button on a GPIO, RESET or the power switch turns it on.
String powerOnHint() { return t("To turn on: RESET or the power switch"); }
bool farewell = false; // the board is about to turn off: the last frame
void drawOff() {
  panel(30, 62, 260, 116, surfaceCard, radiusDialog);
  icon(IcPower, 160, 96, 14, farewell ? iconMuted : warn, surfaceCard);
  textCenter(160, 138, farewell ? t("Device is off") : t("Turning off..."), textPrimary, bold);
  textCenter(160, 160, powerOnHint(), textMuted, small);
}
uint32_t powerArmed = 0; // the first OK on "Turn off" arms it for 5 s
uint32_t discardArmed = 0; // the first BACK on an unsaved Radio or Display draft arms discarding it for 5 s
void drawSettings() {
  Icon icons[] = {IcRadio, IcScreen, IcCompass, IcWifi, IcPulse, IcHelp, IcPin, IcTower, IcPower};
  uint16_t hues[] = {accent, info, warn, info, ok, iconMuted, ok, secondary, bad};
  String names[] = {t("Radio"), t("Screen & device"), t("GPS & compass"), t("Connections"), t("Module health"), t("Keys & help"), t("Saved maps"), t("Device mode"), t("Turn off")};
  String details[] = {String(config.frequency, 3) + t(" MHz · SF") + String(config.sf) + " · " + String(config.power) + " dBm",
                      t("Brightness ") + String(config.brightness) + " · " + langNames[config.lang < LangCount ? config.lang : 0],
                      "GPS " + flag(config.gps) + " · " + (navigation.calibrated ? t("compass calibrated") : t("compass not calibrated")),
                      "Wi-Fi " + flag(portalActive()) + " · BLE " + flag(bleActive()), "RX " + String(meshRadio.rxCount) + " · TX " + String(meshRadio.txCount) + " · " + String(meshRadio.relayed) + t(" relayed"),
                      t("What every key does"), maps.title.length() ? maps.title + " · " + count(maps.tileCount, "tile", "tiles") : t("No maps"), config.role == RoleRepeater ? t("MeshCore repeater") : config.role == RoleRoom ? t("MeshCore room server")
                                                                                                                                                                                                                                 : t("Normal: chats, maps, radar"),
                      powerOnHint()};
  const int shown = 7; // menu rows as on home, from under the status bar to the footer
  int first = menuFirst(settingsCount, shown);
  for(int i = first; i < settingsCount && i < first + shown; i++) menuRow(24 + (i - first) * tileRowH, i, icons[i], hues[i], names[i], details[i]);
  scrollbar(first, shown, settingsCount, 24, shown * tileRowH - 1);
  footer({{"OK", t("Open")}, {"^v", t("Select")}, {"BACK", t("Menu")}});
}
int settingRows() { return 13; } // Radio: the preset, 8 values, the region, its search and the previous profile; Display: 10 values; then Save
// Radio row 0: the regional preset (RadioPresets.h), <> sets frequency, BW, SF, CR and the path hash it gives.
// Rows 9 and 10: the default region of our floods (OK types one, <> the regions found) and the region search;
// row 11: the profile before the last change of frequency, BW, SF or CR (OK returns to it), or during a test of a
// network profile (radio try from the page) the test (OK keeps it; a restart returns the saved one).
const int RowPreset = 0, RowRegion = 9, RowFind = 10, RowPrev = 11;
String profileText(const RadioProfile& r) { return String(r.frequency, 3) + " SF" + String(r.sf) + " BW" + String(r.bandwidth, 1) + " CR" + String(r.cr); }
String regionSearchText() {
  auto& s = regions::search;
  if(regions::searching()) return t("Searching: ") + count(s.repeaters, "repeater", "repeaters");
  return s.state == regions::Search::Done ? count(s.count, "region", "regions") : t("OK: start");
}
String regionsFound() {
  String n;
  for(unsigned i = 0; i < regions::search.count; i++) n += String(i ? ", " : "") + regions::search.found[i].name;
  return n;
}
String settingName(int i) {
  if(page == Radio) {
    const char* names[] = {tr("Preset"), tr("Frequency"), tr("Bandwidth"), tr("Spreading factor"), tr("Coding rate"), tr("TX power"), tr("Relay hop limit"), tr("Relay packets"), tr("Path hash size"), tr("Region"), tr("Find regions")};
    if(i == RowPrev) return config.trial.set() ? t("Radio test") : t("Previous radio");
    return i < 11 ? names[i] : "";
  }
  const char* names[] = {tr("Name"), tr("Language"), tr("Brightness"), tr("Sounds"), tr("Auto lock"), tr("Dim screen"), tr("GPS receiver"), tr("UTC offset"), tr("Battery shows"), tr("Lock screen"), tr("Wake on message"), tr("Message popup")};
  return i < 12 ? names[i] : "";
}
String settingValue(const Config& c, int i) {
  if(page == Radio) {
    switch(i) {
    case RowPreset: {
      int p = radiopresets::match(c);
      return p < 0 ? String(t("Custom")) : String(radiopresets::list[p].name);
    }
    case 1: return String(c.frequency, 3) + t(" MHz");
    case 2: return String(c.bandwidth, 1) + t(" kHz");
    case 3: return "SF" + String(c.sf);
    case 4: return "4/" + String(c.cr);
    case 5: return String(c.power) + " dBm";
    case 6: return String(c.hops);
    case 7: return flag(c.relay);
    case 8: return count(c.pathHash, "byte", "bytes");
    case RowRegion: return c.region[0] ? String(c.region) : String(t("None"));
    case RowFind: return regionSearchText();
    case RowPrev: return config.trial.set() ? t("OK: keep") : config.previous.set() ? profileText(config.previous)
                                                                                    : t("None");
    }
    return "";
  }
  switch(i) {
  case 0: return c.name;
  case 1: return langNames[c.lang < LangCount ? c.lang : 0];
  case 2: return String(c.brightness);
  case 3: return flag(c.sound);
  case 4: return c.autoLock ? String(c.autoLock) + t(" s") : t("Off");
  case 5: return c.dimAfter ? String(c.dimAfter) + t(" s") : t("Off");
  case 6: return flag(c.gps);
  case 7: {
    int m = abs(c.utcOffset);
    char b[12];
    snprintf(b, sizeof b, "%c%02d:%02d", c.utcOffset < 0 ? '-' : '+', m / 60, m % 60);
    return b;
  }
  case 8: return c.batteryVolts ? t("Volts") : t("Percent");
  case 9: return c.lockDetails ? t("Details") : t("Hidden");
  case 10:
  case 11: {
    unsigned mode = i == 10 ? c.notifyWake : c.notifyPopup;
    return mode == 0 ? t("Off") : mode == 1 ? t("All")
                                            : t("Mentions");
  }
  }
  return "";
}
String settingHint(int i) {
  if(page == Radio) {
    switch(i) {
    case RowPreset: return t("MeshCore regional settings: frequency, BW, SF, CR");
    case 1: return t("1 kHz steps; must match every node");
    case 2: return t("62.5 / 125 / 250 / 500 kHz");
    case 3: return t("Higher SF: longer range, slower");
    case 4: return t("Error correction 4/5 to 4/8");
    case 5: return t("Radio chip setting, 0-22 dBm");
    case 6: return t("Relay packets with fewer hops than this");
    case 7: return t("Forward other nodes' MeshCore packets");
    case 8: return t("Per hop in our floods; older MeshCore drops 2-3");
    case RowRegion: return t("Repeaters pass on our floods by it; OK: type");
    case RowFind: return regions::search.count ? t("Found: ") + regionsFound() : t("Asks repeaters that hear this device");
    case RowPrev: return config.trial.set() ? t("A restart returns ") + profileText(config.trial) : config.previous.set() ? t("OK: return to it")
                                                                                                                          : t("Kept on a change of frequency, BW, SF or CR");
    }
    return t("Applied to the radio when saved");
  }
  switch(i) {
  case 0: return t("OK: edit, 1-24 UTF-8 bytes");
  case 1: return t("Menu language");
  case 2: return "10-255";
  case 3: return t("Tone for messages and ACKs");
  case 4: return t("0 = off, 30-600 s");
  case 5: return t("0 = off, 10-600 s");
  case 6: return t("Receiver power");
  case 7: return t("Local time, 15 minute steps");
  case 8: return t("Status bar: charge estimate or measured voltage");
  case 9: return t("Chess moves and message senders while locked");
  }
  return t("Stored in device memory");
}
bool draftChanged() {
  for(int i = 0; i < settingRows() - 1; i++)
    if(settingValue(draft, i) != settingValue(config, i)) return true;
  return false;
}
void drawEditor() {
  int rows = settingRows(), first = max(0, selected - 6);
  bool changed = draftChanged();
  for(int i = first; i < rows && i < first + 7; i++) {
    int y = 25 + (i - first) * 23;
    bool focus = selected == i;
    target(8, y, 304, 21, i);
    if(i == rows - 1) {
      panel(8, y, 304, 21, changed ? (focus ? accent : accentDeep) : surfaceCard, radiusRowCompact);
      if(focus && !changed) ring(8, y, 304, 21, radiusRowCompact, border);
      textCenter(160, y + 15, changed ? t("Save changes") : t("No changes"), changed ? (focus ? onFill : textPrimary) : textMuted, bold);
      continue;
    }
    listRow(y, 21, focus);
    String value = settingValue(draft, i);
    bool diff = value != settingValue(config, i);
    if(diff) g().fillCircle(17, y + 10, 2, accent);
    text(24, y + 15, settingName(i), focus ? textPrimary : textMuted);
    if(focus && !(page == Display && i == 0) && !(page == Radio && (i == RowFind || i == RowPrev))) {
      int w = measure(value, bold);
      tri(300, y + 10, 1, 4, accent);
      text(292 - w, y + 15, value, accent, bold);
      tri(284 - w, y + 10, 3, 4, accent);
    } else textRight((page == Display && i == 0) || (page == Radio && (i == RowFind || i == RowPrev)) ? 306 : 300, y + 15, fit(value, 150), diff ? accent : textPrimary, focus ? bold : body);
  }
  scrollbar(first, 7, rows, 25, 160);
  text(10, 207, fit(settingHint(selected), 300, small), textSubtle, small);
  if(page == Display && selected == 0) footer({{"OK", t("Edit name")}, {"^v", t("Select")}, {"BACK", t("Cancel")}});
  else if(page == Radio && selected == RowPrev) footer({{"OK", config.trial.set() ? t("Keep") : t("Return")}, {"^v", t("Select")}, {"BACK", t("Cancel")}});
  else if(page == Radio && selected == RowFind) footer({{"OK", t("Search")}, {"^v", t("Select")}, {"BACK", t("Cancel")}});
  else if(page == Radio && selected == RowRegion) footer({{"<>", t("Found")}, {"OK", t("Type")}, {"BACK", t("Cancel")}});
  else footer({{"<>", t("Change")}, {"OK", selected == rows - 1 ? t("Save") : t("Next value")}, {"BACK", t("Cancel")}});
}
// The network search row: stop a search, test the network found, or search (all: every profile).
void alter(int dir) {
  if(page == Radio) {
    switch(selected) {
    case RowPreset: {
      int p = radiopresets::step(draft, dir);
      if(p < 0) {
        notice(t("No preset for this radio"), warn);
        return;
      }
      radiopresets::apply(draft, p);
      break;
    }
    case 1: draft.frequency = constrain(roundf((draft.frequency + dir * .001f) * 1000) / 1000, 863.f, 870.f); break;
    case 2: {
      float bw[] = {62.5, 125, 250, 500};
      int i = 0;
      while(i < 3 && draft.bandwidth != bw[i]) i++;
      draft.bandwidth = bw[(i + dir + 4) % 4];
      break;
    }
    case 3: draft.sf = constrain(int(draft.sf) + dir, 7, 12); break;
    case 4: draft.cr = constrain(int(draft.cr) + dir, 5, 8); break;
    case 5: draft.power = constrain(int(draft.power) + dir, 0, MM_MAX_POWER); break;
    case 6: draft.hops = constrain(int(draft.hops) + dir, 0, 7); break;
    case 7: draft.relay = !draft.relay; break;
    case 8: draft.pathHash = constrain(int(draft.pathHash) + dir, 1, 3); break;
    case RowRegion: strlcpy(draft.region, regions::cycle(draft.region, dir, false).c_str(), sizeof draft.region); break;
    }
  } else switch(selected) {
    case 1: draft.lang = langStep(draft.lang, dir); break;
    case 2: draft.brightness = constrain(int(draft.brightness) + dir * 15, 10, 255); break;
    case 3: draft.sound = !draft.sound; break;
    case 4: draft.autoLock = constrain(int(draft.autoLock) + dir * 30, 0, 600); break;
    case 5: draft.dimAfter = constrain(int(draft.dimAfter) + dir * 10, 0, 600); break;
    case 6: draft.gps = !draft.gps; break;
    case 7: draft.utcOffset = constrain(int(draft.utcOffset) + dir * 15, -720, 840); break;
    case 8: draft.batteryVolts = !draft.batteryVolts; break;
    case 9: draft.lockDetails = !draft.lockDetails; break;
    case 10: draft.notifyWake = (draft.notifyWake + dir + 3) % 3; break;
    case 11: draft.notifyPopup = (draft.notifyPopup + dir + 3) % 3; break;
    }
  dirty = true;
}
// Keep the settings JSON off uiKey's stack: the saved-map library also needs SD/JSON stack space.
void saveDraft() {
  DynamicJsonDocument d(4096);
  auto error = deserializeJson(d, configJson());
  if(error) {
    notice(error.c_str(), bad);
    return;
  }
  if(page == Radio) {
    d["frequency"] = draft.frequency;
    d["bandwidth"] = draft.bandwidth;
    d["sf"] = int(draft.sf);
    d["cr"] = int(draft.cr);
    d["power"] = int(draft.power);
    d["hops"] = int(draft.hops);
    d["relay"] = draft.relay;
    d["path_hash"] = int(draft.pathHash);
    d["region"] = draft.region;
    d["preset"] = draft.preset;
  } else {
    d["name"] = draft.name;
    d["lang"] = langCodes[draft.lang];
    d.remove("russian");
    d["brightness"] = int(draft.brightness);
    d["sound"] = draft.sound;
    d["gps"] = draft.gps;
    d["auto_lock"] = int(draft.autoLock);
    d["dim_after"] = int(draft.dimAfter);
    d["utc_offset"] = int(draft.utcOffset);
    d["battery_volts"] = draft.batteryVolts;
    d["lock_details"] = draft.lockDetails;
    d["notify_wake"] = draft.notifyWake;
    d["notify_popup"] = draft.notifyPopup;
  }
  String reply = applySettings(d.as<JsonObjectConst>());
  bool saved = reply.startsWith("OK");
  notice(saved ? t("Settings saved") : reply, saved ? ok : bad);
  if(saved) change(Settings);
}
void openRolePick(bool atBoot); // UiServer.inc
// OK on a settings row opens its page; "Turn off" needs a second OK within 5 s.
void settingsEnter() {
  if(selected == settingsCount - 1) {
    if(!powerArmed || millis() - powerArmed > 5000) {
      powerArmed = millis();
      notice(t("OK again: turn off"), warn);
      return;
    }
    powerArmed = 0;
    powerOff();
    return;
  }
  Page pages[] = {Radio, Display, Sensors, Network, Diagnostics, Help, Library, RolePick};
  if(pages[selected] == RolePick) openRolePick(false);
  else change(pages[selected]);
}
// OK in the Radio or Screen & device editor: save, type the name or region, return to the previous profile,
// search the regions, or step the value.
void editorEnter() {
  if(selected == settingRows() - 1) {
    if(draftChanged()) saveDraft();
    else notice(t("Nothing to save"), graphicMuted);
  } else if(page == Display && selected == 0) {
    editing = true;
    edit = draft.name;
  } else if(page == Radio && selected == RowRegion) {
    editing = true;
    edit = draft.region;
  } else if(page == Radio && selected == RowPrev) {
    if(!config.trial.set() && !config.previous.set()) {
      notice(t("No previous settings"), graphicMuted);
      return;
    }
    String r = executeCommand(config.trial.set() ? "radio keep" : "radio previous");
    bool done = r.startsWith("OK");
    if(done) draft = config;
    notice(done ? t("Radio settings changed") : r.startsWith("ERR radio busy") ? t("Radio busy, retry")
                                                                               : r.substring(4),
           done ? ok : bad);
  } else if(page == Radio && selected == RowFind) {
    if(config.role != RoleNormal) notice(t("Not in this mode"), warn);
    else if(!regions::find()) notice(t("Radio offline"), bad);
  } else alter(1);
}
