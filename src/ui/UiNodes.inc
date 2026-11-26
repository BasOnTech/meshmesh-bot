// Nodes (included by Ui.cpp): the node list, a node's card with its actions and the "Play" menu.
void drawNodes() {
  sortNodes();
  if(!nodeTotal) {
    icon(IcMesh, 160, 86, 16, iconSubtle);
    textCenter(160, 126, t("No nodes discovered yet"), textPrimary);
    textCenter(160, 144, t("ADV sends your announcement"), textMuted, small);
  }
  int first = max(0, selected - 3);
  uint32_t now = millis();
  for(unsigned i = first; i < nodeTotal && i < unsigned(first + 4); i++) {
    auto& p = meshRadio.peers[nodeOrder[i]];
    int y = 24 + (i - first) * 48;
    bool focus = selected == int(i);
    listRow(y, 45, focus);
    target(8, y, 304, 45, i);
    avatar(32, y + 22, 15, p.id, p.name, p.type);
    String age = p.heard ? ago(now - p.seen) : t("saved");
    textRight(306, y + 18, age, p.heard && now - p.seen < 1800000 ? ok : textMuted, small);
    if(p.heard) bars(286 - measure(age, small), y + 18, snrLevel(p.snr), ok);
    text(56, y + 18, fit(p.name, 200 - measure(age, small), bold), textPrimary, bold);
    float metres, bearing;
    String detail = typeText(p.type) + " · " + pathText(p);
    if(p.heard) detail += " · " + String(int(p.rssi)) + " dBm";
    if(distanceTo(p, metres, bearing)) detail += " · " + distanceText(metres, bearing);
    else if(p.position) detail += " · GPS";
    text(56, y + 36, fit(detail, 250, small), typeColor(p.type) == accent ? textMuted : typeColor(p.type), small);
  }
  scrollbar(first, 4, nodeTotal, 24, 190);
  footer({{"OK", t("Details")}, {"P", t("On map")}, {"ADV", t("Announce")}, {"BACK", t("Back")}});
}
enum NodeAction { ActWrite,
                  ActMap,
                  ActResetPath,
                  ActForget,
                  ActServer,
                  ActTrace,
                  ActPlay };
