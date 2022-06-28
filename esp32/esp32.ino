
#include <AccelStepper.h>
#include <WebServer.h>
#include <WiFi.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#define dirPin 18 // DIR+
#define stepPin 19 // PUL+
#define motorInterfaceType 1

const char* ssid = "target.Wifi";

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

String header;
WebServer server(80); //Server on port 80


// Auxiliar variables to store the current output state
//String activationPinState = "off";
const int trimPot = 34;
const int activationPin = 23;
const int ledPin = 2;

int edgePos = 90;

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

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);

  pinMode(trimPot, INPUT);
  
  

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
  
  int analogValue = analogRead(trimPot);
  // Rescale to potentiometer's voltage (from 0V to 3.3V):
  int angle = map(analogValue, 0, 4096,-20,20);
  //Serial.println(voltage);
  edgePos = (90 + angle);
  Serial.println(edgePos);
  delay(500);
}
void handleRoot() {
  server.send(200, "text/plain", "Target system requires http://192.168.4.1/face or http://192.168.4.1/edge");
}
void targetEdge() {
  server.send(200, "text/plain", "Targets Edged");
  Serial.println("Edged...");
  digitalWrite(activationPin, HIGH);
  digitalWrite(ledPin, HIGH);
  stepper.moveTo(edgePos);
  stepper.runToPosition();
}
void targetFace() {
  server.send(200, "text/plain", "Targets Faced");
  Serial.println("Faced...");
  
  digitalWrite(activationPin, LOW);
  digitalWrite(ledPin, LOW);
  stepper.moveTo(0);
  stepper.runToPosition();

  
}
