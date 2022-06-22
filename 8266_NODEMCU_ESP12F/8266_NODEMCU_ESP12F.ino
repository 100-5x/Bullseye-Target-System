
#include <AccelStepper.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#define dirPin 5 // DIR+
#define stepPin 4 // PUL+
const int activationPin = 14;

#define motorInterfaceType 1

const char* ssid = "target.Wifi";

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

String header;
ESP8266WebServer server(80); //Server on port 80


//----------------
//---------------- SETUP
//----------------
void setup() {
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(700);
  Serial.begin(115200);
  pinMode(activationPin, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(activationPin, LOW);
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.mode(WIFI_AP);
  WiFi.softAP("target.Wifi");
  WiFi.softAP(ssid);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  server.on("/edge", targetEdge);      //Which routine to handle at edge location
  server.on("/face", targetFace);      //Which routine to handle at face location
  server.on("/", handleRoot);      //Which routine to handle at root location
  Serial.println(IP);
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}

void loop(){
  server.handleClient();          //Handle client requests
}
void handleRoot() {
  server.send(200, "text/plain", "Target system requires http://192.168.4.1/face or http://192.168.4.1/edge");
}
void targetEdge() {
  server.send(200, "text/plain", "Targets Edged");
  Serial.println("Edged...");
  
  digitalWrite(activationPin, HIGH);
  stepper.moveTo(0);
  stepper.runToPosition();
  digitalWrite(BUILTIN_LED, LOW);
}
void targetFace() {
  server.send(200, "text/plain", "Targets Faced");
  Serial.println("Faced...");
  
  stepper.moveTo(90);
  digitalWrite(activationPin, LOW);
  stepper.runToPosition();
  digitalWrite(BUILTIN_LED, HIGH);
  
}