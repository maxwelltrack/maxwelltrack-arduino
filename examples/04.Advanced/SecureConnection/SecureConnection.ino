/*
  MaxwellTrack - Secure Connection (ESP32 / ESP8266)

  By default the connection is encrypted but the device does not check who it
  is talking to. Giving it the cloud's root certificate makes it refuse any
  server that cannot prove it is the real MaxwellTrack broker.

    1. Get the root CA certificate of mqtt.maxwelltrack.com in PEM format
       (from MaxwellTrack support, or export it from your browser).
    2. Paste it into ROOT_CA below, including the BEGIN/END lines.

  Certificates are checked against the current date, so the clock is set
  from the internet before connecting.
*/

#include <maxwelltrack.h>
#include <time.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"
#define WIFI_SSID  "YOUR_WIFI_SSID"
#define WIFI_PASS  "YOUR_WIFI_PASSWORD"

static const char ROOT_CA[] = R"PEM(
-----BEGIN CERTIFICATE-----
PASTE THE ROOT CA CERTIFICATE HERE
-----END CERTIFICATE-----
)PEM";

unsigned long lastSend = 0;

void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  Serial.print("Setting clock");
  time_t now = time(nullptr);
  while (now < 1700000000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(" done");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);
  setClock();

  MaxwellTrack.setRootCA(ROOT_CA);
  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  MaxwellTrack.run();

  if (millis() - lastSend >= 10000) {
    lastSend = millis();
    MaxwellTrack.write("V0", millis() / 1000);
  }
}
