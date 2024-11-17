#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// WiFi Credentials
const char* ssid = "CHAY";
const char* password = "027041216";

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
  // ตรวจสอบว่ามีการ์ดใหม่หรือไม่
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // อ่าน UID และแปลงเป็น String
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println("RFID Tag Scanned: " + uid);

  // เปิดเสียงบัซเซอร์เมื่อสแกนสำเร็จ
  digitalWrite(BUZZER_PIN, 0);
  delay(200);                  // รอจนเสียงจบ
  digitalWrite(BUZZER_PIN, 1);


  // เช็คการเชื่อมต่อ WiFi
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    // เตรียมข้อมูล POST
    String postData = "uid=" + uid;

    // ส่ง POST ไปยัง Google Apps Script
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

  // หยุดการอ่าน RFID ชั่วคราว
  rfid.PICC_HaltA();
}

