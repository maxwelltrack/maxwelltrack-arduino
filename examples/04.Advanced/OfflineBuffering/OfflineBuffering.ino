/*
  MaxwellTrack - Offline Buffering (ESP32 / ESP8266)

  Readings taken while the connection is down are kept on the device and sent
  in order once it is back, each with an id so the cloud never stores one
  twice. When the buffer fills up the oldest readings make room for new ones.

  This sketch sizes the buffer, shows the connection state on the LED and
  prints how many readings are waiting or were dropped.
*/

#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"
#define WIFI_SSID  "YOUR_WIFI_SSID"
#define WIFI_PASS  "YOUR_WIFI_PASSWORD"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

unsigned long lastSample = 0;
unsigned long lastReport = 0;

void onConnection(bool online) {
  digitalWrite(LED_BUILTIN, online ? HIGH : LOW);
  Serial.println(online ? "Cloud: online" : "Cloud: offline, buffering");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  MaxwellTrack.setQueueSize(8192);  // bytes; a reading takes ~30
  MaxwellTrack.onConnectionChange(onConnection);
  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  MaxwellTrack.run();

  if (millis() - lastSample >= 5000) {
    lastSample = millis();
    MaxwellTrack.write("V0", analogRead(A0));
  }

  if (millis() - lastReport >= 30000) {
    lastReport = millis();
    Serial.print("Waiting: ");
    Serial.print((unsigned long)MaxwellTrack.queuedMessages());
    Serial.print("  dropped: ");
    Serial.println(MaxwellTrack.droppedMessages());
  }
}
