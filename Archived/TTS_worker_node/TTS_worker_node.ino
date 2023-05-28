

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

#define CW 1
#define CCW -1
#define directionPin 17               // directionPin defines the switch direction.



// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#define dirPin 18 // DIR+
#define stepPin 19 // PUL+
#define motorInterfaceType 1
const int trimPot = 34;             // Adjust the Speed of the trun
const int ledPin = 2;               // LED Built-in for Esp32
const int rotatePin = 35;           // Pin for Pot to adjust turn.
int speedy = 1000;                   // initial speed of stepper motor in steps / second.  Adjustable via trimPot
int Step = 0;
int moveSteps = 200;
SpeedyStepper stepper;              // Create a new instance of the Stepper class:
const int mosfetActivationPin = 23;   // For MOSFET
const int relayActivationPin = 22;    // Low Level Trigger!


//Must match the sender structure
//Create a struct_message called myData

typedef struct test_struct {
  char cmd;
} test_struct;

//Create a struct_message called myData
test_struct myData;

//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  if (myData.cmd == 'E') { targetEdge(); }
  if (myData.cmd == 'F') { targetFace(); }
  debug_print("Bytes received %i, command: %s", len, myData.cmd);
   
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
 
  delay(250);

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


//---------------------------------------//
//------------ MAIN LOOP ----------------//
//---------------------------------------//

void loop(){
  
 
  moveSteps = map(analogRead(rotatePin), 5, 4095,210,45); // Rescale to potentiometer's voltage (from 0V to 3.3V):
  if (digitalRead(17) == HIGH) { Step = (CW * moveSteps); } else { Step =  (CCW * moveSteps); }
  speedy = map(analogRead(trimPot), 0, 4095,2000,400);   // Rescale to potentiometer's voltage (from 0V to 3.3V):
  stepper.setAccelerationInStepsPerSecondPerSecond(speedy);
  stepper.setSpeedInStepsPerSecond(speedy);

#ifdef _DEBUG_
  delay(1000);
  debug_println("Speed: \t%i\tDirection: \t%i\t Steps: \t%i", speedy, moveSteps, Step);
#endif

  //delay(250);
  
}


//---------------------------------------//
//------------- Target Edge -------------//
//---------------------------------------//

void targetEdge() {
  digitalWrite(relayActivationPin, HIGH);
  digitalWrite(mosfetActivationPin, HIGH);
  digitalWrite(ledPin, HIGH);
  debug_println("Edged.  Position: %i\tSpeed: \t%i", Step, speedy);
  stepper.moveToPositionInSteps(Step);
 
}

//---------------------------------------//
//------------ Face the Targets ---------//
//---------------------------------------//

void targetFace() {
  digitalWrite(relayActivationPin, LOW);
  digitalWrite(mosfetActivationPin, LOW);
  digitalWrite(ledPin, LOW);
  debug_println("Faced.  Position: 0 \tSpeed: \t%i", speedy);
  

  stepper.moveToPositionInSteps(0);
  
}
