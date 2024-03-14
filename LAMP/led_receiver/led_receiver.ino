#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "todo";
const char *password = "todotodo";

WebServer server(3000);

int l1 = 14;
int l2 = 27;
int l3 = 26;
int l4 = 33;
int l5 = 32;
int l6 = 19;
int l7 = 21;

void setup() {
  pinMode(l1, OUTPUT);
  pinMode(l2, OUTPUT);
  pinMode(l3, OUTPUT);
  pinMode(l4, OUTPUT);
  pinMode(l5, OUTPUT);
  pinMode(l6, OUTPUT);
  pinMode(l7, OUTPUT);

  digitalWrite(l1, HIGH);
  digitalWrite(l2, HIGH);
  digitalWrite(l3, HIGH);
  digitalWrite(l4, HIGH);
  digitalWrite(l5, HIGH);
  digitalWrite(l6, HIGH);
  digitalWrite(l7, HIGH);
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println("IP: ");
  Serial.print(WiFi.localIP());

  server.on("/t1", HTTP_GET, []() {
      digitalWrite(l1, !digitalRead(l1));
      if (digitalRead(l1) == LOW){
        server.send(200, "text/plain", "on");
      }
      else{
        server.send(200, "text/plain", "off");
      }
  });

  server.on("/t2", HTTP_GET, []() {
      digitalWrite(l2, !digitalRead(l2));
      if (digitalRead(l2) == LOW){
        server.send(200, "text/plain", "on");
      }
      else{
        server.send(200, "text/plain", "off");
      }
  });

  server.on("/t3", HTTP_GET, []() {
      digitalWrite(l3, !digitalRead(l3));
      if (digitalRead(l3) == LOW){
        server.send(200, "text/plain", "on");
      }
      else{
        server.send(200, "text/plain", "off");
      }
  });

  server.on("/t4", HTTP_GET, []() {
      digitalWrite(l4, !digitalRead(l4));
      if (digitalRead(l4) == LOW){
        server.send(200, "text/plain", "on");
      }
      else{
        server.send(200, "text/plain", "off");
      }
  });

  server.on("/t5", HTTP_GET, []() {
      digitalWrite(l5, !digitalRead(l5));
      if (digitalRead(l5) == LOW){
        server.send(200, "text/plain", "on");
      }
      else{
        server.send(200, "text/plain", "off");
      }
  });

  server.on("/t6", HTTP_GET, []() {
      digitalWrite(l6, !digitalRead(l6));
      if (digitalRead(l6) == LOW){
        server.send(200, "text/plain", "on");
      }
      else{
        server.send(200, "text/plain", "off");
      }
  });

  server.on("/t7", HTTP_GET, []() {
      digitalWrite(l7, !digitalRead(l7));
      if (digitalRead(l7) == LOW){
        server.send(200, "text/plain", "on");
      }
      else{
        server.send(200, "text/plain", "off");
      }
  });

  server.on("/adminoff", HTTP_GET, []() {
      digitalWrite(l1, HIGH);
      digitalWrite(l2, HIGH);
      digitalWrite(l3, HIGH);
      digitalWrite(l4, HIGH);
      digitalWrite(l5, HIGH);
      digitalWrite(l6, HIGH);
      digitalWrite(l7, HIGH);
  });

  server.on("/adminon", HTTP_GET, []() {
      digitalWrite(l1, LOW);
      digitalWrite(l2, LOW);
      digitalWrite(l3, LOW);
      digitalWrite(l4, LOW);
      digitalWrite(l5, LOW);
      digitalWrite(l6, LOW);
      digitalWrite(l7, LOW);
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
