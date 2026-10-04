/*
  MaxwellTrack - Multi-WiFi Failover (ESP32)

  A resilient connection pattern for sites with more than one network:
    - Tries the first network in the list; if it is not found or does not
      connect within a timeout, moves on to the next.
    - Once WiFi is up, the library handles the secure MQTT connection.
    - Sends temperature + humidity to datastreams V0/V1 every 10 seconds.
    - If WiFi drops, restarts the failover from the first network.
    - If every network fails, restarts the ESP32 to try again from scratch.

  The sketch owns WiFi here (so it can fail over between networks), so begin()
  is called with EMPTY ssid/pass - the library then manages only MQTT. Call
  MaxwellTrack.run() once WiFi is connected.

  Setup:
    1. Dashboard -> your device -> Settings: copy the Device ID and Auth Token.
    2. Paste them below and list your 2.4 GHz networks (ESP32 has no 5 GHz).
*/

#include <maxwelltrack.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// --- From your MaxwellTrack dashboard (Device -> Settings) ---
#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"

// --- Your networks, tried in order (2.4 GHz only) ---
struct WiFiNetwork {
  const char* ssid;
  const char* password;
};

WiFiNetwork networks[] = {
  {"YOUR_PRIMARY_SSID",   "YOUR_PRIMARY_PASSWORD"},
  {"YOUR_SECONDARY_SSID", "YOUR_SECONDARY_PASSWORD"},
};
const int NETWORK_COUNT = sizeof(networks) / sizeof(networks[0]);

// How long to wait for one network before moving to the next.
static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
// How often to publish a reading once connected.
static const unsigned long SEND_INTERVAL_MS = 10000;

int currentNetwork = 0;
bool wifiConnected = false;
unsigned long wifiConnectStart = 0;

// Handle commands sent from the dashboard (button/switch on a datastream).
void onDashboardWrite(String pin, String value) {
  Serial.println("Dashboard command: " + pin + " = " + value);
  if (pin == "V3") {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, value == "1" ? HIGH : LOW);
  }
}

// Begin connecting to networks[currentNetwork]. Scans first so we can skip a
// network that is not in range and fail over immediately. Restarts the board if
// no configured network is found.
void connectToWiFi() {
  WiFi.mode(WIFI_STA);

  while (currentNetwork < NETWORK_COUNT) {
    Serial.printf("\nScanning for '%s'...\n", networks[currentNetwork].ssid);
    int found = WiFi.scanNetworks();
    bool inRange = false;
    for (int i = 0; i < found; i++) {
      if (WiFi.SSID(i) == networks[currentNetwork].ssid) { inRange = true; break; }
    }
    WiFi.scanDelete();

    if (inRange) {
      Serial.printf("Connecting to %s\n", networks[currentNetwork].ssid);
      WiFi.begin(networks[currentNetwork].ssid, networks[currentNetwork].password);
      wifiConnectStart = millis();
      return;  // wait for the result in loop()
    }

    Serial.printf("Network '%s' not in range, trying next.\n", networks[currentNetwork].ssid);
    currentNetwork++;
  }

  Serial.println("No configured network found. Restarting in 5s...");
  delay(5000);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== MaxwellTrack Multi-WiFi Failover ===");

  connectToWiFi();

  // Empty ssid/pass: the sketch manages WiFi, the library manages MQTT only.
  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, "", "");
  MaxwellTrack.onWrite(onDashboardWrite);
}

void loop() {
  // --- WiFi connection state machine (only while not yet connected) ---
  if (!wifiConnected) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.print("\nWiFi connected. IP: ");
      Serial.print(WiFi.localIP());
      Serial.printf("  RSSI: %d dBm\n", WiFi.RSSI());
    } else if (millis() - wifiConnectStart > WIFI_CONNECT_TIMEOUT_MS) {
      Serial.printf("\nTimed out on '%s'.\n", networks[currentNetwork].ssid);
      currentNetwork++;
      if (currentNetwork < NETWORK_COUNT) {
        connectToWiFi();
      } else {
        Serial.println("All networks failed. Restarting in 10s...");
        delay(10000);
        ESP.restart();
      }
    }
    delay(10);
    return;  // do not run MQTT until WiFi is up
  }

  // --- Connected: keep MQTT alive and publish readings ---
  MaxwellTrack.run();  // keep this in loop() while connected

  static unsigned long lastSend = 0;
  if (millis() - lastSend > SEND_INTERVAL_MS) {
    lastSend = millis();

    // Replace with real sensor readings.
    float temperature = random(200, 350) / 10.0;  // 20.0 - 35.0
    int humidity = random(30, 80);                 // 30 - 80

    // One batch -> both pins share the same server timestamp (no CSV gaps).
    String payload = "{\"V0\":" + String(temperature, 1) + ",\"V1\":" + String(humidity) + "}";
    MaxwellTrack.sendBatch(payload);
    Serial.printf("Sent: %s\n", payload.c_str());
  }

  // --- Detect a dropped link and fail over from the top ---
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi lost. Reconnecting...");
    wifiConnected = false;
    currentNetwork = 0;
    delay(1000);
    connectToWiFi();
  }

  delay(10);
}
