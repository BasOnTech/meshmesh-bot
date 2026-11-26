#include "core/App.h"
#include "hw/Hardware.h"
#include "ui/UiIcons.h"
#include "mesh/MeshRadio.h"
#include "mesh/Remote.h"
#include "apps/Maps.h"
#include "ui/Navigation.h"
#include "apps/Radar.h"
#include "net/Internet.h"
#include "games/solitaire/Solitaire.h"
#include "games/chess/ChessNet.h"
#include "games/duel/Duel.h"
#include "games/chess/ChessRating.h"
#include "games/chess/ChessTour.h"
#include "games/chess/ChessSync.h"
#include "mesh/MeshServer.h"
#include "games/pet/Pet.h"
#include "games/dice/Dice.h"
#include "core/Power.h"
#include "mesh/Regions.h"
#include "mesh/RadioPresets.h"
#include "ui/Notifications.h"
#include "mesh/ChannelPolicy.h"
#include "mesh/QuickSend.h"
#include "apps/People.h"
#include "apps/Wardrive.h"
#include <Preferences.h>
#include <Mm1Packet.h>
#include <time.h>
#include <math.h>
#include <initializer_list>
#include "ui/Emoji.h"
#include <algorithm>
#include "ui/AaFonts.h"
#include "ui/Theme.h"
// Defined in U8g2_for_Adafruit_GFX.cpp; used to fall back to Latin-1 and placeholders for missing glyphs.
uint8_t u8g2_IsGlyph(u8g2_font_t* u8g2, uint16_t encoding);
int8_t u8g2_GetGlyphWidth(u8g2_font_t* u8g2, uint16_t encoding);
// The home tiles, in drawHome() order; the chosen order and the hidden ones: config.apps (App.h).
// The default: messages, places, tools, games, then the device itself.
const char* const uiApps[] = {"chats", "quick", "nodes", "nav", "map", "radar", "wardrive", "people", "chess", "checkers", "sea", "pet", "dice", "solitaire", "connect", "health", "settings"};
const uint8_t uiAppCount = sizeof uiApps / sizeof *uiApps;
namespace {
enum Page { Home,
            Threads,
            Chat,
            Map,
            Nodes,
            Sensors,
            Settings,
            Radio,
            Display,
            Network,
            Diagnostics,
            Help,
            Library,
            Node,
            Scope,
            Homing,
            Motion,
            Game,
            NetList,
            ChessList,
            ChessPick,
            ChessBoard,
            RolePick,
            ServerHome,
            ChannelAdd,
            ChannelInfo,
            ChessTour,
            ChessTourNew,
            PetView,
            DiceView,
            Remote,
            QuickView,
            PeopleView,
            WardriveView,
            Folder,
            CheckersList,
            SeaList,
            DuelPick,
            CheckersBoard,
            SeaBoard,
            ChatMenu };
const char* pageNames[] = {"home", "threads", "chat", "map", "nodes", "sensors", "settings", "radio", "display", "network", "diagnostics", "help", "library", "node", "radar", "homing", "motion", "solitaire", "internet", "chess", "chess_pick", "chess_board", "role", "server", "channel_add", "channel", "chess_tour", "chess_tour_new", "pet", "dice", "remote", "quick", "people", "wardrive", "folder", "checkers", "sea", "duel_pick", "checkers_board", "sea_board", "chat_menu"};
enum Key { Enter = 13,
           Erase = 8,
           KeyMsg = 0x81,
           KeyHome = 0x82,
           KeyAt = 0x83,
           KeyAdv = 0x84,
           KeyMap = 0x85,
           KeyBack = 0x86,
           KeyGps = 0x87,
           KeyMic = 0x88,
           KeySet = 0x90,
           KeyHold = 0xa3,
           KeyLeft = 0xb4,
           KeyUp = 0xb5,
           KeyDown = 0xb6,
           KeyRight = 0xb7 };
Page page = Home, chatReturn = Threads, subReturn = Settings, pickReturn = Home; // subReturn: where BACK leaves a page opened from several places; pickReturn: a game's contact choice
// App folders (Apps.cpp): the folder on the Folder page, and the folder the open app came from (BACK returns there, -1: Home).
int folderOpen = -1, appFolder = -1;
Duel::Kind duelKind = Duel::Checkers; // the checkers or sea battle list and contact choice (UiDuel.inc)
bool playMenu = false;
int playSel = 0; // the node card's "Play": chess, checkers, sea battle
bool quickTargets = false, tallyNew = false;
uint64_t quickReplyTo = 0; // tallyNew: the quick counter shows its "new card"
bool scopeManual = false, csiBeaconRole = false; // radar: selection moved by the user; CSI page role
 // radar list: the user moved the selection off the strongest signal
uint32_t lastDraw = 0, lastInput = 0, toastAt = 0;
String toast, eventSeen;
uint16_t toastColor = 0;
int selected = 0, chatOffset = 0, action = 0;
uint64_t recipient = meshmesh::Broadcast, focusNode = 0, focusChannel = 0;
String composer, edit;
bool editing = false, dirty = true, keyed = false, locked = false, wakeOnly = false, deleteArmed = false;
Config draft;
struct ComposerDraft {
  uint64_t recipient = 0;
  String text;
  uint32_t touched = 0;
} drafts[16];
struct ReadMark {
  uint64_t id;
  uint32_t at;
} marks[65];
unsigned markCount = 0;
DynamicJsonDocument library(8192);
uint64_t conversations[65];
unsigned conversationCount = 1;
unsigned nodeOrder[24], nodeTotal = 0;

// Palette: the active theme (ui/Theme.h, ui/themes/) as RGB565 and its corner radii; theme keeps its 0xRRGGBB values for mix().
constexpr uint16_t rgb(uint32_t v) { return ((v >> 8) & 0xf800) | ((v >> 5) & 0x07e0) | ((v >> 3) & 0x1f); }
const Theme* theme = themes[0];
uint16_t surfacePage, surfaceBar, surfaceCard, surfaceFocus, surfaceWell, border, gridLine, accent, accentDeep;
uint16_t textPrimary, textMuted, textSubtle, iconPrimary, iconMuted, iconSubtle, graphicPrimary, graphicMuted, graphicSubtle, onFill;
uint16_t secondary, secondaryDeep, tertiary, info, ok, okDeep, warn, warnDeep, alert, bad, badDeep, outBubble, inBubble;
uint16_t avatarHues[8];
uint8_t radiusDialog, radiusCard, radiusBubble, radiusRow, radiusRowCompact, radiusField, radiusWell, radiusToast, radiusOverlay, radiusKey, radiusPill, radiusBar, radiusStripe;
void applyTheme(const Theme& t) {
  theme = &t;
  surfacePage = rgb(t.surfacePage), surfaceBar = rgb(t.surfaceBar), surfaceCard = rgb(t.surfaceCard), surfaceFocus = rgb(t.surfaceFocus), surfaceWell = rgb(t.surfaceWell), border = rgb(t.border), gridLine = rgb(t.gridLine);
  textPrimary = rgb(t.textPrimary), textMuted = rgb(t.textMuted), textSubtle = rgb(t.textSubtle);
  iconPrimary = rgb(t.iconPrimary), iconMuted = rgb(t.iconMuted), iconSubtle = rgb(t.iconSubtle);
  graphicPrimary = rgb(t.graphicPrimary), graphicMuted = rgb(t.graphicMuted), graphicSubtle = rgb(t.graphicSubtle);
  onFill = rgb(t.onFill);
  accent = rgb(t.accent), accentDeep = rgb(t.accentDeep);
  secondary = rgb(t.secondary), secondaryDeep = rgb(t.secondaryDeep), tertiary = rgb(t.tertiary), info = rgb(t.info);
  ok = rgb(t.ok), okDeep = rgb(t.okDeep), warn = rgb(t.warn), warnDeep = rgb(t.warnDeep), alert = rgb(t.alert), bad = rgb(t.bad), badDeep = rgb(t.badDeep);
  outBubble = rgb(t.outBubble), inBubble = rgb(t.inBubble);
  for(unsigned i = 0; i < 8; i++) avatarHues[i] = rgb(t.avatarHues[i]);
  radiusDialog = t.radiusDialog, radiusCard = t.radiusCard, radiusBubble = t.radiusBubble, radiusRow = t.radiusRow, radiusRowCompact = t.radiusRowCompact;
  radiusField = t.radiusField, radiusWell = t.radiusWell, radiusToast = t.radiusToast, radiusOverlay = t.radiusOverlay, radiusKey = t.radiusKey;
  radiusPill = t.radiusPill, radiusBar = t.radiusBar, radiusStripe = t.radiusStripe;
}
const bool themeApplied = (applyTheme(*themes[0]), true); // before any drawing, also in the host preview

// Text: the Cyrillic fonts lack Latin-1 and punctuation such as "·", "°", "№"; other languages' letters
// come from the fallback fonts of I18n.h, and Arabic is drawn in visual order.
// A face draws from its anti-aliased Inter face (aa, src/ui/AaFonts.h) first, sized to the cap height of the
// bitmap face it replaces; what aa lacks (CJK, Arabic, ...) comes from the u8g2 faces on the same baseline.
struct Face {
  const uint8_t* main;
  const uint8_t* latin;
  const AaFont& aa;
};
const Face body{u8g2_font_6x13_t_cyrillic, u8g2_font_6x13_tf, mmAaBody}, bold{u8g2_font_6x13B_t_cyrillic, u8g2_font_6x13B_tf, mmAaBold}, small{u8g2_font_5x8_t_cyrillic, u8g2_font_5x8_tf, mmAaSmall};
const Face card{u8g2_font_9x15_t_cyrillic, u8g2_font_9x15B_tf, mmAaCard}; // menu row names
const Face big{u8g2_font_10x20_t_cyrillic, u8g2_font_10x20_tf, mmAaBig}, digits{u8g2_font_logisoso42_tn, nullptr, mmAaDigits};
const AaGlyph* aaFind(const AaFont& f, uint32_t cp) {
  int lo = 0, hi = int(f.count) - 1;
  while(lo <= hi) {
    int m = (lo + hi) / 2;
    uint16_t c = f.glyphs[m].cp;
    if(c == cp) return &f.glyphs[m];
    if(c < cp) lo = m + 1;
    else hi = m - 1;
  }
  return nullptr;
}
// Blends the glyph's 4-bit coverage into the RGB565 canvas, each channel bg+(fg-bg)*a/15, clipped to the canvas.
int aaDraw(int x, int y, const AaGlyph& gl, const AaFont& f, uint16_t color) {
  GFXcanvas16& c = *hardware.canvas;
  uint16_t* buf = c.getBuffer();
  const int W = c.width(), H = c.height();
  const int fr = color >> 11, fg = (color >> 5) & 0x3f, fb = color & 0x1f, x0 = x + gl.dx, y0 = y - gl.top;
  const uint8_t* d = f.data + gl.offset;
  // Clipped once per glyph rather than per pixel: only the rows and columns on the canvas are visited.
  const int r0 = max(0, -y0), r1 = min(int(gl.h), H - y0), k0 = max(0, -x0), k1 = min(int(gl.w), W - x0);
  for(int r = r0; r < r1; r++) {
    uint16_t* row = buf + (y0 + r) * W + x0;
    for(int k = k0, n = r * gl.w + k0; k < k1; k++, n++) {
      int a = n & 1 ? d[n >> 1] & 0x0f : d[n >> 1] >> 4;
      if(!a) continue;
      uint16_t& p = row[k];
      if(a == 15) {
        p = color;
        continue;
      }
      int br = p >> 11, bg_ = (p >> 5) & 0x3f, bb = p & 0x1f;
      p = uint16_t(((br + ((fr - br) * a + 7) / 15) << 11) | ((bg_ + ((fg - bg_) * a + 7) / 15) << 5) | (bb + ((fb - bb) * a + 7) / 15));
    }
  }
  return gl.advance;
}
const uint8_t* activeFont = nullptr;
void useFont(const uint8_t* f) {
  if(f != activeFont) {
    hardware.font.setFont(f);
    hardware.font.setFontMode(1);
    activeFont = f;
  }
} // SetFont resets transparency
uint32_t nextCp(const String& s, unsigned& i) {
  uint8_t c = s[i];
  unsigned n = c < 0x80 ? 1 : (c & 0xe0) == 0xc0 ? 2
                            : (c & 0xf0) == 0xe0 ? 3
                                                 : 4;
  uint32_t cp = n == 1 ? c : n == 2 ? c & 0x1f
                           : n == 3 ? c & 0x0f
                                    : c & 0x07;
  for(unsigned k = 1; k < n && i + k < s.length(); k++) cp = cp << 6 | (uint8_t(s[i + k]) & 0x3f);
  i += n;
  return cp;
}
const char* substitute(uint32_t cp) {
  switch(cp) {
  case 0x2026: return "...";
  case 0x2013:
  case 0x2014:
  case 0x2212: return "-";
  case 0x2116: return "No";
  case 0x2018:
  case 0x2019: return "'";
  case 0x201c:
  case 0x201d:
  case 0x201e: return "\"";
  case 0x2192: return "->";
  case 0x2190: return "<-";
  case 0x2022: return "\xc2\xb7";
  }
  return nullptr;
}
// Emoji (Emoji.h): the picture in its colours, 12 px (8 px in the small face) from 1 px under the baseline,
// 2 px apart (the pictures fill their squares).
int glyph(int x, int y, uint32_t cp, const Face& f, uint16_t color, bool paint) {
  auto& u = hardware.font;
  const uint8_t* use = nullptr;
  if(char l = emoji::flagLetter(cp)) cp = uint8_t(l);
  if(cp >= 0x203c) {
    if(emoji::modifier(cp)) return 0;
    int e = emoji::find(cp);
    if(e >= 0) {
      int s = f.main == small.main ? 8 : emoji::Size;
      uint8_t px[emoji::Size * emoji::Size];
      if(paint && emoji::decode(e, px)) emoji::draw(px, x, y - s + 1 + s / 12, s, [](int a, int b, uint8_t v) { hardware.canvas->drawPixel(a, b, rgb(emoji::palette[v - 1])); });
      return s + 2;
    }
  }
  if(const AaGlyph* a = aaFind(f.aa, cp)) return paint ? aaDraw(x, y, *a, f.aa, color) : a->advance;
  if(cp <= 0xffff) {
    useFont(f.main);
    if(u8g2_IsGlyph(&u.u8g2, cp)) use = f.main;
    else if(f.latin) {
      useFont(f.latin);
      if(u8g2_IsGlyph(&u.u8g2, cp)) use = f.latin;
    }
  }
  if(!use && cp <= 0xffff)
    for(const uint8_t* const* x = fallbackFonts(f.main); *x; x++)
      if(fontHasGlyph(*x, cp)) {
        use = *x;
        break;
      } // other languages' letters
  if(!use) {
    if(const char* sub = substitute(cp)) {
      String s(sub);
      int w = 0;
      for(unsigned i = 0; i < s.length();) w += glyph(x + w, y, nextCp(s, i), f, color, paint);
      return w;
    }
    int w = f.aa.zeroWidth, a = f.aa.capHeight;
    if(paint) hardware.canvas->drawRect(x + 1, y - a, max(2, w - 2), a, color);
    return w; // visible placeholder, e.g. emoji
  }
  useFont(use);
  if(!paint) return u8g2_GetGlyphWidth(&u.u8g2, cp);
  u.setForegroundColor(color);
  return u.drawGlyph(x, y, cp);
}
int measure(const String& text, const Face& f = body) {
  const String& s = text;
  int w = 0;
  for(unsigned i = 0; i < s.length();) w += glyph(0, 0, nextCp(s, i), f, 0, false);
  return w;
}
int text(int x, int y, const String& s, uint16_t color = textPrimary, const Face& f = body) {
  for(unsigned i = 0; i < s.length();) x += glyph(x, y, nextCp(s, i), f, color, true);
  return x;
}
void textRight(int x, int y, const String& s, uint16_t color = textPrimary, const Face& f = body) { text(x - measure(s, f), y, s, color, f); }
void textCenter(int x, int y, const String& s, uint16_t color = textPrimary, const Face& f = body) { text(x - measure(s, f) / 2, y, s, color, f); }
// By value: a temporary (t(), a concatenation) is moved in and back out, so text that fits is not copied.
String fit(String s, int px, const Face& f = body) {
  if(measure(s, f) <= px) return s;
  int dots = measure("...", f), w = 0;
  unsigned i = 0, end = 0;
  while(i < s.length()) {
    int g = glyph(0, 0, nextCp(s, i), f, 0, false);
    if(w + g + dots > px) break;
    w += g;
    end = i;
  }
  s.remove(end);
  s += "...";
  return s;
}
// Word wrap by pixel width; long words break anywhere.
unsigned wrap(const String& s, String* rows, unsigned maxRows, int px, const Face& f = body) {
  unsigned count = 0, i = 0;
  String row;
  int rowW = 0;
  auto push = [&] {if(count<maxRows)rows[count++]=row;row="";rowW=0; };
  while(i < s.length() && count < maxRows) {
    if(s[i] == '\n') {
      push();
      i++;
      continue;
    }
    unsigned start = i;
    int w = 0;
    while(i < s.length() && s[i] != '\n') {
      uint32_t cp = nextCp(s, i);
      w += glyph(0, 0, cp, f, 0, false);
      if(cp == ' ') break;
    }
    String word = s.substring(start, i);
    if(rowW + w <= px) {
      row += word;
      rowW += w;
      continue;
    }
    if(rowW) {
      push();
      if(count >= maxRows) break;
    }
    if(w <= px) {
      row = word;
      rowW = w;
      continue;
    }
    for(unsigned j = 0; j < word.length();) {
      unsigned at = j;
      int g = glyph(0, 0, nextCp(word, j), f, 0, false);
      if(rowW + g > px) {
        push();
        if(count >= maxRows) break;
      }
      row += word.substring(at, j);
      rowW += g;
    }
  }
  if(row.length() && count < maxRows) rows[count++] = row;
  return count;
}
#define t(en) String(tr(en)) // a macro: tr() keys its translation at compile time (I18n.h)
String flag(bool v) { return v ? t("On") : t("Off"); }
String count(unsigned n, const char* one, const char* many) { return plural(n, one, many); }
// Right after a letter toggles its case: capitals for letters typed on digit keys.
bool toggleLastCase(String& text) {
  if(!text.length()) return false;
  unsigned at = meshmesh::previousCharacter(text.c_str(), text.length()), i = at;
  uint32_t cp = nextCp(text, i), to = cp;
  if(cp >= 'a' && cp <= 'z') to = cp - 32;
  else if(cp >= 'A' && cp <= 'Z') to = cp + 32;
  if(to == cp) return false;
  text.remove(at);
  text += char(to);
  return true;
}
void notice(const String& value, uint16_t color = accent) {
  toast = value;
  toastAt = millis();
  toastColor = color;
  dirty = true;
}

// Drawing primitives and icons (drawn, not font glyphs, so they scale and take state colours).
GFXcanvas16& g() { return *hardware.canvas; }
void panel(int x, int y, int w, int h, uint16_t fill, int r = 6) { g().fillRoundRect(x, y, w, h, r, fill); }
void ring(int x, int y, int w, int h, int r = 6, uint16_t color = accent) {
  g().drawRoundRect(x, y, w, h, r, color);
  g().drawRoundRect(x + 1, y + 1, w - 2, h - 2, max(r - 1, 1), color);
}
void tri(int x, int y, int dir, int s, uint16_t c) {
  if(dir == 0) g().fillTriangle(x - s, y + s / 2, x + s, y + s / 2, x, y - s / 2 - 1, c);
  else if(dir == 2) g().fillTriangle(x - s, y - s / 2, x + s, y - s / 2, x, y + s / 2 + 1, c);
  else if(dir == 1) g().fillTriangle(x - s / 2, y - s, x - s / 2, y + s, x + s / 2 + 1, y, c);
  else g().fillTriangle(x + s / 2, y - s, x + s / 2, y + s, x - s / 2 - 1, y, c);
}
void thickLine(int x0, int y0, int x1, int y1, uint16_t c) {
  g().drawLine(x0, y0, x1, y1, c);
  g().drawLine(x0 + 1, y0, x1 + 1, y1, c);
}
void icon(Icon id, int cx, int cy, int s, uint16_t c, uint16_t hole = surfacePage) {
  drawIcon(g(), id, cx, cy, s, c, hole, iconSubtle);
  if(id == IcHelp) textCenter(cx, cy + 5, "?", c, bold);
}
void bars(int x, int y, unsigned level, uint16_t color) {
  for(unsigned k = 0; k < 4; k++) {
    int h = 3 + k * 2;
    g().fillRect(x + k * 4, y - h, 3, h, k < level ? color : border);
  }
}
unsigned snrLevel(float snr) { return snr >= 5 ? 4 : snr >= 0   ? 3
                                                   : snr >= -5  ? 2
                                                   : snr >= -10 ? 1
                                                                : 0; }
// Avatars: initial of the name, colour from the ID.
String initial(const String& name) {
  if(!name.length()) return "?";
  unsigned i = 0;
  uint32_t cp = nextCp(name, i);
  if(cp >= 0x430 && cp <= 0x44f) cp -= 0x20;
  else if(cp == 0x451) cp = 0x401;
  else if(cp < 128) cp = toupper(cp);
  else if(cp >= 0x800) return "?";
  char b[4] = {};
  if(cp < 0x80) b[0] = cp;
  else {
    b[0] = 0xc0 | cp >> 6;
    b[1] = 0x80 | (cp & 0x3f);
  }
  return b;
}
void avatar(int cx, int cy, int r, uint64_t id, const String& name, int type = -1) {
  uint16_t hue = avatarHues[(id ^ id >> 17 ^ id >> 41) % 8];
  if(!id) {
    g().fillCircle(cx, cy, r, surfaceCard);
    g().fillRect(cx - r / 2, cy - 1, r, 3, accent);
    g().fillRect(cx - 1, cy - r / 2, 3, r, accent);
    return;
  } // "add a channel"
  if(channels::isChannel(id)) {
    const channels::Channel* c = meshRadio.channel(id);
    bool priv = c && !channels::isPublic(c->secret) && !channels::isHashtag(*c);
    g().fillCircle(cx, cy, r, priv ? secondaryDeep : accentDeep);
    // The hash is as wide as the "add a channel" plus; the lock needs more size to keep its shackle readable.
    if(priv) icon(IcLock, cx, cy, r * 3 / 5, secondary);
    else icon(IcHash, cx, cy, r / 2, accent);
    return;
  }
  if(type > 1) {
    uint16_t tc = type == 2 ? secondary : type == 3 ? warn
                                                    : info;
    g().fillCircle(cx, cy, r, surfaceCard);
    g().drawCircle(cx, cy, r, tc);
    icon(type == 2 ? IcTower : type == 3 ? IcRoom
                                         : IcSensor,
         cx, cy, r / 2 + 1, tc, surfaceCard);
    return;
  }
  g().fillCircle(cx, cy, r, hue);
  textCenter(cx, cy + (r > 12 ? 5 : 4), initial(name), textPrimary, r > 12 ? bold : small);
}
// Touch targets of the frame on screen (uiTouch): the items a tap selects (index as in "selected")
// and the footer hints, which act as their keys. draw() clears them.
struct Target {
  int16_t x, y, w, h, index;
};
Target targets[16];
unsigned targetCount = 0;
struct HintSpot {
  int16_t x0, x1;
  int key;
};
HintSpot hintSpots[6];
unsigned hintCount = 0;
// index >= 0 selects that item; a negative index is a key (-KeyRight).
void target(int x, int y, int w, int h, int index) {
  if(targetCount < 16) targets[targetCount++] = {int16_t(x), int16_t(y), int16_t(w), int16_t(h), int16_t(index)};
}
int hintKey(const char* k) {
  static const struct {
    const char* name;
    int key;
  } keys[] = {{"OK", Enter}, {"BACK", KeyBack}, {"DEL", Erase}, {"MSG", KeyMsg}, {"MAP", KeyMap}, {"HOME", KeyHome}, {"ADV", KeyAdv}, {"MIC", KeyMic}, {"CTRL", KeySet}, {"SPC", ' '}, {"<>", KeyRight}, {"<", KeyLeft}, {"^v", KeyDown}, {"+/-", '+'}, {"@", KeyAt}};
  for(auto& v : keys)
    if(!strcmp(k, v.name)) return v.key;
  return k[0] >= 'A' && k[0] <= 'Z' && !k[1] ? k[0] + 32 : 0; // letter keys (chess, solitaire, map)
}
// Softkey footer: [KEY] action pairs; "<>" and "^v" draw arrow pairs.
struct Hint {
  const char* key;
  String action;
};
void footer(std::initializer_list<Hint> hints) {
  auto& d = g();
  d.fillRect(0, 221, 320, 19, surfaceBar);
  d.drawFastHLine(0, 220, 320, border);
  int x = 6;
  for(auto& h : hints) {
    bool arrows = !strcmp(h.key, "<>") || !strcmp(h.key, "^v") || !strcmp(h.key, "<");
    int kw = arrows ? 17 : measure(h.key, small) + 8, aw = measure(h.action, body);
    if(x + kw + 4 + aw > 316) break;
    if(int key = hintKey(h.key))
      if(hintCount < 6) hintSpots[hintCount++] = {int16_t(x - 3), int16_t(x + kw + 4 + aw + 5), key};
    d.fillRoundRect(x, 224, kw, 13, radiusKey, border);
    if(!strcmp(h.key, "<>")) {
      tri(x + 5, 230, 3, 3, iconPrimary);
      tri(x + 12, 230, 1, 3, iconPrimary);
    } else if(!strcmp(h.key, "<")) tri(x + 8, 230, 3, 3, iconPrimary);
    else if(!strcmp(h.key, "^v")) {
      tri(x + 5, 231, 0, 3, iconPrimary);
      tri(x + 12, 229, 2, 3, iconPrimary);
    } else text(x + 4, 234, h.key, textPrimary, small);
    text(x + kw + 4, 235, h.action, textMuted);
    x += kw + 4 + aw + 10;
  }
}
// Conversations, read marks and drafts.
bool belongs(const ChatMessage& m, uint64_t id) { return channels::isChannel(id) ? m.destination == id : (m.outgoing ? m.destination == id : m.source == id && (m.destination == meshRadio.nodeId || m.protocol == 1)); }
// Chats: the joined channels (Public first), "add a channel" (ID 0), then direct conversations, newest first.
// A left channel's messages stay in the history but out of the list.
unsigned channelRows = 1;
void threads() {
  conversationCount = 0;
  for(unsigned i = 0; i < meshRadio.channelCount; i++) conversations[conversationCount++] = meshRadio.channelList[i].id;
  conversations[conversationCount++] = 0;
  channelRows = conversationCount;
  for(int i = int(meshRadio.historyCount) - 1; i >= 0; i--) {
    auto& m = meshRadio.history[i];
    if(channels::isChannel(m.destination)) continue;
    uint64_t id = m.outgoing ? m.destination : m.source;
    bool found = false;
    for(unsigned j = channelRows; j < conversationCount; j++)
      if(conversations[j] == id) found = true;
    if(!found && conversationCount < 65) conversations[conversationCount++] = id;
  }
}
Peer* peerOf(uint64_t id) {
  return meshRadio.contact(id);
}
String nameOf(uint64_t id) {
  if(channels::isChannel(id)) {
    const channels::Channel* c = meshRadio.channel(id);
    return c ? String(c->name) : t("Channel");
  }
  if(Peer* p = peerOf(id)) return p->name;
  for(int i = meshRadio.historyCount - 1; i >= 0; i--)
    if(meshRadio.history[i].source == id) return meshRadio.history[i].name;
  return meshRadio.idText(id);
}
String readKey(uint64_t id) {
  if(id == meshmesh::Broadcast) return "all";
  char b[15] = {};
  unsigned n = 0;
  do {
    b[n++] = "0123456789abcdefghijklmnopqrstuvwxyz"[id % 36];
    id /= 36;
  } while(id);
  String key = "r";
  while(n) key += b[--n];
  return key;
}
// Read marks: one file in MeshMesh storage (NVS is small); a mark an older firmware left in NVS is read
// once and removed when that chat is read again. Without the storage they stay in NVS.
struct ReadFile {
  uint32_t version = 1, count = 0;
  ReadMark marks[65] = {};
  uint32_t check = 0;
};
uint32_t fnv(const uint8_t* p, size_t n) {
  uint32_t h = 2166136261u;
  while(n--) h = (h ^ *p++) * 16777619u;
  return h;
}
bool marksLoaded = false;
void loadMarks() {
  marksLoaded = true;
  ReadFile* f = new ReadFile;
  if(readStored("/meshmesh/read.bin", "/meshmesh/read.tmp", f, sizeof(*f))) {
    if(f->version == 1 && f->count <= 65 && f->check == fnv((const uint8_t*)f, offsetof(ReadFile, check)))
      for(unsigned i = 0; i < f->count && markCount < 65; i++) marks[markCount++] = f->marks[i];
  }
  delete f;
}
bool saveMarks() {
  ReadFile* f = new ReadFile;
  f->count = markCount;
  for(unsigned i = 0; i < markCount; i++) f->marks[i] = marks[i];
  f->check = fnv((const uint8_t*)f, offsetof(ReadFile, check));
  bool saved = writeStored("/meshmesh/read.bin", "/meshmesh/read.tmp", f, sizeof(*f));
  delete f;
  return saved;
}
uint32_t readAt(uint64_t id) {
  if(!marksLoaded) loadMarks();
  for(unsigned i = 0; i < markCount; i++)
    if(marks[i].id == id) return marks[i].at;
  uint32_t v = 0;
  Preferences p;
  if(p.begin("meshmesh-ui", true)) {
    v = p.getUInt(readKey(id).c_str(), 0);
    p.end();
  }
  if(markCount < 65) marks[markCount++] = {id, v};
  return v; // cached: status bar needs unread totals every frame
}
unsigned unread(uint64_t id) {
  uint32_t at = readAt(id);
  unsigned n = 0;
  for(unsigned i = 0; i < meshRadio.historyCount; i++) {
    auto& m = meshRadio.history[i];
    if(belongs(m, id) && !m.outgoing && !m.seen && (m.timestamp > at || (!m.timestamp && m.uptime))) n++;
  }
  return n;
}
unsigned unreadTotal() {
  threads();
  unsigned n = 0;
  for(unsigned i = 0; i < conversationCount; i++)
    if(conversations[i]) n += unread(conversations[i]);
  return n;
}
void markRead() {
  uint32_t at = 0;
  for(unsigned i = 0; i < meshRadio.historyCount; i++) {
    auto& m = meshRadio.history[i];
    if(belongs(m, recipient) && !m.outgoing) {
      at = max(at, m.timestamp);
      m.seen = true;
    }
  }
  if(at <= readAt(recipient)) return;
  for(unsigned i = 0; i < markCount; i++)
    if(marks[i].id == recipient) marks[i].at = at;
  Preferences p;
  String key = readKey(recipient);
  if(saveMarks()) {
    if(p.begin("meshmesh-ui", false)) {
      if(p.isKey(key.c_str())) p.remove(key.c_str());
      p.end();
    }
  } else if(p.begin("meshmesh-ui", false)) {
    p.putUInt(key.c_str(), at);
    p.end();
  }
}
void rememberComposer() {
  int chosen = -1;
  for(int i = 0; i < 16; i++)
    if(drafts[i].recipient == recipient) {
      chosen = i;
      break;
    }
  if(chosen < 0)
    for(int i = 0; i < 16; i++)
      if(!drafts[i].recipient) {
        chosen = i;
        break;
      }
  if(chosen < 0) {
    chosen = 0;
    for(int i = 1; i < 16; i++)
      if(drafts[i].touched < drafts[chosen].touched) chosen = i;
  }
  drafts[chosen].recipient = recipient;
  drafts[chosen].text = composer;
  drafts[chosen].touched = millis();
}
String restoredComposer() {
  for(auto& d : drafts)
    if(d.recipient == recipient) return d.text;
  return "";
}
// Nodes: most recently heard first; focus follows the node, not its row.
void sortNodes() {
  nodeTotal = meshRadio.peerCount;
  for(unsigned i = 0; i < nodeTotal; i++) nodeOrder[i] = i;
  uint32_t now = millis();
  auto before = [&](const Peer& a, const Peer& b) {if(a.heard!=b.heard)return a.heard;return a.heard&&now-a.seen<now-b.seen; };
  for(unsigned i = 1; i < nodeTotal; i++)
    for(unsigned j = i; j > 0 && before(meshRadio.peers[nodeOrder[j]], meshRadio.peers[nodeOrder[j - 1]]); j--) std::swap(nodeOrder[j], nodeOrder[j - 1]);
  if(!nodeTotal) {
    selected = 0;
    return;
  }
  for(unsigned i = 0; i < nodeTotal; i++)
    if(meshRadio.peers[nodeOrder[i]].id == focusNode) {
      selected = i;
      return;
    }
  selected = constrain(selected, 0, int(nodeTotal) - 1);
  focusNode = meshRadio.peers[nodeOrder[selected]].id;
}
Peer* focusedPeer() { return peerOf(focusNode); }
String netError();
String petTileDetail();
String diceTileDetail();
String tallyTileDetail();
uint16_t tallyHue(unsigned i);
bool tallyAdding();
String diceEditTitle();
unsigned diceEditLimit();
String diceEditHint();
String diceEditOk();
bool diceEditRaw();
void gameOpen();
void gameLeave();
String gameTitle();
String gameTileDetail();
String chessTileDetail();
String chessTitle();
String tourTitle();
String duelTileDetail(int kind);
String duelTitle();
bool sharedPage(Page p) { return p == Radio || p == Display || p == Sensors || p == Network || p == Diagnostics || p == Help || p == Library; }
// Leaving the radar pages keeps the radar running while the web page or the wardrive nets log holds it.
// A page opened from the menu, the settings, the map or a hotkey goes back there (NetList is Network's own).
// BACK returns to the menu tile or the settings row that was opened.
int homeSelected = 0, settingsSelected = 0, folderSelected = 0;
// An app opened from a folder goes back to the folder (with its tile selected) where it would go Home.
void change(Page next, bool back = false) {
  if(next == Home && appFolder >= 0 && page != Home && page != Folder) {
    uint8_t o[AppEntriesMax];
    if(appsShown(o, appFolder)) {
      folderOpen = appFolder;
      next = Folder;
      back = true;
    }
  }
  if(next == Home) appFolder = -1;
  playMenu = false;
  if(page == Folder) folderSelected = selected;
  if(next == PeopleView) tallyNew = false;
  if(sharedPage(next) && !sharedPage(page) && page != NetList) subReturn = page;
  if(page == Home) homeSelected = selected;
  if(page == Settings) settingsSelected = selected;
  if(page == Game && next != Game) gameLeave();
  if(next == Game && page != Game) gameOpen();
  if(page == Chat && next != Chat) rememberComposer();
  bool radarPage = next == Scope || next == Homing || next == Motion;
  if(radarPage && page != Scope && page != Homing && page != Motion) scopeManual = false;
  if(radarPage) radar.open();
  else if(!webRadarActive() && !wardrive::holdsRadar()) radar.close();
  if(radarPage || !webRadarActive()) radar.setCsi(next != Motion ? Radar::CsiOff : csiBeaconRole ? Radar::CsiBeacon
                                                                                                 : Radar::CsiSensor);
  if(next == Scope) radar.untrack();
  page = next;
  selected = back && next == Home ? homeSelected : back && next == Folder   ? folderSelected
                                                 : back && next == Settings ? settingsSelected
                                                                            : 0;
  chatOffset = 0;
  action = 0;
  editing = false;
  deleteArmed = false;
  dirty = true;
  if(next == Radio || next == Display) draft = config;
  if(next == Threads) threads();
  if(next == Chat) {
    composer = restoredComposer();
    markRead();
  }
  if(next == Library) deserializeJson(library, maps.areas());
  if(next == Nodes) sortNodes();
  if(next == NetList) internet.rescan();
}

// Time, distances and short labels.
bool localTime(time_t at, tm& out) {
  if(at < 1700000000) return false;
  at += config.utcOffset * 60;
  out = *gmtime(&at);
  return true;
}
String clockText() {
  tm v;
  if(!localTime(time(nullptr), v)) return "--:--";
  char b[6];
  snprintf(b, sizeof b, "%d:%02d", v.tm_hour, v.tm_min);
  return b;
}
String timeText(uint32_t at) {
  tm v, today;
  if(!localTime(at, v)) return "";
  char b[24];
  if(localTime(time(nullptr), today) && (v.tm_yday != today.tm_yday || v.tm_year != today.tm_year)) snprintf(b, sizeof b, "%02d.%02d", v.tm_mday, v.tm_mon + 1);
  else snprintf(b, sizeof b, "%d:%02d", v.tm_hour, v.tm_min);
  return b;
}
String dateText() {
  tm v;
  if(!localTime(time(nullptr), v)) return t("Clock not set");
  const char* days[] = {tr("Sunday"), tr("Monday"), tr("Tuesday"), tr("Wednesday"), tr("Thursday"), tr("Friday"), tr("Saturday")};
  // Month names as the date below uses them (Russian: genitive).
  const char* months[] = {tr("January"), tr("February"), tr("March"), tr("April"), tr("May"), tr("June"), tr("July"), tr("August"), tr("September"), tr("October"), tr("November"), tr("December")};
  String s = t("{weekday}, {month} {day}");
  s.replace("{weekday}", days[v.tm_wday]);
  s.replace("{month}", months[v.tm_mon]);
  s.replace("{day}", String(v.tm_mday));
  return s;
}
String ago(uint32_t ms) {
  uint32_t s = ms / 1000;
  if(s < 60) return t("now");
  if(s < 3600) return String(s / 60) + t(" min");
  if(s < 86400) return String(s / 3600) + t(" h");
  return String(s / 86400) + t(" d");
}
// Wardrive signal: a path hash, SNR in quarter dB and its colour (Wardrive.h).
String hashText(const uint8_t* h, uint8_t size) {
  String s;
  char b[3];
  for(uint8_t i = 0; i < size && i < 3; i++) {
    snprintf(b, 3, "%02X", h[i]);
    s += b;
  }
  return s;
}
String snrText(int8_t q) { return String(q / 4.0f, 1) + t(" dB"); }
uint16_t snrColor(int8_t q) { return q == -128 ? bad : q >= 20  ? ok
                                                     : q >= -20 ? warn
                                                                : alert; } // quarter dB: 5 dB and over, -5 dB and over, weaker; -128 nobody
String pathText(const Peer& p) {
  if(p.pathLength == 255) return t("path unknown");
  if(!(p.pathLength & 63)) return t("direct");
  return count(p.pathLength & 63, "hop", "hops");
}
String typeText(uint8_t type) {
  switch(type) {
  case 1: return t("Chat node");
  case 2: return t("Repeater");
  case 3: return t("Room server");
  case 4: return t("Sensor");
  }
  return t("Node");
}
uint16_t typeColor(uint8_t type) { return type == 2 ? secondary : type == 3 ? warn
                                                                : type == 4 ? info
                                                                            : accent; }
bool distanceTo(const Peer& p, float& metres, float& bearing) {
  if(!p.position || !hardware.gpsFix()) return false;
  double la1 = hardware.gps.location.lat() * M_PI / 180, la2 = p.latitude * M_PI / 180, dl = (p.longitude - hardware.gps.location.lng()) * M_PI / 180;
  double a = sin((la2 - la1) / 2) * sin((la2 - la1) / 2) + cos(la1) * cos(la2) * sin(dl / 2) * sin(dl / 2);
  metres = 12742000 * atan2(sqrt(a), sqrt(1 - a));
  bearing = fmod(atan2(sin(dl) * cos(la2), cos(la1) * sin(la2) - sin(la1) * cos(la2) * cos(dl)) * 180 / M_PI + 360, 360);
  return true;
}
String distanceText(float m, float bearing) {
  const char* dirs[] = {tr("N"), tr("NE"), tr("E"), tr("SE"), tr("S"), tr("SW"), tr("W"), tr("NW")};
  int k = int((bearing + 22.5f) / 45) % 8;
  return (m < 1000 ? String(int(m / 10) * 10) + t(" m") : String(m / 1000, m < 10000 ? 1 : 0) + t(" km")) + " " + dirs[k];
}

// Status bar (HUD): title on the left; radio, GPS, links, battery and clock on the right.
String title() {
  if(locked) return config.name;
  switch(page) {
  case Home: return "BoT Mesh";
  case Threads: return t("Chats");
  case Chat: return nameOf(recipient);
  case Map: return t("Map");
  case Nodes: return t("Nodes");
  case Node: {
    Peer* p = focusedPeer();
    return p ? String(p->name) : t("Node");
  }
  case Sensors: return t("Navigation");
  case Settings: return t("Settings");
  case Radio: return t("Radio");
  case Display: return t("Screen & device");
  case Network: return t("Connections");
  case Diagnostics: return t("Module health");
  case Help: return t("Keys");
  case Library: return t("Saved maps");
  case Scope: return t("Radar: signals");
  case Homing: return t("Homing");
  case Game: return gameTitle();
  case Motion: return t("Radar: motion (CSI)");
  case NetList: return t("Internet over Wi-Fi");
  case ChessList: return t("Chess") + " · ELO " + String(rating::book.myElo());
  case ChessPick: return t("New chess game");
  case ChessBoard: return chessTitle();
  case ChessTour: return tourTitle();
  case ChessTourNew: return t("New tournament");
  case PetView: return t("Pet");
  case DiceView: return t("Dice");
  case PeopleView: return t("Quick counter");
  case WardriveView: return t("Wardrive");
  case QuickView: return t("Quick send");
  case Remote: return t("Server and route");
  case RolePick: return t("Device mode");
  case ChannelAdd: return t("Add a channel");
  case ChannelInfo: return nameOf(focusChannel);
  case ChatMenu: return nameOf(recipient);
  case Folder: return appsFolderName(folderOpen);
  case CheckersList:
  case SeaList:
  case DuelPick:
  case CheckersBoard:
  case SeaBoard: return duelTitle();
  case ServerHome: return config.role == RoleRoom ? t("Room server") : t("Repeater");
  }
  return "";
}
void statusBar() {
  auto& d = g();
  d.fillRect(0, 0, 320, 20, surfaceBar);
  d.drawFastHLine(0, 20, 320, border);
  int x = 315;
  String clock = clockText();
  textRight(x, 15, clock, textPrimary, bold);
  x -= measure(clock, bold) + 8;
  uint16_t mv = hardware.batteryMv;
  bool external = hardware.usbPower >= 0 ? hardware.usbPower > 0 : mv > 4250;
  unsigned pct = batteryPercent(mv);
  if(mv && (config.batteryVolts || !external)) {
    String p = config.batteryVolts ? String(mv / 1000.f, 2) + "V" : String(pct) + "%";
    textRight(x, 14, p, textMuted, small);
    x -= measure(p, small) + 3;
  }
  // A vertical battery in the colour of its state: body 6×8 with the nub on top, filled from the bottom; a plug
  // on its left on external power.
  uint16_t level = !mv ? graphicMuted : external || pct > 30 ? ok
                                      : pct > 15             ? warn
                                                             : bad;
  int bx = x - 6;
  d.drawRect(bx, 7, 6, 8, level);
  d.fillRect(bx + 2, 5, 2, 2, level);
  if(mv) {
    int h = external ? 6 : max(1u, 6 * pct / 100);
    d.fillRect(bx + 1, 14 - h, 4, h, level);
  }
  x = bx - 3;
  if(external) {
    int px = x - 6; // a power plug 6×10: two prongs, the body, the cable
    d.fillRect(px + 1, 5, 1, 2, level);
    d.fillRect(px + 4, 5, 1, 2, level);
    d.fillRect(px, 7, 6, 3, level);
    d.fillRect(px + 1, 10, 4, 1, level);
    d.fillRect(px + 2, 11, 2, 4, level);
    x -= 10;
  }
  bool fresh = meshRadio.lastRxAt && millis() - meshRadio.lastRxAt < 600000;
  bars(x - 15, 15, meshRadio.ready && fresh ? max(1u, snrLevel(meshRadio.lastSnr)) : 0, ok);
  if(!meshRadio.ready) {
    d.drawLine(x - 15, 5, x - 4, 15, bad);
    d.drawLine(x - 4, 5, x - 15, 15, bad);
  }
  x -= 21;
  if(meshRadio.busy()) {
    tri(x + 2, 10, 0, 3, accent);
    x -= 9;
  }
  if(config.gps) {
    icon(IcPin, x - 4, 9, 5, hardware.gpsFix() ? ok : hardware.clockConflict ? bad
                                                                              : warn,
         surfaceBar);
    x -= 13;
  }
  if(portalActive()) {
    icon(IcWifi, x - 6, 10, 7, accent, surfaceBar);
    x -= 17;
  } else if(internet.state != Internet::Off && internet.state != Internet::Paused) {
    icon(IcWifi, x - 6, 10, 7, internet.online() ? ok : iconSubtle, surfaceBar);
    x -= 17;
  }
  if(bleActive()) {
    icon(IcBle, x - 3, 9, 5, info, surfaceBar);
    x -= 10;
  }
  unsigned n = page == Threads || page == Chat ? 0 : unreadTotal();
  if(n) {
    String c = String(n);
    textRight(x, 14, c, warn, small);
    x -= measure(c, small) + 2;
    icon(IcMail, x - 7, 10, 7, warn, surfaceBar);
    x -= 19;
  }
  String name = title();
  if(page == Chat || page == Node) {
    tri(10, 10, 3, 4, iconMuted);
    text(17, 15, fit(name, x - 22, bold), textPrimary, bold);
  } else text(8, 15, fit(name, x - 14, bold), page == Home ? accent : textPrimary, bold);
}
void drawToast() {
  if(!toast.length() || millis() - toastAt >= 3500) return;
  String s = fit(toast, 280);
  int w = measure(s) + 26, x = 160 - w / 2;
  panel(x, 193, w, 23, surfaceFocus, radiusToast);
  g().drawRoundRect(x, 193, w, 23, radiusToast, border);
  g().fillRoundRect(x + 5, 198, 3, 13, radiusStripe, toastColor);
  text(x + 15, 209, s, textPrimary);
}
void scrollbar(int first, int visible, int total, int y, int h) {
  if(total <= visible) return;
  int th = max(12, h * visible / total), ty = y + (h - th) * first / max(1, total - visible);
  g().fillRoundRect(315, y, 3, h, radiusStripe, surfaceCard);
  g().fillRoundRect(315, ty, 3, th, radiusStripe, graphicSubtle);
}
// The highlight alone marks the selection, as on home. An unselected row is not painted: it would only write
// bg over the cleared bg, and on the M9 every filled pixel is a PSRAM write (half of a home frame was fills).
void listRow(int y, int h, bool focus) {
  if(focus) panel(8, y, 304, h, surfaceFocus, radiusRow);
}
// The first row of a game list (chess, checkers, sea battle): a plus in a disc centred at cy, "New game" and a hint.
void newGameRow(int cy, int r, int hintY, const String& hint, int hintW) {
  g().fillCircle(32, cy, r, accentDeep);
  g().fillRect(31, cy - 8, 3, 17, accent);
  g().fillRect(24, cy - 1, 17, 3, accent);
  text(56, cy - 4, t("New game"), textPrimary, bold);
  text(56, hintY, fit(hint, hintW, small), textMuted, small);
}

// Module health: the same eight modules as the diagnostics page.
unsigned moduleStates(bool* state) {
  bool s[] = {meshRadio.ready, hardware.keyboardOk, hardware.sdOk, hardware.fsOk, hardware.rtcValid, hardware.gps.passedChecksum() > 0, hardware.compassSample, hardware.imuSample};
  unsigned faults = 0;
  for(int i = 0; i < 8; i++) {
    if(state) state[i] = s[i];
    faults += !s[i];
  }
  return faults;
}
#include "ui/UiHome.inc"
#include "ui/UiChat.inc"
#include "ui/UiMap.inc"
#include "ui/UiNodes.inc"
#include "ui/UiSensors.inc"
#include "ui/UiRadar.inc"
#include "ui/UiSettings.inc"
#include "ui/UiWardrive.inc"
#include "ui/UiQuick.inc"
#include "ui/UiNetwork.inc"
#include "ui/UiSystem.inc"
#include "ui/UiServer.inc"
#include "ui/UiChannels.inc"
#include "ui/UiRemote.inc"
void drawEditing() {
  targetCount = hintCount = 0; // the page under the dialog does not take taps
  uint16_t* px = g().getBuffer();
  for(int i = 0; i < 320 * 240; i++) px[i] = (px[i] >> 1) & 0x7bef; // dim the page under the dialog
  bool key = page == Network, pass = page == NetList || page == ServerHome;
  panel(12, 60, 296, 122, surfaceFocus, radiusDialog);
  g().drawRoundRect(12, 60, 296, 122, radiusDialog, border);
  text(26, 82, key ? t("Network key · 64 hex") : page == ChannelAdd   ? channelEditTitle()
                                               : page == ChessTourNew ? String(t("Tournament name"))
                                               : page == PetView      ? String(t("Pet name"))
                                               : page == PeopleView   ? String(t("Counter name"))
                                               : page == DiceView     ? diceEditTitle()
                                               : page == ServerHome   ? serverEditTitle()
                                               : page == Remote       ? remoteEditTitle()
                                               : page == Radio        ? String(t("Region"))
                                               : regionEditPage()     ? String(t("Region of the channel"))
                                               : pass                 ? fit(t("Wi-Fi password: ") + pendingSsid, 268, bold)
                                                                      : t("Device name"),
       textPrimary, bold);
  panel(24, 92, 272, 52, surfaceWell, radiusField);
  g().drawRoundRect(24, 92, 272, 52, radiusField, accent);
  String rows[10];
  unsigned n = wrap(edit, rows, 10, 258), first = n > 3 ? n - 3 : 0;
  int cx = 32;
  for(unsigned i = first; i < n; i++) cx = text(32, 108 + (i - first) * 15, rows[i], textPrimary);
  g().fillRect(cx + 1, 98 + max(0, int(n - first) - 1) * 15, 2, 12, accent); // a long text shows its end, where the cursor is
  textRight(294, 160, String(edit.length()) + "/" + String(key ? 64 : page == Radio || regionEditPage() ? int(regions::NameBytes)
                                                                    : page == ChannelAdd                ? channelEditLimit()
                                                                    : page == ServerHome                ? serverEditLimit()
                                                                    : page == Remote                    ? remoteEditLimit()
                                                                    : pass                              ? 63
                                                                    : page == ChessTourNew              ? 32
                                                                    : page == PetView                   ? 15
                                                                    : page == DiceView                  ? diceEditLimit()
                                                                    : page == PeopleView                ? int(people::NameBytes)
                                                                                                        : 24) +
                        t(" bytes"),
            textSubtle, small);
  if(page == DiceView) {
    text(26, 160, diceEditHint(), textSubtle, small);
  } else if(page == ChannelAdd) {
    text(26, 160, fit(channelEditHint(), 230, small), textSubtle, small);
  } else if(page == Remote) {
    text(26, 160, remoteEdit == EditPassword ? t("Saved after a successful login") : t("As in the MeshCore app: ver, get name, neighbors..."), textSubtle, small);
  } else if(page == ServerHome) {
    text(26, 160, serverEditHint(), textSubtle, small);
  } else if(page == Radio || regionEditPage()) {
    text(26, 160, page == Radio ? t("As on repeaters: msk, sao...; empty: none") : t("As on repeaters: msk, sao...; empty: default"), textSubtle, small);
  } else if(pass) text(26, 160, t("8-63 characters; -> after a letter: capital"), textSubtle, small);
  text(26, 175, page == DiceView ? diceEditOk() : page == ChannelAdd ? channelEditOk()
                                                : page == Remote     ? (remoteEdit == EditPassword ? t("OK: log in   BACK: cancel") : t("OK: send   BACK: cancel"))
                                                : serverPostEdit()   ? t("OK: post   BACK: cancel")
                                                                     : t("OK: save   BACK: cancel"),
       textMuted, small);
}
#include "games/solitaire/UiSolitaire.inc"
#include "games/chess/UiChess.inc"
#include "games/duel/UiDuel.inc"
#include "games/chess/UiTour.inc"
#include "games/pet/UiPet.inc"
#include "games/dice/UiDice.inc"
#include "apps/UiTally.inc"
String tourTitle() { return tourOpen && tourOpen->state != tour::Free ? String(tourOpen->name) : t("Tournament"); }
String chessTitle() { return chessOpen && chessOpen->state != ChessMatch::Free ? t("Chess · ") + chessOpen->name : t("Chess"); }
// "uiperf" over USB: frames drawn per page since the last report, with the time spent drawing into the
// canvas and sending it to the panel (average/longest, ms) and the rows sent per frame.
constexpr int PerfSlots = SeaBoard + 2; // every page, then the lock screen
struct FrameStats {
  uint32_t frames, drawUs, drawMax, flushUs, flushMax, rows;
};
FrameStats frameStats[PerfSlots];
uint32_t perfSince = 0;
String perfReport() {
  String r = "UIPERF " + String((millis() - perfSince) / 1000) + " s";
  for(int i = 0; i < PerfSlots; i++) {
    auto& p = frameStats[i];
    if(!p.frames) continue;
    const char* name = i < PerfSlots - 1 ? pageNames[i] : "locked";
    char line[128];
    snprintf(line, sizeof line, "\n%s frames=%lu draw=%.1f/%.1f flush=%.1f/%.1f ms rows=%lu", name, (unsigned long)p.frames, p.drawUs / 1000.f / p.frames, p.drawMax / 1000.f, p.flushUs / 1000.f / p.frames, p.flushMax / 1000.f, (unsigned long)(p.rows / p.frames));
    r += line;
  }
  memset(frameStats, 0, sizeof frameStats);
  perfSince = millis();
  return r;
}
void draw() {
  uint32_t started = micros();
  auto& c = g();
  c.fillScreen(surfacePage);
  targetCount = hintCount = 0;
  if(powerOffPending()) {
    drawOff();
    hardware.flush();
    return;
  }
  if(locked) drawLocked();
  else switch(page) {
    case Home: drawHome(); break;
    case Threads: drawThreads(); break;
    case Chat: drawChat(); break;
    case Map: drawMap(); break;
    case Library: drawLibrary(); break;
    case Nodes: drawNodes(); break;
    case Node: drawNode(); break;
    case Sensors: drawSensors(); break;
    case Settings: drawSettings(); break;
    case Radio:
    case Display: drawEditor(); break;
    case Network: drawNetwork(); break;
    case NetList: drawNetList(); break;
    case Diagnostics: drawDiagnostics(); break;
    case Help: drawHelp(); break;
    case Scope: drawScope(); break;
    case Homing: drawHoming(); break;
    case Game: drawGame(); break;
    case Motion: drawMotion(); break;
    case ChessList: drawChessList(); break;
    case ChessPick: drawChessPick(); break;
    case ChessBoard: drawChessBoard(); break;
    case ChessTour: drawTourCard(); break;
    case ChessTourNew: drawTourNew(); break;
    case RolePick: drawRolePick(); break;
    case ServerHome: drawServer(); break;
    case PetView: drawPet(); break;
    case DiceView: drawDice(); break;
    case QuickView: drawQuick(); break;
    case PeopleView: drawTally(); break;
    case WardriveView: drawWardrive(); break;
    case ChannelAdd: drawChannelAdd(); break;
    case ChannelInfo: drawChannelInfo(); break;
    case ChatMenu: drawChatMenu(); break;
    case Remote: drawRemote(); break;
    case Folder: drawFolder(); break;
    case CheckersList:
    case SeaList: drawDuelList(); break;
    case DuelPick: drawDuelPick(); break;
    case CheckersBoard:
    case SeaBoard: drawDuelGame(); break;
    }
  statusBar();
  if(editing && !locked) drawEditing();
  drawToast();
  uint32_t drawn = micros();
  hardware.flush();
  uint32_t sent = micros();
  auto& p = frameStats[locked ? PerfSlots - 1 : page];
  p.frames++;
  p.drawUs += drawn - started;
  p.drawMax = max(p.drawMax, drawn - started);
  p.flushUs += sent - drawn;
  p.flushMax = max(p.flushMax, sent - drawn);
  p.rows += hardware.flushRows;
}
}
static bool realErase = false; // DEL tapped in the footer: a real delete, also on the T-Deck
bool uiRadarPage() { return page == Scope || page == Homing || page == Motion; }
bool uiScreenOff() { return wakeOnly; }
String uiPerf() { return perfReport(); }
void uiFarewell() {
  farewell = true;
  wakeOnly = locked = false;
  hardware.brightness(config.brightness);
  draw();
}
void uiBegin() {
  lastInput = millis();
  page = homePage();
  openRolePick(true);
  draw();
} // the role choice after every boot
String uiStatus() {
  StaticJsonDocument<1024> d;
  d["page"] = pageNames[page];
  d["role"] = roleName(config.role);
  if(page == RolePick) d["boot_pick"] = bootPick;
  if(page == Radio) d["radio_preset"] = settingValue(draft, RowPreset);
  d["locked"] = locked;
  d["selected"] = selected;
  if(page == PeopleView) {
    d["tally_current"] = people::tally.current;
    d["tally_new"] = tallyAdding();
    d["tally_value"] = people::value();
  }
  if(page == QuickView) {
    d["quick_targets"] = quickTargets;
    d["quick_to"] = meshRadio.idText(quickReplyTo ? quickReplyTo : quickSend::state.to);
    if(!quickTargets && selected > 0) d["quick_index"] = selected - 1;
  }
  d["recipient"] = recipient == meshmesh::Broadcast ? "ALL" : meshRadio.idText(recipient);
  d["composer"] = composer;
  d["composer_bytes"] = composer.length();
  d["editing"] = editing;
  d["chat_offset"] = chatOffset;
  d["idle_seconds"] = (millis() - lastInput) / 1000;
  if(page == Game) gameStatus(d);
  chessStatus(d);
  duelStatus(d);
  tourStatus(d);
  if(page == Folder) {
    d["folder"] = folderOpen;
    d["folder_name"] = appsFolderName(folderOpen);
  }
  if(appFolder >= 0) d["from_folder"] = appFolder;
  if(page == Node) d["play_menu"] = playMenu;
  if((page == Nodes || page == Node) && focusNode) d["selected_node"] = meshRadio.idText(focusNode);
  if(page == Node) {
    d["action"] = action;
    if(playMenu) d["play_selected"] = playSel;
  }
  if(page == ChannelInfo) d["channel"] = meshRadio.idText(focusChannel);
  if(page == Network) d["wifi_qr"] = wifiQr;
  if(page == ChatMenu) chatMenuStatus(d);
  if(page == ChannelAdd) d["add_step"] = int(addStep);
  d["channels"] = meshRadio.channelCount;
  if(page == Scope || page == Homing || page == Motion) {
    d["csi_role"] = radar.csi;
    d["radar_targets"] = radar.count;
    d["radar_selected"] = scopeSelected();
    d["radar_sound"] = radarSound;
  }
  if(page == DiceView) {
    const char* sheets[] = {"main", "saved", "characters", "templates", "send", "offer", "block"};
    d["dice_sheet"] = sheets[diceSheet];
    d["dice_selected"] = diceSel;
    d["sheet_block"] = sheetSel;
    d["sheet_note"] = sheetNote;
    d["block_field"] = blockField;
  }
  String s;
  serializeJson(d, s);
  return s;
}
void uiKey(int key) {
  if(powerOffPending()) return;
  bool asleep = wakeOnly;
  lastInput = millis();
  hardware.brightness(config.brightness);
  wakeOnly = false;
  dirty = true;
  keyed = true;
  if(locked) { // a hold (timed by the M9 keyboard controller, not by us) or two OK presses within a second unlock
    static uint32_t firstOk = 0;
    bool twice = key == Enter && firstOk && millis() - firstOk < 1000;
    firstOk = key == Enter && !twice ? millis() : 0;
    if(key == KeyHold || twice) {
      locked = false;
      int game = lockGame();
      if(game == 1) chessEnter(const_cast<ChessMatch*>(lockChess()));
      else if(game == 2) duelEnter(const_cast<Duel*>(lockDuel()));
    }
    return;
  }
  if(asleep) return;
  if(key == KeyMic || (key == KeyHold && !hardware.micKey && page != Chat)) {
    locked = true;
    return;
  } // M9 V1.1: no MIC key, the long OK press locks (stock firmware)
  if(key == KeyAt && page == Chat) {
    openChatMenu();
    return;
  }
  if(editing) {
    if(key == KeyBack) {
      editing = false;
      return;
    }
    if(key == Erase) {
      edit.remove(meshmesh::previousCharacter(edit.c_str(), edit.length()));
      return;
    }
    if(key == KeyRight && page != Network) {
      toggleLastCase(edit);
      return;
    }
    if(key == Enter) {
      if(page == ChannelAdd) {
        channelEditSave();
        return;
      }
      if(page == ChessTourNew) {
        tourNameSave();
        return;
      }
      if(page == DiceView) {
        diceEditSave();
        return;
      }
      if(page == PeopleView) {
        tallySaveName();
        return;
      }
      if(page == PetView) {
        if(creature.rename(edit)) {
          editing = false;
          notice(t("Name saved"), ok);
        } else notice(t("Name: 1-15 UTF-8 bytes"), bad);
        return;
      }
      if(page == ServerHome) {
        serverSaveEdit();
        return;
      }
      if(page == Remote) {
        remoteSaveEdit();
        return;
      }
      if(regionEditPage()) {
        channelRegionSave();
        return;
      }
      if(page == Radio) {
        String r = regions::normalize(edit);
        if(edit.length() && !r.length()) {
          notice(t("Region: letters, digits, '-'"), bad);
          return;
        }
        strlcpy(draft.region, r.c_str(), sizeof draft.region);
        editing = false;
        return;
      }
      if(page == NetList) {
        if(edit.length() < 8) {
          notice(t("Password: 8-63 characters"), bad);
          return;
        }
        if(internet.save(pendingSsid, edit)) {
          editing = false;
          edit = "";
          internet.connectTo(pendingSsid);
          notice(t("Connecting to ") + pendingSsid);
        } else notice(netError(), bad);
        return;
      }
      if(page == Network) {
        StaticJsonDocument<128> d;
        d["key"] = edit;
        String r = applySettings(d.as<JsonObjectConst>());
        if(r.startsWith("OK")) editing = false;
        notice(r);
      } else if(edit.length() && edit.length() <= 24 && meshmesh::validUtf8((const uint8_t*)edit.c_str(), edit.length())) {
        strlcpy(draft.name, edit.c_str(), sizeof draft.name);
        editing = false;
      } else notice(t("Name: 1-24 UTF-8 bytes"), bad);
      return;
    }
    if(key >= 32 && key < 127) {
      String ch = String(char(key));
      if(edit.length() + ch.length() <= (page == Network ? 64u : page == Radio || regionEditPage() ? regions::NameBytes
                                                               : page == NetList                   ? 63u
                                                               : page == ServerHome                ? serverEditLimit()
                                                               : page == Remote                    ? remoteEditLimit()
                                                               : page == ChannelAdd                ? channelEditLimit()
                                                               : page == ChessTourNew              ? 32u
                                                               : page == PetView                   ? 15u
                                                               : page == DiceView                  ? diceEditLimit()
                                                               : page == PeopleView                ? people::NameBytes
                                                                                                   : 24u)) edit += ch;
    }
    return;
  }
  if(page == Node && playMenu && playMenuKey(key)) return; // the games of "Play"; other keys (HOME, MSG...) leave the card as usual
  if(page == Node && key != Enter && key != KeyHold) deleteArmed = false;
  if(page == RolePick && rolePickKey(key)) return;
  if(key == KeyHome || key == KeyMsg || key == KeyMap || key == KeySet) appFolder = -1; // straight there, not back to a folder
  if(key == KeyHome) {
    change(homePage());
    return;
  }
  if(config.role != RoleNormal && (key == KeyMsg || key == KeyMap)) {
    notice(t("Not in this mode: Settings > Device mode"), warn);
    return;
  }
  if(key == KeyMsg) {
    change(Threads);
    return;
  }
  if(key == KeyMap) {
    change(Map);
    return;
  }
  if(key == KeySet) {
    change(Settings);
    return;
  }
  if(key == KeyAdv) {
    bool sent = meshRadio.sendHello();
    notice(sent ? t("Node announced") : t("Announcement failed"), sent ? ok : bad);
    return;
  }
  if(key == KeyGps) {
    gpsToggle();
    return;
  }
  if(page == Game && gameKey(key)) return; // the game handles BACK, arrows, OK and letters itself
  if(chessPage() && chessKey(key)) return;
  if(duelPage() && duelKey(key)) return;
  if(page == Folder && key == KeyBack) {
    change(Home, true);
    return;
  }
  if(page == ChatMenu && key == KeyBack && chatMenuBack()) return; // the folder's tile stays selected
  if(page == Network && networkKey(key)) return;
  if(page == PetView && petKey(key)) return;
  if(page == DiceView && diceKey(key)) return;
  if(page == QuickView && quickKey(key)) return;
  if(page == PeopleView && tallyKey(key)) return;
  if(page == ServerHome && (key == 'p' || key == 'P')) {
    change(PetView);
    return;
  } // the pet of a repeater or room
  if(page == ServerHome && serverKey(key)) return;
  if(page == Remote && remoteKey(key)) return;
  if(key == KeyBack && (page == Radio || page == Display) && draftChanged() && (!discardArmed || millis() - discardArmed > 5000)) { // unsaved: go to Save, a second BACK discards
    discardArmed = millis();
    selected = settingRows() - 1;
    notice(t("Unsaved: OK saves, BACK discards"), warn);
    return;
  }
  if(key == KeyBack) discardArmed = 0;
  if(key == KeyBack) {
    change(page == ChannelAdd ? Threads : page == ChannelInfo ? infoReturn
                                        : page == ChatMenu    ? Chat
                                        : page == NetList     ? Network
                                        : page == Homing      ? Scope
                                        : page == Chat        ? chatReturn
                                        : page == Node        ? Nodes
                                        : sharedPage(page)    ? subReturn
                                                              : homePage(),
           true);
    return;
  }
  if(key == KeyAt) {
    change(Diagnostics);
    return;
  }
  if(page == ChannelInfo) {
    channelInfoKey(key);
    return;
  }
  if(page == ChatMenu) {
    chatMenuKey(key);
    return;
  }
  if(page == Threads && threadsKey(key)) return;
  if(page == Chat) {
    chatKey(key);
    return;
  }
  if(page == NetList && key == Erase) {
    netListErase();
    return;
  }
  if(page == Map) {
    mapKey(key);
    return;
  }
  if((page == Nodes || page == Node) && nodesKey(key)) return;
  if((page == Home || page == Folder) && homeKey(key)) return;
  if(page == Sensors && sensorsKey(key)) return;
  if(page == WardriveView && wardriveKey(key)) return;
  if((page == Scope || page == Motion || page == Homing) && radarKey(key)) return;
  if(key == KeyUp || key == KeyDown) {
    if(page == Threads) threads();
    if(page == Nodes) sortNodes();
    int total = page == Threads ? conversationCount : page == ChannelAdd               ? addRows()
                                                    : page == Nodes                    ? nodeTotal
                                                    : page == Settings                 ? settingsCount
                                                    : page == Radio || page == Display ? settingRows()
                                                    : page == Network                  ? 4
                                                    : page == NetList                  ? 1 + int(netCount())
                                                    : page == Sensors                  ? 3
                                                    : page == Library                  ? int(library.size())
                                                                                       : 1;
    selected = total ? (selected + (key == KeyUp ? -1 : 1) + total) % total : 0;
    if(page == Nodes && nodeTotal) focusNode = meshRadio.peers[nodeOrder[selected]].id;
    return;
  }
  if((page == Radio || page == Display) && (key == KeyLeft || key == KeyRight)) {
    alter(key == KeyLeft ? -1 : 1);
    return;
  }
  if(key != Enter && key != KeyHold) return;
  if(page == Library) libraryEnter();
  else if(page == ChannelAdd) channelAddEnter();
  else if(page == Settings) settingsEnter();
  else if(page == Radio || page == Display) editorEnter();
  else if(page == Network) networkEnter();
  else if(page == NetList) netListEnter();
  else if(page == Diagnostics) diagnosticsEnter();
}
// Touch: the title bar goes back, footer hints are their keys, a tap on a list item or tile opens it
// (settings rows, the mode choice and challenge contacts: the first tap selects), the chess board
// takes a tap as the cursor and OK; swipes are the arrows and a hold is "hold OK", as on the ball.
void uiTouch(char gesture, int x, int y) {
  keyed = true;
  if(locked || wakeOnly) {
    uiKey(gesture == 'h' ? KeyHold : 0);
    return;
  } // a tap only wakes; a hold unlocks
  if(gesture == 'h') {
    uiKey(KeyHold);
    return;
  }
  if(page == PeopleView && !editing && (gesture == 'l' || gesture == 'r')) {
    uiKey(gesture == 'l' ? KeyRight : KeyLeft);
    return;
  } // cards follow the finger
  if(gesture != 't') {
    uiKey(gesture == 'u' ? KeyUp : gesture == 'd' ? KeyDown
                                 : gesture == 'l' ? KeyLeft
                                                  : KeyRight);
    return;
  }
  if(dirty) return; // the screen has not caught up with the last action: the tap was aimed at the old one
  if(y < 21) {
    uiKey(KeyBack);
    return;
  }
  if(y >= 221) {
    for(unsigned i = 0; i < hintCount; i++)
      if(x >= hintSpots[i].x0 && x < hintSpots[i].x1) {
        realErase = hintSpots[i].key == Erase;
        uiKey(hintSpots[i].key);
        return;
      }
    return;
  }
  if(page == DiceView && !editing && diceTap(x, y)) return;
  if((page == CheckersBoard || page == SeaBoard) && duelTap(x, y)) return;
  if(page == ChessBoard && chessOpen && chessOpen->state == ChessMatch::Playing && chessPromo < 0 && x >= BoardX && x < BoardX + Cell * 8 && y >= BoardY && y < BoardY + Cell * 8) {
    int f = (x - BoardX) / Cell, r = 7 - (y - BoardY) / Cell;
    if(flipped()) {
      f = 7 - f;
      r = 7 - r;
    }
    chessCursor = r * 8 + f;
    uiKey(Enter);
    return;
  }
  for(unsigned i = 0; i < targetCount; i++) {
    auto& v = targets[i];
    if(x < v.x || x >= v.x + v.w || y < v.y || y >= v.y + v.h) continue;
    if(v.index < 0) {
      uiKey(-v.index);
      return;
    }
    if(page == Node) {
      if(playMenu) playSel = v.index;
      else action = v.index;
      uiKey(Enter);
      return;
    }
    bool confirm = page == ChatMenu || page == Radio || page == Display || page == ServerHome || page == RolePick || page == ChessPick || page == DuelPick;
    if(confirm && selected != v.index) {
      selected = v.index;
      dirty = true;
      lastInput = millis();
      return;
    }
    selected = v.index;
    if(page == Nodes && nodeTotal) focusNode = meshRadio.peers[nodeOrder[selected]].id;
    uiKey(Enter);
    return;
  }
}
String eventLabel(const String& value) {
  if(value.startsWith("New message from ")) return tr("New message from ") + value.substring(17);
  if(value.startsWith("Delivered to ")) return tr("Delivered to ") + value.substring(13);
  if(value == "Queued: waiting for delivery") return tr("Queued: waiting for delivery");
  if(value == "Queued: broadcast") return tr("Queued: broadcast");
  if(value == "No delivery ACK") return tr("No delivery ACK");
  if(config.lang != LangEn && (value.startsWith("Radio TX error") || value.startsWith("TX failed"))) return tr("Radio TX error");
  return value;
}
void uiTick() {
  uint32_t now = millis();
  notifications::Event notification;
  while(notifications::take(notification)) {
    if(notification.kind == notifications::Incoming) {
      if(notifications::sound(notification.message)) hardware.beep();
      if(notifications::enabled(notifications::mode(notification.message, config.notifyWake, 1), notification.mention)) {
        wakeOnly = false;
        hardware.brightness(config.brightness);
        lastInput = now;
      }
      if(notifications::enabled(notifications::mode(notification.message, config.notifyPopup, 2), notification.mention) && page != DiceView && page != PeopleView) notice(t("New message from ") + notification.message.name, accent);
      dirty = true;
    } else if(config.notifyFailed && (notification.kind == notifications::TxError || notification.kind == notifications::Unconfirmed) && page != DiceView && page != PeopleView) {
      notice(notification.kind == notifications::TxError ? t("Radio TX error") : channels::isChannel(notification.message.destination) ? t("No repeat heard")
                                                                                                                                       : t("No delivery ACK"),
             warn);
    }
  }

  if(powerOffPending()) {
    static bool shown = false;
    if(!shown) {
      shown = true;
      wakeOnly = locked = false;
      hardware.brightness(config.brightness);
      draw();
    }
    return;
  } // drawn once: the board turns off
  if(bootPick && page == RolePick && now - bootPickAt >= bootPickMs) {
    bootPick = false;
    change(homePage());
  }
  if(page == Game) gameTick(now);
  // Rolls from the web page or the app tumble on the screen too.
  if(dicer.rolls != diceRolls) {
    diceRolls = dicer.rolls;
    if(page == DiceView) diceRollAt = sheetRollAt = now;
  }
  if(page == DiceView && dicer.events != diceSeen) {
    diceSeen = dicer.events;
    dirty = true;
  }
  if(dicer.notes != diceNotes) {
    diceNotes = dicer.notes;
    notice(dicer.note, dicer.out.failed ? warn : accent);
  } // a sheet sent or received
  chessTick();
  duelTick();
  tourTick();
  // Homing is used while walking without pressing keys: the screen stays on and unlocked.
  if((page == Homing || page == Motion) && !locked) lastInput = now;
  // Motion start beeps once (at most every 5 s).
  if(page == Motion && radar.moving && !wasMoving && now - motionBeepAt > 5000) {
    motionBeepAt = now;
    hardware.ping(1800, 120);
  }
  wasMoving = page == Motion && radar.moving;
  if(config.autoLock && now - lastInput >= config.autoLock * 1000UL && !locked) {
    locked = true;
    dirty = true;
  }
  if(config.dimAfter && now - lastInput >= config.dimAfter * 1000UL) {
    hardware.brightness(0);
    wakeOnly = true;
  }
  // Ping: faster and higher as the signal strengthens (-85..-30 dBm, steeper when close); LoRa pings
  // once per new packet. A lost target gets a quiet low tick every 2 s instead of a stale ping.
  if(page == Homing && !locked && radarSound && radar.tracking) {
    float x = constrain((radar.fast + 85) / 55.f, 0.f, 1.f);
    if(!radarFresh()) {
      if(radar.samples && now - pingAt >= 2000) {
        pingAt = now;
        hardware.ping(300, 30);
      }
    } else if(radar.focus.kind == RadarTarget::Lora) {
      if(radar.samples != pingedSamples) {
        pingedSamples = radar.samples;
        hardware.ping(600 + 1000 * x, 60);
      }
    } else if(now - pingAt >= uint32_t(1200 - 1140 * x * x)) {
      pingAt = now;
      hardware.ping(600 + 1000 * x, 40);
    }
  }
  if(meshRadio.event != eventSeen) {
    eventSeen = meshRadio.event;
    bool incoming = eventSeen.startsWith("New message from ");
    if(page == Chat && !incoming) {
      markRead();
      notice(eventLabel(eventSeen), eventSeen.startsWith("Delivered") ? ok : eventSeen.startsWith("No delivery") || eventSeen.startsWith("Radio TX") ? bad
                                                                                                                                                     : accent);
    }
  }
  if(toast.length() && now - toastAt >= 3500) {
    toast = "";
    dirty = true;
  }
  // The HUD clock, signal and battery change on every page.
  bool animate = !locked && ((page == Game && gameAnimating()) || page == Scope || page == Homing || page == Motion || page == PetView || (page == DiceView && now - diceRollAt < DiceSpin + 150)); // the sweep beam moves every frame
 // The network list refreshes itself while open; a scan briefly pauses traffic.
  if(page == NetList && !locked && internet.enabled && (!internet.scannedAt || now - internet.scannedAt > 20000) && now - netScanAt > 20000) {
    netScanAt = now;
    internet.rescan();
  }
  // A dark screen is not drawn: a key lights it and redraws (and light sleep stays short).
  bool update = dirty || meshRadio.dirty || maps.dirty || internet.dirty || animate || now - lastDraw >= 1000;// A key or touch draws on the next pass; the radar sweep runs at up to 30 fps (a frame costs ~32 ms on M9), other animations at 20 fps; radio, map and network updates at most every 150 ms.
  uint32_t gap = keyed && dirty ? 0 : animate ? (uiRadarPage() ? 33 : 50)
                                              : 150;
  if(update && !wakeOnly && now - lastDraw >= gap) {
    draw();
    lastDraw = now;
    dirty = false;
    keyed = false;
    meshRadio.dirty = false;
    maps.dirty = false;
    radar.dirty = false;
    internet.dirty = false;
  }
}
