
#include <AccelStepper.h>
#include <WebServer.h>
#include <WiFi.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#define dirPin 2 // DIR+
#define stepPin 3 // PUL+
#define motorInterfaceType 1

const char* ssid = "target.Wifi";

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

String header;
WebServer server(80); //Server on port 80


// Auxiliar variables to store the current output state
//String activationPinState = "off";
const int activationPin = 12;
const int ledPin = 11;

//---------------------------------------//
//---------------- SETUP ----------------//
//---------------------------------------//

void setup() {
  // Accel Stepper Variables
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(700);
  Serial.begin(115200);

  pinMode(activationPin, OUTPUT);
  digitalWrite(activationPin, LOW);

  // Set up Wifi
  Serial.print("Setting AP (Access Point)…");
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
  digitalWrite(ledPin, HIGH);
  stepper.moveTo(0);
  stepper.runToPosition();
}
void targetFace() {
  server.send(200, "text/plain", "Targets Faced");
  Serial.println("Faced...");
  stepper.moveTo(90);
  digitalWrite(activationPin, LOW);
  stepper.runToPosition();
  
}
