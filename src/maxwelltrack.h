/*
  MaxwellTrack - IoT cloud client for Arduino (WiFi and GSM/LTE).
  Copyright (c) 2026 MaxwellTrack. All rights reserved.
*/

#ifndef MAXWELLTRACK_H
#define MAXWELLTRACK_H

#include <Arduino.h>

#if defined(ESP32)
  #define MT_HAS_WIFI 1
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPUpdate.h>
  #include <Update.h>
  #include <esp_task_wdt.h>
  #include <WebServer.h>
  #include <Preferences.h>
  #define MT_PROV_SERVER WebServer
#elif defined(ESP8266)
  #define MT_HAS_WIFI 1
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecure.h>
  #include <ESP8266httpUpdate.h>
  #include <ESP8266WebServer.h>
  #include <EEPROM.h>
  #define MT_PROV_SERVER ESP8266WebServer
#else
  #define MT_HAS_WIFI 0
#endif

#if defined(ESP32)
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR == 3
    #define MT_NS mt_esp32_v3
  #elif defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR == 2
    #define MT_NS mt_esp32_v2
  #else
    #error "MaxwellTrack needs ESP32 Arduino core 2.x or 3.x"
  #endif
#elif defined(ESP8266)
  #include <core_version.h>
  #if defined(ARDUINO_ESP8266_MAJOR) && ARDUINO_ESP8266_MAJOR == 3
    #define MT_NS mt_esp8266_v3
  #else
    #error "MaxwellTrack needs ESP8266 Arduino core 3.x"
  #endif
#elif defined(ARDUINO_ARCH_STM32)
  #define MT_NS mt_stm32
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)
  #define MT_NS mt_rp2040
#elif defined(ARDUINO_ARCH_SAMD)
  #define MT_NS mt_samd
#elif defined(ARDUINO_ARCH_AVR)
  #define MT_NS mt_avr
#else
  #error "MaxwellTrack has no build for this board. Supported: ESP32, ESP8266, STM32, RP2040, SAMD, AVR."
#endif

#include <PubSubClient.h>

#define MAXWELLTRACK_VERSION "1.1.0"

#ifndef MIOT_FW_VERSION
#define MIOT_FW_VERSION ""
#endif

// Called for dashboard -> device commands: pin is the datastream ("V1" or its
// name), value is the text the dashboard sent.
typedef void (*MaxwellWriteHandler)(String pin, String value);

// Called when the cloud connection goes up (true) or down (false).
typedef void (*MaxwellConnectionHandler)(bool connected);

namespace MT_NS {

class MaxwellTrackClass {
public:
#if MT_HAS_WIFI
  // WiFi: the library owns the TLS socket. ESP32/ESP8266 only.
  constexpr MaxwellTrackClass() {}
#endif

  // Cellular or any other network: pass a connected Client (e.g. TinyGsmClientSecure).
  constexpr explicit MaxwellTrackClass(Client& transport) : _transport(&transport) {}

  // ---- Start ---------------------------------------------------------------

  // deviceId and authToken come from the dashboard (Device -> Settings). Pass
  // ssid/pass to let the library manage WiFi, or leave them empty when the
  // sketch manages the network. All strings must stay valid while running.
  void begin(const char* deviceId, const char* authToken,
             const char* ssid = "", const char* pass = "") {
    beginImpl(deviceId, authToken, ssid, pass, MIOT_FW_VERSION, &Serial);
  }

  // ESP32 + SIM7600: pass the modem's serial port, already registered on the
  // network. The library handles TLS on the modem; no TinyGSM needed.
  void beginGSM(Stream& atSerial, const char* deviceId, const char* authToken) {
    beginGSMImpl(atSerial, deviceId, authToken, MIOT_FW_VERSION, &Serial);
  }

#if MT_HAS_WIFI
  // WiFi credentials come from the device, not the sketch. Without stored
  // credentials the device opens a "MIoT-Setup-XXXX" network for the phone app.
  void beginProvisioned(const char* deviceId, const char* authToken) {
    beginProvisionedImpl(deviceId, authToken, MIOT_FW_VERSION, &Serial);
  }

  bool isProvisioned();
  void clearProvisioning();

  // Hold this pin LOW for holdMs to forget WiFi and reopen setup.
  void setResetPin(int pin, unsigned long holdMs = 5000);

