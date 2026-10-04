/*
  MaxwellTrack - Hello Cloud (ESP32 / ESP8266)

  The smallest working sketch: connects to WiFi and the MaxwellTrack cloud and
  sends the device uptime (seconds) to datastream V0 every 10 seconds.

    1. Dashboard -> your device -> Settings: copy the Device ID and Auth Token.
    2. Fill in the four values below and upload.
    3. Watch V0 count up on the dashboard.
*/

#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"
#define WIFI_SSID  "YOUR_WIFI_SSID"
#define WIFI_PASS  "YOUR_WIFI_PASSWORD"

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  MaxwellTrack.run();

  if (millis() - lastSend >= 10000) {
    lastSend = millis();
    MaxwellTrack.write("V0", millis() / 1000);
  }
}
