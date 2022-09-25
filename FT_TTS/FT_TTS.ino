//#define __DEBUG__


/*
 * 
 * Except as written in this agreement, Developer’s work product is provided ​“AS IS”.”
 * This code will replace Frank Thomas' TTS controller, making this target turn wirelessly.
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

const char* ssid = "target.Wifi";
WebServer server(80); //Server on port 80

const int lowActivationPin = 23;    // Low Level Trigger!
const int ledPin = 2;               // LED Built-in for Esp32


//---------------------------------------//
//---------------- SETUP ----------------//
//---------------------------------------//

void setup() {
  

#ifdef __DEBUG__
  Serial.begin(115200);
#endif

  pinMode(lowActivationPin, OUTPUT);
   digitalWrite(lowActivationPin, HIGH);
  delay(500); // give power time to stabilize.
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);

#ifdef __DEBUG__
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
  print("IP address: ");
  println(IP);
#endif

  delay(500);

}


//---------------------------------------//
//------------ MAIN LOOP ----------------//
//---------------------------------------//

void loop(){
  
  server.handleClient();          //Handle client requests 
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
  
  digitalWrite(lowActivationPin, LOW);
  digitalWrite(ledPin, HIGH);
  delay(250);
    digitalWrite(ledPin, LOW);
  digitalWrite(lowActivationPin, HIGH);

  
#ifdef __DEBUG__
  println("Edged...");
#endif
 
}

//---------------------------------------//
//------------ Face the Targets ---------//
//---------------------------------------//

void targetFace() {
  server.send(200, "text/plain", "Targets Faced");
  digitalWrite(lowActivationPin, LOW);
  digitalWrite(ledPin, HIGH);
  delay(250);
    digitalWrite(ledPin, LOW);
  digitalWrite(lowActivationPin, HIGH);
  
  
#ifdef __DEBUG__
  println("Faced...");

#endif

}
