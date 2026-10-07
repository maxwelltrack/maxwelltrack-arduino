<h1 align="center">MaxwellTrack for Arduino</h1>

<p align="center">
  Connect ESP32, ESP8266, STM32, Raspberry Pi Pico, SAMD and AVR boards to the
  <a href="https://maxwelltrack.com">MaxwellTrack IoT Cloud</a>, over WiFi or a
  cellular modem.
</p>

<p align="center">
  <img alt="Version" src="https://img.shields.io/badge/version-1.2.0-0a7cff">
  <img alt="Arduino IDE" src="https://img.shields.io/badge/Arduino%20IDE-1.8%20%7C%202.x-00979d?logo=arduino&logoColor=white">
  <img alt="PlatformIO" src="https://img.shields.io/badge/PlatformIO-supported-f5822a?logo=platformio&logoColor=white">
  <img alt="Boards" src="https://img.shields.io/badge/boards-ESP32%20%7C%20ESP8266%20%7C%20STM32%20%7C%20RP2040%20%7C%20SAMD%20%7C%20AVR-555">
  <img alt="Connectivity" src="https://img.shields.io/badge/connect-WiFi%20%7C%202G%20%7C%204G-555">
  <img alt="License" src="https://img.shields.io/badge/license-proprietary-lightgrey">
</p>

<p align="center">
  <a href="#installation">Installation</a> ·
  <a href="#quick-start">Quick start</a> ·
  <a href="#examples">Examples</a> ·
  <a href="#api-reference">API reference</a> ·
  <a href="#troubleshooting">Troubleshooting</a> ·
  <a href="CHANGELOG.md">Changelog</a>
</p>

---

You give the library a Device ID and an Auth Token from your dashboard. It
takes care of the secure connection, reconnects, online status, the offline buffer,
firmware updates and the message format, so your sketch only sends readings
and reacts to commands.

```cpp
#include <maxwelltrack.h>

void setup() {
  MaxwellTrack.begin("DEVICE_ID", "AUTH_TOKEN", "WiFi name", "WiFi password");
}

void loop() {
  MaxwellTrack.run();
  MaxwellTrack.write("V0", analogRead(A0));
  delay(5000);
}
```

## Contents

