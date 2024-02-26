#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

byte Led1_UID[4] = {0x03, 0xB2, 0x5C, 0x1F};
byte Led2_UID[4] = {0xF2, 0x1B, 0x46, 0x20};

const char *ssid = "Celeste_2.4";
const char *password = "7907003596";
const char* led1 = "http://192.168.1.8:3000/toggle-led1";
const char* led2 = "http://192.168.1.8:3000/toggle-led2";
const char* unauth = "http://192.168.1.8:3000/unauth";

void setup() {
  Serial.begin(115200);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println("IP: ");
  Serial.print(WiFi.localIP());

  SPI.begin();
  // Initialize RFID reader
  rfid.PCD_Init();

  Serial.println("\nTap RFID/NFC Tag on reader");
}

void sendHttpRequest(const char* url) {
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("HTTP request sent successfully");
  } else {
    Serial.println("Failed to send HTTP request");
    Serial.println(httpCode);
  }

  http.end();
}

int auth(byte card[]) {
  if(memcmp(rfid.uid.uidByte, card, sizeof(card)) == 0) {
    return 1;
  }
  else {
    return 0;
  }
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    if (auth(Led1_UID) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led1);
    }
    
    else if (auth(Led2_UID) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led2);
    }

    else {
      Serial.println("Unauthorized Tag with UID:");
      sendHttpRequest(unauth);

      for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }
      Serial.println();
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}