// A room's posts open as its chat; a repeater or room has its server page, a chat node behind repeaters a trace.
// A chat node can be invited to a game ("Play": chess, checkers, sea battle).
unsigned nodeActions(const Peer& p, NodeAction* out) {
  unsigned n = 0;
  if(p.type == 1 || p.type == 3) out[n++] = ActWrite;
  if(p.type == 1) out[n++] = ActPlay;
  if(p.type == 2 || p.type == 3) out[n++] = ActServer;
  else if(remote::traceable(p.id)) out[n++] = ActTrace;
  if(p.position) out[n++] = ActMap;
  if(p.pathLength != 255) out[n++] = ActResetPath;
  out[n++] = ActForget;
  return n;
}
// "Play" on a chat node's card: the games; OK opens the game's contact choice with this contact selected.
const int playGames = 3;
void drawPlayMenu(const Peer& p) {
  targetCount = hintCount = 0;
  uint16_t* px = g().getBuffer();
  for(int i = 0; i < 320 * 240; i++) px[i] = (px[i] >> 1) & 0x7bef; // dim the card under the menu
  panel(40, 46, 240, 142, surfaceFocus, radiusDialog);
  g().drawRoundRect(40, 46, 240, 142, radiusDialog, border);
  text(54, 66, fit(t("Play with ") + p.name, 212, bold), textPrimary, bold);
  const Icon icons[] = {IcChess, IcCheckers, IcShip};
  const String names[] = {t("Chess"), t("Checkers"), t("Sea battle")};
  const String about[] = {t("Rated or friendly"), t("Russian draughts"), t("Ten ships each")};
  for(int i = 0; i < playGames; i++) {
    int y = 76 + i * 36;
    bool focus = playSel == i;
    target(48, y, 224, 32, i);
    panel(48, y, 224, 32, focus ? surfaceCard : surfaceFocus, radiusRow);
    if(focus) ring(48, y, 224, 32, radiusRow);
    g().fillRoundRect(54, y + 5, 22, 22, radiusWell, surfaceWell);
    icon(icons[i], 65, y + 16, 6, focus ? accent : iconMuted, surfaceWell);
    text(84, y + 14, names[i], textPrimary, bold);
    text(84, y + 26, fit(about[i], 180, small), textMuted, small);
  }
  footer({{"OK", t("Choose")}, {"^v", t("Select")}, {"BACK", t("Close")}});
}
void drawNode() {
  Peer* p = focusedPeer();
  if(!p) {
    textCenter(160, 110, t("Node was removed"), textMuted);
    footer({{"BACK", t("Nodes")}});
    return;
  }
  auto& d = g();
  avatar(36, 50, 20, p->id, p->name, p->type);
  text(66, 48, fit(p->name, 240, big), textPrimary, big);
  text(66, 64, typeText(p->type), typeColor(p->type), small);
  float metres, bearing;
  String rows[][2] = {
    {t("Last heard"), p->heard ? (millis() - p->seen < 60000 ? ago(0) : ago(millis() - p->seen) + t(" ago")) : t("not since boot")},
    {t("Signal"), p->heard ? String(int(p->rssi)) + " dBm · SNR " + String(p->snr, 1) : String("-")},
    {t("Route"), pathText(*p) + (p->pathLength == 255 ? t(" · flood") : String())},
    {t("Position"), distanceTo(*p, metres, bearing) ? distanceText(metres, bearing) : p->position ? String(p->latitude, 5) + ", " + String(p->longitude, 5)
                                                                                                  : t("not shared")},
    {"ID", meshRadio.idText(p->id)}};
  NodeAction acts[6];
  unsigned n = nodeActions(*p, acts);
  action = constrain(action, 0, int(n) - 1);
  bool two = n > 4; // five or six actions: two rows of buttons
  for(int i = 0; i < 5; i++) {
    int y = two ? 86 + i * 16 : 90 + i * 18;
    text(14, y, rows[i][0], textMuted);
    textRight(306, y, fit(rows[i][1], 180), textPrimary);
    if(i < 4) d.drawFastHLine(14, y + (two ? 4 : 5), 292, surfaceCard);
  }
  const String names[] = {p->type == 3 ? t("Room") : t("Message"), t("On map"), t("Reset path"), deleteArmed ? t("Sure?") : t("Forget"), t("Server"), t("Trace"), t("Play")};
  unsigned per = two ? (n + 1) / 2 : n;
  for(unsigned i = 0; i < n; i++) {
    unsigned row = i / per, k = i % per, inRow = min(per, n - row * per);
    int w = (304 - (inRow - 1) * 6) / inRow, x = 8 + k * (w + 6), h = two ? 25 : 28, y = two ? 160 + row * 29 : 184;
    bool focus = action == int(i), danger = acts[i] == ActForget;
    target(x, y, w, h, i);
    panel(x, y, w, h, focus ? (danger && deleteArmed ? bad : surfaceFocus) : surfaceCard, radiusRow);
    if(focus) ring(x, y, w, h, radiusRow, danger ? bad : accent);
    if(acts[i] == ActPlay) {
      int tw = measure(names[ActPlay]) + 16;
      icon(IcChess, x + w / 2 - tw / 2 + 5, y + h / 2, 5, focus ? accent : iconMuted, focus ? surfaceFocus : surfaceCard);
      text(x + w / 2 - tw / 2 + 16, y + h / 2 + 5, names[ActPlay], textPrimary);
      continue;
    }
    textCenter(x + w / 2, y + h / 2 + 5, fit(names[acts[i]], w - 8), danger ? (focus ? textPrimary : bad) : textPrimary);
  }
  footer({{"<>", t("Action")}, {"OK", t("Run")}, {"BACK", t("Nodes")}});
  if(playMenu) drawPlayMenu(*p);
}
void openPick(Page p, uint64_t peer); // UiDuel.inc
void openRemote(); // UiRemote.inc
void playChoose() {
  Peer* p = focusedPeer();
  playMenu = false;
  if(!p) return;
  if(playSel == 0) {
    openPick(ChessPick, p->id);
    return;
  }
  duelKind = playSel == 2 ? Duel::Sea : Duel::Checkers;
  openPick(DuelPick, p->id);
}
void runNodeAction() {
  Peer* p = focusedPeer();
  if(!p) return;
  NodeAction acts[6];
  unsigned n = nodeActions(*p, acts);
  NodeAction a = acts[constrain(action, 0, int(n) - 1)];
  if(a != ActForget) deleteArmed = false;
  switch(a) {
  case ActWrite:
    recipient = p->id;
    chatReturn = Node;
    composer = "";
    change(Chat);
    break;
  case ActServer:
    openRemote();
    break;
  case ActPlay:
    playMenu = true;
    playSel = 0;
    break;
  case ActTrace:
    openRemote();
    if(!remote::traceTo(p->id)) notice(t("Not sent: radio or route"), bad);
    break;
  case ActMap:
    maps.follow = false;
    maps.center(p->latitude, p->longitude);
    change(Map);
    break;
  case ActResetPath: {
    if(meshRadio.busy()) {
      notice(t("Radio busy, try again"), warn);
      break;
    }
    bool reset = meshRadio.resetPath(p->id);
    notice(reset ? t("Path reset: next message floods") : t("Path reset failed"), reset ? ok : bad);
    break;
  }
  case ActForget:
    if(!deleteArmed) {
      deleteArmed = true;
      notice(t("OK again removes the contact"), warn);
      break;
    }
    deleteArmed = false;
    if(meshRadio.removeContact(p->id)) {
      notice(t("Contact removed; its next advert adds it again"), ok);
      change(Nodes);
    } else notice(t("Cannot remove while sending"), bad);
    break;
  }
}
// The games of "Play" on a node card; other keys (HOME, MSG...) leave the card as usual.
bool playMenuKey(int key) {
  if(key == KeyUp || key == KeyDown || key == KeyLeft || key == KeyRight) {
    playSel = (playSel + (key == KeyUp || key == KeyLeft ? playGames - 1 : 1)) % playGames;
    return true;
  }
  if(key == Enter || key == KeyHold) {
    playChoose();
    return true;
  }
  if(key == KeyBack || key == Erase) {
    playMenu = false;
    return true;
  }
  return false;
}
// The node list and a node's card: P shows the node on the map, the arrows pick a card's action, OK opens or runs it.
bool nodesKey(int key) {
  if(key == 'p' || key == 'P') {
    if(Peer* p = focusedPeer()) {
      if(p->position) {
        maps.follow = false;
        maps.center(p->latitude, p->longitude);
        change(Map);
      } else notice(t("This node has not shared GPS"), warn);
    }
    return true;
  }
  if(page == Node && (key == KeyLeft || key == KeyRight || key == KeyUp || key == KeyDown)) {
    Peer* p = focusedPeer();
    if(!p) return true;
    NodeAction acts[6];
    int n = nodeActions(*p, acts);
    action = (action + (key == KeyLeft || key == KeyUp ? -1 : 1) + n) % n;
    return true;
  }
  if(key != Enter && key != KeyHold) return false;
  if(page == Node) runNodeAction();
  else if(nodeTotal) {
    focusNode = meshRadio.peers[nodeOrder[selected]].id;
    change(Node);
  }
  return true;
}