- [Features](#features)
- [Supported boards](#supported-boards)
- [Installation](#installation)
- [Quick start](#quick-start)
- [Ways to connect](#ways-to-connect)
- [Sending data](#sending-data)
- [Receiving commands](#receiving-commands)
- [Offline buffer](#offline-buffer)
- [Firmware updates (OTA)](#firmware-updates-ota)
- [Examples](#examples)
- [API reference](#api-reference)
- [Memory use](#memory-use)
- [Staying connected](#staying-connected)
- [Troubleshooting](#troubleshooting)
- [FAQ](#faq)
- [Security](#security)
- [Support](#support)
- [License](#license)

## Features

- **One library, many boards.** WiFi on ESP32 and ESP8266; any supported board
  over a SIM800 or SIM7600 modem.
- **Per-device security.** Each device logs in with its own Device ID and Auth
  Token over TLS. A leaked token affects one device and is revoked from the
  dashboard.
- **Never lose a reading.** Messages written while offline are buffered and
  sent in order after reconnecting. Every message carries an id, so the cloud
  drops duplicates.
- **Fleet friendly.** Reconnects back off from 2 s to 60 s with a random
  spread per device, so a thousand devices don't reconnect at the same
  moment after an outage.
- **Dashboard control.** One handler for all commands, or one per datastream.
- **Setup from a phone.** No WiFi password in the sketch: the device opens a
  setup network and the MaxwellTrack app sends it the credentials.
- **Firmware updates over the air** on ESP32 and ESP8266, over WiFi or 4G.
- **Small and predictable.** The offline buffer is a single block of memory,
  sized per board, so long offline periods do not fragment the heap.

## Supported boards

| Board family | Arduino core | WiFi | Cellular | OTA |
|---|---|:---:|:---:|:---:|
| ESP32, ESP32-S2, ESP32-S3, ESP32-C3 | esp32 2.x or 3.x | ✓ | ✓ | ✓ |
| ESP32-C6 | esp32 3.x | ✓ | ✓ | ✓ |
| ESP8266 (NodeMCU, Wemos D1 mini) | esp8266 3.x | ✓ | | ✓ |
| STM32 F1, F4, G0, H7 and others with the same CPU | STM32 core by STMicroelectronics | | ✓ | |
| Raspberry Pi Pico (RP2040), Pico 2 (RP2350) | Raspberry Pi Pico/RP2040 by Earle Philhower | | ✓ | |
| Arduino Zero, MKR boards | Arduino SAMD | | ✓ | |
| Arduino Mega 2560, Uno, Nano | Arduino AVR | | ✓ | |

On any other board the build stops with *"MaxwellTrack has no build for this
board"*. The Uno and Nano compile, but with 2 KB of RAM there is little left
for your own code; use a Mega for real projects.

**Modems:** SIM800C, SIM800L and other modems supported by
[TinyGSM](https://github.com/vshymanskyy/TinyGSM). The SIM7600 can also run
TLS on the modem itself (`beginGSM`), which needs no TinyGSM and saves memory.

## Installation

The library needs one helper library from the Library Manager,
[PubSubClient](https://github.com/knolleary/pubsubclient) (2.8 or newer). Cellular sketches also use
[TinyGSM](https://github.com/vshymanskyy/TinyGSM).

### Arduino IDE

1. Download `MaxwellTrack-<version>.zip` from the
   [releases page](https://github.com/maxwelltrack/maxwelltrack-arduino/releases).
2. **Sketch → Include Library → Add .ZIP Library…** and pick the ZIP.
3. **Tools → Manage Libraries…**, search for **PubSubClient** (by Nick O'Leary)
   and install it. For cellular, install **TinyGSM** too.
4. Open **File → Examples → MaxwellTrack** and start with *HelloCloud*.

### PlatformIO

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    https://github.com/maxwelltrack/maxwelltrack-arduino.git
    ; vshymanskyy/TinyGSM      ; for cellular sketches
```

Required libraries are installed automatically. Tested platforms:

| Platform | Boards |
|---|---|
| `espressif32` | esp32dev, esp32-s3-devkitc-1, and others |
| pioarduino (`https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip`) | ESP32 on Arduino core 3.x |
| `espressif8266` | nodemcuv2, d1_mini, and others |
| `ststm32` | bluepill_f103c8, blackpill_f411ce, and others |
| `atmelsam` | zeroUSB, MKR boards |
| `atmelavr` | megaatmega2560, uno |
| `https://github.com/maxgerhardt/platform-raspberrypi.git` | pico, with `board_build.core = earlephilhower` |

### arduino-cli

```sh
arduino-cli config set library.enable_unsafe_install true
arduino-cli lib install --zip-path MaxwellTrack-1.2.0.zip
arduino-cli lib install PubSubClient
```

## Quick start

1. In your MaxwellTrack dashboard, open the device, go to **Settings**, and
   copy the **Device ID** and **Auth Token**. The token is shown when the
   device is created; if you lost it, regenerate it from the same tab.
2. Paste them into the sketch below, with your WiFi name and password.
3. Upload, and open the Serial Monitor at 115200 baud. You should see
   `[MaxwellTrack] Connecting... connected`, and the device turns online in
   the dashboard.

```cpp
#include <maxwelltrack.h>

#define DEVICE_ID  "YOUR_DEVICE_ID"
#define AUTH_TOKEN "YOUR_AUTH_TOKEN"
#define WIFI_SSID  "YOUR_WIFI_SSID"
#define WIFI_PASS  "YOUR_WIFI_PASSWORD"

void setup() {
  Serial.begin(115200);
  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  MaxwellTrack.run();          // keep this in loop() at all times

  static unsigned long last = 0;
  if (millis() - last > 10000) {
    last = millis();
    MaxwellTrack.write("V0", 25.3);
  }
}
```

Avoid long `delay()` calls in `loop()`: `run()` needs to be called often to
keep the connection alive and to receive commands.

## Ways to connect

| You have | Use | Example |
|---|---|---|
| ESP32 / ESP8266, WiFi password in the sketch | `begin(id, token, ssid, pass)` | 01.Basics/HelloCloud |
| ESP32 / ESP8266, WiFi set from the phone app | `beginProvisioned(id, token)` | 02.WiFi/WiFiSetupFromPhone |
| ESP32 / ESP8266, your own WiFi code | `begin(id, token)` | 02.WiFi/MultiWiFiFailover |
| ESP32 + SIM7600 | `beginGSM(SerialAT, id, token)` | 03.Cellular/ESP32_SIM7600 |
| Any board + SIM800 / SIM7600 via TinyGSM | `MaxwellTrackClass MaxwellTrack(client);` | 03.Cellular/AnyBoardSIM800 |

Any other `Client` works the same way (Ethernet, a different modem library),
as long as it provides a secure (TLS) connection.

## Sending data

Readings go to **datastreams** (`V0`, `V1`, `V2`, …) that you map to widgets
in the dashboard.

```cpp
MaxwellTrack.write("V0", 25.3);    // float
MaxwellTrack.write("V1", 60);      // int
MaxwellTrack.write("V2", "OPEN");  // text
```

To send several values that belong together (one timestamp, aligned in charts
and CSV exports), send them in one message:

```cpp
MaxwellTrack.sendBatch("{\"V0\":25.3,\"V1\":60}");
```

Numbers are sent as JSON numbers and text as JSON strings, with quotes and
control characters escaped. Something that only looks like a number but isn't
valid JSON (`1.2.3`, `007`) is sent as text.

## Receiving commands

When someone moves a switch, button or slider in the dashboard, your handler
runs with the datastream and its new value:

```cpp
void onLed(String pin, String value) {
  digitalWrite(LED_BUILTIN, value == "1" ? HIGH : LOW);
}

void onAnyCommand(String pin, String value) {
  Serial.println(pin + " = " + value);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  MaxwellTrack.begin(DEVICE_ID, AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  MaxwellTrack.onWrite("V3", onLed);      // only V3
  MaxwellTrack.onWrite(onAnyCommand);     // everything else
}
```

A per-pin handler takes priority over the general one.

To react when the device goes online or offline:

```cpp
MaxwellTrack.onConnectionChange([](bool online) {
  digitalWrite(STATUS_LED, online ? HIGH : LOW);
});
```

## Offline buffer

While the device is offline, `write()` and `sendBatch()` keep messages in a
buffer and send them, oldest first, as soon as the connection is back. When
the buffer is full the oldest message is dropped to make room.

```cpp
MaxwellTrack.setQueueSize(8192);           // bytes, call before begin()
Serial.println(MaxwellTrack.queuedMessages());
Serial.println(MaxwellTrack.droppedMessages());
```

See *04.Advanced/OfflineBuffering*.

## Firmware updates (OTA)

ESP32 and ESP8266 devices update themselves when you push a new firmware from
the dashboard. Nothing is needed in the sketch beyond calling `run()`.

- WiFi boards download over HTTPS.
- ESP32 with a SIM7600 downloads over 4G (the modem's HTTP stack when
  available, otherwise a TLS stream).
- On ESP32 over 4G, when the update carries an MD5 the image is checked
  before it is accepted; on a mismatch the device keeps the old firmware.
- The download token is never printed to the serial log.

The version shown in the dashboard comes from `MAXWELLTRACK_FW_VERSION`.
Define it before the include:

```cpp
#define MAXWELLTRACK_FW_VERSION "1.4.0"
#include <maxwelltrack.h>
```

## Examples

| Folder | Example | Shows |
|---|---|---|
| 01.Basics | **HelloCloud** | The smallest working sketch |
| | **SendSensorData** | Sensor readings, numbers and text |
| | **ControlFromDashboard** | Switches and sliders driving pins, one handler per pin |
| 02.WiFi | **WiFiSetupFromPhone** | No password in the sketch; setup from the app, reset button |
| | **MultiWiFiFailover** | Several networks tried in order |
| 03.Cellular | **ESP32_SIM7600** | 4G with TLS on the modem (`beginGSM`) |
| | **AnyBoardSIM800** | SIM800 on STM32, Pico, Zero, Mega, Uno or ESP32 |
| | **SIM7600Diagnostics** | Step-by-step check when a SIM7600 will not connect |
| 04.Advanced | **OfflineBuffering** | Store-and-forward while offline, connection events |
| | **MultipleSensors** | Several readings in one message with one timestamp |
| | **SecureConnection** | Verifying the cloud's certificate |

Each example is compiled on the boards it targets before every release.

## API reference

All strings passed to the library (device ID, token, WiFi name and password,
certificate) are used in place, so pass string literals or globals, not
temporary `String` objects.

### Starting

| Call | |
|---|---|
| `begin(deviceId, authToken, ssid = "", pass = "")` | Start. With `ssid` the library keeps WiFi connected; without it your sketch manages the network. |
| `beginGSM(serial, deviceId, authToken)` | ESP32 + SIM7600: TLS runs on the modem, no TinyGSM client needed. |
| `beginProvisioned(deviceId, authToken)` | WiFi credentials come from the phone app instead of the sketch. |
| `MaxwellTrackClass MaxwellTrack(client);` | Any other network: declare your own instance with a connected `Client` (e.g. `TinyGsmClientSecure`). |
| `run()` | Call on every pass of `loop()`. Keeps the connection up, delivers commands, sends heartbeats. |

### Sending

| Call | Returns |
|---|---|
| `write(pin, value)` | `value` may be any integer type, `float`, `double`, `const char*` or `String`. |
| `virtualWrite(pin, value)` | Same as `write`. |
| `sendBatch(json)` | Several values in one message, e.g. `{"V0":25.1,"V1":60}`. |

All three return `false` only when the message is rejected: larger than the
packet size, or out of memory. Being offline is not an error; the message is
buffered.

### Receiving

| Call | |
|---|---|
| `onWrite(handler)` | `void handler(String pin, String value)` for every dashboard command. |
| `onWrite("V1", handler)` | Handler for one datastream; takes priority over the general one. |
| `onConnectionChange(handler)` | `void handler(bool online)` when the cloud connection goes up or down. |

### Status

| Call | |
|---|---|
| `connected()` | `true` while connected to the cloud. |
| `connectionState()` | `0` connected, `-1` disconnected, `-2` connect/TLS failed, `-3` lost, `-4` timeout, `5` bad token. |
| `queuedMessages()` | Messages waiting in the offline buffer. |
| `droppedMessages()` | Messages dropped because the buffer was full. |
| `isProvisioned()` | WiFi boards: credentials are stored from the phone app. |

### Configuration

Call these before `begin()` unless noted.

| Call | Default | |
|---|---|---|
| `setQueueSize(bytes)` | see [Memory use](#memory-use) | Offline buffer size. |
| `setMaxPayload(bytes)` | see [Memory use](#memory-use) | Largest message, 64 to 65535 bytes. |
| `setHeartbeatInterval(ms)` | `10000` | Keep below the cloud's 45 s offline timeout. |
| `setSignal(rssi)` | | Cellular signal reported in heartbeats; call any time. |
| `setRootCA(pem)` | off | Verify the cloud's certificate (WiFi boards). |
| `setResetPin(pin, ms)` | off | Holding the pin low for `ms` forgets WiFi and reopens setup. |
| `clearProvisioning()` | | Forget the WiFi credentials stored from the app. |
| `useTransport(client)` | | Switch an existing instance to another `Client`. |
| `setServer(host, port)` | MaxwellTrack cloud | Server override, for private MaxwellTrack deployments. |

## Memory use

Defaults are chosen per board so the library fits next to your code. Raise
them with `setQueueSize()` / `setMaxPayload()` if you have RAM to spare.

| Board | Offline buffer | Max packet | Per-pin handlers |
|---|---|---|---|
| ESP32 | 4096 B | 512 B | 16 |
| ESP8266, STM32, RP2040, SAMD | 2048 B | 512 B | 16 |
| Mega 2560 | 512 B | 256 B | 8 |
| Uno, Nano | 128 B | 192 B | 4 |

The buffer and the packet memory are allocated once at `begin()` and reused,
so sending does not grow or fragment the heap.

## Staying connected

`run()` keeps the device connected; call it on every pass of `loop()`. It:

- reconnects WiFi (when the library manages it) and the cloud connection on
  its own, waiting longer between attempts while the cloud is unreachable;
- keeps the device shown as online in the dashboard, with signal strength,
  uptime and firmware version;
- lets the dashboard mark the device offline when it loses power or
  connection;
- delivers dashboard commands and firmware updates, and sends buffered
  readings once the connection is back.

You never need reconnect code in your sketch.

## Troubleshooting

**`failed, rc=5`.** The cloud rejected the credentials. Check that the Device
ID and Auth Token match the dashboard exactly, with no extra spaces, and that
the device exists in your account.

**`failed, rc=-2`.** The TLS connection could not be opened. On WiFi, check
the internet connection. On cellular, check the APN, the signal and that the
SIM has data; run *SIM7600Diagnostics* for a step-by-step check.

**It never joins WiFi.** ESP boards only join 2.4 GHz networks. Check the
name and password, including upper and lower case.

**Connected, but nothing shows in the dashboard.** The datastreams you write
(`V0`, `V1`, …) must exist for that device in the dashboard.

**`Message too large, dropped`.** The message is bigger than the packet size.
Send fewer values per batch or raise `setMaxPayload()`.

**ESP8266 resets or runs out of memory.** TLS takes a large share of the
ESP8266's RAM. Avoid big buffers and building large `String`s in `loop()`.

**`MaxwellTrack has no build for this board`.** The board is not in the
[supported list](#supported-boards). Ask us if you need it.

## FAQ

**Why is there no `.cpp` source in the library?** It ships precompiled for
each supported processor. You get the header, the examples and the compiled
code; the Arduino IDE and PlatformIO link it automatically.

**Can I connect to my own server?** The library is made for the MaxwellTrack
cloud. `setServer()` exists for private MaxwellTrack deployments; contact us
if you need one.

**Does it work without the Arduino `loop()` running often?** Commands and
heartbeats are handled inside `run()`, so it has to be called regularly. Use
`millis()` timers instead of long `delay()`s.

**Can I use FreeRTOS tasks on the ESP32?** Yes, but call every MaxwellTrack
function from the same task.

## Security

- The device authenticates with its own Device ID and Auth Token, so a leaked
  token affects only that device and can be revoked from the dashboard.
- Traffic is encrypted with TLS. Call `setRootCA()` to also verify the
  cloud's certificate (see *04.Advanced/SecureConnection*).
- Security rests on the per-device token and encryption, never on keeping
  the server address secret.
- Never put account passwords or other devices' tokens in firmware.

Found a security problem? Email security@maxwelltrack.com rather than posting
it publicly.

## Support

- Documentation and dashboard: [maxwelltrack.com](https://maxwelltrack.com)
- Email: support@maxwelltrack.com
- Changes in each version: [CHANGELOG.md](CHANGELOG.md)

## License

Copyright © 2026 MaxwellTrack. All rights reserved. See [LICENSE](LICENSE).

The library is distributed in precompiled form for use with the MaxwellTrack
cloud. You may use it in your own devices and firmware, including commercial
products. You may not decompile, reverse-engineer or redistribute the library
itself.
