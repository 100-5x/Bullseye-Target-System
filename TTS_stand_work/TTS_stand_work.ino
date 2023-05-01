/*
 * 
 * Except as written in this agreement, Developer’s work product is provided ​“AS IS”.”
 * 
 */

//#define _DEBUG_
#if defined _DEBUG_
   char printBuf[100];
   #define debug_print(...) \
     sprintf(printBuf, __VA_ARGS__); \
     Serial.print(printBuf)
   #define debug_println(...) \
     sprintf(printBuf, __VA_ARGS__); \
     Serial.println(printBuf)
#else
   #define debug_print(...)
   #define debug_println(...)
#endif


#include <SpeedyStepper.h>
#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>


// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#define dirPin 18                     // DIR+
#define stepPin 19                    // PUL+
#define motorInterfaceType 1
const int trimPot = 34;               // Adjust the Speed of the trun
const int ledPin = 2;                 // LED Built-in for Esp32
const int rotatePin = 35;             // Pin for Pot to adjust turn.
int speedy = 1000;                    // initial speed of stepper motor in steps / second.  Adjustable via trimPot
int Step = 0;
int moveSteps = 200;
SpeedyStepper stepper;                // Create a new instance of the Stepper class:
const int mosfetActivationPin = 23;   // For MOSFET
const int relayActivationPin = 22;    // External Trigger





//Wifi Variables
const char* ssid		   = "target.Wifi"; 
const char* password       = "";   // SSID Password - Set to NULL to have an open AP
const int   channel        = 1;                        // WiFi Channel number between 1 and 13
const bool  hide_SSID      = false;                     // To disable SSID broadcast -> SSID will not appear in a basic WiFi scan
const int   max_connection = 1;                         // Maximum simultaneous connected clients on the AP
#define CW 1
#define CCW -1
WebServer server(80); //Server on port 80


//Structure example to receive data
//Must match the sender structure
//Create a struct_message called myData

typedef struct tts_struct {
  char cmd;
} tts_struct;

//Create a struct_message called myData
tts_struct myData;

//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  if (myData.cmd == 'E') { targetEdge(); }
  if (myData.cmd == 'F') { targetFace(); }
  debug_println("Bytes received: %i Command: %s", len, myData.cmd);
  
}

//---------------------------------------//
//---------------- SETUP ----------------//
//---------------------------------------//

void setup() {
  
 
#ifdef _DEBUG_
  Serial.begin(115200);
#endif

  pinMode(relayActivationPin, OUTPUT);
  pinMode(mosfetActivationPin, OUTPUT);
  pinMode(17, INPUT); // pin will determine direction of stepper motor travel
  pinMode(25, INPUT); // pin will determine worker or standalone mode
  digitalWrite(relayActivationPin, LOW);
  digitalWrite(mosfetActivationPin, LOW);  
  delay(200); // give power time to stabilize.
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);
  pinMode(trimPot, INPUT);
  pinMode(rotatePin,INPUT);
  debug_println("Connecting to Stepper Driver…");
  stepper.connectToPins(stepPin, dirPin);

if (digitalRead(25) == LOW) {
    // Standalone Mode
  debug_println("Setting AP (Access Point)…");
  WiFi.mode(WIFI_AP);
  delay(250);
  //WiFi.softAP("target.Wifi");
  delay(150); // give power time to stabilize.
  debug_println("HTTP server starting");
  WiFi.softAP(ssid, password, channel, hide_SSID, max_connection);
  debug_println("HTTP server started");
  delay(100);
  debug_println("Obtaining IP Address.");
  IPAddress IP = WiFi.softAPIP();
  delay(100);
  server.on("/edge", targetEdge);      //Which routine to handle at edge location
  server.on("/face", targetFace);      //Which routine to handle at face location
  server.on("/", handleRoot);          //Which routine to handle at root location
  delay(100);
  server.begin();                      //Start server
  debug_println("IP Address for system:  %u.%u.%u.%u", IP[0], IP[1], IP[2], IP[3]);

} else {
  //Woker Mode
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    debug_println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
  delay(100);
}


//---------------------------------------//
//------------ MAIN LOOP ----------------//
//---------------------------------------//

void loop(){

#ifdef _DEBUG_
  delay(1000);
#endif
  
  if (digitalRead(25) == LOW) {
  server.handleClient();          //Handle client requests  s
  }
  moveSteps = map(analogRead(rotatePin), 5, 4095,210,45); // Rescale to potentiometer's voltage (from 0V to 3.3V):
  if (digitalRead(17) == HIGH) { Step = (CW * moveSteps); } else { Step =  (CCW * moveSteps); }
  speedy = map(analogRead(trimPot), 0, 4095,2000,400);   // Rescale to potentiometer's voltage (from 0V to 3.3V):
  stepper.setAccelerationInStepsPerSecondPerSecond(speedy);
  stepper.setSpeedInStepsPerSecond(speedy);
  debug_println("Speed: %i\tDirection: %i\tSteps: %i", speedy,moveSteps,Step);
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
  digitalWrite(relayActivationPin, HIGH);
  digitalWrite(mosfetActivationPin, HIGH);
  digitalWrite(ledPin, HIGH);
  debug_println("Edged.  Step: %i\tSpeed: %i", Step, speedy);
  stepper.moveToPositionInSteps(Step);
 
}

//---------------------------------------//
//------------ Face the Targets ---------//
//---------------------------------------//

void targetFace() {
  server.send(200, "text/plain", "Targets Faced");
  digitalWrite(relayActivationPin, LOW);
  digitalWrite(mosfetActivationPin, LOW);
  digitalWrite(ledPin, LOW);
  debug_println("Faced.  Step: 0\tSpeed: %i", speedy);
  stepper.moveToPositionInSteps(0);
  
}
