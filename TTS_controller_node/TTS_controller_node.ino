/*

 AS-IS No Warranty.  

  Parts of code dervied from:
  https://dronebotworkshop.com
*/

//#define _DEBUG_
#if defined _DEBUG_
   char printBuf[200];
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

// Include Libraries
#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>



typedef struct target_command {
    char dir;
} target_command;

target_command command;

const char* ssid           = "target.Wifi"; 
const char* password       = "";               // SSID Password - Set to NULL to have an open AP
const int   channel        = 1;                // WiFi Channel number between 1 and 13
const bool  hide_SSID      = false;            // To disable SSID broadcast -> SSID will not appear in a basic WiFi scan
const int   max_connection = 1;                // Maximum simultaneous connected clients on the AP

WebServer server(80);

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
  // Formats MAC Address
  {
    snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],macAddr[6],macAddr[7],macAddr[8],macAddr[9]);
  }

void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
  // Called when data is sent
  {
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    debug_println("Last Packet Sent to: ");
    debug_println(macStr);
    debug_print("Last Packet Send Status: ");
    debug_println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }

void broadcast(const char message)
  // Emulates a broadcast
  {
    // Broadcast a message to every device in range
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, broadcastAddress, 10 );
    if (!esp_now_is_peer_exist(broadcastAddress)) { esp_now_add_peer(&peerInfo); }
    // Send message
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&command, sizeof(target_command));
  
    #ifdef _DEBUG_
        // Print results to serial monitor
        if (result == ESP_OK)                         { debug_println("Broadcast message success"); }
        else if (result == ESP_ERR_ESPNOW_NOT_INIT)   { debug_println("ESP-NOW not Init."); }
        else if (result == ESP_ERR_ESPNOW_ARG)        { debug_println("Invalid Argument"); }
        else if (result == ESP_ERR_ESPNOW_INTERNAL)   { debug_println("Internal Error"); }
        else if (result == ESP_ERR_ESPNOW_NO_MEM)     { debug_println("ESP_ERR_ESPNOW_NO_MEM"); }
        else if (result == ESP_ERR_ESPNOW_NOT_FOUND)  { debug_println("Peer not found."); }
        else                                          { debug_println("Unknown error"); }
    #endif
  }

void setup()
{
  // Set up Serial Monitor
  #ifdef _DEBUG_
    Serial.begin(115200);
  #endif
  delay(500);
  // Set ESP32 in STA mode to begin with
  debug_println("Setting AP (Access Point)…");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password, channel, hide_SSID, max_connection);
  IPAddress IP =WiFi.softAPIP();
  server.on("/edge", targetEdge);
  server.on("/face", targetFace);
  server.on("/", handleRoot);
  debug_println("IP Address for system:  %u.%u.%u.%u", IP[0], IP[1], IP[2], IP[3]);
  debug_println("Starting http Server...");
  server.begin();
  // Initialize ESP-NOW
  if (esp_now_init() == ESP_OK)
  {
    debug_println("ESP-NOW Init Success");
    //6esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    debug_println("ESP-NOW Init Failed");
    debug_println(" .... restarting.");
    delay(3000);
    ESP.restart();
  }
}

void loop() { server.handleClient(); }

void handleRoot() { server.send(200,"text/plain", "Target system should use face or edge"); }

void targetEdge() {
    debug_println("Edge command received.");
    server.send(200,"text/plain", "Targets Edged");
    command.dir = 'E';
    broadcast(command.dir);
}

void targetFace() {
  debug_println("Face command received.");
  server.send(200,"text/plain", "Targets Faced");
  command.dir = 'F';
  broadcast(command.dir);
}
