/**
   epicDiceRoller
   by Team 2
   Braden Harwood, Stephen Short, Michael Weston, Drew Seidel

*/

#include <AltSoftSerial.h>
#include <EnableInterrupt.h>
#include <RotaryEncoder.h>
#include <Entropy.h>

#define NUM_POSSIBLE_DICE 9
#define MAX_NUM_DICE      10
#define MIN_NUM_DICE      1

// Define IO Pins
#define ROTARYPINA        3
#define ROTARYPINB        2 // May need to switch these if the direction is backwards
#define LEFTMENU          5
#define MIDDLEMENU        6
#define TILTSWITCH        7
#define LCDSCREENRX       9
#define ROTARYBUTTON      A0
#define RIGHTMENU         A1
#define TOGGLEUP          A2
#define TOGGLEDOWN        A3

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
void diceExplode();
void natural20();

// Object definitions for encoder and LCD screen
RotaryEncoder encoder(ROTARYPINA, ROTARYPINB);

AltSoftSerial OpenLCD(7, LCDSCREENRX); //RX, TX

// Enum to determine what state we are in, used in global interrupts
typedef enum _device_state_e {SETUP, ROLL, PREVIOUS, DISP} state_t;
state_t currentState = SETUP;

// Contrast definition for the LCD Screen
byte contrast = 1; //Lower is more contrast. 0 to 5 works for most displays.

// Global Dice Array that holds all of the values of dice that are available
const int dice[NUM_POSSIBLE_DICE] = {2, 3, 4, 6, 8, 10, 12, 20, 100};
int rolledDice[10] = {0};
int diceIndex = 7;
int numDice = 1;
int diceRoll = 0;
int previousDiceRoll = diceRoll;
int tiltCount = 0;
int Rval = 0, Gval = 0, Bval = 0;
bool rlDice = false;
bool pntRoll = false;
bool pntDice = false;
bool pntDisplay = false;
bool canTilt = false;
bool canTiltMessage = false;
bool prevMessage = false;
bool randomMessage = false;
bool shakeIt = false;
bool R = false, G = false, B = false;

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
  enableInterrupt(ROTARYPINA, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYPINB, encoderUpdate, CHANGE); // commented out because noise from turning the encoder either direction is likely enough to trigger this interrupt
  enableInterrupt(LEFTMENU, leftMenuFunc, CHANGE);
  enableInterrupt(MIDDLEMENU, middleMenuFunc, CHANGE);
  enableInterrupt(RIGHTMENU, rightMenuFunc, CHANGE);
  enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  enableInterrupt(TOGGLEUP, toggleUpFunc, CHANGE);
  enableInterrupt(TOGGLEDOWN, toggleDownFunc, CHANGE);

  OpenLCD.begin(9600); //Start communication with OpenLCD
//  OpenLCD.write('|'); //Put LCD into setting mode
//  OpenLCD.write(13); //Send contrast command
//  exit(1);
  Entropy.initialize();
  //Send contrast setting
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(47); //Send disable messages command
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(24); //Send contrast command
  OpenLCD.write(contrast);
  OpenLCD.write(124); // set line width and height
  OpenLCD.write(6);
  OpenLCD.write(124);
  OpenLCD.write(4);
  
  
  Rval = 157;
  Gval = 187;
  Bval = 217;
  // set background
  OpenLCD.write('|'); 
  OpenLCD.write(Rval); 
  OpenLCD.write('|'); 
  OpenLCD.write(Gval); 
  OpenLCD.write('|'); 
  OpenLCD.write(Bval); 
  

  // Record custom characters
  OpenLCD.write('|');
  OpenLCD.write(27);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write(31);
  OpenLCD.write((int)0);

  OpenLCD.write('|');
  OpenLCD.write(28);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write((int)0);

  OpenLCD.write('|');
  OpenLCD.write(29);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write((int)0);

  OpenLCD.write('|');
  OpenLCD.write(30);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write((int) 0);

  OpenLCD.write('|');
  OpenLCD.write(31);
  OpenLCD.write((int)0);
  OpenLCD.write((int)0);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write((int) 0);

  OpenLCD.write('|');
  OpenLCD.write(32);
  OpenLCD.write((int)0);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write((int)0);
  
  OpenLCD.write('|');
  OpenLCD.write(33);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write(31);
  OpenLCD.write((int)0);

  delay(2000);

  OpenLCD.write('|'); //Setting character
  OpenLCD.write('-'); //Clear display
  OpenLCD.write(254);
  OpenLCD.write(128 + 0 + 0);
  
  // Opening Message
  OpenLCD.print("EPIC DICE ROLLER  May the rolls ");
  delay(1500);
  OpenLCD.print("EPIC DICE ROLLER                ");
  delay(500);
  OpenLCD.print("EPIC DICE ROLLER  be with you!  ");
  delay(1500);
  // Initialize Screen
  printDice();
}


