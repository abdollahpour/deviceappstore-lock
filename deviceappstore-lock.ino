#include <Arduino.h>
#include <TFT_eSPI.h>
#include "SPI.h"

#ifdef ESP32
  #include <SPIFFS.h>
#else
  #include <FS.h>
  #include <EEPROM.h>
#endif

TFT_eSPI tft = TFT_eSPI();
boolean lock = true;

void drawSpiffs(const char* name, int16_t x, int16_t y, uint16_t width, uint16_t height) {
  if (SPIFFS.exists(name)) {
    fs::File file = SPIFFS.open(name, "r");
    uint16_t buf[200];   // Pixel buffer (16 bits per pixel)
    // Set up a window the right size to stream pixels into
    tft.setWindow(x, y, x + width - 1, y + height - 1);
    // Work out the number whole buffers to send
    unsigned long fileSize = file.size();
    unsigned long pos = 0;

    while (pos < fileSize) {
      uint8_t long chunk = pos + 200 > fileSize ? fileSize - pos : 200;
      file.read((uint8_t *) buf, chunk);
      tft.pushColors(buf, chunk / 2);
      pos += chunk;
      yield();
    }
  
    // Restore it
    tft.setWindow(0, 0, TFT_WIDTH, TFT_HEIGHT);
  } else {
    Serial.print("Image did not find: ");
    Serial.print(name);
    Serial.println();
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  digitalWrite(4, HIGH); // LCD
  digitalWrite(5, lock ? HIGH : LOW);  // LOCK

  SPIFFS.begin();

  tft.begin();
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.fillScreen(TFT_WHITE);

  drawSpiffs("/lock.565", 50, 90, 140, 140);
}

void loop() {
  // put your main code here, to run repeatedly:
  #ifdef ESP32
    uint8_t touch = touchRead(T6);
    static long countTouch = 0;
    if (touch < 80) {
      countTouch ++;

      if (countTouch == 8) {
        lock = !lock; // Toggle
        digitalWrite(5, lock ? HIGH : LOW);
        drawSpiffs(lock ? "/lock.565" : "/unlock.565", 50, 90, 140, 140);
      }
    } else {
      countTouch = 0;
    }
  #endif
}
