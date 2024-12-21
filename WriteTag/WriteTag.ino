#if 0
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);
#else
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
#endif

void setup() {
    Serial.begin(9600);
    Serial.println("NDEF Writer");
    nfc.begin();
}

void loop() {
    Serial.println("\nPlace a formatted Mifare Classic or Ultralight NFC tag on the reader.");
    if (nfc.tagPresent()) {
        // vCard MIME Type Content
        String vcard = "BEGIN:VCARD\n"
                       "VERSION:3.0\n"
                       "FN:Febin\n"
                       "TEL:+918086157425\n"
                       "EMAIL:febintthomas@gmail.com\n"
                       "END:VCARD";

        // Create an NDEF MIME Type record
        NdefMessage message = NdefMessage();
        message.addMimeMediaRecord("text/vcard", (byte*)vcard.c_str(), vcard.length());

        // Write the vCard to the tag
        bool success = nfc.write(message);
        if (success) {
            Serial.println("Contact written successfully. Try reading this tag with your phone.");        
        } else {
            Serial.println("Write failed.");
        }
    }
    delay(5000);
}
