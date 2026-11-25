// Device role (included by Ui.cpp): the choice offered for 5 s after boot or from Settings, and the
// home page of a repeater or room server with its MeshCore settings. Changes go through the server's
// CLI, as from the MeshCore app.
bool bootPick = false;
uint32_t bootPickAt = 0;
constexpr uint32_t bootPickMs = 5000;
enum ServerRow { RowForward,
                 RowAdmin,
                 RowGuest,
                 RowReadOnly,
                 RowPost,
                 RowAdvert,
                 RowFlood,
                 RowAnnounce,
                 RowMode };
const ServerRow repeaterRows[] = {RowForward, RowAdmin, RowGuest, RowAdvert, RowFlood, RowAnnounce, RowMode};
const ServerRow roomRows[] = {RowPost, RowAdmin, RowGuest, RowReadOnly, RowAdvert, RowFlood, RowForward, RowAnnounce, RowMode};
int serverRowCount() { return config.role == RoleRoom ? 9 : 7; }
ServerRow serverRow(int i) { return config.role == RoleRoom ? roomRows[constrain(i, 0, 8)] : repeaterRows[constrain(i, 0, 6)]; }
bool serverPostEdit() { return page == ServerHome && serverRow(selected) == RowPost; }
unsigned serverEditLimit() { return serverPostEdit() ? 151 : 15; }
Page homePage() { return config.role == RoleNormal ? Home : ServerHome; }
String roleTitle(int role) { return role == RoleRepeater ? t("MeshCore repeater") : role == RoleRoom ? t("MeshCore room server")
                                                                                                     : t("Normal mode"); }
void drawRolePick() {
  for(int r = 0; r < RoleCount; r++) {
    int y = 24 + r * 58;
    bool focus = selected == r, current = config.role == r;
    target(8, y, 304, 54, r);
    panel(8, y, 304, 54, focus ? surfaceFocus : surfaceCard, radiusCard);
    if(focus) ring(8, y, 304, 54, radiusCard);
    Icon ic = r == RoleRepeater ? IcTower : r == RoleRoom ? IcRoom
                                                          : IcChat;
    uint16_t hue = r == RoleRepeater ? secondary : r == RoleRoom ? warn
                                                                 : accent;
    g().fillRoundRect(16, y + 8, 28, 28, radiusWell, surfaceWell);
    icon(ic, 30, y + 22, 8, hue, surfaceWell);
    text(54, y + 19, roleTitle(r), textPrimary, bold);
    if(current) {
      String now = t("running");
      int w = measure(now, small) + 10;
      g().fillRoundRect(302 - w, y + 8, w, 14, radiusPill, accentDeep);
      textCenter(302 - w / 2, y + 18, now, accent, small);
    }
    const char* about[][2] = {{tr("Screen, chats, maps, radar and games"), tr("A MeshCore chat node; relays as set")},
                              {tr("Relays the network's packets; no chats"), tr("Login and settings from the MeshCore app")},
                              {tr("Keeps posts and hands them to members"), tr("Room password and settings from the MeshCore app")}};
    for(int i = 0; i < 2; i++) text(54, y + 33 + i * 12, fit(about[r][i], 250, small), textMuted, small);
  }
  if(bootPick) {
    uint32_t gone = millis() - bootPickAt;
    unsigned left = gone >= bootPickMs ? 0 : (bootPickMs - gone + 999) / 1000;
    textCenter(160, 212, t("Continues as now in ") + String(left) + t(" s"), warn);
  } else textCenter(160, 212, t("A change restarts; data and settings stay"), textSubtle, small);
  footer({{"OK", t("Choose")}, {"^v", t("Select")}, {"BACK", bootPick ? t("Continue") : t("Cancel")}});
}
bool rolePickKey(int key) {
  bootPick = false;
  if(key == KeyUp || key == KeyDown) {
    selected = (selected + (key == KeyUp ? RoleCount - 1 : 1)) % RoleCount;
    return true;
  }
  if(key == KeyBack) {
    change(homePage());
    return true;
  }
  if(key != Enter) return false; // HOME, CTRL, ADV keep their meaning
  if(selected == config.role) {
    change(homePage());
    return true;
  }
  String reply = setRole(selected);
  bool done = reply.startsWith("OK");
  notice(done ? t("Restarting: ") + roleTitle(selected) : reply, done ? ok : bad);
  return true;
}
void openRolePick(bool atBoot) {
  change(RolePick);
  selected = config.role;
  bootPick = atBoot;
  bootPickAt = millis();
}

// Server page: status, then the settings an owner changes on the spot.
String uptimeText(uint32_t s) { return s < 3600 ? String(s / 60) + t(" min") : s < 86400 ? String(s / 3600) + t(" h ") + String(s / 60 % 60) + t(" min")
                                                                                         : String(s / 86400) + t(" d ") + String(s / 3600 % 24) + t(" h"); }
