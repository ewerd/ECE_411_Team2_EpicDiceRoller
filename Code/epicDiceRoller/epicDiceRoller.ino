/**
 * epicDiceRoller
 * by Team 2
 * Braden Harwood, Stephen Short, Michael Weston, Drew Seidel
 * 
 */

#include <AltSoftSerial.h>
#include <EnableInterrupt.h>
#include <RotaryEncoder.h>
#include <Entropy.h>

#define NUM_POSSIBLE_DICE 9
#define MAX_NUM_DICE 10
#define MIN_NUM_DICE 1

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

// Interrupt Functions
void encoderUpdate();
void leftMenuFunc();
void middleMenuFunc();
void rightMenuFunc();
void tiltFunc();
void toggleUpFunc();
void toggleDownFunc();
void encoderUpdate();

// Run time Functions
void printDice();
void printRoll();

// Counter to determine the number of interrupts triggered. Used for tweaking the tilt sensor
volatile uint32_t anyInterruptCounter=0;

// Object definitions for encoder and LCD screen
RotaryEncoder encoder(ROTARYPINA, ROTARYPINB);

AltSoftSerial OpenLCD(8, LCDSCREENRX); //RX, TX

// Enum to determine what state we are in, used in global interrupts
typedef enum _device_state_e {SETUP, ROLL, PREVIOUS} state_t;
state_t currentState = SETUP;

// Contrast definition for the LCD Screen
byte contrast = 2; //Lower is more contrast. 0 to 5 works for most displays.

// Global Dice Array that holds all of the values of dice that are available
const int dice[NUM_POSSIBLE_DICE] = {2, 3, 4, 6, 8, 10, 12, 20, 100};
volatile int diceIndex = 7;
volatile int numDice = 1;
int diceRoll = 0;
int previousDiceRoll = diceRoll;

void setup() {
  Serial.begin(9600);

  // Pins without external pull ups
  pinMode(ROTARYPINA, INPUT_PULLUP);
  pinMode(ROTARYPINB, INPUT_PULLUP);

  // Pins with external pull ups
  pinMode(LEFTMENU, INPUT);
  pinMode(MIDDLEMENU, INPUT);
  pinMode(RIGHTMENU, INPUT);
  pinMode(TILTSWITCH, INPUT);
  pinMode(TOGGLEUP, INPUT);
  pinMode(TOGGLEDOWN, INPUT);

  // Enable the interrupts for the initial state
  enableInterrupt(LEFTMENU, leftMenuFunc, FALLING);
  enableInterrupt(MIDDLEMENU, middleMenuFunc, FALLING);
  enableInterrupt(RIGHTMENU, rightMenuFunc, FALLING);
  enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  enableInterrupt(TOGGLEUP, toggleUpFunc, FALLING);
  enableInterrupt(TOGGLEDOWN, toggleDownFunc, FALLING);
  
  // Rotary Encoder
  enableInterrupt(ROTARYPINA, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYPINB, encoderUpdate, CHANGE);

  OpenLCD.begin(9600); //Start communication with OpenLCD
  //Send contrast setting
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(24); //Send contrast command
  OpenLCD.write(contrast);

  // Set mode to SETUP
  currentState = SETUP;

  // Initialize Screen
  printDice();
}


void printDice()
{
  char line1[16] = "      ";
  char numDice_s[5];
  char diceValue_s[5];
  const char menuLine[17] = "Less  Roll  More";
  char output[32];
  if (currentState == SETUP)
  {
    disableAllInterrupts();
    
    itoa(numDice, numDice_s, 10);
    strcat(line1, numDice_s);
    
    strcat(line1, "d");
    
    itoa(dice[diceIndex],diceValue_s,10);
    strcat(line1, diceValue_s);
    
    int line1Length = strlen(line1);
    if (line1Length < 16)
    {
      for (unsigned int i = 0; i < 16-line1Length; i++)
      {
        strcat(line1, " ");
      }
    }
    
    Serial.print("line1 = ");Serial.println(line1);

    strcpy(output, line1);
    
    Serial.print("output = ");Serial.println(output);
    Serial.print("menuLine = ");Serial.println(menuLine);
    strcat(output, menuLine);

    OpenLCD.write('|'); //Setting character
    OpenLCD.write('-'); //Clear display

    // Print ndm dice and menu options
    OpenLCD.print(output);
    
    enableAllInterrupts();
  }
}

void rollDice()
{
  disableAllInterrupts();
  
  currentState = ROLL;
  
  previousDiceRoll = diceRoll;
  diceRoll = 0;
  for (int i = 0; i < numDice; i++) {
    diceRoll = diceRoll + Entropy.random(1, dice[diceIndex] + 1);
  }
  Serial.print("diceRoll: ");Serial.println(diceRoll);
  
  printRoll();
}

