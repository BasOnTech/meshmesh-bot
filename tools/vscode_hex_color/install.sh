#!/bin/sh
# Package the 0xRRGGBB colour picker extension and install it: tools/vscode_hex_color/install.sh [code|code-insiders]
set -e
DIR=$(cd "$(dirname "$0")" && pwd)
CODE=${1:-code}
VSIX=${TMPDIR:-/tmp}/hex0x-color.vsix
rm -f "$VSIX"
(cd "$DIR" && zip -qr "$VSIX" '[Content_Types].xml' extension.vsixmanifest extension)
"$CODE" --install-extension "$VSIX"
