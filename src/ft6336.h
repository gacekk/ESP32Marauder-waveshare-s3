#pragma once
#ifndef ft6336_h
#define ft6336_h

#ifdef HAS_CAP_TOUCH

#include <Wire.h>

// ---------------------------------------------------------------------------
// Touch controller abstraction for capacitive panels.
//
// Supported controllers (auto-detected at init time):
//   - Focaltech FT6336 / FT6336U  : I2C addr 0x38 (chip-id 0x64)
//   - Hynitron   CST816D / CST816S : I2C addr 0x15 (chip-id 0xB6)
//
// Both share the same register layout for touch data (TD_STATUS = 0x02,
// T1_XH = 0x03) so a single read path works for both.
// ---------------------------------------------------------------------------

// Logical "touch data registers" — same layout on FT6336 and CST816 family.
#define FT6336_TD_STATUS 0x02
#define FT6336_T1_XH     0x03

// I2C addresses of known supported controllers.
static const uint8_t FT6336_ADDR = 0x38;
static const uint8_t CST816_ADDR = 0x15;

// The address of whichever controller we detected, or 0 if none.
// Populated by ft6336_init(); used by the read path.
static uint8_t _touch_addr = 0;

static bool _touch_probe(uint8_t addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() != 0) return false;
    // Most capacitive controllers will ACK any single-byte read even if the
    // chosen register is not strictly valid — a successful ACK on the
    // address is enough to call the device present.
    Wire.requestFrom((int)addr, (int)1);
    return Wire.available() > 0 || true;  // probe by address ACK
}

static bool _ft6336_read(uint8_t reg, uint8_t *buf, uint8_t len) {
    if (_touch_addr == 0) return false;
    Wire.beginTransmission(_touch_addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    Wire.requestFrom((int)_touch_addr, (int)len);
    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.available() ? Wire.read() : 0;
    return true;
}

static void ft6336_init() {
    // Reset the controller if a reset pin is wired.
    #ifdef CTP_RST
    if (CTP_RST >= 0) {
        pinMode(CTP_RST, OUTPUT);
        digitalWrite(CTP_RST, LOW);
        delay(10);
        digitalWrite(CTP_RST, HIGH);
        delay(300);
    } else {
        delay(50);
    }
    #else
    delay(50);
    #endif

    Wire.begin(CTP_SDA, CTP_SCL, 400000U);

    // Auto-detect the controller. FT6336 first (legacy boards), then CST816.
    _touch_addr = 0;

    Wire.beginTransmission(FT6336_ADDR);
    if (Wire.endTransmission() == 0) {
        uint8_t chipId = 0;
        Wire.write(0xA3);
        if (Wire.endTransmission(false) == 0) {
            Wire.requestFrom((int)FT6336_ADDR, 1);
            if (Wire.available()) chipId = Wire.read();
        }
        if (chipId == 0x64) {
            _touch_addr = FT6336_ADDR;
            Serial.printf("[Touch] FT6336U detected at 0x%02X (chip id 0x%02X)\n",
                          _touch_addr, chipId);
        }
    }

    if (_touch_addr == 0) {
        Wire.beginTransmission(CST816_ADDR);
        if (Wire.endTransmission() == 0) {
            uint8_t chipId = 0;
            Wire.write(0xA7);   // CST816 family chip-id register
            if (Wire.endTransmission(false) == 0) {
                Wire.requestFrom((int)CST816_ADDR, 1);
                if (Wire.available()) chipId = Wire.read();
            }
            if (chipId == 0xB6) {
                _touch_addr = CST816_ADDR;
                Serial.printf("[Touch] CST816D detected at 0x%02X (chip id 0x%02X)\n",
                              _touch_addr, chipId);
            }
        }
    }

    if (_touch_addr == 0) {
        Serial.println(F("[Touch] ERROR: no FT6336 or CST816D found on I2C bus"));
        Serial.printf("             SDA=%d SCL=%d freq=%lu\n",
                      (int)CTP_SDA, (int)CTP_SCL, 400000UL);
        return;
    }

    // Chip-specific tuning.
    if (_touch_addr == FT6336_ADDR) {
        // Raise touch threshold to reduce phantom touches on FT6336 panels.
        // Register 0x80 = IDTHRESHOLD, default 22. Range 0..255; 40 is a
        // good balance for panel-in-case use.
        Wire.beginTransmission(FT6336_ADDR);
        Wire.write(0x80);
        Wire.write(40);
        Wire.endTransmission();
    } else if (_touch_addr == CST816_ADDR) {
        // CST816D defaults to "polling / point-only" mode after reset, which
        // is exactly what we want. Disable auto-sleep and gesture mode so
        // taps register even if the firmware is slow to poll.
        // Register 0xFE: auto-sleep time in seconds (0 = disabled).
        Wire.beginTransmission(CST816_ADDR);
        Wire.write(0xFE);
        Wire.write(0x00);
        Wire.endTransmission();

        // Register 0xA4: mode select (0 = point-only, no gestures).
        Wire.beginTransmission(CST816_ADDR);
        Wire.write(0xA4);
        Wire.write(0x00);
        Wire.endTransmission();

        // Register 0xD0: touch threshold (default 0x3F). Higher = less
        // sensitive. 0x2C is a reasonable starting point for a panel-in-case.
        Wire.beginTransmission(CST816_ADDR);
        Wire.write(0xD0);
        Wire.write(0x2C);
        Wire.endTransmission();
    }
}

// Reads raw panel coordinates in panel-native portrait space (0..W-1 × 0..H-1).
// Call this when you need to apply a rotation-specific transform.
static uint8_t ft6336_read_raw(uint16_t *raw_x, uint16_t *raw_y) {
    uint8_t data[7];
    if (!_ft6336_read(FT6336_TD_STATUS, data, 7)) return 0;
    if ((data[0] & 0x0F) == 0) return 0;
    *raw_x = ((uint16_t)(data[1] & 0x0F) << 8) | data[2];
    *raw_y = ((uint16_t)(data[3] & 0x0F) << 8) | data[4];
    return 1;
}

// Portrait-only convenience wrapper (rotation 0).
// For rotation-aware transforms, use ft6336_read_raw() via Display::updateTouch().
static uint8_t ft6336_update(uint16_t *x, uint16_t *y) {
    uint16_t raw_x, raw_y;
    if (!ft6336_read_raw(&raw_x, &raw_y)) return 0;
    *x = (raw_x < SCREEN_WIDTH)  ? raw_x : (uint16_t)(SCREEN_WIDTH  - 1);
    *y = (raw_y < SCREEN_HEIGHT) ? raw_y : (uint16_t)(SCREEN_HEIGHT - 1);
    return 1;
}

// Backwards-compat probe retained for any callers that referenced it.
static bool _ft6336_present() { return _touch_addr != 0; }

#endif // HAS_CAP_TOUCH
#endif // ft6336_h
