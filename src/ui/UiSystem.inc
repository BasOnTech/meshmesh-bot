// System pages (included by Ui.cpp): module health (diagnostics), keys & help and the lock screen.
String keyName(uint32_t k) {
  switch(k) {
  case 0: return "-";
  case KeyMsg: return "MSG";
  case KeyHome: return "HOME";
  case KeyAt: return t("@ (chat settings)");
  case KeyAdv: return "ADV";
  case 0x87: return t("hold ADV (GPS)");
  case KeyMap: return "MAP";
  case KeyBack: return "BACK";
  case KeyMic: return "MIC";
  case KeySet: return "CTRL";
  case KeyHold: return t("hold OK");
  case KeyLeft: return t("Left");
  case KeyUp: return t("Up");
  case KeyDown: return t("Down");
  case KeyRight: return t("Right");
  case Enter: return "OK";
  case Erase: return "DEL";
  case ' ': return t("space");
  }
  char b[16];
  if(k > 32 && k < 127) snprintf(b, sizeof b, "'%c'", char(k));
  else snprintf(b, sizeof b, "0x%02X", unsigned(k));
  return b;
}
void drawDiagnostics() {
  const char* names[] = {"LoRa", nullptr, "SD", "LittleFS", "RTC", "GPS / NMEA", nullptr, "IMU"};
  String label[8];
  for(int i = 0; i < 8; i++) label[i] = names[i] ? String(names[i]) : i == 1 ? t("Keyboard")
                                                                             : t("Compass");
  bool state[8];
  moduleStates(state);
  for(int i = 0; i < 8; i++) {
    int x = 8 + (i % 2) * 154, y = 25 + (i / 2) * 22;
    panel(x, y, 150, 19, surfaceCard, radiusRowCompact);
    g().fillCircle(x + 11, y + 9, 4, state[i] ? ok : bad);
    text(x + 21, y + 14, label[i], textPrimary);
    textRight(x + 144, y + 14, state[i] ? "OK" : t("n/a"), state[i] ? ok : bad, small);
  }
  panel(8, 116, 304, 74, surfaceCard, radiusCard);
  uint32_t up = millis() / 1000;
  char uptime[16];
  snprintf(uptime, sizeof uptime, "%lu:%02lu:%02lu", (unsigned long)(up / 3600), (unsigned long)(up / 60 % 60), (unsigned long)(up % 60));
  String cells[][2] = {{"RX", String(meshRadio.rxCount)}, {"TX", String(meshRadio.txCount)}, {t("Relayed"), String(meshRadio.relayed)}, {t("Rejected"), String(meshRadio.rejected)}, {"RSSI", String(int(meshRadio.lastRssi)) + " dBm"}, {"SNR", String(meshRadio.lastSnr, 1) + " dB"}, {"RAM", String(ESP.getFreeHeap() / 1024) + " K"}, {t("Uptime"), uptime}};
  for(int i = 0; i < 8; i++) {
    int x = 16 + (i % 4) * 75, y = 134 + (i / 4) * 32;
    text(x, y, cells[i][0], textMuted, small);
    text(x, y + 15, cells[i][1], textPrimary, bold);
  }
  text(10, 207, t("Boot ") + String(config.bootCounter) + t(" · last key: ") + keyName(hardware.lastKey), textSubtle, small);
  footer({{"OK", hardware.fsOk ? t("Crypto test + sound") : t("Create storage...")}, {"BACK", t("Back")}});
}
void drawHelp() {
  Hint keys[] = {{"MSG", t("Chats")}, {"MAP", t("Map; L - saved maps")}, {"HOME", t("Menu")}, {"BACK", t("Previous screen")}, {"CTRL", t("Settings")}, {"ADV", t("Announce this node; hold: GPS on/off")}, {"@", t("function key, not Sym+@: chat settings")}, hardware.micKey ? Hint{"MIC", t("Lock screen (there is no microphone)")} : Hint{tr("hold OK"), t("Lock")}, {"OK", t("Open / send; twice or hold: unlock")}, {"→", t("in text, after a letter: capital")}};
  for(int i = 0; i < 10; i++) {
    int y = 24 + i * 19;
    int kw = max(30, measure(keys[i].key, small) + 10);
    g().fillRoundRect(10, y, kw, 15, radiusKey, border);
    textCenter(10 + kw / 2, y + 11, keys[i].key, textPrimary, small);
    text(max(50, kw + 16), y + 12, fit(keys[i].action, 258), i % 2 ? textMuted : textPrimary);
  }
  footer({{"BACK", t("Settings")}});
}
const ChessMatch* lockChess();
void drawLockChess(int y);
void drawLockPet(int y);
const Duel* lockDuel();
void drawLockDuel(int y);
void duelEnter(Duel* m);
// The game card of the locked screen: 1 chess, 2 checkers or sea battle (the latest change of the two), 0 none.
int lockGame() {
  const ChessMatch* c = lockChess();
  const Duel* d = lockDuel();
  if(c && d) return int32_t(d->changedAt - c->changedAt) > 0 ? 2 : 1;
  return c ? 1 : d ? 2
                   : 0;
}
void drawLocked() {
  unsigned n = unreadTotal();
  int game = lockGame();
  bool chess = game != 0;
  int top = chess ? (n ? -30 : -20) : 0; // a game card moves the clock up
  textCenter(160, 106 + top, clockText(), textPrimary, digits);
  textCenter(160, 130 + top, dateText(), textMuted);
  int y = 144 + top;
  if(chess) {
    if(game == 1) drawLockChess(y);
    else drawLockDuel(y);
    y += 46;
  }
  if(n) {
    const ChatMessage* last = nullptr;
    for(int i = meshRadio.historyCount - 1; i >= 0; i--)
      if(!meshRadio.history[i].outgoing) {
        last = &meshRadio.history[i];
        break;
      }
    panel(40, y, 240, 40, surfaceCard, radiusCard);
    icon(IcMail, 62, y + 20, 9, warn, surfaceCard);
    text(80, y + 16, count(n, "new message", "new messages"), textPrimary, bold);
    if(!config.lockDetails) text(80, y + 32, t("Unlock to read"), textMuted, small);
    else if(last) text(80, y + 32, fit(t("Last from ") + String(last->name), 190, small), textMuted, small);
  } else if(!chess && creature.alive() && creature.s.stage != pet::Egg) drawLockPet(140);
  else if(!chess) textCenter(160, 166, meshRadio.ready ? (config.role == RoleRepeater ? t("The repeater keeps relaying") : config.role == RoleRoom ? t("The room keeps serving members")
                                                                                                                                                   : t("Radio and GPS keep working"))
                                                       : t("Radio error"),
                             meshRadio.ready ? textSubtle : bad, small);
  icon(IcLock, 160, chess && n ? 210 : 200, chess && n ? 6 : 7, iconMuted);
  if(hardware.micKey) footer({{"OK", chess ? t("Twice or hold: the game") : t("Twice or hold: unlock")}, {"MIC", t("Lock")}});
  else footer({{"OK", chess ? t("Twice or hold: the game") : t("Twice or hold: unlock")}});
}
// OK on module health: the crypto self-test with a beep, or, on storage left by another firmware, creating ours
// after a second OK.
void diagnosticsEnter() {
  if(!hardware.fsOk) {
    static uint32_t armed = 0;
    if(!armed || millis() - armed > 5000) {
      armed = millis();
      notice(t("OK again: erase old data"), warn);
      return;
    }
    armed = 0;
    String r = executeCommand("fsformat");
    bool done = r.startsWith("OK");
    if(done) executeCommand("restart");
    notice(done ? t("Storage created, restart") : r, done ? ok : bad);
    return;
  }
  hardware.beep();
  bool passed = meshRadio.selfTest();
  notice(passed ? t("Encryption test passed") : t("Encryption test failed"), passed ? ok : bad);
}
