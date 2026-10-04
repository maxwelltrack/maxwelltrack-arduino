/*
  MaxwellTrack - Control From Dashboard (ESP32 / ESP8266)

  Switches and sliders on the dashboard drive the board:
    V1  switch  -> on-board LED (0/1)
    V2  switch  -> relay on RELAY_PIN (0/1)
    V3  slider  -> LED brightness (0-255, PWM)

  Each pin gets its own handler. The device reports the relay state back on
  V4, so the dashboard always shows what the hardware is really doing.
*/

#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"
#define WIFI_SSID  "YOUR_WIFI_SSID"
#define WIFI_PASS  "YOUR_WIFI_PASSWORD"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const int RELAY_PIN = 5;
const int PWM_PIN = 4;

void onLed(String pin, String value) {
  digitalWrite(LED_BUILTIN, value.toInt() ? HIGH : LOW);
}

void onRelay(String pin, String value) {
  bool on = value.toInt() != 0;
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);
  MaxwellTrack.write("V4", on ? "on" : "off");
}

void onBrightness(String pin, String value) {
  analogWrite(PWM_PIN, constrain(value.toInt(), 0, 255));
}

// Commands for pins without their own handler end up here.
void onOtherCommand(String pin, String value) {
  Serial.print("Command for ");
  Serial.print(pin);
  Serial.print(": ");
  Serial.println(value);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);

  MaxwellTrack.onWrite("V1", onLed);
  MaxwellTrack.onWrite("V2", onRelay);
  MaxwellTrack.onWrite("V3", onBrightness);
  MaxwellTrack.onWrite(onOtherCommand);

  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  MaxwellTrack.run();
}
