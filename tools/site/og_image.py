#!/usr/bin/env python3
"""The site's link preview image (og:image), 1200x630: the name and tagline beside the M9 home screen.

tools/site/og_image.py [SHOT] [OUT]   (defaults: site/img/m9-home-en.png -> site/img/og.png)
SHOT is a 640x480 M9 screenshot from tools/site/site_shots.py; regenerate the screenshots first when the
home screen changes. Needs Pillow and Helvetica Neue (macOS: /System/Library/Fonts/HelveticaNeue.ttc).
Link previews are cached by the sites that show them, so a new image can take a while to appear there.
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parents[2]
FONT = '/System/Library/Fonts/HelveticaNeue.ttc'  # faces: 0 regular, 1 bold
W, H = 1200, 630
PAGE, FRAME, FRAME_EDGE, GLOW = '#0a0e13', '#1a2128', '#3a444e', '#123330'
ACCENT, TEXT, MUTED = '#1fc2ae', '#e4e9ee', '#8b96a1'  # the site's teal and text colours


def font(size, face):
    return ImageFont.truetype(FONT, size, index=face)


def main():
    shot = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT/'site/img/m9-home-en.png'
    out = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT/'site/img/og.png'
    img = Image.new('RGB', (W, H), PAGE)
    glow = Image.new('RGB', (W, H), PAGE)  # a soft teal light behind the device
    ImageDraw.Draw(glow).ellipse((620, 60, 1260, 640), fill=GLOW)
    img = Image.blend(img, glow.filter(ImageFilter.GaussianBlur(160)), 0.9)
    d = ImageDraw.Draw(img)
    x = 70
    d.text((x, 92), 'LORA · MESHCORE · ESP32', font=font(24, 1), fill=ACCENT)
    d.text((x, 150), 'Murmur', font=font(98, 1), fill=TEXT)
    d.text((x, 278), 'Talk without the internet', font=font(32, 0), fill=TEXT)
    d.text((x, 320), 'or a cell network', font=font(32, 0), fill=TEXT)
    d.text((x, 395), 'Chats · maps · radar', font=font(30, 0), fill=MUTED)
    d.text((x, 522), 'Install from the browser', font=font(26, 1), fill=ACCENT)
    fx, fy, fw, fh = 590, 95, 600, 460  # the device frame, the screen inset by 20 px
    d.rounded_rectangle((fx, fy, fx + fw, fy + fh), radius=26, fill=FRAME, outline=FRAME_EDGE, width=3)
    screen = Image.open(shot).convert('RGB').resize((fw - 40, fh - 40), Image.LANCZOS)
    img.paste(screen, (fx + 20, fy + 20))
    img.save(out, optimize=True)
    print(out, f'{W}x{H}')


if __name__ == '__main__':
    main()
