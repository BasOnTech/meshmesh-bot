// Chats (included by Ui.cpp): the conversation list and a chat with its composer; the read marks and drafts
// they use are in Ui.cpp (home, the status bar and the lock screen count unread messages too).
bool isRoom(uint64_t id) {
  Peer* p = id && !channels::isChannel(id) ? peerOf(id) : nullptr;
  return p && p->type == 3;
} // a MeshCore room server
String bubbleText(const ChatMessage& m);
String chatLeftAction();
void drawThreads() {
  threads();
  if(selected >= int(conversationCount)) selected = conversationCount - 1;
  int first = max(0, selected - 3);
  for(int i = first; i < int(conversationCount) && i < first + 4; i++) {
    uint64_t id = conversations[i];
    int y = 24 + (i - first) * 48;
    bool focus = selected == i;
    listRow(y, 45, focus);
    target(8, y, 304, 45, i);
    Peer* p = id ? peerOf(id) : nullptr;
    avatar(32, y + 22, 15, id, nameOf(id), p ? p->type : -1);
    if(!id) {
      text(56, y + 18, t("Add a channel"), accent, bold);
      text(56, y + 36, fit(t("Hashtag, name and key, new, heard on air"), 244), textMuted);
      continue;
    }
    const channels::Channel* c = meshRadio.channel(id);
    const ChatMessage* last = nullptr;
    for(int j = meshRadio.historyCount - 1; j >= 0; j--)
      if(belongs(meshRadio.history[j], id)) {
        last = &meshRadio.history[j];
        break;
      }
    String when = last ? timeText(last->timestamp) : "";
    if(when.length()) textRight(306, 19 + y, when, textMuted, small);
    text(56, y + 18, fit(nameOf(id), 240 - measure(when, small), bold), textPrimary, bold);
    String preview = last                            ? (last->outgoing ? t("You: ") : c || isRoom(id) ? String(last->name) + ": "
                                                                                                      : String()) +
                                                         bubbleText(*last)
                     : !c                            ? t("No messages yet")
                     : channels::isPublic(c->secret) ? t("Open MeshCore channel")
                     : channels::isHashtag(*c)       ? t("Open channel · no messages yet")
                                                     : t("Private channel · no messages yet");
    preview.replace("\n", " ");
    unsigned n = unread(id);
    int pillW = 0;
    if(n) {
      String b = String(n);
      pillW = max(18, measure(b, small) + 10);
      g().fillRoundRect(306 - pillW, y + 26, pillW, 14, radiusPill, accent);
      textCenter(306 - pillW / 2, y + 36, b, onFill, small);
    }
    text(56, y + 36, fit(preview, 244 - pillW), n ? textPrimary : textMuted);
  }
  scrollbar(first, 4, conversationCount, 24, 190);
  if(channels::isChannel(conversations[selected])) footer({{"OK", t("Open")}, {"<>", t("Channel")}, {"ADV", t("Announce")}, {"HOME", t("Menu")}});
  else footer({{"OK", t("Open")}, {"^v", t("Select")}, {"ADV", t("Announce")}, {"HOME", t("Menu")}});
}
void statusMark(int x, int y, const ChatMessage& m) {
  auto& d = g();
  switch(m.status) {
  case ChatMessage::Queued:
    d.drawCircle(x + 3, y - 3, 3, graphicMuted);
    d.drawFastVLine(x + 3, y - 5, 3, graphicMuted);
    d.drawFastHLine(x + 3, y - 3, 2, graphicMuted);
    break;
  case ChatMessage::Sent:
    d.drawLine(x, y - 3, x + 2, y - 1, graphicMuted);
    d.drawLine(x + 2, y - 1, x + 6, y - 6, graphicMuted);
    break;
  case ChatMessage::Delivered:
    for(int k : {0, 4}) {
      d.drawLine(x + k, y - 3, x + k + 2, y - 1, accent);
      d.drawLine(x + k + 2, y - 1, x + k + 6, y - 6, accent);
    }
    break;
  case ChatMessage::Failed:
    d.fillCircle(x + 3, y - 3, 4, bad);
    d.drawFastVLine(x + 3, y - 5, 3, graphicPrimary);
    d.drawPixel(x + 3, y - 1, graphicPrimary);
    break;
  default: break;
  }
}
void drawChat() {
  auto& d = g();
  Peer* p = peerOf(recipient);
  const channels::Channel* ch = meshRadio.channel(recipient);
  String sub = channels::isChannel(recipient) ? (ch && !channels::isPublic(ch->secret) && !channels::isHashtag(*ch) ? t("Private channel · senders unverified · no ACK") : t("Open channel · senders unverified · no ACK")) : p && p->type == 3 ? typeText(p->type) + " · " + (remote::find(p->id) && remote::find(p->id)->login == remote::Done ? t("logged in") : t("log in: node card > Server"))
                                                                                                                                                                                                                            : p                 ? typeText(p->type) + " · " + pathText(*p) + (p->heard ? " · " + ago(millis() - p->seen) : String())
                                                                                                                                                                                                                                                : t("Not in contacts");
  text(8, 33, fit(sub, 270, small), textMuted, small);
  String rows[6];
  unsigned inputRows = max(1u, wrap(composer, rows, 6, 276));
  unsigned shown = min(inputRows, 3u);
  int boxH = shown * 13 + 11, boxY = 216 - boxH;
  int indices[64], total = 0;
  for(unsigned i = 0; i < meshRadio.historyCount; i++)
    if(belongs(meshRadio.history[i], recipient)) indices[total++] = i;
  int end = max(0, total - chatOffset), bottom = boxY - 5;
  const int top = 41;
  for(int j = end - 1; j >= 0 && bottom > top + 20; j--) {
    auto& m = meshRadio.history[indices[j]];
    String lines[7];
    unsigned n = wrap(bubbleText(m), lines, 7, 214);
    bool named = !m.outgoing && (channels::isChannel(recipient) || isRoom(recipient)); // a room post names its author
    String when = timeText(m.timestamp), state = m.status == ChatMessage::Failed ? t("not confirmed") : meshRadio.routeText(m);
    if(m.protocol == 1) state = "MM/1 " + state;
    int metaW = measure(when, small) + (state.length() ? measure(state, small) + 4 : 0) + (m.outgoing ? 12 : 0), w = metaW;
    for(unsigned k = 0; k < n; k++) w = max(w, measure(lines[k]));
    if(named) w = max(w, measure(m.name, small));
    w = max(44, w + 16);
    int h = 7 + (named ? 11 : 0) + n * 13 + 10;
    if(bottom - h < top) {
      if(j != end - 1) break;
      n = max(1, (bottom - top - 7 - (named ? 11 : 0) - 10) / 13);
      h = 7 + (named ? 11 : 0) + n * 13 + 10;
    }
    int x = m.outgoing ? 312 - w : 8, y = bottom - h, ty = y + 14;
    panel(x, y, w, h, m.outgoing ? outBubble : inBubble, radiusBubble);
    if(named) {
      text(x + 8, y + 10, fit(m.name, w - 16, small), avatarHues[(m.source ^ m.source >> 17 ^ m.source >> 41) % 8] | 0x4208, small);
      ty += 11;
    }
    for(unsigned k = 0; k < n; k++) text(x + 8, ty + k * 13, lines[k]);
    int mx = x + w - 7 - (m.outgoing ? 11 : 0), my = y + h - 4;
    textRight(mx, my, when, textMuted, small);
    if(state.length()) textRight(mx - measure(when, small) - 4, my, state, m.status == ChatMessage::Failed ? bad : textSubtle, small);
    if(m.outgoing) statusMark(x + w - 15, my, m);
    bottom = y - 5;
  }
  if(!total) {
    icon(ch && !channels::isPublic(ch->secret) && !channels::isHashtag(*ch) ? IcLock : channels::isChannel(recipient) ? IcHash
                                                                                                                      : IcChat,
         160, 86, 14, iconSubtle);
    textCenter(160, 124, t("No messages yet"), textMuted);
    textCenter(160, 142, t("Type and press OK to send"), textSubtle, small);
  }
  if(chatOffset) {
    String c = String(chatOffset);
    int w = measure(c, small) + 18;
    d.fillRoundRect(304 - w, boxY - 19, w, 14, radiusPill, accent);
    tri(304 - w + 7, boxY - 12, 2, 3, onFill);
    text(304 - w + 12, boxY - 8, c, onFill, small);
  }
  panel(8, boxY, 304, boxH, surfaceCard, radiusField);
  d.drawRoundRect(8, boxY, 304, boxH, radiusField, composer.length() ? accent : border);
  if(composer.length()) {
    unsigned firstRow = inputRows > 3 ? inputRows - 3 : 0;
    int cx = 14;
    for(unsigned i = firstRow; i < inputRows; i++) cx = text(14, boxY + 14 + (i - firstRow) * 13, rows[i], textPrimary);
    d.fillRect(cx + 1, boxY + 4 + (shown - 1) * 13, 2, 12, accent);
  } else {
    d.fillRect(14, boxY + 5, 2, 12, accent);
    text(20, boxY + 15, t("Message..."), textSubtle);
  }
  unsigned used = composer.length(), limit = meshRadio.messageLimit(recipient);
  String counter = String(used) + "/" + String(limit);
  textRight(306, boxY + boxH - 3, counter, used * 10 >= limit * 9 ? (used >= limit ? bad : warn) : textSubtle, small);
  String left = chatLeftAction();
  if(left.length()) footer({{"OK", t("Send")}, {"<", left}, {"BACK", t("Back")}});
  else footer({{"OK", t("Send")}, {"@", t("Settings")}, {"BACK", t("Back")}});
}
void openChannelInfo(uint64_t id, Page from); // UiChannels.inc
void chatLeft(); // UiChannels.inc
// Chats: <> on a channel opens its card, OK the conversation (or "add a channel").
bool threadsKey(int key) {
  if(key == KeyLeft || key == KeyRight) {
    threads();
    if(selected < int(conversationCount) && channels::isChannel(conversations[selected])) openChannelInfo(conversations[selected], Threads);
    return true;
  }
  if(key != Enter && key != KeyHold) return false;
  threads();
  if(!conversations[selected]) {
    change(ChannelAdd);
    return true;
  }
  recipient = conversations[selected];
  chatReturn = Threads;
  composer = "";
  change(Chat);
  return true;
}
// A chat takes every key that reaches it: OK sends, ^v scroll the history, > on an empty composer opens the quick
// replies (after a letter: its case), < the chat's left action; letters go into the composer.
void chatKey(int key) {
  if(key == Enter) {
    if(meshRadio.sendMessage(composer, recipient)) {
      composer = "";
      chatOffset = 0;
      notice(t("Message queued"));
    } else notice(t("Cannot send: empty, full queue or radio"), bad);
    return;
  }
  if(key == Erase) {
    composer.remove(meshmesh::previousCharacter(composer.c_str(), composer.length()));
    return;
  }
  if(key == KeyUp) {
    unsigned total = 0;
    for(unsigned i = 0; i < meshRadio.historyCount; i++)
      if(belongs(meshRadio.history[i], recipient)) total++;
    chatOffset = min(chatOffset + 1, max(0, int(total) - 1));
    return;
  }
  if(key == KeyDown) {
    chatOffset = max(0, chatOffset - 1);
    return;
  }
  if(key == KeyRight) {
    if(!composer.length()) {
      quickReplyTo = recipient;
      quickTargets = false;
      change(QuickView);
    } else toggleLastCase(composer);
    return;
  }
  if(key == KeyLeft) {
    chatLeft();
    return;
  }
  if(key >= 32 && key < 127) {
    String ch = String(char(key));
    if(composer.length() + ch.length() <= meshRadio.messageLimit(recipient)) composer += ch;
    else notice(t("UTF-8 byte limit: ") + String(meshRadio.messageLimit(recipient)), warn);
  }
}
