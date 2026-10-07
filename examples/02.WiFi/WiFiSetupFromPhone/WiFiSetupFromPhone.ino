/*
  MaxwellTrack - WiFi Setup From Phone (ESP32 / ESP8266)

  No WiFi password in the sketch: one firmware for every customer.

  First boot (or after a reset):
    1. The device opens a WiFi network called "MaxwellTrack-Setup-XXXX".
    2. The MaxwellTrack app joins it, lists nearby networks and sends the one
       you pick. The device tests it before saving, so a wrong password just
       lets you try again.
    3. From then on the device connects by itself, also after OTA updates.

  Hold the button on RESET_PIN for 5 seconds to forget the network and
  start setup again (wire a push button from the pin to GND).
*/

#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"

const int RESET_PIN = 0;  // the BOOT button on most ESP32 / ESP8266 boards

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);

  MaxwellTrack.setResetPin(RESET_PIN, 5000);

  if (!MaxwellTrack.isProvisioned()) {
    Serial.println("No WiFi saved yet - starting setup mode");
  }
  MaxwellTrack.beginProvisioned(DEVICE_ID, AUTH_TOKEN);
}

void loop() {
  MaxwellTrack.run();

  if (millis() - lastSend >= 30000) {
    lastSend = millis();
    MaxwellTrack.write("V0", (long)WiFi.RSSI());
  }
}
