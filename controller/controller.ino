#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char *ssid = "todo";
const char *password = "todotodo";

WebServer server(3000);

Servo servo;
int ledPin1 = 26;
int ledPin2 = 27;
int servopin = 13;

void setup() {
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  servo.attach(servopin);
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  servo.write(90);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println("IP: ");
  Serial.print(WiFi.localIP());

  server.on("/toggle-led1", HTTP_GET, []() {
      digitalWrite(ledPin1, !digitalRead(ledPin1));
      server.send(200, "text/plain", "LED Toggled");
  });

  server.on("/toggle-led2", HTTP_GET, []() {
      digitalWrite(ledPin2, !digitalRead(ledPin2));
      server.send(200, "text/plain", "LED Toggled");
  });

  server.on("/blink-led1", HTTP_GET, []() {
      digitalWrite(ledPin1, !digitalRead(ledPin1));
      server.send(200, "text/plain", "LED Toggled");
  });

  server.on("/unlock", HTTP_GET, [](){
    servo.write(0);
    Serial.println("Dropped the scroll");
    server.send(200, "text/plain", "Dropped the scroll");
  });

  server.on("/lock", HTTP_GET, [](){
    servo.write(90);
    Serial.println("Returned to initial position");
    server.send(200, "text/plain", "Returned to initial position");
  });

  // Add CORS headers for all requests
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

  server.begin();
}

void loop() {
    server.handleClient();
}