#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN 5
#define RST_PIN 27
#define SERVO_PIN 13
#define RED_PIN 33
#define BLUE_PIN 26

MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo;

byte authorizedUID[4] = {0x03, 0xB2, 0x5C, 0x1F};
int angle = 0;

const char* ssid = "todo";
const char* password = "todotodo";
const char* serverAddress = "http://192.168.175.99:3000/start";

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  SPI.begin();
  rfid.PCD_Init();
  servo.attach(SERVO_PIN);
  servo.write(angle);

  Serial.println("Tap RFID/NFC Tag on reader");
}

void sendHttpRequest() {
  HTTPClient http;
  http.begin(serverAddress);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("HTTP request sent successfully");
  } else {
    Serial.println("Failed to send HTTP request");
    Serial.println(httpCode);
  }

  http.end();
}

void loop() {
  if (rfid.PICC_IsNewCardPresent()) {
    if (rfid.PICC_ReadCardSerial()) {
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

      if (rfid.uid.uidByte[0] == authorizedUID[0] &&
          rfid.uid.uidByte[1] == authorizedUID[1] &&
          rfid.uid.uidByte[2] == authorizedUID[2] &&
          rfid.uid.uidByte[3] == authorizedUID[3] ) {
        Serial.println("Authorized Tag");
        digitalWrite(BLUE_PIN, HIGH);
        digitalWrite(RED_PIN, LOW);

        if (angle == 0){
          angle = 90;
          servo.write(angle);
          Serial.println("Latch released");
          delay(5000);
          sendHttpRequest();
        } else {
          angle = 0;
          servo.write(angle);
          Serial.println("Latch in place");
        }
      } else {
        Serial.println("Unauthorized Tag with UID:");
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(BLUE_PIN, LOW);
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();
      }

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }
}
