/*
  MaxwellTrack - Cellular on any board

  For boards without WiFi (STM32, Raspberry Pi Pico, Arduino Zero/MKR,
  Arduino Mega/Uno) or any board using a SIM800C / SIM800L modem.
  Works on ESP32 too.

  Wiring: modem TX -> board RX, modem RX -> board TX, common GND. Serial1 is
  used where the board has it; STM32 uses pins PA3/PA2 and Uno/Nano pins 7/8.
  The modem needs its own supply that can deliver 2 A peaks.

  Needs the TinyGSM library (Tools > Manage Libraries > "TinyGSM").
*/

#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"

#define APN       "internet"
#define APN_USER  ""
#define APN_PASS  ""

#if defined(ARDUINO_ARCH_STM32)
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(PA3, PA2);  // modem TX -> PA3, modem RX -> PA2
#elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(7, 8);      // modem TX -> 7, modem RX -> 8
#else
#define SerialAT Serial1
#endif

TinyGsm modem(SerialAT);
TinyGsmClientSecure gsm_client(modem);
MaxwellTrackClass MaxwellTrack(gsm_client);

unsigned long lastSend = 0;

void onDashboardWrite(String pin, String value) {
  Serial.print("Command ");
  Serial.print(pin);
  Serial.print(" = ");
  Serial.println(value);
}

bool connectNetwork() {
  Serial.println("Waiting for network...");
  if (!modem.waitForNetwork(60000L)) return false;
  Serial.println("Connecting GPRS...");
  return modem.gprsConnect(APN, APN_USER, APN_PASS);
}

void setup() {
  Serial.begin(115200);
  SerialAT.begin(9600);
  delay(3000);

  Serial.println("Starting modem...");
  modem.restart();

  while (!connectNetwork()) {
    Serial.println("No network, retrying in 10 s");
    delay(10000);
  }

  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN);
  MaxwellTrack.onWrite(onDashboardWrite);
}

void loop() {
  if (!modem.isGprsConnected()) {
    connectNetwork();
  }

  MaxwellTrack.run();

  if (millis() - lastSend >= 10000) {
    lastSend = millis();
    MaxwellTrack.setSignal(modem.getSignalQuality());
    MaxwellTrack.write("V0", analogRead(A0));
  }
}
