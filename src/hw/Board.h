#pragma once
// Board identity and features. Each PlatformIO target (platformio.ini) sets one board flag;
// MM_COMPACT selects the 128x64 one-button interface (UiHeltec.cpp), otherwise the
// 320x240 keyboard interface (Ui.cpp) is used. Pins are in BoardPins.h.
//   MM_BOARD_ID    status "board" for the web page, the app and the checks
//   MM_BOARD_NAME  shown on the device page and in the USB boot line
//   MM_NODE_NAME   default node name
//   MM_MAX_POWER   highest LoRa power the board's front end takes, dBm
//   MM_ABSENT      diagnostic modules the board does not have, each followed by a comma
//                  (web page indices: 1 keyboard, 2 SD, 4 RTC, 5 GPS, 6 compass, 7 IMU)
//   MM_BUTTON      label of the single button on one-button boards
#if defined(MM_HELTEC_V4)
#define MM_BOARD_ID "heltec_v4"
#define MM_BOARD_NAME "Heltec V4"
#define MM_NODE_NAME "Heltec V4"
#define MM_ABSENT 1, 2, 4, 6, 7,
#else
#define MM_BOARD_ID "m9"
#define MM_BOARD_NAME "ThinkNode M9"
#define MM_NODE_NAME "M9"
#define MM_GPS_DEFAULT true
#define MM_ABSENT
#endif
#ifndef MM_BUTTON
#define MM_BUTTON "PRG"
#endif
#ifndef MM_GPS_DEFAULT
#define MM_GPS_DEFAULT false
#endif
#ifndef MM_MAX_POWER
#define MM_MAX_POWER 22
#endif
// LoRa transceiver.
#if defined(MM_COMPACT)
#define MM_RADIO_SX1262 1
#else
#define MM_RADIO_LR1110 1
#endif
// One button (click, hold): the Heltec.
#if defined(MM_COMPACT)
#define MM_ONE_BUTTON 1
#endif
// Native USB (ESP32-S3 USB Serial/JTAG) instead of a USB-UART bridge.
#if defined(ARDUINO_USB_CDC_ON_BOOT) && ARDUINO_USB_CDC_ON_BOOT
#define MM_NATIVE_USB 1
#endif
