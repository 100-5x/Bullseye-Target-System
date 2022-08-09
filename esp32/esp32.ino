//#define __DEBUG__

#ifdef __DEBUG__
   #define print(...)   Serial.print(__VA_ARGS__)
   #define println(...) Serial.println(__VA_ARGS__)
#else
   #define print(...)
   #define println(...)
#endif

#include <SpeedyStepper.h>
#include <WebServer.h>
#include <WiFi.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#define dirPin 18 // DIR+
#define stepPin 19 // PUL+
#define motorInterfaceType 1

const char* ssid = "target.Wifi";

// Create a new instance of the Stepper class:
SpeedyStepper stepper;

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
  //stepper.setMaxSpeed(1000);
  stepper.connectToPins(stepPin, dirPin);

#ifdef __DEBUG__
  Serial.begin(115200);
#endif

  pinMode(activationPin, OUTPUT);
  digitalWrite(activationPin, LOW);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);

  pinMode(trimPot, INPUT);
  
  int analogValue = analogRead(trimPot);
  // Rescale to potentiometer's voltage (from 0V to 3.3V):
  int angle = map(analogValue, 0, 4096,40,100);
  //Serial.println(voltage);
  

  // Set up Wifi
#ifdef __DEBUG__
  Serial.print("Setting AP (Access Point)…");
#endif
  WiFi.mode(WIFI_AP);
  WiFi.softAP("target.Wifi");
  WiFi.softAP(ssid);
  IPAddress IP = WiFi.softAPIP();
  server.on("/edge", targetEdge);      //Which routine to handle at edge location
  server.on("/face", targetFace);      //Which routine to handle at face location
  server.on("/", handleRoot);      //Which routine to handle at root location
#ifdef __DEBUG__
  Serial.print("AP IP address: ");
  Serial.println(IP);
#endif
  server.begin();                  //Start server

#ifdef __DEBUG__
  Serial.println("HTTP server started");
#endif
}

void loop(){
  server.handleClient();          //Handle client requests
  stepper.setSpeedInStepsPerSecond(100);
  stepper.setAccelerationInStepsPerSecondPerSecond(100);

  
  delay(500);
}
void handleRoot() {
  server.send(200, "text/plain", "Target system requires http://192.168.4.1/face or http://192.168.4.1/edge");
}
void targetEdge() {
  server.send(200, "text/plain", "Targets Edged");
 
  digitalWrite(activationPin, HIGH);
  digitalWrite(ledPin, LOW);
#ifdef __DEBUG__
   Serial.println("Edged...");
  Serial.println(edgePos);
#endif
  stepper.moveToPositionInSteps(50);
 
}
void targetFace() {
  server.send(200, "text/plain", "Targets Faced");

  
  digitalWrite(activationPin, LOW);
  digitalWrite(ledPin, LOW);
#ifdef __DEBUG__
  Serial.println("Faced...");
  Serial.println(0);
#endif
  stepper.moveToPositionInSteps(0);
  
}
