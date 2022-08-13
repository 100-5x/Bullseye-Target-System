

#include <WebServer.h>
#include <WiFi.h>
#include <SpeedyStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#define dirPin 18 // DIR+
#define stepPin 19 // PUL+
#define motorInterfaceType 1


const char* ssid = "target.Wifi";
WebServer server(80); //Server on port 80

// Create a new instance of the Stepper class:
SpeedyStepper stepper;

//String header;



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
  


  //Serial.begin(115200);

  pinMode(activationPin, OUTPUT);
  digitalWrite(activationPin, LOW);
  delay(500); // give power time to stabilize.
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);

  pinMode(trimPot, INPUT);
  
  //int analogValue = analogRead(trimPot);
  // Rescale to potentiometer's voltage (from 0V to 3.3V):
  //int angle = map(analogValue, 0, 4096,40,100);
  //Serial.println(voltage);
  

  Serial.print("Setting AP (Access Point)…");


  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAP("target.Wifi");
  delay(500); // give power time to stabilize.
  WiFi.softAP(ssid);
  delay(100);
  IPAddress IP = WiFi.softAPIP();
  server.on("/edge", targetEdge);      //Which routine to handle at edge location
  server.on("/face", targetFace);      //Which routine to handle at face location
  server.on("/", handleRoot);      //Which routine to handle at root location
  delay(500);
  server.begin();                  //Start server
  Serial.println("HTTP server started");
  delay(500);
  stepper.connectToPins(stepPin, dirPin);
}

void loop(){
  server.handleClient();          //Handle client requests  stepper.setSpeedInStepsPerSecond(200);
  stepper.setAccelerationInStepsPerSecondPerSecond(200);
  delay(100);
}
void handleRoot() {
  server.send(200, "text/plain", "Target system requires http://192.168.4.1/face or http://192.168.4.1/edge");
}


void targetEdge() {
  server.send(200, "text/plain", "Targets Edged");
  digitalWrite(activationPin, HIGH);
  digitalWrite(ledPin, LOW);
  Serial.println("Edged...");
  Serial.println(edgePos);
  stepper.moveToPositionInSteps(-200);
 
}
void targetFace() {
  server.send(200, "text/plain", "Targets Faced");
  digitalWrite(activationPin, LOW);
  digitalWrite(ledPin, LOW);
  Serial.println("Faced...");
  Serial.println(0);
  stepper.moveToPositionInSteps(0);
  
}