void printDice()
{
  char line1[16] = "      ";
  char numDice_s[5];
  char diceValue_s[5];
  const char menuLine[17] = " -d   Roll   +d ";
  char output[32];
  disableAllInterrupts();
  if (currentState == SETUP)
  {
    R = G = B = false;
    itoa(numDice, numDice_s, 10);
    strcat(line1, numDice_s);
    
    strcat(line1, "d");
    
    itoa(dice[diceIndex],diceValue_s, 10);
    strcat(line1, diceValue_s);
    
    int line1Length = strlen(line1);
    if (line1Length < 16)
    {
      for (unsigned int i = 0; i < 16-line1Length; i++)
      {
        strcat(line1, " ");
      }
    }
    
    strcpy(output, line1);
    strcat(output, menuLine);

    OpenLCD.write('|'); //Setting character
    OpenLCD.write('-'); //Clear display
    OpenLCD.write(254);
    OpenLCD.write(128 + 0 + 0);
    // Print ndm dice and menu options
    OpenLCD.print(output);
   
  }
  else if (currentState == PREVIOUS)
  {
    char diceVal[3] = "";
    //itoa(numDice, numDice_s, 10);
    char line1[16] = "";
    char line2[16] = "";
    int line1Length = 0;
    int line2Length = 0;

    for (unsigned int i = 0; i < 5; i++)
    {
      if (rolledDice[i] != 0)
      {
        itoa(rolledDice[i], diceVal, 10);
        strcat(line1, diceVal);
        strcat(line1, " "); 
      }
    }
    for (unsigned int i = 5; i < 10; i++)
    {
      if (rolledDice[i] != 0)
      {
        itoa(rolledDice[i], diceVal, 10);
        strcat(line2, diceVal);
        strcat(line2, " "); 
      }
    }

    line1Length = strlen(line1);
    if (line1Length < 16)
    {
      for (unsigned int i = 0; i < 16 - line1Length; i++)
      {
        strcat(line1, " ");
      }
    }

    line2Length = strlen(line2);
    if (line2Length < 16)
    {
      for (unsigned int i = 0; i < 16 - line1Length; i++)
      {
        strcat(line2, " ");
      }
    }
    
    strcpy(output, line1);
    strcat(output, line2);
  
    OpenLCD.write('|'); //Setting character
    OpenLCD.write('-'); //Clear display
    OpenLCD.write(254);
    OpenLCD.write(128 + 0 + 0);
    // Print ndm dice and menu options
    OpenLCD.print(output);
  }
  enableAllInterrupts();
  
}

void rollDice()
{
    disableAllInterrupts();

    currentState = ROLL;
    
    previousDiceRoll = diceRoll;
    diceRoll = 0;
    for (int i = 0; i < MAX_NUM_DICE; i++) {
      if (i < numDice)
      {
        rolledDice[i] = Entropy.random(1, dice[diceIndex] + 1);
        diceRoll = diceRoll + rolledDice[i];
      }
      else
      {
        rolledDice[i] = 0;
      }
    }
    OpenLCD.write(254);
    OpenLCD.write(128 + 0 + 0);
    OpenLCD.write('|'); //Setting character
    OpenLCD.write('-'); //Clear display
    tiltCount = 0;
    canTilt = false;
    pntRoll = true;
}

void printRoll()
{
  char line1[16] = "";
  char numDice_s[5];
  char diceValue_s[5];
  char rollValue_s[5];
  const char menuLine[17] = "Dice  Home  Redo";
  char output[32];
  if (currentState == ROLL)
  {
    itoa(numDice, numDice_s, 10);
    strcat(line1, numDice_s);

    strcat(line1, "d");

    itoa(dice[diceIndex], diceValue_s, 10);
    strcat(line1, diceValue_s);

    strcat(line1, " = ");

    itoa(diceRoll, rollValue_s, 10);
    strcat(line1, rollValue_s);

    int line1Length = strlen(line1);
    if (line1Length < 16)
    {
      for (unsigned int i = 0; i < 16 - line1Length; i++)
      {
        strcat(line1, " ");
      }
    }

    strcpy(output, line1);
    strcat(output, menuLine);

    OpenLCD.write('|'); //Setting character
    OpenLCD.write('-'); //Clear display
    OpenLCD.write(254);
    OpenLCD.write(128 + 0 + 0);
    // Print ndm dice and menu options
    OpenLCD.print(output);
    enableAllInterrupts();
  }
}

