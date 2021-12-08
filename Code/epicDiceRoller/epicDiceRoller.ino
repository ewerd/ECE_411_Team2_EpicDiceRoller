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
#define ROTARYBTN         A0
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
void encoderBtn();

// Run time Functions
void printDice();
void printRoll();
void printRolledDice();
void printDisplay();
void changeDice(int pos);
void changeColor(int pos);
void custom32Message(char *stringInput, unsigned long delayTime);
void myDelay(unsigned long delayTime);
int setCustomChar(int val);
void critical1();
void natural20();
void yahtzee();
void setColors(int Red, int Green, int Blue);
void Print(char *string);
void Write(int val);
void WriteS(int val);
void moveCursor(int row, int col);

// Object definitions for encoder and LCD screen
RotaryEncoder encoder(ROTARYPINA, ROTARYPINB);

AltSoftSerial OpenLCD(7, LCDSCREENRX); //RX, TX

// Enum to determine what state we are in, used in global interrupts
typedef enum _device_state_e {SETUP, ROLL, PREVIOUS, DISP} state_t;
state_t currentState = SETUP;

// Contrast definition for the LCD Screen
byte contrast = 1; //Lower is more contrast. 0 to 5 works for most displays.

// Globals
const int dice[NUM_POSSIBLE_DICE] = {2, 3, 4, 6, 8, 10, 12, 20, 100};
int rolledDice[10] = {0};
int diceIndex = 7;
int numDice = 1;
int diceRoll = 0;
int previousDiceRoll = diceRoll;
int tiltCount = 0;
int Rval = 157, Gval = 187, Bval = 217;
bool rlDice = false;
bool pntRoll = false;
bool pntDice = false;
bool pntDisplay = false;
bool pntRolledDice = false;
bool canTilt = false;
bool canTiltMessage = false;
bool randomMessage = false;
bool loaded = false;
bool R = false, G = false, B = false;

void setup() {

  // Pins without external pull ups
  pinMode(ROTARYPINA, INPUT_PULLUP);
  pinMode(ROTARYPINB, INPUT_PULLUP);

  // Pins with external pull ups
  pinMode(LEFTMENU, INPUT);
  pinMode(MIDDLEMENU, INPUT);
  pinMode(RIGHTMENU, INPUT);
  pinMode(ROTARYBTN, INPUT);
  pinMode(TILTSWITCH, INPUT);
  pinMode(TOGGLEUP, INPUT);
  pinMode(TOGGLEDOWN, INPUT);

  // Enable the interrupts for the initial state
  enableInterrupt(ROTARYPINA, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYPINB, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYBTN, encoderBtn, CHANGE);
  enableInterrupt(LEFTMENU, leftMenuFunc, CHANGE);
  enableInterrupt(MIDDLEMENU, middleMenuFunc, CHANGE);
  enableInterrupt(RIGHTMENU, rightMenuFunc, CHANGE);
  enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  enableInterrupt(TOGGLEUP, toggleUpFunc, CHANGE);
  enableInterrupt(TOGGLEDOWN, toggleDownFunc, CHANGE);

  OpenLCD.begin(9600); //Start communication with OpenLCD
//  OpenLCD.write('|');
//  OpenLCD.write(13);
//  exit(1);
  Entropy.initialize();
  WriteS(47); //Send disable messages command
  WriteS(24); //Send contrast command
  Write(contrast);
  WriteS(6);
  WriteS(4);
  
  Rval = 157;
  Gval = 187;
  Bval = 217;
  
  // set background
  setColors(Rval, Bval, Gval);
  
  // Record custom characters
  WriteS(27);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B00000);

  WriteS(28);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  WriteS(29);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  WriteS(30);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  WriteS(31);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  WriteS(32);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);
  
  WriteS(33);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  delay(3000);
  
  // Opening Message
  custom32Message("EPIC DICE ROLLER May the rolls  ", 1500);
  custom32Message("EPIC DICE ROLLER be with you!   ", 1500);
  // Initialize Screen
  printDice();
}

// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine of the tilt switch.
// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine of the tilt switch.
void loop() {
  // for some reason fewer errors when nesting functions in interrupt handlers less...
  if (canTilt)
    enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  else
    disableInterrupt(TILTSWITCH);
  
  if (rlDice)
  {
    rlDice = false;
    rollDice();
  }
  else if (pntDice) 
  {
    pntDice = false;
    printDice();
  }
  else if (pntRolledDice)
  {
    pntRolledDice = false;
    printRolledDice();
  }
  else if (pntRoll) 
  {
    pntRoll = false;
    printRoll();
  }
  else if (pntDisplay)
  {
    pntDisplay = false;
    printDisplay();
  }
  
  if (currentState == SETUP)
  {
    // Check Rotary Encoder
    changeDice();
    if (canTiltMessage)
    {
      custom32Message("  Shaking Mode    Activated!", 750);
      canTilt = true;
      canTiltMessage = false;
    }
  }

  else if (currentState == PREVIOUS)
  {
    
  }
  
  else if (currentState == DISP)
  {
    changeColor();
    if (randomMessage)
    {
      custom32Message("Use buttons to  set RGB display!", 1250);
      custom32Message("Or shake for a  random color!", 1250);
      canTilt = true;
      pntDisplay = true;
      randomMessage = false;
    }
  }
}

// Runtime Function Code
void printDice()
{
  disableAllInterrupts();
  char setupOutput[32] = "";
  char numDice_s[10] = "";
  char diceValue_s[10] = "";
  R = G = B = false;

  itoa(numDice, numDice_s, 10);
  strcat(setupOutput, "      ");
  strcat(setupOutput, numDice_s);
  strcat(setupOutput, "d");
  itoa(dice[diceIndex],diceValue_s, 10);
  strcat(setupOutput, diceValue_s);

  int diceLineLength = strlen(setupOutput);
  if (diceLineLength < 16)
  {
    for (unsigned int i = 0; i < 16 -diceLineLength; i++)
    {
      strcat(setupOutput, " ");
    }
  }

  strcat(setupOutput, " -d   Roll   +d ");
  
  // Print ndm dice and menu options
  custom32Message(setupOutput, 0);
  enableAllInterrupts();
}

void printRolledDice()
{
  disableAllInterrupts();
  char diceVal[5] = "";
  int outputLength = 0;
  int i = 0;
  char diceOutput[32] = "";

  for (i = 0; i < 5; i++)
  {
    if (rolledDice[i] != 0)
    {
      itoa(rolledDice[i], diceVal, 10);
      strcat(diceOutput, diceVal);
      strcat(diceOutput, " "); 
    }
  }
  
  outputLength = strlen(diceOutput);
  
  if (outputLength < 16)
  {
    for (i = 0; i < 16 - outputLength; i++)
    {
      strcat(diceOutput, " ");
    }
  }
  
  for (i = 5; i < 10; i++)
  {
    if (rolledDice[i] != 0)
    {
      itoa(rolledDice[i], diceVal, 10);
      strcat(diceOutput, diceVal);
      strcat(diceOutput, " "); 
    }
  }
  
  outputLength = strlen(diceOutput);
  
  if (outputLength < 32)
  {
    for (i = 0; i < 32 - outputLength; i++)
    {
      strcat(diceOutput, " ");
    }
  }

  custom32Message(diceOutput, 1000);
  moveCursor(1, 12);
  Print("Back");
  enableAllInterrupts();
}

void rollDice()
{
  disableAllInterrupts();
  int same = 0;
  
  diceRoll = 0;
  for (int i = 0; i < MAX_NUM_DICE; i++) 
  {
    if (i < numDice)
    {
      rolledDice[i] = Entropy.random(1, dice[diceIndex] + 1);
    if (loaded && i == 0)
    {
      rolledDice[i] = dice[diceIndex];
      loaded = false;
    }
    
    // Check for a Yahtzee
    if (diceRoll == rolledDice[i])
      same++;
      
    diceRoll = diceRoll + rolledDice[i];
      
    }
    else
    {
      rolledDice[i] = 0;
    }
  }

  

  if (numDice == 1 && rolledDice[0] == 20 && dice[diceIndex] == 20)
    natural20();
  else if (numDice == 1 && rolledDice[0] == 1 && dice[diceIndex] == 20)
    critical1();
  else if (numDice > 1 && same == numDice)
    yahtzee();

  tiltCount = 0;
  canTilt = false;
  pntRoll = true;
  enableAllInterrupts();
}

