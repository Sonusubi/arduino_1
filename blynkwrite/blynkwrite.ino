#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <WiFi.h>

// Blynk Authentication Token
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3Z1TY9FNk"
#define BLYNK_TEMPLATE_NAME "NFC"
#define BLYNK_AUTH_TOKEN "I7bTNt3f2hbOfmuOlhzrOpcAks1EMAcj"
#include <BlynkSimpleEsp32.h>

// Variables for SSID and Password
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[50] = "Malabar 2";
char password[50] = "1234567890";

// NFC Setup
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc(pn532_i2c);

// Variable to hold the link from Blynk
String blynkLink = "https://default-link.com";

// Blynk Handlers for SSID and Password
BLYNK_WRITE(V2) {
  String newSSID = param.asString(); // Get the new SSID
  newSSID.toCharArray(ssid, sizeof(ssid));
  Serial.println("Updated SSID from Blynk: " + newSSID);
}

BLYNK_WRITE(V3) {
  String newPassword = param.asString(); // Get the new Password
  newPassword.toCharArray(password, sizeof(password));
  Serial.println("Updated Password from Blynk: " + newPassword);
}

BLYNK_WRITE(V0) {
  blynkLink = param.asString(); // Get the new NFC link
  Serial.println("Updated NFC Link from Blynk: " + blynkLink);
}

void setup() {
  Serial.begin(9600);
  Serial.println("NDEF Writer with Blynk and Dynamic Wi-Fi");

  // Start NFC
  nfc.begin();

  // Connect to Wi-Fi
  connectWiFi();

  // Initialize Blynk
  Blynk.config(auth); // Use config to avoid blocking if Wi-Fi fails initially
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(); // Reconnect if disconnected
  }
  Blynk.run();

  Serial.println("\nPlace a formatted Mifare Classic or Ultralight NFC tag on the reader.");
  if (nfc.tagPresent()) {
    NdefMessage message = NdefMessage();
    message.addUriRecord(blynkLink.c_str()); // Use the link from Blynk

    bool success = nfc.write(message);
    if (success) {
      Serial.println("Success. Try reading this tag with your phone.");
    } else {
      Serial.println("Write failed.");
    }
  }
  delay(5000);
}

void connectWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  // Wait for connection
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi. Check credentials.");
  }
}
