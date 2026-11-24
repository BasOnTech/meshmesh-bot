#pragma once
#include <Adafruit_GFX.h>
// GFXcanvas16 whose fills run along rows. Adafruit_GFX::fillRect draws one vertical line per column, so every
// pixel steps a whole row ahead (640 bytes on the M9) through the PSRAM canvas, and fillScreen stores one pixel
// at a time. Panels and the clear were half of an M9 frame (uiperf); the pixels written are the same.
class FastCanvas16 : public GFXcanvas16 {
 public:
  using GFXcanvas16::GFXcanvas16;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
    if(getRotation() || !buffer) return GFXcanvas16::fillRect(x, y, w, h, color);
    if(w <= 0) return; // as the columns loop of Adafruit_GFX: nothing
    if(h < 0) { // as drawFastVLine: a negative height grows upwards
      h = -h;
      y -= h - 1;
    }
    const int x0 = x < 0 ? 0 : x, x1 = x + w > WIDTH ? WIDTH : x + w;
    const int y0 = y < 0 ? 0 : y, y1 = y + h > HEIGHT ? HEIGHT : y + h;
    for(int r = y0; r < y1; r++) fillRun(buffer + r * WIDTH + x0, x1 - x0, color);
  }
  void fillScreen(uint16_t color) override {
    if(buffer) fillRun(buffer, WIDTH * HEIGHT, color);
  }

 private:
  // Two pixels per 32-bit store once the run is word-aligned.
  static void fillRun(uint16_t* p, int n, uint16_t color) {
    if(n <= 0) return;
    if(uintptr_t(p) & 2) {
      *p++ = color;
      n--;
    }
    const uint32_t two = color | uint32_t(color) << 16;
    uint32_t* q = reinterpret_cast<uint32_t*>(p);
    for(int i = 0; i < n / 2; i++) q[i] = two;
    if(n & 1) p[n - 1] = color;
  }
};
