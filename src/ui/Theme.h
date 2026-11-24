#pragma once
#include <stdint.h>
// A colour theme of the colour screen UI (Ui.cpp), as 0xRRGGBB. The fields are roles, not hues: each theme file
// in src/ui/themes/ names its own palette and assigns those colours to the roles. Register a theme in themes[]
// below; applyTheme() (Ui.cpp) converts the chosen one to the RGB565 colours the UI draws with.
// Game art (chess boards, cards, dice, pets, emoji) keeps its own colours.
struct Theme {
  const char* name;
  uint32_t surfacePage, surfaceBar, surfaceCard, surfaceFocus;    // surfaces: page, status and key bars, panels, focused panel
  uint32_t surfaceWell;                                           // inset on a surface: icon squares, input fields
  uint32_t border, gridLine;                                      // outlines and dividers; chart grid
  uint32_t textPrimary, textMuted, textSubtle;                    // text: main, secondary info, hints and disabled
  uint32_t iconPrimary, iconMuted, iconSubtle;                    // icons and arrow glyphs, same levels
  uint32_t graphicPrimary, graphicMuted, graphicSubtle;           // drawn shapes: outlines, knobs, dividers, chart strokes, same levels
  uint32_t onFill;                                                // text and glyphs on a coloured fill: badges, buttons, knobs, checkmarks
  uint32_t accent, accentDeep;                                    // focus and the brand colour; its dark fill behind accent icons
  uint32_t secondary, secondaryDeep;                              // the second category: private channels, repeaters, nodes, LoRa, tournaments
  uint32_t tertiary;                                              // a third: the pet, BLE devices
  uint32_t info, ok, okDeep, warn, warnDeep, alert, bad, badDeep; // state, from neutral to worst (alert: between warn and bad)
  uint32_t outBubble, inBubble;                                   // chat bubbles: sent, received
  uint32_t avatarHues[8];                                         // contact avatars, by key
};
#include "ui/themes/Midnight.h"
// The themes to choose from; the first is the default.
constexpr const Theme* themes[] = {&themeMidnight};
constexpr unsigned themeCount = sizeof(themes) / sizeof(themes[0]);
