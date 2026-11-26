// Quick send (included by Ui.cpp): the templates, the recipient choice and their keys.
String quickName(uint64_t id) {
  if(auto* c = meshRadio.channel(id)) return c->name;
  if(const Peer* p = meshRadio.findContact(id)) return p->name;
  return t("Choose recipient");
}
void drawQuick() {
  unsigned n = quickTargets ? quickSend::targetCount() : quickSend::Count + 1;
  selected = constrain(selected, 0, max(0, int(n) - 1));
  if(quickTargets) {
    text(12, 44, fit(t("Choose recipient"), 296, bold), accent, bold);
    int first = max(0, selected - 5);
    for(unsigned i = first; i < n && i < unsigned(first + 6); i++) {
      int y = 52 + (i - first) * 26;
      listRow(y, 24, int(i) == selected);
      target(8, y, 304, 24, i);
      text(18, y + 17, fit(quickName(quickSend::target(i)), 284), int(i) == selected ? textPrimary : textMuted);
    }
    scrollbar(first, 6, n, 52, 156);
  } else {
    bool focus = selected == 0;
    target(8, 28, 304, 40, 0);
    panel(8, 28, 304, 40, focus ? surfaceFocus : surfaceCard, radiusRow);
    if(focus) ring(8, 28, 304, 40, radiusRow);
    text(18, 42, fit(t("Choose recipient"), 260, small), textMuted, small);
    text(18, 59, fit(quickName(quickReplyTo ? quickReplyTo : quickSend::state.to), 260, bold), accent, bold);
    tri(299, 49, 1, 4, accent);
    int first = max(1, selected - 4);
    for(unsigned i = first; i < n && i < unsigned(first + 5); i++) {
      int y = 75 + (i - first) * 26;
      listRow(y, 24, int(i) == selected);
      target(8, y, 304, 24, i);
      text(18, y + 17, fit(String(quickSend::state.text[i - 1]), 284), int(i) == selected ? textPrimary : textMuted);
    }
    scrollbar(first - 1, 5, quickSend::Count, 75, 130);
  }
  footer({{"^v", t("choose")}, {"OK", quickTargets || selected == 0 ? t("select") : t("send")}, {"BACK", t("exit")}});
}
bool quickKey(int key) {
  unsigned n = quickTargets ? quickSend::targetCount() : quickSend::Count + 1;
  if(key == KeyBack) {
    if(quickTargets) {
      quickTargets = false;
      selected = 0;
    } else {
      quickReplyTo = 0;
      change(Home, true);
    }
    return true;
  }
  if(key == KeyUp || key == KeyDown) {
    if(n) selected = (selected + (key == KeyUp ? int(n) - 1 : 1)) % n;
    return true;
  }
  if(key == Enter) {
    if(quickTargets) {
      if(!n) return true;
      StaticJsonDocument<128> d;
      d["action"] = "target";
      d["to"] = meshRadio.idText(quickSend::target(selected % n));
      String r = quickSend::command(d.as<JsonObjectConst>());
      notice(r, r.startsWith("OK") ? ok : bad);
      if(r.startsWith("OK")) {
        quickTargets = false;
        quickReplyTo = 0;
        selected = 1;
      }
    } else if(selected == 0) {
      quickTargets = true;
      selected = 0;
    } else {
      String r = quickSend::send(selected - 1, quickReplyTo);
      notice(r, r.startsWith("OK") ? ok : bad);
    }
    return true;
  }
  return true;
}
