//                            USER DEFINED SETTINGS
//   TFT_eSPI setup for Waveshare ESP32-S3 Touch LCD 2
//   https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2
//
//   Hardware:
//     - ESP32-S3R8 (Xtensa LX7 dual-core, 240 MHz, 8 MB PSRAM, 16 MB Flash)
//     - ST7789T3 240x320 IPS panel via SPI2
//     - CST816D capacitive touch via I2C (SDA=48, SCL=47, addr=0x15)
//     - Backlight on GPIO1 (PWM via LEDC)
//
//   Notes:
//     - Display reset is tied to board reset (TFT_RST = -1).
//     - Touch is I2C, not SPI resistive -> TOUCH_CS = -1.
//     - Touch controller handled by modified esp32_marauder/ft6336.h,
//       which auto-detects CST816D vs FT6336.

// ##################################################################################
//
// Section 1. Driver
//
// ##################################################################################

#define ST7789_DRIVER      // ST7789T3 is command-compatible with ST7789

// Native panel geometry (portrait orientation)
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ##################################################################################
//
// Section 2. Colour & inversion
//
// ##################################################################################

// Colour order: the panel datasheet shows RGB; on this module BGR renders correctly
// (matches LovyanGFX rgb_order=false in the reference test repo).
#define TFT_RGB_ORDER TFT_BGR

// Inversion ON: reference firmware sets invert=true; without this the colours
// look washed out / wrong polarity.
#define TFT_INVERSION_ON

// ##################################################################################
//
// Section 3. Pin assignments (ESP32-S3 GPIO numbers)
//
// ##################################################################################

// SPI bus — panel uses the ESP32-S3 SPI2 (FSPI) peripheral
#define TFT_MISO  -1    // Display is write-only; MISO not connected
#define TFT_MOSI  38
#define TFT_SCLK  39
#define TFT_CS    45
#define TFT_DC    42
#define TFT_RST   -1    // Reset is hard-wired to board reset

// Backlight (LEDC PWM channel is configured by esp32_marauder.ino via TFT_BL)
#define TFT_BL    1

// Capacitive touch (I2C) — no SPI touch chip select
#define TOUCH_CS  -1

// Touch I2C bus pins (consumed by esp32_marauder/ft6336.h via CTP_SDA / CTP_SCL)
#define CTP_SDA   48
#define CTP_SCL   47
#define CTP_RST   -1    // CST816D reset line is not routed to a GPIO on this module

// ##################################################################################
//
// Section 4. Fonts
//
// ##################################################################################

#define LOAD_GLCD    // Font 1
#define LOAD_FONT2   // Font 2
#define LOAD_FONT4   // Font 4
#define LOAD_GFXFF   // Adafruit GFX free fonts (FF1..FF48)
#define SMOOTH_FONT

// ##################################################################################
//
// Section 5. SPI clock
//
// ##################################################################################

// ST7789 reliably drives 40 MHz on ESP32-S3 with short fly-back wires.
#define SPI_FREQUENCY        40000000
#define SPI_READ_FREQUENCY   20000000
// SPI_TOUCH_FREQUENCY is not used (no SPI touch)