void printRoll()
{
  disableAllInterrupts();
  char numDice_s[5] = "";
  char diceValue_s[5] = "";
  char rollValue_s[5] = "";
  char rollOutput[32] = "";
  int outputLength = 0; 
  itoa(numDice, numDice_s, 10);
  strcat(rollOutput, numDice_s);

  strcat(rollOutput, "d");

  itoa(dice[diceIndex], diceValue_s, 10);
  strcat(rollOutput, diceValue_s);

  strcat(rollOutput, " = ");

  itoa(diceRoll, rollValue_s, 10);
  strcat(rollOutput, rollValue_s);

  outputLength = strlen(rollOutput);
  if (outputLength < 16)
  {
    for (unsigned int i = 0; i < 16 - outputLength; i++)
    {
      strcat(rollOutput, " ");
    }
  }

  strcat(rollOutput, "Dice  Home  Redo");

  custom32Message(rollOutput, 0);
  
  enableAllInterrupts();
}

void printDisplay()
{
  disableAllInterrupts();
  if (!R && !G && !B)
  {
    custom32Message("Set RGB Display!R      G      B ", 0);
    moveCursor(1, 1);
    WriteS(setCustomChar(Rval - 128));
    moveCursor(1, 8);
    WriteS(setCustomChar(Gval - 158));
    moveCursor(1, 15);
    WriteS(setCustomChar(Bval - 188));
  }
  if (R)
  {
    moveCursor(1, 1);
    WriteS(setCustomChar(Rval - 128));
  }

  else if (G)
  {
    moveCursor(1, 8);
    WriteS(setCustomChar(Gval - 158));
  }

  else if (B)
  {
    moveCursor(1, 15);
    WriteS(setCustomChar(Bval - 188));
  }
  // set background
  setColors(Rval, Bval, Gval);
  enableAllInterrupts();
}

void changeDice()
{
  static int dicePos = 0;
  int newPos = encoder.getPosition();
  if (newPos != dicePos) {
    if (newPos > dicePos) { // Increment position
      diceIndex = (diceIndex + 1) % NUM_POSSIBLE_DICE;
    }
    else if (newPos < dicePos) { // Decrement position
      if (diceIndex == 0) { // Don't go negative while decrementing, go to top of encoder
        diceIndex = NUM_POSSIBLE_DICE - 1;
      }
      else { // Decrement position
        diceIndex = (diceIndex - 1) % NUM_POSSIBLE_DICE;
      }
    }
    
    dicePos = newPos;
    pntDice = true;
  }
}

void changeColor()
{
  static int colorPos = 0;
  if (R || G || B)
  {
    int newPos = encoder.getPosition();
    if (newPos != colorPos) {
      if (newPos > colorPos) { // Increment position
        if (R && Rval < 153)
          Rval = (Rval + 4);
        else if (G && Gval < 183)
          Gval = (Gval + 4);
        else if (B && Bval < 213)
          Bval = (Bval + 4);
      }
      else if (newPos < colorPos) { // Decrement position
        if (R && Rval > 131)
          Rval  = Rval - 4;
        else if (G && Gval > 161)
          Gval  = Gval - 4;
        else if (B && Bval > 191)
          Bval = Bval - 4;
      }
      canTilt = false;
      pntDisplay = true;
      colorPos = newPos;
    }
  }
}

void custom32Message(char *stringInput, unsigned long delayTime)
{
  int stringLen = strlen(stringInput);
  char customOutput[48] = "";
  strcpy(customOutput, stringInput);
  int i = 0;

  WriteS('-');
  moveCursor(0, 0);

  for (i = 0; i < 32 - stringLen; i++)
  {
    strcat(customOutput, " ");
  }
  
  Print(customOutput);
  myDelay(delayTime);
  moveCursor(0, 0);
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
  for (int x = 0; x < 3; x++)
  {
    // clear screen
    setColors(128, 187, 188);
    custom32Message("   NATURAL 20                   ", 350);
    setColors(128, 158, 217);
    custom32Message("                   NATURAL 20   ", 350);
  }
  setColors(Rval, Bval, Gval);
}

void critical1()
{
  for (int x = 0; x < 3; x++) 
  {
    // clear screen
    setColors(157, 158, 188);
    custom32Message("   CRITICAL 1                   ", 350);
    setColors(157, 158, 200);
    custom32Message("                   CRITICAL 1   ", 350);
  }
  setColors(Rval, Bval, Gval);
}

void yahtzee()
{
  for (int x = 0; x < 3; x++) 
  {
    // clear screen
    setColors(157, 187, 188);
    custom32Message("   YAHTZEE!!!                   ", 350);
    setColors(157, 158, 217);
    custom32Message("                   YAHTZEE!!!   ", 350);
  }
  setColors(Rval, Bval, Gval);
}

