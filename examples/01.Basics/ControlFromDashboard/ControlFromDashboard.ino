/*
  MaxwellTrack - Control From Dashboard (ESP32 / ESP8266)

  Switches and sliders on the dashboard drive the board:
    V1  switch  -> on-board LED (0/1)
    V2  switch  -> relay on RELAY_PIN (0/1)
    V3  slider  -> LED brightness (0-255, PWM)

  Each pin gets its own handler. After switching the relay the device sends
  its real state back on V2, so the dashboard switch always shows what the
  hardware is doing, also after a reconnect.

  Most relay modules switch on when their input is LOW; set RELAY_ACTIVE_LOW
  to false for one that switches on when HIGH.
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
const bool RELAY_ACTIVE_LOW = true;
const int PWM_PIN = 4;

bool relayOn = false;

void setRelay(bool on) {
  relayOn = on;
  digitalWrite(RELAY_PIN, (on != RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void onLed(String pin, String value) {
  digitalWrite(LED_BUILTIN, value.toInt() ? HIGH : LOW);
}

void onRelay(String pin, String value) {
  setRelay(value.toInt() != 0);
  MaxwellTrack.write("V2", relayOn ? 1 : 0);
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

// After a reconnect, tell the dashboard the relay state again.
void onConnection(bool online) {
  if (online) MaxwellTrack.write("V2", relayOn ? 1 : 0);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  setRelay(false);   // relay off at power-up

  MaxwellTrack.onWrite("V1", onLed);
  MaxwellTrack.onWrite("V2", onRelay);
  MaxwellTrack.onWrite("V3", onBrightness);
  MaxwellTrack.onWrite(onOtherCommand);
  MaxwellTrack.onConnectionChange(onConnection);

  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  MaxwellTrack.run();
}
