/**
 * epicDiceRoller
 * by Team 2
 * Braden Harwood, Stephen Short, Michael Weston, Drew Seidel
 * 
 */

#include <AltSoftSerial.h>
#include <EnableInterrupt.h>
#include <RotaryEncoder.h>

// Define IO Pins
#define ROTARYPINA 2
#define ROTARYPINB 3 // May need to switch these if the direction is backwards
#define LEFTMENU 5
#define MIDDLEMENU 6
#define TILTSWITCH 7
#define LCDSCREENRX 9
#define ROTARYBUTTON A0
#define RIGHTMENU A1
#define TOGGLEUP A2
#define TOGGLEDOWN A3


// Rotary Functions
void encoderUpdate();
RotaryEncoder encoder(ROTARYPINA, ROTARYPINB);

volatile uint32_t anyInterruptCounter=0;

byte contrast = 2; //Lower is more contrast. 0 to 5 works for most displays.
AltSoftSerial OpenLCD(8, LCDSCREENRX); //RX, TX

void leftMenuFunc() {
    anyInterruptCounter++;
    int buttonState = digitalRead(LEFTMENU);
}

void middleMenuFunc() {
    anyInterruptCounter++;
    int buttonState = digitalRead(MIDDLEMENU);
}

void rightMenuFunc() {
    anyInterruptCounter++;
    int buttonState = digitalRead(RIGHTMENU);
}

void tiltFunc() {
    anyInterruptCounter++;
}

void encoderUpdate() {
  encoder.tick(); // just call tick() to check the state.
  anyInterruptCounter++;
}

// Attach the interrupt in setup()
void setup() {
  Serial.begin(9600);

  // initialize the pin modes
  pinMode(ROTARYPINA, INPUT_PULLUP);
  pinMode(ROTARYPINB, INPUT_PULLUP);

  pinMode(LEFTMENU, INPUT);
  pinMode(MIDDLEMENU, INPUT);
  pinMode(RIGHTMENU, INPUT);
  
  pinMode(TILTSWITCH, INPUT);

  pinMode(TOGGLEUP, INPUT);
  pinMode(TOGGLEDOWN, INPUT);

  // PINS 0 and 1 NOT USED BECAUSE OF Serial.print()
  enableInterrupt(LEFTMENU, leftMenuFunc, CHANGE);
  enableInterrupt(MIDDLEMENU, middleMenuFunc, CHANGE);
  enableInterrupt(RIGHTMENU, rightMenuFunc, CHANGE);
  enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  
  // Rotary Encoder
  enableInterrupt(ROTARYPINA, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYPINB, encoderUpdate, CHANGE);

  OpenLCD.begin(9600); //Start communication with OpenLCD
  //Send contrast setting
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(24); //Send contrast command
  OpenLCD.write(contrast);
}

// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine.
void loop() {
  delay(2000);                          // Perform the loop every second.
  
  EI_printPSTR("Consolidated interrupt count: "); Serial.println(anyInterruptCounter);
  // Check Rotary Encoder
    static int pos = 0;
    int newPos = encoder.getPosition();
    if (pos != newPos) {
      EI_printPSTR("Position "); Serial.println(newPos);
      pos = newPos;
    }
  OpenLCD.write('|'); //Setting character
  OpenLCD.write('-'); //Clear display

  OpenLCD.print("Interrupt Count!   Counter: "); //For 16x2 LCD
  OpenLCD.print(anyInterruptCounter);
  
  anyInterruptCounter = 0;
}