void setColors(int Red, int Green, int Blue)
{
  WriteS(Red);
  WriteS(Green);
  WriteS(Blue);
}

// wrapper functions to only allow writes or prints when LCD is ready.
void Print(char *string)
{
  while (OpenLCD.available() != 0)
  {
  }
  OpenLCD.print(string);
}

void Write(int val)
{
  while (OpenLCD.available() != 0)
  {
  }
  OpenLCD.write(val);
}

// write a setting command
void WriteS(int val)
{
  while (OpenLCD.available() != 0)
  {
  }
  OpenLCD.write(124);
  while (OpenLCD.available() != 0)
  {
  }
  OpenLCD.write(val);
}

void moveCursor(int row, int col)
{
  Write(254);
  Write(128 + (row*64) + col);
}

// Interrupt Function Code
void leftMenuFunc() {
  int buttonState = digitalRead(LEFTMENU);
  
  if (buttonState == LOW)
  {
    if (currentState == SETUP)
    {
        if ((numDice > MIN_NUM_DICE) && (numDice <= MAX_NUM_DICE))
        {
          numDice--;
          pntDice = true;
        }
    }
    else if (currentState == ROLL)
    {
      currentState = PREVIOUS;
      pntRolledDice = true;
    }
    else if (currentState == DISP)
    {
      R = true;
      G = false;
      B = false;
    }
  }
}

void middleMenuFunc() {
  int buttonState = digitalRead(MIDDLEMENU);
  
  if (buttonState == LOW)
  {
    if (currentState == SETUP)
    {
      currentState = ROLL;
      rlDice = true;
    }
    else if (currentState == ROLL)
    {
      currentState = SETUP;
      pntDice = true;
    }
    else if (currentState == DISP)
    {
      R = false;
      G = true;
      B = false;
    }
  }
}

void rightMenuFunc() {
  int buttonState = digitalRead(RIGHTMENU);
    
  if (buttonState == LOW)
  {
    if (currentState == SETUP)
    {
      if ((numDice >= MIN_NUM_DICE) && (numDice < MAX_NUM_DICE))
      {
        numDice++;
        pntDice = true;
      }
    }
    else if (currentState == ROLL)
    {
      rlDice = true;
    }
    else if (currentState == PREVIOUS)
    {
      currentState = ROLL;
      pntRoll = true;
    }
    else if (currentState == DISP)
    {
      R = false;
      G = false;
      B = true;
    }
  }
}

void toggleUpFunc() {
  int switchState = digitalRead(TOGGLEUP);
  if (currentState == SETUP && switchState == LOW)
  {
    currentState = DISP;
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
    if (currentState == DISP)
    {
      if (tiltCount == 400)
      {
        Rval = Entropy.random(128,157);
        Gval = Entropy.random(158,187);
        Bval = Entropy.random(188,217);
        setColors(Rval, Bval, Gval);
        pntDisplay = true;
        tiltCount = 0;
      }
    }
    else if (currentState == SETUP)
    {
      if (tiltCount % 150 == 0)
      {
        setColors(Entropy.random(128,157), Entropy.random(158,187), Entropy.random(188,217));
      }
      if (tiltCount > 1000)
      {
        disableInterrupt(TILTSWITCH);
        setColors(Rval, Bval, Gval);
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

void encoderBtn()
{
  int buttonState = digitalRead(ROTARYBTN);
  
  if (buttonState == LOW)
  {
    if (digitalRead(TILTSWITCH) != LOW)
      loaded = true;
    if (currentState == SETUP)
    {
      currentState = ROLL;
      rlDice = true;
    }
    else if (currentState == ROLL)
    {
      rlDice = true;
    }
  }
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
  disableInterrupt(ROTARYBTN);
}

void enableAllInterrupts()
{
  // Enable the interrupts for the initial state
  enableInterrupt(ROTARYPINA, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYPINB, encoderUpdate, CHANGE);
  enableInterrupt(ROTARYBTN, encoderBtn, CHANGE);
  enableInterrupt(LEFTMENU, leftMenuFunc, CHANGE);
  enableInterrupt(MIDDLEMENU, middleMenuFunc, CHANGE);
  enableInterrupt(RIGHTMENU, rightMenuFunc, CHANGE);
  enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  enableInterrupt(TOGGLEUP, toggleUpFunc, CHANGE);
  enableInterrupt(TOGGLEDOWN, toggleDownFunc, CHANGE);
}
