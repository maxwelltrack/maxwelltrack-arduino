/*
  MaxwellTrack - SIM7600 Diagnostics

  Checks, step by step, that a SIM7600 modem can reach the MaxwellTrack cloud
  over TLS, using the modem's own SSL commands. Run it when an ESP32 + SIM7600
  device does not connect, and read the verdict printed at the end:

    SUCCESS  "+CCHOPEN: 0,0"  the modem can open a secure connection to the
                              cloud; check the Device ID and Auth Token.
    FAILED   "+CCHOPEN: 0,<n>" with one of these codes:
               5  timeout            13 DNS lookup failed
               14 socket error       15 TLS handshake failed

  Wiring: ESP32 + SIM7600, modem on Serial1 (RX 16, TX 17), PWRKEY on GPIO 4.
  Set APN below for your SIM card.

  Needs the TinyGSM library (Tools > Manage Libraries > "TinyGSM").
*/

#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#include <TinyGsmClient.h>

#define SerialMon Serial
#define SerialAT  Serial1

#define MODEM_RX     16
#define MODEM_TX     17
#define MODEM_PWRKEY 4

const char APN[]  = "internet";                 // your carrier APN (TZ: internet)
const char HOST[] = "cloud.maxwelltrack.com";   // MaxwellTrack IoT Cloud
const int  PORT   = 443;                        // TLS port

TinyGsm modem(SerialAT);

// Final one-line verdict, printed repeatedly in loop() so it can't be missed.
String VERDICT = "(test still running...)";

// Send one raw AT line and stream the reply. Returns as soon as `stopToken` (or
// "ERROR") appears, otherwise after maxMs. stopToken lets us wait for the late URC
// that carries the real result (e.g. "+CCHOPEN:") without sitting the full window.
String atSend(const String& cmd, unsigned long maxMs, const char* stopToken = "OK") {
  while (SerialAT.available()) SerialAT.read();   // flush stale bytes
  SerialMon.print(">> "); SerialMon.println(cmd);
  SerialAT.print(cmd); SerialAT.print("\r\n");

  String resp;
  unsigned long deadline = millis() + maxMs;
  while (millis() < deadline) {
    while (SerialAT.available()) {
      char c = SerialAT.read();
      resp += c;
      SerialMon.write(c);                          // echo modem reply live
    }
    if (resp.indexOf(stopToken) != -1 || resp.indexOf("ERROR") != -1) break;
  }
  SerialMon.println();
  return resp;
}

void setup() {
  SerialMon.begin(115200);
  delay(3000);
  SerialMon.println("\n================ SIM7600 TLS raw-AT probe ================");

  // Power the modem on with its PWRKEY pulse.
  pinMode(MODEM_PWRKEY, OUTPUT);
  digitalWrite(MODEM_PWRKEY, LOW);  delay(100);
  digitalWrite(MODEM_PWRKEY, HIGH); delay(3500);
  digitalWrite(MODEM_PWRKEY, LOW);  delay(2000);

  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(2000);

  // Full restart (not init) so any NETOPEN/CCH state left by a previous sketch is
  // cleared - the ESP32 reset alone does NOT reboot the modem, and a stale CIP
  // (NETOPEN) session blocks AT+CCHSTART.
  SerialMon.println("[1] Modem restart (clean slate)...");
  if (!modem.restart()) {
    SerialMon.println("    restart() failed, trying init()...");
    modem.init();
  }

  SerialMon.println("[2] Waiting for network registration...");
  if (!modem.waitForNetwork(60000L)) {
    SerialMon.println("    NETWORK FAIL - check antenna/SIM. Aborting.");
    return;
  }
  SerialMon.print("    Network OK. Signal (CSQ): ");
  SerialMon.println(modem.getSignalQuality());   // 31 = best, 99 = none

  // ---- SIMCom SSL sequence (manual sections 1.2 + 3.2: no cert) ----------------
  SerialMon.println("\n[3] Configure PDP + SSL context (authmode 0 = no cert)...");
  atSend(String("AT+CGDCONT=1,\"IP\",\"") + APN + "\"", 2000);  // APN for the PDP
  atSend("AT+CSSLCFG=\"sslversion\",0,4", 2000);                // TLS, all versions
  atSend("AT+CSSLCFG=\"authmode\",0,0", 2000);                  // do not verify server

  SerialMon.println("\n[3b] Clear any leftover network/SSL state, then attach...");
  atSend("AT+CGACT?", 3000);          // show PDP state
  atSend("AT+NETOPEN?", 3000);        // is the CIP/NETOPEN stack already open?
  atSend("AT+CCHSTOP", 5000);         // stop SSL service if a prior run left it on (ignore ERROR)
  atSend("AT+NETCLOSE", 10000);       // close the CIP stack if open - frees it so CCH can start (ignore ERROR)
  atSend("AT+CGATT=1", 10000);        // ensure attached to the packet network

  SerialMon.println("\n[4] Start SSL service (AT+CCHSTART activates the PDP)...");
  String startResp = atSend("AT+CCHSTART", 30000, "+CCHSTART:");   // wait for the URC
  if (startResp.indexOf("+CCHSTART: 0") == -1) {
    SerialMon.println("    CCHSTART failed - retrying once after NETCLOSE...");
    atSend("AT+NETCLOSE", 10000);
    atSend("AT+CCHSTART", 30000, "+CCHSTART:");
  }

  SerialMon.println("\n[5] Bind SSL context 0 to session 0...");
  atSend("AT+CCHSSLCFG=0,0", 2000);

  SerialMon.println("\n[6] THE TEST - open TLS socket to the MaxwellTrack cloud...");
  String r = atSend(String("AT+CCHOPEN=0,\"") + HOST + "\"," + PORT + ",2", 40000, "+CCHOPEN:");

  if (r.indexOf("+CCHOPEN: 0,0") != -1) {
    VERDICT = "TLS RESULT: SUCCESS - modem opened TLS to the MaxwellTrack cloud. "
              "Fix is software; I will build the secure client (no firmware flash).";
  } else {
    // Pull the error number out of "+CCHOPEN: 0,<err>" for a precise verdict.
    int p = r.indexOf("+CCHOPEN: 0,");
    String err = (p != -1) ? r.substring(p + 12, r.indexOf('\n', p)) : "no response";
    err.trim();
    VERDICT = "TLS RESULT: FAILED - CCHOPEN err=" + err +
              "  (15=handshake/SNI, 13=DNS, 14=connect, 5=timeout, 19=no certs)";
  }

  // Cleanup so a re-run starts fresh.
  atSend("AT+CCHCLOSE=0", 5000);
  atSend("AT+CCHSTOP", 5000);
}

void loop() {
  // Keep printing the verdict so you only need to read this one line.
  SerialMon.println("\n========================================================");
  SerialMon.println(VERDICT);
  SerialMon.println("========================================================");
  delay(3000);
}