  // Verify the server certificate against this root CA (PEM). Without it the
  // connection is encrypted but the server is not authenticated. ESP8266 also
  // needs the clock set (configTime) before begin().
  void setRootCA(const char* pem);
#endif

  // Call from loop() as often as possible. Never blocks for long.
  void run();

  // ---- Send ----------------------------------------------------------------

  // Publish a datastream value. Numbers are sent as JSON numbers, anything
  // else as a string. Returns false only if the message was rejected (too
  // large or out of memory); while offline it is queued and sent later.
  bool write(const char* pin, const char* value);
  bool write(const char* pin, const String& value) { return write(pin, value.c_str()); }
  bool write(const char* pin, int value) { return write(pin, (long)value); }
  bool write(const char* pin, unsigned int value) { return write(pin, (unsigned long)value); }
  bool write(const char* pin, long value);
  bool write(const char* pin, unsigned long value);
  bool write(const char* pin, float value) { return write(pin, (double)value); }
  bool write(const char* pin, double value);

  bool write(const String& pin, const char* value) { return write(pin.c_str(), value); }
  bool write(const String& pin, const String& value) { return write(pin.c_str(), value.c_str()); }
  bool write(const String& pin, int value) { return write(pin.c_str(), (long)value); }
  bool write(const String& pin, unsigned int value) { return write(pin.c_str(), (unsigned long)value); }
  bool write(const String& pin, long value) { return write(pin.c_str(), value); }
  bool write(const String& pin, unsigned long value) { return write(pin.c_str(), value); }
  bool write(const String& pin, float value) { return write(pin.c_str(), (double)value); }
  bool write(const String& pin, double value) { return write(pin.c_str(), value); }

  // Older name for write(), kept so existing sketches compile.
  template <typename P, typename T>
  bool virtualWrite(const P& pin, T value) { return write(pin, value); }

  // Several values in one message (same server timestamp), e.g. {"V0":25.1,"V1":60}.
  bool sendBatch(const char* json);
  bool sendBatch(const String& json) { return sendBatch(json.c_str()); }

  // ---- Receive -------------------------------------------------------------

  // Handler for every dashboard command.
  void onWrite(MaxwellWriteHandler handler);

  // Handler for one pin; takes priority over the general handler. Pass
  // nullptr to remove. Returns false when the pin table is full.
  bool onWrite(const char* pin, MaxwellWriteHandler handler);

  void onConnectionChange(MaxwellConnectionHandler handler);

  // ---- Status --------------------------------------------------------------

  bool connected();

  // 0 connected, -1 disconnected, -2 connect/TLS failed, -3 lost,
  // -4 timeout, 1-5 refused (5 = bad token).
  int connectionState();
  int mqttState() { return connectionState(); }

  size_t queuedMessages();
  unsigned long droppedMessages();

  // ---- Advanced ------------------------------------------------------------

  // Use an external Client on this instance (call before begin()).
  void useTransport(Client& transport);

  // Server override, for private deployments.
  void setServer(const char* host, int port);

  // Cellular signal for heartbeats (WiFi boards report RSSI themselves).
  void setSignal(int rssi);

  // Heartbeat period; the cloud marks a device offline after ~45 s of silence.
  void setHeartbeatInterval(unsigned long ms);

  // Bytes kept for messages sent while offline (oldest dropped when full).
  void setQueueSize(size_t bytes);

  // Largest message the device sends or receives. Default 512 (256 on AVR).
  void setMaxPayload(size_t bytes);

private:
  struct Impl;
  Impl* _impl = nullptr;
  Client* _transport = nullptr;

  Impl& impl();
  void beginImpl(const char* deviceId, const char* authToken, const char* ssid,
                 const char* pass, const char* fw, Print* log);
  void beginGSMImpl(Stream& atSerial, const char* deviceId, const char* authToken,
                    const char* fw, Print* log);
#if MT_HAS_WIFI
  void beginProvisionedImpl(const char* deviceId, const char* authToken, const char* fw, Print* log);
#endif
};

}

using MT_NS::MaxwellTrackClass;

#if MT_HAS_WIFI
#define MT_STR2(x) #x
#define MT_STR(x) MT_STR2(x)
// Default instance for WiFi sketches. A sketch may define its own
// "MaxwellTrackClass MaxwellTrack(client);" instead, which replaces this one.
extern MaxwellTrackClass MaxwellTrack __asm__(MT_STR(MT_NS) "_MaxwellTrack");
#endif

#endif
