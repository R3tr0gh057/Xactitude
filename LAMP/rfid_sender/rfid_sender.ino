#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

byte t1[4] = {0xE3, 0x91, 0xC2, 0x12};
byte t2[4] = {0x33, 0x3A, 0x00, 0xED};
byte t3[4] = {0x23, 0x88, 0x65, 0xFC};
byte t4[4] = {0xD3, 0x1B, 0x2D, 0x11};
byte t5[4] = {0x23, 0x14, 0xB0, 0x12};
byte t6[4] = {0x43, 0x36, 0x3A, 0x11};
byte t7[4] = {0xA3, 0xD5, 0x5E, 0x10};

byte adminon[4] = {0xC3, 0xD2, 0x83, 0xFC};
byte adminoff[4] = {0x63, 0x58, 0x2E, 0x11};

const char* led1 = "http://192.168.76.10:3000/t1";
const char* led2 = "http://192.168.76.10:3000/t2";
const char* led3 = "http://192.168.76.10:3000/t3";
const char* led4 = "http://192.168.76.10:3000/t4";
const char* led5 = "http://192.168.76.10:3000/t5";
const char* led6 = "http://192.168.76.10:3000/t6";
const char* led7 = "http://192.168.76.10:3000/t7";

const char* bypasson = "http://192.168.76.10:3000/adminon";
const char* bypassoff = "http://192.168.76.10:3000/adminoff";

const char *ssid = "todo";
const char *password = "todotodo";

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

    // logic for the first bulb
    if (auth(t1) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led1);
    }
    
    // logic for the second bulb
    else if (auth(t2) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led2);
    }

    // logic for the third bulb
    else if (auth(t3) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led3);
    }

    // logic for the fourth bulb
    else if (auth(t4) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led4);
    }

    // logic for the fifth bulb
    else if (auth(t5) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led5);
    }

    // logic for the sixth bulb
    else if (auth(t6) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led6);
    }

    // logic for the seventh bulb
    else if (auth(t7) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(led7);
    }

    // logic for the bypass on
    else if (auth(adminon) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(bypasson);
    }

    // logic for the bypass off
    else if (auth(adminoff) == 1) {
      Serial.println("Authorized Tag");
      sendHttpRequest(bypassoff);
    }

    // unauthorized tags
    else {
      Serial.println("Unauthorized Tag with UID:");

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
