/*

 1. Connection and Diagram for DFPlayerMini can be found here: <https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299#Connection_Diagram>
 2. Board Design: https://easyeda.com/editor#id=9732d78825114e1f9470af595e8c2a05|cfccfb0ac82742d599bb365bfc38d4f2|02da22ace5ad4361b509632e239951c8|54cbbeacb0e44b48bd5351eefe88220c
 3. Supplying 12v to the board seems to yield the best result for the antenna.  Voltage can be reduced with the 10k pot

*/

//#define __DEBUG__
#ifdef __DEBUG__
   #define print(...) Serial.print(__VA_ARGS__)
   #define println(...) Serial.println(__VA_ARGS__)
#else
   #define print(...)
   #define println(...)
#endif


#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

unsigned long myTime;
unsigned long play_time;

const int relayPin = 3;     // the pin of the relay to activate the TARGET
const int rapidPin = 4;     // Pin for Rapid
const int timedPin = 6;     // pin for Timed
const int practicePin = 2;  // pin for practice iterations

// The follow pre-processor variables are used to set the type of transmitter / relay that is used, either LOW or HIGH depending
// on the type.
#define FACE 1 
#define AWAY 0

void setup() {
   pinMode(timedPin, INPUT_PULLUP);
   pinMode(rapidPin, INPUT_PULLUP);
   pinMode(practicePin, INPUT_PULLUP);
   pinMode(relayPin,OUTPUT);
   #ifdef __DEBUG__
    Serial.begin(115200);
   #endif
   mySoftwareSerial.begin(9600);
   myDFPlayer.begin(mySoftwareSerial);
   myDFPlayer.volume(30);  //Set volume value. From 0 to 30
   #ifdef __DEBUG__
    sysready();
   #endif
}



void loop() {
  int sensorTimed = digitalRead(timedPin);
  int sensorRapid = digitalRead(rapidPin);
  int sensorPractice = digitalRead(practicePin);

  // Practice Pin  
   if (sensorPractice == LOW) {
     println("Practice Button Pressed\n");
     turn_targets(1,5);
   }

  // Timed Pin
   if (sensorTimed == LOW) {
     println ("Timed Button Pressed\n");
     turn_targets(20,2);
     }
  // Rapid Pin
  
   if (sensorRapid == LOW) {
    println ("Rapid Button Pushed\n");
     turn_targets(10,2);
   }

}

void play_load_commands(int seconds) {
    /*
   * Order of Songs:  
   *    3 : Timed 26.5s
   *    2 : Rapid 26.1s
   *    1 : Practice 9.6s
   */
   
    println ("Entering Range Commands...Turning LED off");
    switch (seconds) {
      case 10:
        println("Playing Range Commands Rapid");
        myDFPlayer.play(2); 
        println("call for sound for 0002.mp3");
        delay(30100);
        break;
      case 20:
        println("Playing Range Commands Timed");
        myDFPlayer.play(3); 
        print ("call for sound for 0003.mp3");
        delay(30500);
        break;
      case 1:
        println ("Playing Range Commands Practice");
        myDFPlayer.play(1);
        println ("call for sound for 0003.mp3");
        delay(9600);
        break;
      default:
        print ("Error!  Shouldn't be here.  Case logic incorrect?\n");
        break;
       }
      
}
void turn_targets( int seconds, int iterations ) {
   for (int i = 1; i <= iterations; i++) {
       // Move the targes away
       digitalWrite(relayPin, AWAY);
       print("   Seconds for loop: ");
       println(seconds);
       print("    Entering Loop #: ");
       print(i);
       print(" of ");
       println(iterations);
       play_load_commands(seconds);
       println("Waiting for 3 seconds & target Turn");
       // It takes a few seconds after the last range command before targets should turn. 
       // NRA rules have it roughly 3 seconds after "Ready on the firing line"... 
       delay(3000);
       println ("Initiating Turn:");
       digitalWrite(relayPin, FACE); 
       delay(500);  // give targets time to turn.
       delay(seconds*1000);  
       digitalWrite(relayPin, AWAY);  
     
   }    
     // sequence over;  Wait 3 seconds for all firing to cease.  Then return targets facing
     // for center paper replacement.
     
     delay(3000);
     digitalWrite(relayPin, FACE); 

      #ifdef __DEBUG__
        sysready();   
      #endif 
  
}

void sysready() {
   println("*    SYSTEM READY.    *"); \\
}
