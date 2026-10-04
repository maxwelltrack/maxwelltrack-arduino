/*
  MaxwellTrack - Multiple Sensors (ESP32 / ESP8266)

  Sends several readings in one message, so they share one timestamp on the
  server and line up in charts and CSV exports. Each sensor keeps its own
  sampling period without blocking loop().
*/

#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"
#define WIFI_SSID  "YOUR_WIFI_SSID"
#define WIFI_PASS  "YOUR_WIFI_PASSWORD"

struct Sensor {
  const char* pin;
  unsigned long everyMs;
  unsigned long last;
  float value;
  float (*read)();
};

float readTemperature() { return random(200, 350) / 10.0; }
float readHumidity()    { return random(300, 800) / 10.0; }
float readPressure()    { return random(9900, 10300) / 10.0; }

Sensor sensors[] = {
  {"V0", 2000, 0, 0, readTemperature},
  {"V1", 5000, 0, 0, readHumidity},
  {"V2", 10000, 0, 0, readPressure},
};
const int SENSOR_COUNT = sizeof(sensors) / sizeof(sensors[0]);

unsigned long lastUpload = 0;

void setup() {
  Serial.begin(115200);
  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  MaxwellTrack.run();

  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (millis() - sensors[i].last >= sensors[i].everyMs) {
      sensors[i].last = millis();
      sensors[i].value = sensors[i].read();
    }
  }

  if (millis() - lastUpload >= 10000) {
    lastUpload = millis();

    char json[128];
    int n = snprintf(json, sizeof(json), "{");
    for (int i = 0; i < SENSOR_COUNT; i++) {
      n += snprintf(json + n, sizeof(json) - n, "%s\"%s\":%.1f", i ? "," : "",
                    sensors[i].pin, sensors[i].value);
    }
    snprintf(json + n, sizeof(json) - n, "}");

    MaxwellTrack.sendBatch(json);
    Serial.println(json);
  }
}