String serverValue(const ServerView& v, ServerRow r) {
  switch(r) {
  case RowForward: return flag(v.forwarding);
  case RowAdmin: return v.password;
  case RowGuest: return v.guest.length() ? v.guest : t("none");
  case RowReadOnly: return flag(v.readOnlyLogin);
  case RowAdvert: return v.advertMinutes ? String(v.advertMinutes) + t(" min") : t("Off");
  case RowFlood: return v.floodAdvertHours ? String(v.floodAdvertHours) + t(" h") : t("Off");
  case RowMode: return roleTitle(config.role);
  default: return "";
  }
}
String serverName(ServerRow r) {
  bool room = config.role == RoleRoom;
  switch(r) {
  case RowForward: return t("Forwarding");
  case RowAdmin: return t("Admin password");
  case RowGuest: return room ? t("Room password") : t("Guest password");
  case RowReadOnly: return t("Read without password");
  case RowPost: return t("Post to the room");
  case RowAdvert: return t("Nearby advert");
  case RowFlood: return t("Network advert");
  case RowAnnounce: return t("Advertise now");
  case RowMode: return t("Device mode");
  }
  return "";
}
String serverHint(ServerRow r) {
  bool room = config.role == RoleRoom;
  switch(r) {
  case RowForward: return room ? t("Forward other nodes' packets (off for a room)") : t("Forward other MeshCore nodes' packets");
  case RowAdmin: return t("Admin login from the MeshCore app; OK: change");
  case RowGuest: return room ? t("Members read and post with it; none: blank") : t("Read-only login; none: a blank password logs in");
  case RowReadOnly: return t("Any password gives read-only access");
  case RowPost: return t("A post from the room itself; OK: write");
  case RowAdvert: return t("Zero-hop advert: off or 60-240 min");
  case RowFlood: return t("Flooded advert: off or 3-168 h");
  case RowAnnounce: return t("Tell the whole network about this node");
  case RowMode: return t("Normal, repeater or room; a change restarts");
  }
  return "";
}
void drawServer() {
  ServerView v = meshServer.view();
  auto& d = g();
  bool room = config.role == RoleRoom;
  panel(8, 25, 304, 58, surfaceCard, radiusCard);
  d.fillRoundRect(16, 33, 26, 26, radiusWell, surfaceWell);
  icon(room ? IcRoom : IcTower, 29, 46, 8, !meshRadio.ready ? bad : room         ? warn
                                                                  : v.forwarding ? secondary
                                                                                 : bad,
       surfaceWell);
  text(50, 40, fit(String(config.name), 150, bold), textPrimary, bold);
  text(50, 54, fit(meshRadio.ready ? String(config.frequency, 3) + t(" MHz · SF") + String(config.sf) + " · " + String(config.power) + " dBm" : t("Radio error ") + String(meshRadio.radioError), 170, small), meshRadio.ready ? textMuted : bad, small);
  textRight(302, 40, String(meshRadio.txCount), textPrimary, small);
  tri(302 - measure(String(meshRadio.txCount), small) - 7, 37, 0, 3, accent);
  textRight(302, 54, String(meshRadio.rxCount), textPrimary, small);
  tri(302 - measure(String(meshRadio.rxCount), small) - 7, 51, 2, 3, ok);
  String line = room ? t("Posts ") + String(v.posts) + t(" · members ") + String(v.clients) : t("Relayed ") + String(meshRadio.relayed) + t(" · neighbours ") + String(v.neighbours) + t(" · logged in ") + String(v.clients);
  text(16, 76, fit(line + " · " + uptimeText(v.uptime), 288, small), textMuted, small);
  int rows = serverRowCount(), first = max(0, selected - 4);
  for(int i = first; i < rows && i < first + 5; i++) {
    int y = 87 + (i - first) * 22;
    bool focus = selected == i;
    ServerRow r = serverRow(i);
    target(8, y, 304, 21, i);
    if(r == RowAnnounce || r == RowPost) {
      panel(8, y, 304, 21, focus ? accent : surfaceCard, radiusRowCompact);
      textCenter(160, y + 15, serverName(r), focus ? onFill : textPrimary, bold);
      continue;
    }
    listRow(y, 21, focus);
    text(24, y + 15, serverName(r), focus ? textPrimary : textMuted);
    if(r == RowForward || r == RowReadOnly) {
      toggle(274, y + 3, r == RowForward ? v.forwarding : v.readOnlyLogin);
      continue;
    }
    String value = serverValue(v, r);
    bool opens = r == RowMode || r == RowAdmin || r == RowGuest;
    if(focus && (r == RowAdvert || r == RowFlood)) {
      int w = measure(value, bold);
      tri(300, y + 10, 1, 4, accent);
      text(292 - w, y + 15, value, accent, bold);
      tri(284 - w, y + 10, 3, 4, accent);
    } else {
      textRight(opens ? 292 : 300, y + 15, fit(value, 140), focus ? accent : textPrimary, focus ? bold : body);
      if(opens) tri(302, y + 10, 1, 4, focus ? accent : iconSubtle);
    }
  }
  scrollbar(first, 5, rows, 87, 108);
  text(10, 210, fit(serverHint(serverRow(selected)), 300, small), textSubtle, small);
  ServerRow r = serverRow(selected);
  if(r == RowAdvert || r == RowFlood) footer({{"<>", t("Change")}, {"^v", t("Select")}, {"ADV", t("Advertise")}, {"P", t("Pet")}});
  else footer({{"OK", r == RowForward || r == RowReadOnly ? t("Switch") : t("Open")}, {"^v", t("Select")}, {"ADV", t("Advertise")}, {"P", t("Pet")}});
}
void serverNotice(const String& reply) {
  bool done = reply.startsWith("OK") || reply.startsWith("password now");
  notice(done ? t("Saved") : reply, done ? ok : bad);
}
// Intervals step through the values the CLI accepts.
void serverAlter(int dir) {
  ServerView v = meshServer.view();
  ServerRow r = serverRow(selected);
  if(r == RowForward) serverNotice(meshServer.command(v.forwarding ? "set repeat off" : "set repeat on"));
  if(r == RowReadOnly) serverNotice(meshServer.command(v.readOnlyLogin ? "set allow.read.only off" : "set allow.read.only on"));
  if(r == RowAdvert) {
    int m = v.advertMinutes;
    m = dir > 0 ? (m ? min(240, m + 30) : 60) : (m <= 60 ? 0 : m - 30);
    serverNotice(meshServer.command("set advert.interval " + String(m)));
  }
  if(r == RowFlood) {
    const int steps[] = {0, 3, 6, 12, 24, 47, 72, 120, 168};
    int n = sizeof(steps) / sizeof(*steps), i = 0;
    if(dir > 0) {
      while(i < n - 1 && steps[i] <= int(v.floodAdvertHours)) i++;
    } else {
      i = n - 1;
      while(i > 0 && steps[i] >= int(v.floodAdvertHours)) i--;
    }
    serverNotice(meshServer.command("set flood.advert.interval " + String(steps[i])));
  }
}
bool serverKey(int key) {
  int rows = serverRowCount();
  if(key == KeyUp || key == KeyDown) {
    selected = (selected + (key == KeyUp ? rows - 1 : 1)) % rows;
    return true;
  }
  if(key == KeyLeft || key == KeyRight) {
    serverAlter(key == KeyLeft ? -1 : 1);
    return true;
  }
  if(key != Enter) return false;
  switch(serverRow(selected)) {
  case RowForward:
  case RowReadOnly:
  case RowAdvert:
  case RowFlood: serverAlter(1); break;
  case RowAdmin:
    editing = true;
    edit = meshServer.view().password;
    break;
  case RowGuest:
    editing = true;
    edit = meshServer.view().guest;
    break;
  case RowPost:
    editing = true;
    edit = "";
    break;
  case RowAnnounce: {
    bool sent = meshServer.advertise(true);
    notice(sent ? t("Announced to the network") : t("Announcement failed"), sent ? ok : bad);
    break;
  }
  case RowMode: openRolePick(false); break;
  }
  return true;
}
String serverEditTitle() {
  ServerRow r = serverRow(selected);
  return r == RowPost ? t("Post to the room") : serverName(r);
}
String serverEditHint() {
  ServerRow r = serverRow(selected);
  return r == RowPost ? t("Up to 151 bytes; @: input language") : r == RowAdmin ? t("1-15 characters; -> after a letter: capital")
                                                                                : t("0-15 characters; empty: blank login");
}
// Passwords: up to 15 bytes of printable ASCII; the admin password cannot be empty (a blank login would be admin).
void serverSaveEdit() {
  ServerRow r = serverRow(selected);
  if(r == RowPost) {
    if(!edit.length()) {
      notice(t("Empty post"), bad);
      return;
    }
    bool stored = meshServer.post(edit);
    notice(stored ? t("Posted; members get it in turn") : t("Post not stored"), stored ? ok : bad);
    if(stored) editing = false;
    return;
  }
  for(unsigned i = 0; i < edit.length(); i++)
    if(edit[i] < 32 || edit[i] > 126) {
      notice(t("Latin letters, digits and symbols only"), bad);
      return;
    }
  if(r == RowAdmin && !edit.length()) {
    notice(t("Admin password cannot be empty"), bad);
    return;
  }
  if(edit.length() > 15) {
    notice(t("Up to 15 characters"), bad);
    return;
  }
  serverNotice(meshServer.command(r == RowAdmin ? "password " + edit : "set guest.password " + edit));
  editing = false;
}