void printDisplay()
{
  disableAllInterrupts();
  char diceVal[3] = "";
  //itoa(numDice, numDice_s, 10);
  char line1[16] = "Set RGB Display!";
  char line2[16] = "R      G      B ";
  char output[32] = "";
  int line1Length = 0;
  int line2Length = 0;
  
  
  strcpy(output, line1);
  strcat(output, line2);

  OpenLCD.write('|'); //Setting character
  OpenLCD.write('-'); //Clear display
  OpenLCD.write(254);
  OpenLCD.write(128 + 0 + 0);
  // Print ndm dice and menu options
  OpenLCD.print(output);

  OpenLCD.write(254);
  OpenLCD.write(128 + 64 + 1);
  OpenLCD.write('|');
  OpenLCD.write(setCustomChar(Rval - 128));
  
  OpenLCD.write(254);
  OpenLCD.write(128 + 64 + 8);
  OpenLCD.write('|');
  OpenLCD.write(setCustomChar(Gval - 158));
  
  OpenLCD.write(254);
  OpenLCD.write(128 + 64 + 15);
  OpenLCD.write('|');
  OpenLCD.write(setCustomChar(Bval - 188));

  // set background
  OpenLCD.write('|'); 
  OpenLCD.write(Rval); 
  OpenLCD.write('|'); 
  OpenLCD.write(Gval); 
  OpenLCD.write('|'); 
  OpenLCD.write(Bval); 
  
  enableAllInterrupts();
}


// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine of the tilt switch.
// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine of the tilt switch.
void loop() {
  // for some reason fewer errors when nesting functions in interrupt handlers less...
  while (OpenLCD.available() < 0)
  {
    delay(50);
  }
  if (shakeIt)
  {
    OpenLCD.print("SHAKE IT! ");
    shakeIt = false;
    return;
  }
  if (rlDice)
  {
    rollDice();
    rlDice = false;
  }
  if (pntDice) 
  {
    printDice();
    pntDice = false;
  }
  else if (pntRoll) 
  {
    printRoll();
    pntRoll = false;
  }
  else if (pntDisplay)
  {
    printDisplay();
    pntDisplay = false;
  }
  static int pos = 0;
  if (currentState == SETUP)
  {
    // Check Rotary Encoder
    int newPos = encoder.getPosition();
    if (newPos != pos) {
      if (newPos > pos) { // Increment position
        diceIndex = (diceIndex + 1) % NUM_POSSIBLE_DICE;
      }
      else if (newPos < pos) { // Decrement position
        if (diceIndex == 0) { // Don't go negative while decrementing, go to top of encoder
          diceIndex = NUM_POSSIBLE_DICE - 1;
        }
        else { // Decrement position
          diceIndex = (diceIndex - 1) % NUM_POSSIBLE_DICE;
        }
      }
      
      pos = newPos;
      pntDice = true;
    }
    if (canTiltMessage)
    {
        OpenLCD.write('|');
        OpenLCD.write('-');
        OpenLCD.print("  Shaking Mode    Activated!");
        myDelay((unsigned long)1500);
        canTiltMessage = false;
    }
  }
  
  else if (currentState == PREVIOUS)
  {
    if (prevMessage)
    {
      myDelay((unsigned long)1000);
      OpenLCD.write(254);
      OpenLCD.write(128 + 64 + 12);
      OpenLCD.print("Back");
      prevMessage = false;
    }
  }
  else if (currentState == DISP)
  {
    if (R || G || B)
    {
      int newPos = encoder.getPosition();
      if (newPos != pos) {
        if (newPos > pos) { // Increment position
          if (R && Rval < 153)
            Rval = (Rval + 4);
          else if (G && Gval < 183)
            Gval = (Gval + 4);
          else if (B && Bval < 213)
            Bval = (Bval + 4);
        }
        else if (newPos < pos) { // Decrement position
          if (R && Rval > 131)
            Rval  = Rval - 4;
          else if (G && Gval > 161)
            Gval  = Gval - 4;
          else if (B && Bval > 191)
            Bval = Bval - 4;
        }
        canTilt = false;
        pntDisplay = true;
        pos = newPos;
      }
    }
    if (randomMessage)
    {
      OpenLCD.write('|');
      OpenLCD.write('-');
      OpenLCD.print("Use buttons to  set RGB display!");
      myDelay((unsigned long)2000);
      OpenLCD.write('|');
      OpenLCD.write('-');
      OpenLCD.print("Or shake for a  random color!");
      myDelay((unsigned long)2000);
      pntDisplay = true;
      randomMessage = false;
    }
  }
}

