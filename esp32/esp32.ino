//#define __DEBUG__


/*
 * 
 * Except as written in this agreement, Developer’s work product is provided ​“AS IS”.”
 * 
 */

#ifdef __DEBUG__
   #define print(...)   Serial.print(__VA_ARGS__)
   #define println(...) Serial.println(__VA_ARGS__)
#else
   #define print(...)
   #define println(...)
#endif

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


SpeedyStepper stepper;        // Create a new instance of the Stepper class:
const int trimPot = 34;       // Adjust the Speed of the trun
const int activationPin = 23; // Low Level Trigger!
const int ledPin = 2;         // LED Built-in for Esp32
const int rotatePin = 35;     // Pin for Pot to adjust turn.
int speedy = 600;             // initial speed of stepper motor in steps / second.  Adjustable via trimPot
#define CW 1
#define CCW -1
int Step = 0;

//---------------------------------------//
//---------------- SETUP ----------------//
//---------------------------------------//

void setup() {
  

#ifdef __DEBUG__
  Serial.begin(115200);
#endif

  pinMode(activationPin, OUTPUT);
  pinMode(17, INPUT); // pin will determine direction of stepper motor travel
  digitalWrite(activationPin, HIGH);
  delay(500); // give power time to stabilize.
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);
  pinMode(trimPot, INPUT);
  pinMode(rotatePin,INPUT);

 

#ifdef __DEBUG__
  print("Move Steps: ");
  println(Step);
  print("Setting AP (Access Point)…");
#endif


  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAP("target.Wifi");
  delay(500); // give power time to stabilize.
  WiFi.softAP(ssid);
  delay(100);
  IPAddress IP = WiFi.softAPIP();
  server.on("/edge", targetEdge);      //Which routine to handle at edge location
  server.on("/face", targetFace);      //Which routine to handle at face location
  server.on("/", handleRoot);          //Which routine to handle at root location
  delay(500);
  server.begin();                      //Start server
  
#ifdef __DEBUG__
  println("HTTP server started");
#endif

  delay(500);
  stepper.connectToPins(stepPin, dirPin);
}


//---------------------------------------//
//------------ MAIN LOOP ----------------//
//---------------------------------------//

void loop(){
  
  server.handleClient();          //Handle client requests  s
  
  
#ifdef __DEBUG__

#endif
  int moveSteps = map(analogRead(rotatePin), 0, 4096,200,1000);
   if (digitalRead(17) == HIGH) { Step = (CW * moveSteps); } else { Step =  (CCW * moveSteps); }
  speedy = map(analogRead(trimPot), 0, 4096,400,2000);   // Rescale to potentiometer's voltage (from 0V to 3.3V):
  stepper.setAccelerationInStepsPerSecondPerSecond(speedy);
  stepper.setSpeedInStepsPerSecond(speedy);

#ifdef __DEBUG__
  delay(750);
  print("Speed: ");
  println(speedy);
  print("Direction: ");
  println(moveSteps);
#endif

  delay(250);
  
}

//---------------------------------------//
//------------ HTTP root-----------------//
//---------------------------------------//
void handleRoot() {
  server.send(200, "text/plain", "Target system requires http://192.168.4.1/face or http://192.168.4.1/edge");
}

//---------------------------------------//
//------------- Target Edge -------------//
//---------------------------------------//

void targetEdge() {
  server.send(200, "text/plain", "Targets Edged");
  digitalWrite(activationPin, LOW);
  digitalWrite(ledPin, HIGH);
  
#ifdef __DEBUG__
  println("Edged...");
  println(Step);
  print("Speed: ");
  println(speedy);
#endif

  stepper.moveToPositionInSteps(Step);
 
}

//---------------------------------------//
//------------ Face the Targets ---------//
//---------------------------------------//

void targetFace() {
  server.send(200, "text/plain", "Targets Faced");
  digitalWrite(activationPin, HIGH);
  digitalWrite(ledPin, LOW);
  
#ifdef __DEBUG__
  println("Faced...");
  println(0);
  print("Speed: ");
  println(speedy);
#endif

  stepper.moveToPositionInSteps(0);
  
}
