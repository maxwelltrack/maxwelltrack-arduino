# Changelog

## 1.3.0
- One library to install: no separate helper library is needed anymore.
- Available in the Arduino IDE Library Manager.
- Removed the old name of `connectionState()`.

## 1.2.0
- **Update required:** earlier versions can no longer connect to the
  MaxwellTrack IoT Cloud.
- The phone setup network is now named `MaxwellTrack-Setup-XXXX`.
- `MAXWELLTRACK_FW_VERSION` sets the firmware version shown in the dashboard.
- Improved relay control in the ControlFromDashboard example.

## 1.1.0

### New boards
- STM32 (F1, F4, G0, H7 and others with the same CPU), Raspberry Pi Pico and
  Pico 2, Arduino Zero / MKR, Arduino Mega and Uno, over a cellular modem.
- ESP32 Arduino core 2.x as well as 3.x.
- PlatformIO support.

### New features
- `onWrite("V1", handler)`: a handler per datastream pin.
- `onConnectionChange(handler)`: called when the cloud connection goes up or down.
- `setRootCA(pem)`: verify the server certificate (ESP32 / ESP8266).
- `connectionState()`: the connection status as a number, for diagnostics.
- `queuedMessages()`, `droppedMessages()`, `setQueueSize()`, `setMaxPayload()`,
  `setHeartbeatInterval()`.
- `write()` accepts every integer type and returns whether the message was accepted.

### Improvements
- Offline buffer is a fixed block of memory instead of separate strings, so
  long offline periods no longer fragment the heap. Messages are always sent
  in the order they were written.
- A message too large to send is rejected straight away instead of blocking
  every message queued behind it.
- Reconnects back off from 2 s to 60 s with a random spread per device, so a
  fleet does not overload the cloud after an outage.
- Text values are JSON-escaped, so quotes in a value can no longer break or
  alter the message. Values that look numeric but are not valid JSON numbers
  (`1.2.3`, `007`) are sent as text.
- Dashboard commands with spaces in the JSON or numeric values are understood.
- Log output goes to the same `Serial` the sketch uses (fixes missing logs on
  boards with native USB).
- OTA: the download token is no longer printed to the serial log. On ESP32
  over 4G, an MD5 sent with the update is verified before the new firmware is
  accepted.
- A sketch's own `MaxwellTrackClass MaxwellTrack(client);` now reliably
  replaces the default WiFi instance.

### Examples
- Reorganized into 01.Basics, 02.WiFi, 03.Cellular and 04.Advanced, with new
  sketches for dashboard control, phone setup, ESP32 + SIM7600, SIM800 on any
  board, offline buffering, multiple sensors and certificate checking.

## 1.0.0
- First release.
