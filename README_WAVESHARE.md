# ESP32Marauder on Waveshare ESP32-S3 Touch LCD 2

This is a port of [ESP32Marauder](https://github.com/justcallmekoko/ESP32Marauder)
to the [Waveshare ESP32-S3 Touch LCD 2](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2)
board.  The upstream Marauder repo had no build target for this hardware; this
fork adds one.

## Hardware summary

| Subsystem   | Component                                       |
|-------------|-------------------------------------------------|
| MCU         | ESP32-S3R8 (Xtensa LX7 dual-core, 240 MHz)      |
| Memory      | 512 KB SRAM + 8 MB OPI PSRAM + 16 MB flash     |
| Display     | 2.0" IPS, ST7789T3, 240x320, SPI on SPI2        |
| Touch       | CST816D capacitive, I2C at 0x15                 |
| IMU         | QMI8658 6-axis (unused by Marauder)             |
| Storage     | microSD / TF card slot (pinout not yet wired)   |
| Wireless    | 2.4 GHz Wi-Fi 802.11 b/g/n, Bluetooth 5 LE      |
| Backlight   | GPIO1 via LEDC PWM                              |

### Display SPI pins (SPI2 / FSPI)

| Signal | GPIO |
|--------|------|
| MOSI   | 38   |
| SCLK   | 39   |
| DC     | 42   |
| CS     | 45   |
| RST    | tied to board reset (use `-1`) |
| BL     | 1 (LEDC PWM, channel assigned by Marauder) |

### Touch I2C pins

| Signal | GPIO |
|--------|------|
| SDA    | 48   |
| SCL    | 47   |
| RST    | not routed (use `-1`) |

The touch controller auto-detection in `esp32_marauder/ft6336.h` probes
**0x38 (FT6336)** first and falls back to **0x15 (CST816D)**; either chip
will work without further changes.

## Files added / changed in this fork

- `User_Setup_waveshare_esp32_s3_touch_lcd_2.h` — TFT_eSPI configuration.
- `platformio.ini` — PlatformIO build target for this board.
- `esp32_marauder/ft6336.h` — auto-detects FT6336 *or* CST816D (was FT6336 only).
- `esp32_marauder/configs.h` — new `MARAUDER_WAVESHARE_TOUCH_LCD_2` board
  target (in board list, hardware name, features, display defines).
- `User_Setup_Select.h` — commented-out reference to the new setup file.
- `README_WAVESHARE.md` — this file.

## Building

### Option A — PlatformIO (recommended)

```bash
cd /path/to/this/fork
pio run -e waveshare_esp32_s3_touch_lcd_2 -t upload
```

You may need to set `upload_port` (in `platformio.ini`) to the actual device
path on your machine, e.g. `/dev/ttyACM0` on Linux or `COM7` on Windows.

The first build downloads:
- `espressif32` platform v6.5+
- `bodmer/TFT_eSPI` 2.5.43
- `lennarthennigs/ESPAsyncWebServer` 3.7+

### Option B — Arduino IDE

1. Install the **esp32** Arduino core v3.x (Board Manager URL
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`).
2. Install libraries: `TFT_eSPI` (Bodmer), `ESPAsyncWebServer` (Hennigs),
   `AsyncTCP`, `LinkedList`, `NimBLE-Arduino`.
3. From this fork, copy `User_Setup_waveshare_esp32_s3_touch_lcd_2.h` into
   your Arduino `libraries/TFT_eSPI/User_Setup.h` (replacing the default).
4. Open `esp32_marauder/esp32_marauder.ino`.
5. Select board **ESP32S3 Dev Module** with these settings:
   - USB CDC On Boot: **Enabled**
   - Flash Size: **16 MB**
   - PSRAM: **OPI PSRAM**
   - Upload Speed: **921600**
6. Edit `esp32_marauder/configs.h` and uncomment
   `#define MARAUDER_WAVESHARE_TOUCH_LCD_2`.
7. Upload.

## Flashing notes

- The Waveshare Touch LCD 2 board is finicky with the standard serial
  bootloader.  Use the **built-in USB-JTAG** upload path (`esp-builtin` in
  PlatformIO terms; the Arduino IDE selects it automatically when USB CDC is
  enabled and the right port is chosen).
- If the first upload fails with `Failed to connect to ESP32-S3: No serial
  data received`, hold **BOOT**, press/release **RESET**, release **BOOT**,
  then retry.
- After the first successful upload subsequent uploads usually just work.

## Known limitations (v1)

- **No SD card support yet.**  The board has a TF card slot but the SPI pin
  mapping has not been verified for this revision.  Marauder still runs fine
  without it — settings live in SPIFFS, only pcap / log captures need SD.
  To enable, uncomment `HAS_SD` / `USE_SD` in the new board's feature block
  and set `SD_CS` (plus `SD_MOSI`/`SD_MISO`/`SD_SCLK` if you wire a separate
  SPI bus).
- **No battery monitoring.**  The Li-ion header exists but no ADC pin is
  defined.  Re-enable by setting `HAS_BATTERY` and the appropriate ADC pin.
- **No physical nav buttons.**  Marauder's `c_btn.justPressed()` "stealth"
  branch is therefore disabled.  If you want it back, define `HAS_BUTTONS`
  and bind `C_BTN` to GPIO0 (the BOOT button).
- **No GPS / no Flipper LED / no RGB.**  These flags are intentionally left
  disabled in the new board's feature block.

## Testing checklist

After first flash, verify on the serial monitor (115 200 baud):

1. **Boot splash** appears centred on the 240x320 screen with the right
   colours (white text on black).
2. `[Touch] CST816D detected at 0x15 (chip id 0xB6)` appears in the log.
3. Touching the screen registers in `menu_function_obj.main()` — tap a menu
   entry and confirm the selection moves.
4. **Wi-Fi scan** (`Menu → WiFi → Scan APs`) returns a list of nearby
   networks and renders them with the scrolling text buffer.
5. **Deauth demo** (only against networks you own!) confirms the radio path
   is healthy.

If the display is washed-out / inverted, swap `TFT_INVERSION_ON` for
`TFT_INVERSION_OFF` in the User_Setup file and rebuild.

If the touch coordinates are mirrored, rotate the panel
(`SCREEN_ORIENTATION` in `configs.h`) — the existing rotation transform in
`Display::updateTouch()` will track the change.
