// Diagnostic sketch for Waveshare ESP32-S3 Touch LCD 2.
// Independent of Marauder — answers three questions:
//   1. Does USB-CDC Serial work at all?  (you should see "DIAG:" lines
//      in pio device monitor after flashing)
//   2. Does the ST7789T3 panel + backlight respond?  (panel should
//      show a white "DIAG OK" screen on a black background)
//   3. Does the firmware boot past setup()?  (loop() runs and you
//      see heartbeat "H @ Ns" lines on Serial every second)
//
// Flash:
//   cd diag
//   pio run -t upload --upload-port COM5
//   pio device monitor -p COM5 -b 115200
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  // Backlight on first (so we can see anything we draw even if
  // tft.init() takes a while)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  Serial.begin(115200);
  delay(2000);  // give USB-CDC time to enumerate on host

  Serial.println();
  Serial.println("=== DIAG: setup() entry ===");
  Serial.print  ("DIAG: __DATE__ "); Serial.println(__DATE__);
  Serial.print  ("DIAG: __TIME__ "); Serial.println(__TIME__);
  Serial.println("DIAG: if you see this, USB-CDC Serial is alive");

  // Init display + draw a screen
  Serial.println("DIAG: calling tft.init() ...");
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("DIAG OK");
  tft.setCursor(10, 40);
  tft.println("USB CDC:");
  tft.setCursor(10, 70);
  tft.println("  alive");
  tft.setCursor(10, 100);
  tft.println("Panel:");
  tft.setCursor(10, 130);
  tft.println("  alive");
  tft.setCursor(10, 170);
  tft.print("Build: ");
  tft.println(__DATE__);

  Serial.println("DIAG: tft.init() + draw done");
  Serial.println("DIAG: panel should now show 'DIAG OK'");
  Serial.println("=== DIAG: setup() done, entering loop() ===");
}

unsigned long lastHeartbeat = 0;
int beatCount = 0;

void loop() {
  if (millis() - lastHeartbeat > 1000) {
    lastHeartbeat = millis();
    Serial.print("DIAG: heartbeat ");
    Serial.print(beatCount++);
    Serial.print(" @ ");
    Serial.print(millis() / 1000);
    Serial.println("s");
  }
}
