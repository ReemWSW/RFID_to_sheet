#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// WiFi Credentials
const char* ssid = "ssid";
const char* password = "password";

// Google Apps Script Webhook URL
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbxTUOlX22ccp0SdzpXS_8ysbp86t2Dv2fKzvXNzIbl-HLX9iJjVYpybkm3q_H4WKr00YQ/exec";

// RFID Pin Configuration
#define RST_PIN D3
#define SS_PIN D4

// Buzzer Pin
#define BUZZER_PIN D1

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, 1);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi!");
}

void loop() {
  // check card RFID
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // read UID convert to String
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println("RFID Tag Scanned: " + uid);

  // Buzzer on
  digitalWrite(BUZZER_PIN, 0);
  delay(200);                 
  digitalWrite(BUZZER_PIN, 1);


  // check WiFi Connecting
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    // prepare data POST
    String postData = "uid=" + uid;

    // POST data to Google Apps Script
    http.begin(client, googleScriptURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from server: " + response);
    } else {
      Serial.println("Error sending data. HTTP Response code: " + String(httpResponseCode));
    }
    http.end();
  }

  // Stop RFID
  rfid.PICC_HaltA();
}

