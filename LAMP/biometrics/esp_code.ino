#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>

// Use RX2 and TX2 pins of ESP32 board
#define mySerial Serial2

// RFID pins
#define SS_PIN 5
#define RST_PIN 27

// fingerprint
bool fConfig = false;
uint8_t id;
uint8_t rBuff;

// leds
int ledPin1 = 33;
int ledPin2 = 26;

// wifi
const char *ssid = "todo";
const char *password = "todotodo";

// Webserver
WebServer server(3000);

//RFID
MFRC522 rfid(SS_PIN, RST_PIN);
byte authorizedUID[4] = {0x03, 0xB2, 0x5C, 0x1F};

// Fingerprint
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t readnumber(void)
{
    uint8_t num = 0;
    while (num == 0)
    {
        while (!Serial.available());
        num = Serial.parseInt();
    }
    return num;
}

void setup()
{
    pinMode(ledPin1, OUTPUT);
    pinMode(ledPin2, OUTPUT);
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);

    // rfid init
    SPI.begin();
    rfid.PCD_Init();

    Serial.begin(115200);

    // Establishing wifi connection
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
    Serial.println("IP: ");
    Serial.print(WiFi.localIP());

    // Set the data rate for the sensor serial port
    finger.begin(57600);
    delay(5);

    if (finger.verifyPassword())
    {
        Serial.println("Found fingerprint sensor!");
    }
    else
    {
        Serial.println("Did not find fingerprint sensor :(");
        while (1)
        {
            delay(1);
        }
    }

    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();

    Serial.print(F("Status: 0x"));
    Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x"));
    Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: "));
    Serial.println(finger.capacity);
    Serial.print(F("Security level: "));
    Serial.println(finger.security_level);
    Serial.print(F("Device address: "));
    Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: "));
    Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: "));
    Serial.println(finger.baud_rate);

    finger.getTemplateCount();

    if (finger.templateCount == 0)
    {
        Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    }
    else
    {
        Serial.println("Waiting for valid finger...");
        Serial.print("Sensor contains ");
        Serial.print(finger.templateCount);
        Serial.println(" templates");
    }

    // Listen for fetch requests
    server.on("/toggle-led1", HTTP_GET, []() {
        digitalWrite(ledPin1, !digitalRead(ledPin1));
        if (digitalRead(ledPin1) == HIGH){
          server.send(200, "text/plain", "hall room lights are turned on");
        }
        else{
          server.send(200, "text/plain", "hall room lights are turned off");
        }
    });

    server.on("/toggle-led2", HTTP_GET, []() {
        digitalWrite(ledPin2, !digitalRead(ledPin2));
        if (digitalRead(ledPin2) == HIGH){
          server.send(200, "text/plain", "Bedroom lights are turned on");
        }
        else{
          server.send(200, "text/plain", "Bedroom lights are turned off");
        }
    });

    // CORS policy fix
    server.onNotFound([](){
      if (server.method() == HTTP_OPTIONS) {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Max-Age", "10000");
        server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "*");
        server.send(204);
      } else {
        server.send(404, "text/plain", "Not Found");
      }
    });

    // start server
    server.begin();
}

void loop()
{
    //loop server handling
    server.handleClient();

    // Fingerprint Logic
    if (Serial.available())
    {
        rBuff = Serial.read();

        if (rBuff == 'Create')
        {
            fConfig = true;
        }
        else if (rBuff == 'Delete')
        {
            Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
            id = readnumber();
            deleteFingerprint(id);
        }
    }

    if (fConfig)
    {
        Serial.println("Ready to enroll a fingerprint!");
        Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
        id = readnumber();

        if (id == 0)
        { // exit enroll id mode
            Serial.print("exit enroll#");
            fConfig = false;
        }
        else
        {
            Serial.print("Enrolling ID #");
            Serial.println(id);
            while (!getFingerprintEnroll())
                ;
        }
    }
    else
    {
        getFingerprintID();
    }

    delay(50); // don't need to run this at full speed

    //RFID Logic
    if (rfid.PICC_IsNewCardPresent()) {
      if (rfid.PICC_ReadCardSerial()) {
        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

        if (rfid.uid.uidByte[0] == authorizedUID[0] &&
            rfid.uid.uidByte[1] == authorizedUID[1] &&
            rfid.uid.uidByte[2] == authorizedUID[2] &&
            rfid.uid.uidByte[3] == authorizedUID[3] ) {
          Serial.println("Authorized Tag");
          digitalWrite(ledPin1, !digitalRead(ledPin1));

        } else {
          Serial.println("Unauthorized Tag with UID:");
          digitalWrite(ledPin2, !digitalRead(ledPin2));
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

// Delete the fingerprint ID given
uint8_t deleteFingerprint(uint8_t id)
{
    uint8_t p = -1;
    p = finger.deleteModel(id);

    if (p == FINGERPRINT_OK)
    {
        Serial.println("Deleted!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not delete in that location");
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
    }
    else
    {
        Serial.print("Unknown error: 0x");
        Serial.println(p, HEX);
    }
    return p;
}

// Check if the fingerprint ID is present
// If present, turn on the led
uint8_t getFingerprintID()
{
    uint8_t p = finger.getImage();

    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
    case FINGERPRINT_NOFINGER:
        //Serial.println("No finger detected");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    p = finger.image2Tz();

    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    p = finger.fingerSearch();

    if (p == FINGERPRINT_OK)
    {
        Serial.println("Found a print match!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.println("Did not find a match");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    //code to turn on and off the led on a valid fingerprint read
    //thumb for red #1, pointer for blue #2
    if (finger.fingerID == 1){
      digitalWrite(ledPin1, !digitalRead(ledPin1));
    }
    else if (finger.fingerID == 2){
      digitalWrite(ledPin2, !digitalRead(ledPin2));
    }

    // found a match!
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(finger.confidence);
    return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez()
{
    uint8_t p = finger.getImage();

    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.image2Tz();

    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.fingerFastSearch();

    if (p != FINGERPRINT_OK)
        return -1;

    // found a match!
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(finger.confidence);
    return finger.fingerID;
}

uint8_t getFingerprintEnroll()
{
    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #");
    Serial.println(id);

    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    Serial.println("Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(id);
    p = -1;
    Serial.println("Place same finger again");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK converted!

    Serial.print("Creating model for #");
    Serial.println(id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        Serial.println("Fingerprints did not match");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID ");
    Serial.println(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Stored!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    return true;
}
