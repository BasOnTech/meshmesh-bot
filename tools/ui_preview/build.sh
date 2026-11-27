#!/bin/sh
# Build and render the screen UI on the host: tools/ui_preview/build.sh m9|heltec OUTDIR [LANG]
# LANG is a code of src/i18n/I18n.h (en by default); a list with spaces ("en ru de") renders each into OUTDIR/<code>/.
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
LIB=$ROOT/.pio/libdeps/m9
BOARD=${1:-m9};OUT=${2:-$ROOT/artifacts/ui-preview};[ -n "$SIM" ]||mkdir -p "$OUT"
DEF="-DARDUINO=10819 -DMM_UI_PREVIEW=1 -DARDUINOJSON_ENABLE_PROGMEM=0 -DARDUINOJSON_ENABLE_ARDUINO_STREAM=0 -DARDUINOJSON_ENABLE_ARDUINO_PRINT=0 -DARDUINOJSON_ENABLE_STD_STRING=0 -DARDUINOJSON_ENABLE_STD_STREAM=0"
SRC=$ROOT/src/ui/Ui.cpp
if [ "$BOARD" = heltec ];then DEF="$DEF -DMM_HELTEC_V4=1 -DMM_COMPACT=1";SRC=$ROOT/src/ui/UiHeltec.cpp;fi
# SIM=1: the interactive simulator in an SDL2 window (tools/ui_preview/live.inc) instead of the screenshots; OUTDIR is unused.
LINK=
if [ -n "$SIM" ];then DEF="$DEF -DMM_UI_SIM=1 $(sdl2-config --cflags)";LINK=$(sdl2-config --libs);fi
INC="-I$ROOT/tools/ui_preview/shim -I$ROOT/src -I$ROOT/lib/MeshProtocol -I$LIB/ArduinoJson/src -I$LIB/Adafruit\ GFX\ Library -I$LIB/U8g2_for_Adafruit_GFX/src"
B=$(mktemp -d)
cc -c -O1 -w "$LIB/U8g2_for_Adafruit_GFX/src/u8g2_fonts.c" -o "$B/fonts.o"
eval c++ -std=gnu++17 -O1 -w $DEF $INC -c "\"$LIB/Adafruit GFX Library/Adafruit_GFX.cpp\"" -o "$B/gfx.o"
eval c++ -std=gnu++17 -O1 -w $DEF $INC -c "$LIB/U8g2_for_Adafruit_GFX/src/U8g2_for_Adafruit_GFX.cpp" -o "$B/u8g2.o"
eval c++ -std=gnu++17 -O1 -Wall -Wno-unused-function $DEF $INC -c "$SRC" -o "$B/ui.o"
eval c++ -std=gnu++17 -O1 -w $DEF $INC -c "$ROOT/tools/ui_preview/preview.cpp" -o "$B/preview.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/i18n/I18n.cpp" -o "$B/i18n.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/ui/UiIcons.cpp" -o "$B/icons.o"
eval c++ -std=gnu++17 -O1 -Wall -Wextra $DEF $INC -c "$ROOT/src/ui/Emoji.cpp" -o "$B/emoji.o"
eval c++ -std=gnu++17 -O1 -Wall -Wextra $DEF $INC -c "$ROOT/src/core/Apps.cpp" -o "$B/apps.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/apps/RadarModel.cpp" -o "$B/radar.o"
eval c++ -std=gnu++17 -O1 -Wall -Wextra $DEF $INC -c "$ROOT/src/games/solitaire/Solitaire.cpp" -o "$B/solitaire.o"
eval c++ -std=gnu++17 -O1 -Wall -Wextra $DEF $INC -c "$ROOT/src/games/chess/Chess.cpp" -o "$B/chess.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/games/chess/ChessNet.cpp" -o "$B/chessnet.o"
eval c++ -std=gnu++17 -O1 -Wall -Wextra $DEF $INC -c "$ROOT/src/games/duel/Checkers.cpp" -o "$B/checkers.o"
eval c++ -std=gnu++17 -O1 -Wall -Wextra $DEF $INC -c "$ROOT/src/games/duel/Sea.cpp" -o "$B/sea.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/games/duel/Duel.cpp" -o "$B/duel.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -I$LIB/Crypto -c "$ROOT/src/games/chess/ChessRating.cpp" -o "$B/chessrating.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/games/chess/ChessTour.cpp" -o "$B/chesstour.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/games/chess/ChessSync.cpp" -o "$B/chesssync.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/games/pet/Pet.cpp" -o "$B/pet.o"
eval c++ -std=gnu++17 -O1 -Wall -Wextra $DEF $INC -c "$ROOT/src/games/dice/Dice.cpp" -o "$B/dice.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -I$LIB/Crypto -c "$ROOT/src/mesh/Channels.cpp" -o "$B/channels.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -I$LIB/Crypto -c "$ROOT/src/mesh/Regions.cpp" -o "$B/regions.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/ui/Notifications.cpp" -o "$B/notifications.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/mesh/ChannelPolicy.cpp" -o "$B/policy.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/mesh/QuickSend.cpp" -o "$B/quick.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/apps/People.cpp" -o "$B/people.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/apps/Wardrive.cpp" -o "$B/wardrive.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/mesh/History.cpp" -o "$B/history.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/mesh/HistoryStorage.cpp" -o "$B/history-storage.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/mesh/HistoryReply.cpp" -o "$B/history-reply.o"
eval c++ -std=gnu++17 -O1 -Wall $DEF $INC -c "$ROOT/src/core/Clock.cpp" -o "$B/clock.o"
for f in SHA256 Hash Crypto;do eval c++ -std=gnu++17 -O1 -w $DEF $INC -c "$LIB/Crypto/$f.cpp" -o "$B/crypto-$f.o";done
c++ "$B"/*.o $LINK -o "$B/preview"
# KEEP=PATH: keep the program (tools/ui_preview/preview.cpp "--dice" serves the dice to the web page checks).
if [ -n "$KEEP" ];then rm -f "$KEEP";cp "$B/preview" "$KEEP";fi # a new file: macOS kills a binary overwritten in place
if [ -n "$SIM" ];then [ -n "$NORUN" ]||"$B/preview" --live ${3:-en};rm -rf "$B";exit 0;fi # NORUN=1: only build (with KEEP)
# Several languages ("en ru de"): one build, a folder per language.
case "${3:-en}" in
*" "*)for L in $3;do mkdir -p "$OUT/$L";"$B/preview" "$OUT/$L" $L >/dev/null;done;;
*)"$B/preview" "$OUT" ${3:-en};;
esac
rm -rf "$B"