// Interrupt Function Code
void leftMenuFunc() {
    int buttonState = digitalRead(LEFTMENU);
    if (currentState == SETUP)
    {
      if (buttonState == LOW)
      {
        if ((numDice > MIN_NUM_DICE) && (numDice <= MAX_NUM_DICE))
        {
          numDice--;
          pntDice = true;
        }
      }
    }
    else if (currentState == ROLL)
    {
      if (buttonState == LOW)
      {
        currentState = PREVIOUS;
        pntDice = true;
        prevMessage = true;
      }
    }
    else if (currentState == DISP)
    {
      if (buttonState == LOW)
      {
        R = true;
        G = false;
        B = false;
      }
    }
}

void middleMenuFunc() {
    int buttonState = digitalRead(MIDDLEMENU);
    if (currentState == SETUP)
    {
      if (buttonState == LOW)
      {
        currentState = ROLL;
        rlDice = true;
      }
    }
    else if (currentState == ROLL)
    {
      if (buttonState == LOW)
      {
        currentState = SETUP;
        pntDice = true;
      }
    }
    else if (currentState == DISP)
    {
      if (buttonState == LOW)
      {
        G = true;
        R = false;
        B = false;
      }
    }
}

void rightMenuFunc() {
    int buttonState = digitalRead(RIGHTMENU);
    
    if (currentState == SETUP)
    {
      if (buttonState == LOW)
      {
        if ((numDice >= MIN_NUM_DICE) && (numDice < MAX_NUM_DICE))
        {
          numDice++;
          pntDice = true;
        }
      }
    }
    else if (currentState == ROLL)
    {
      if (buttonState == LOW)
      {
        rlDice = true;
      }
    }
    else if (currentState == PREVIOUS)
    {
      if (buttonState == LOW)
      {
        currentState = ROLL;
        pntRoll = true;
      }
    }
    else if (currentState == DISP)
    {
      if (buttonState == LOW)
      {
        B = true;
        R = false;
        G = false;
      }
    }
}

void toggleUpFunc() {
    int switchState = digitalRead(TOGGLEUP);
    if (currentState == SETUP && switchState == LOW)
    {
      currentState = DISP;
      canTilt = true;
      randomMessage = true;
    }
    else
    {
      currentState = SETUP;
      pntDice = true;
      R = G = B = false;
      canTilt = false;
    }
}

void toggleDownFunc() {
    int switchState = digitalRead(TOGGLEDOWN);
    if (currentState == SETUP && switchState == LOW)
    {
      canTilt = true;
      canTiltMessage = true;
    }
    else
    {
      currentState = SETUP;
      pntDice = true;
      R = G = B = false;
      canTilt = false;
    }
}

void tiltFunc() {
  if (canTilt)
  {
    tiltCount++;
    if (tiltCount % 200 == 0)
    {
      OpenLCD.write('|');
      Rval = Entropy.random(128,157);
      OpenLCD.write(Rval);
      OpenLCD.write('|');
      Gval = Entropy.random(158,187);
      OpenLCD.write(Gval);
      OpenLCD.write('|');
      Bval = Entropy.random(188,217);
      OpenLCD.write(Bval);
      if (currentState == DISP)
      {
        shakeIt = false;
        pntDisplay = true;
      }
    }
    if (currentState == SETUP)
    {
      if (tiltCount > 1000)
      {
        currentState = ROLL;
        tiltCount = 0;
        canTilt = false;
        rlDice = true;
      }
    }
  }
}

void encoderUpdate() {
  encoder.tick(); // just call tick() to check the state.
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
  disableInterrupt(ROTARYPINB);
}

void enableAllInterrupts()
{
  // Enable the interrupts for the initial state
  enableInterrupt(ROTARYPINA, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYPINB, encoderUpdate, CHANGE);
  enableInterrupt(LEFTMENU, leftMenuFunc, CHANGE);
  enableInterrupt(MIDDLEMENU, middleMenuFunc, CHANGE);
  enableInterrupt(RIGHTMENU, rightMenuFunc, CHANGE);
  enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  enableInterrupt(TOGGLEUP, toggleUpFunc, CHANGE);
  enableInterrupt(TOGGLEDOWN, toggleDownFunc, CHANGE);
}

// UI and graphics
void clearChar(int row, int col)
{
  row = row * 64;
  OpenLCD.write(254);
  OpenLCD.write(128 + row + col);
  OpenLCD.print(" ");
}

void myDelay(unsigned long delayTime)
{
  unsigned long current = millis();
  while ((millis() - current) < delayTime)
  {
    
  }
}

int setCustomChar(int val)
{
  if (val < 5)
    return 35;
  else if (val > 4 && val < 9)
    return 36;
  else if (val > 8 && val < 13)
    return 37;
  else if (val > 12 && val < 17)
    return 38;
  else if (val > 16 && val < 21)
    return 39;
  else if (val > 20 && val < 25)
    return 40;
  else if (val > 24 && val < 31)
    return 41;
}

void natural20()
{
}
