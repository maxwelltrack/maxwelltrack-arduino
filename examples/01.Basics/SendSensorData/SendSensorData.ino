/*
  MaxwellTrack - Send Sensor Data (ESP32 / ESP8266)

  Reads a sensor every 10 seconds and sends it to the dashboard:
    V0  temperature (number)
    V1  status text ("normal" / "hot")

  Replace readTemperature() with your sensor (DHT22, DS18B20, BME280, ...).
  Datastreams can also be addressed by name, e.g. write("temperature", t).
*/

#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"
#define WIFI_SSID  "YOUR_WIFI_SSID"
#define WIFI_PASS  "YOUR_WIFI_PASSWORD"

const unsigned long SEND_EVERY_MS = 10000;
const float HOT_ABOVE = 30.0;

unsigned long lastSend = 0;

float readTemperature() {
  return random(200, 350) / 10.0;  // stand-in for a real sensor: 20.0 - 35.0
}

void setup() {
  Serial.begin(115200);
  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  MaxwellTrack.run();  // keep loop() free of long delay() calls

  if (millis() - lastSend >= SEND_EVERY_MS) {
    lastSend = millis();

    float t = readTemperature();
    MaxwellTrack.write("V0", t);
    MaxwellTrack.write("V1", t > HOT_ABOVE ? "hot" : "normal");

    Serial.print("Temperature: ");
    Serial.println(t);
  }
}
