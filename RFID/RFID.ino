#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// WiFi Credentials
const char* ssid = "KTCM_Building6_2.4G";
const char* password = "1234ktcm";

// Google Apps Script Webhook URL
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbxTUOlX22ccp0SdzpXS_8ysbp86t2Dv2fKzvXNzIbl-HLX9iJjVYpybkm3q_H4WKr00YQ/exec";

// RFID Pin Configuration
#define RST_PIN D3
#define SS_PIN D4

// Buzzer and LED Pins
#define BUZZER_PIN D1
#define LED_PIN D8
#define LED_D0_PIN D0

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(LED_D0_PIN, OUTPUT);     // Initialize LED D0
  digitalWrite(LED_D0_PIN, HIGH);  // LED D0 on initially

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    // Blink LED and sound buzzer in the pattern (on 0.5s, off 1s)
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);  // BUZZER and LED on for 0.5s
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);  // BUZZER and LED off for 1s
    Serial.println("Connecting...");
  }

  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  delay(200);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  delay(200);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  Serial.println("Connected to WiFi!");
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  // Check card RFID
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Read UID and convert to String
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println("RFID Tag Scanned: " + uid);

  // Indicate scanning status using LED D0
  digitalWrite(LED_D0_PIN, LOW);   // Turn off LED D0
  delay(200);                      // Wait for 200ms
  digitalWrite(LED_D0_PIN, HIGH);  // Turn LED D0 back on

  // Buzzer on
  digitalWrite(BUZZER_PIN, LOW);
  delay(200);
  digitalWrite(BUZZER_PIN, HIGH);

  // Check WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    // Prepare data POST
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
