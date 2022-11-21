/*
  ESP-NOW Multi Unit Demo
  esp-now-multi.ino
  Broadcasts control messages to all devices in network
  Load script on multiple devices

  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/

//#define __DEBUG__

#ifdef __DEBUG__
   #define print(...)   Serial.print(__VA_ARGS__)
   #define println(...) Serial.println(__VA_ARGS__)
#else
   #define print(...)
   #define println(...)
#endif


// Include Libraries
#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>

WebServer server(80);

typedef struct target_command {
    char dir;
} target_command;

target_command command;

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#include <SpeedyStepper.h>
#define dirPin 18 // DIR+
#define stepPin 19 // PUL+
#define motorInterfaceType 1
#define CW 1
#define CCW -1
SpeedyStepper stepper;              // Create a new instance of the Stepper class:



const int trimPot = 34;             // Adjust the Speed of the trun
const int mosfetActivationPin = 23;   // For MOSFET
const int relayActivationPin = 22;    // Low Level Trigger!
const int ledPin = 2;               // LED Built-in for Esp32
const int rotatePin = 35;           // Pin for Pot to adjust turn.
int speedy = 1000;                   // initial speed of stepper motor in steps / second.  Adjustable via trimPot
int Step = 0;
int moveSteps = 200;


void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
  // Formats MAC Address
  {
    snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],macAddr[6],macAddr[7],macAddr[8],macAddr[9]);
  }


//void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
//  // Called when data is received
//  {
//    // Only allow a maximum of 250 characters in the message + a null terminating byte
//    char buffer[ESP_NOW_MAX_DATA_LEN + 1];
//    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
//    strncpy(buffer, (const char *)data, msgLen);
//  
//    // Make sure we are null terminated
//    buffer[msgLen] = 0;
//  
//    // Format the MAC address
//    char macStr[18];
//    formatMacAddress(macAddr, macStr, 18);
//  
//    // Send Debug log message to the serial port
//    Serial.printf("Received message from: %s - %s\n", macStr, buffer);
//  
//  
//  }


void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
  // Called when data is sent
  {
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    print("Last Packet Sent to: ");
    println(macStr);
    print("Last Packet Send Status: ");
    println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }

void broadcast(const char message)
  // Emulates a broadcast
  {
    // Broadcast a message to every device in range
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, broadcastAddress, 10 );
    if (!esp_now_is_peer_exist(broadcastAddress))
    {
      esp_now_add_peer(&peerInfo);
    }
    // Send message
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&command, sizeof(target_command));
  
    #ifdef __DEBUG__
    // Print results to serial monitor
    if (result == ESP_OK)
    {
      println("Broadcast message success");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
      println("ESP-NOW not Init.");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
      println("Invalid Argument");
    }
    else if (result == ESP_ERR_ESPNOW_INTERNAL)
    {
      println("Internal Error");
    }
    else if (result == ESP_ERR_ESPNOW_NO_MEM)
    {
      println("ESP_ERR_ESPNOW_NO_MEM");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
      println("Peer not found.");
    }
    else
    {
      println("Unknown error");
    }
    #endif
  }

void setup()
{

  // Set up Serial Monitor
  #ifdef __DEBUG__
    Serial.begin(115200);
  #endif
  delay(500);

  pinMode(relayActivationPin, OUTPUT);
  pinMode(mosfetActivationPin, OUTPUT);
  pinMode(17, INPUT); // pin will determine direction of stepper motor travel
  digitalWrite(relayActivationPin, LOW);
  digitalWrite(mosfetActivationPin, LOW);  
  delay(200); // give power time to stabilize.
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);
  pinMode(trimPot, INPUT);
  pinMode(rotatePin,INPUT);

  // Set ESP32 in STA mode to begin with
  #ifdef __DEBUG__
    print("Setting AP (Access Point)…");
  #endif

  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("target.wifi");
  IPAddress IP =WiFi.softAPIP();
  server.on("/edge", targetEdge);
  server.on("/face", targetFace);
  server.on("/", handleRoot);
  server.begin();

  #ifdef __DEBUG__
    println("ESP-NOW Broadcast Demo");
    print("MAC Address: ");
    println(WiFi.macAddress());
  #endif


  // Initialize ESP-NOW
  if (esp_now_init() == ESP_OK)
  {
     #ifdef __DEBUG__
        println("ESP-NOW Init Success");
     #endif
    //6esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
     #ifdef __DEBUG__
    println("ESP-NOW Init Failed");
    #endif
    delay(3000);
    ESP.restart();
  }
  
  stepper.connectToPins(stepPin,dirPin);
}

void loop()
{
    server.handleClient();
    moveSteps = map(analogRead(rotatePin), 5, 4096,210,45); // Rescale to potentiometer's voltage (from 0V to 3.3V):
    if (digitalRead(17) == HIGH) { Step = (CW * moveSteps); } else { Step =  (CCW * moveSteps); }
    speedy = map(analogRead(trimPot), 0, 4096,2000,400);   // Rescale to potentiometer's voltage (from 0V to 3.3V):
    stepper.setAccelerationInStepsPerSecondPerSecond(speedy);
    stepper.setSpeedInStepsPerSecond(speedy);

#ifdef __DEBUG__
  delay(1000);
  print("Speed: ");
  println(speedy);
  print("Direction: ");
  println(moveSteps);
  print("Steps: ");
  println(Step);
#endif
  
//    broadcast('E');
//    sleep(2000);
//    broadcast('F');
//  
}

void handleRoot() {
    server.send(200,"text/plain", "Target system should use face or edge");
    
}

void targetEdge() {
    server.send(200,"text/plain", "Targets Edged");
    command.dir = 'E';
    broadcast(command.dir);
    digitalWrite(relayActivationPin, HIGH);
    digitalWrite(mosfetActivationPin, HIGH);
    digitalWrite(ledPin, HIGH);
  
#ifdef __DEBUG__
  println("Edged...");
  println(Step);
  print("Speed: ");
  println(speedy);
#endif

  stepper.moveToPositionInSteps(Step);
  
}

void targetFace() {
  server.send(200,"text/plain", "Targets Faced");
  command.dir = 'F';
  broadcast(command.dir);
  digitalWrite(relayActivationPin, LOW);
  digitalWrite(mosfetActivationPin, LOW);
  digitalWrite(ledPin, LOW);
  
  
#ifdef __DEBUG__
  println("Faced...");
  println(0);
  print("Speed: ");
  println(speedy);
#endif

  stepper.moveToPositionInSteps(0);
  
}
