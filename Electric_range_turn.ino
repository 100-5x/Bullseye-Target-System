//#include <LowPower.h>

// Include the AccelStepper library:
//#define __DEBUG__

#ifdef __DEBUG__
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif


#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
// DIR- && PUL- to GND
#define dirPin 5 // DIR+
#define stepPin 7 // PUL+
#define sigPin 9 //From wireless receiver
#define motorInterfaceType 1

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

void setup() {
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(700);
  pinMode(sigPin, INPUT); 
}

void loop() {
 
   LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  // Run to target position with set speed and acceleration/deceleration:

  if (digitalRead(sigPin) == 1) { stepper.moveTo(90); } else {stepper.moveTo(0); }
  stepper.runToPosition();

}
