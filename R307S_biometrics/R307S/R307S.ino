#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

#define mySerial Serial2 // Use RX2 and TX2 pins of ESP32 board

bool fConfig = false;
uint8_t id;
uint8_t rBuff;

// wifi creds
const char *ssid = "Hello";
const char *password = "joshua 11";

// led display and servo init
Servo servo;
int ledPin1 = 18;
int ledPin2 = 19;
int servopin = 21;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t readnumber(void)
{
    uint8_t num = 0;
    while (num == 0)
    {
        while (!Serial.available())
            ;
        num = Serial.parseInt();
    }
    return num;
}

void setup()
{
    Serial.begin(115200);

    pinMode(ledPin1, OUTPUT);
    pinMode(ledPin2, OUTPUT);
    servo.attach(servopin);
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
    servo.write(0);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println("IP: ");
    Serial.print(WiFi.localIP());

    delay(100);
    Serial.println("\n\nAdafruit finger detect test");

    // Set the data rate for the sensor serial port
    finger.begin(57600);
    //delay(5);

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
}

void loop()
{ // run over and over again
    if (Serial.available())
    {
        rBuff = Serial.read();

        if (rBuff == 'C')
        {
            fConfig = true;
        }
        else if (rBuff == 'D')
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

        if (id == 130)
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

    //delay(50); // don't need to run this at full speed
}

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

uint8_t getFingerprintID()
{
    uint8_t p = finger.getImage();

    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
    case FINGERPRINT_NOFINGER:
       Serial.println("No finger detected");
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
        digitalWrite(ledPin1, LOW);
        digitalWrite(ledPin2, HIGH);
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!
    // turn on blue
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    servo.write(90);

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