void printRoll()
{
  char line1[16] = "";
  char numDice_s[5];
  char diceValue_s[5];
  char rollValue_s[5];
  const char menuLine[17] = "      Home      ";
  char output[32];
  if (currentState == ROLL)
  {
    disableAllInterrupts();
    
    itoa(numDice, numDice_s, 10);
    strcat(line1, numDice_s);
    
    strcat(line1, "d");
    
    itoa(dice[diceIndex],diceValue_s,10);
    strcat(line1, diceValue_s);

    strcat(line1, " = ");
    
    itoa(diceRoll,rollValue_s,10);
    strcat(line1, rollValue_s);
    
    int line1Length = strlen(line1);
    if (line1Length < 16)
    {
      for (unsigned int i = 0; i < 16-line1Length; i++)
      {
        strcat(line1, " ");
      }
    }
    
    Serial.print("line1 = ");Serial.println(line1);

    strcpy(output, line1);
    
    Serial.print("output = ");Serial.println(output);
    Serial.print("menuLine = ");Serial.println(menuLine);
    strcat(output, menuLine);

    OpenLCD.write('|'); //Setting character
    OpenLCD.write('-'); //Clear display

    // Print ndm dice and menu options
    OpenLCD.print(output);
    
    enableAllInterrupts();
  }
}


// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine of the tilt switch.
void loop() {
  //delay(2000);                          // Perform the loop every 2 seconds.
  
  //Serial.print("Consolidated interrupt count: "); Serial.println(anyInterruptCounter);
  if (currentState == SETUP)
  {
    // Check Rotary Encoder
    static int pos = 0;
    int newPos = encoder.getPosition();
    if (pos != newPos) {
        //Serial.print("pos = ");
        //Serial.println(pos);
        //Serial.print("newpos = ");
        //Serial.println(newPos);
      if (newPos > pos) { // Increment position
        diceIndex = (diceIndex + 1) % NUM_POSSIBLE_DICE;
        printDice();
      }
      else { // Decrement position
        if (diceIndex == 0) { // Don't go negative while decrementing, go to top of encoder
          diceIndex = NUM_POSSIBLE_DICE - 1;
          printDice();
        }
        else { // Decrement position
          diceIndex = (diceIndex - 1) % NUM_POSSIBLE_DICE;
          printDice();
        }
      }
      pos = newPos;
    }
  }
  /*
  OpenLCD.write('|'); //Setting character
  OpenLCD.write('-'); //Clear display

  OpenLCD.print("Interrupt Count!   Counter: "); //For 16x2 LCD
  OpenLCD.print(anyInterruptCounter);
  */
  
  anyInterruptCounter = 0;
}






// Interrupt Function Code
void leftMenuFunc() {
    anyInterruptCounter++;
    if (currentState == SETUP)
    {
      int buttonState = digitalRead(LEFTMENU);
      if (buttonState == LOW)
      {
        if ((numDice > MIN_NUM_DICE) && (numDice <= MAX_NUM_DICE))
        {
          numDice--;
          printDice();
        }
      }
    }
}

void middleMenuFunc() {
    anyInterruptCounter++;
    int buttonState = digitalRead(MIDDLEMENU);
    if (currentState == SETUP)
    {
      if (buttonState == LOW)
      {
        currentState = ROLL;
        rollDice();
      }
    }
    else if (currentState == ROLL)
    {
      if (buttonState == LOW)
      {
        currentState = SETUP;
        printDice();
      }
    }
}

void rightMenuFunc() {
    anyInterruptCounter++;
    
    if (currentState == SETUP)
    {
      int buttonState = digitalRead(RIGHTMENU);
      if (buttonState == LOW)
      {
        if ((numDice >= MIN_NUM_DICE) && (numDice < MAX_NUM_DICE))
        {
          numDice++;
          printDice();
        }
      }
    }
}

void toggleUpFunc() {
    anyInterruptCounter++;
    int switchState = digitalRead(TOGGLEUP);
    if (currentState == SETUP)
    {
      if (switchState == LOW)
      {
        currentState = ROLL;
        Serial.println("Toggle up triggered.");
        rollDice();
      }
    }
}

void toggleDownFunc() {
    anyInterruptCounter++;
    int switchState = digitalRead(TOGGLEDOWN);
    if (currentState == SETUP)
    {
      if (switchState == LOW)
      {
        currentState = ROLL;
        Serial.println("Toggle up triggered.");
        rollDice();
      }
    }
}

void tiltFunc() {
    anyInterruptCounter++;
}

void encoderUpdate() {
  encoder.tick(); // just call tick() to check the state.
  anyInterruptCounter++;
}


void disableAllInterrupts()
{
  disableInterrupt(LEFTMENU);
  disableInterrupt(MIDDLEMENU);
  disableInterrupt(RIGHTMENU);
  disableInterrupt(TOGGLEUP);
  disableInterrupt(TOGGLEDOWN);
  disableInterrupt(TILTSWITCH);
  disableInterrupt(ROTARYPINA);
  disableInterrupt(ROTARYPINA);
}

void enableAllInterrupts()
{
  // Enable the interrupts for the initial state
  enableInterrupt(LEFTMENU, leftMenuFunc, CHANGE);
  enableInterrupt(MIDDLEMENU, middleMenuFunc, CHANGE);
  enableInterrupt(RIGHTMENU, rightMenuFunc, CHANGE);
  enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  enableInterrupt(TOGGLEUP, toggleUpFunc, CHANGE);
  enableInterrupt(TOGGLEDOWN, toggleDownFunc, CHANGE);
  
  // Rotary Encoder
  enableInterrupt(ROTARYPINA, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYPINB, encoderUpdate, CHANGE);
}
