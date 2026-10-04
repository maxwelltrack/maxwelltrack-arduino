/*
  MaxwellTrack - ESP32 + SIM7600 (4G/LTE)

  Cellular with TLS handled by the modem: the sketch only powers the modem up
  and registers it on the network, then hands the serial port to
  MaxwellTrack.beginGSM(). GPS and SMS on the same modem keep working.

  Wiring: modem on Serial1 (RX 16, TX 17), PWRKEY on GPIO 4.
  Power: the SIM7600 draws ~2 A peaks; use a supply that can deliver it,
  with a large capacitor close to the modem.

  Needs the TinyGSM library (Tools > Manage Libraries > "TinyGSM").
*/

#define TINY_GSM_MODEM_SIM7600
#include <TinyGsmClient.h>
#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"

#define APN      "internet"
#define APN_USER ""
#define APN_PASS ""

#define MODEM_RX     16
#define MODEM_TX     17
#define MODEM_PWRKEY 4
#define SerialAT     Serial1

TinyGsm modem(SerialAT);
unsigned long lastSend = 0;

void powerOnModem() {
  pinMode(MODEM_PWRKEY, OUTPUT);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(100);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(3500);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(2000);
}

// init() talks to the already-powered modem without a reboot; restart() is the
// last resort because the power cycle draws the largest current surge.
bool startModem() {
  for (int attempt = 1; attempt <= 3; attempt++) {
    if (attempt < 3 ? modem.init() : modem.restart()) return true;
    Serial.println("Modem not responding, retrying...");
    delay(3000);
  }
  return false;
}

bool connectNetwork() {
  Serial.println("Waiting for network...");
  if (!modem.waitForNetwork(60000L)) return false;
  return modem.gprsConnect(APN, APN_USER, APN_PASS);
}

void onCommand(String pin, String value) {
  Serial.print(pin);
  Serial.print(" = ");
  Serial.println(value);
}

void setup() {
  Serial.begin(115200);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

  powerOnModem();
  if (!startModem()) Serial.println("Modem did not start - check power and wiring");

  while (!connectNetwork()) {
    Serial.println("No network yet, retrying in 10 s");
    delay(10000);
  }
  Serial.println("Network ready");

  MaxwellTrack.onWrite(onCommand);
  MaxwellTrack.beginGSM(SerialAT, DEVICE_ID, AUTH_TOKEN);
}

void loop() {
  MaxwellTrack.run();

  if (millis() - lastSend >= 15000) {
    lastSend = millis();
    MaxwellTrack.setSignal(modem.getSignalQuality());
    MaxwellTrack.write("V0", analogRead(34));
  }
}
