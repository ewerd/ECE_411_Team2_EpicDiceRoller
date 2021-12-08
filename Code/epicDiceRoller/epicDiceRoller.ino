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
void printRolledDice();
void printDisplay();
void changeDice();
void changeColor();
void customMessage(char *string, unsigned long delayTime);
void clearChar(int row, int col);
void myDelay(unsigned long delayTime);
int setCustomChar(int val);
void critical1();
void natural20();
void yahtzee();
void Print(char *string);
void Write(int val);

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
bool canTilt = false;
bool canTiltMessage = false;
bool prevMessage = false;
bool randomMessage = false;
bool shakeIt = false;
bool R = false, G = false, B = false;
bool nat20 = false, crit1 = false;

void setup() {

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
  enableInterrupt(ROTARYPINB, encoderUpdate, CHANGE);
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
  Write('|'); //Put LCD into setting mode
  Write(47); //Send disable messages command
  Write('|'); //Put LCD into setting mode
  Write(24); //Send contrast command
  Write(contrast);
  Write(124); // set line width and height
  Write(6);
  Write(124);
  Write(4);
  
  Rval = 157;
  Gval = 187;
  Bval = 217;
  
  // set background
  Write('|'); 
  Write(Rval); 
  Write('|'); 
  Write(Gval); 
  Write('|'); 
  Write(Bval); 
  
  // Record custom characters
  Write('|');
  Write(27);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B00000);

  Write('|');
  Write(28);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  Write('|');
  Write(29);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  Write('|');
  Write(30);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  Write('|');
  Write(31);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  Write('|');
  Write(32);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);
  
  Write('|');
  Write(33);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  delay(2000);

  Write('|'); //Setting character
  Write('-'); //Clear display
  delay(500);
  Write(254);
  Write(128 + 0 + 0);
  
  // Opening Message
  Print("EPIC DICE ROLLER  May the rolls ");
  delay(1500);
  Print("EPIC DICE ROLLER  be with you!  ");
  delay(1500);
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
  {
    enableInterrupt(TILTSWITCH, tiltFunc, CHANGE);
  }
  else
  {
    disableInterrupt(TILTSWITCH);
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
    else if (crit1)
    {
      
    }
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
        Write('|');
        Write('-');
        Print("  Shaking Mode    Activated!");
        myDelay((unsigned long)500);
        pntDice = true;
        canTilt = true;
        canTiltMessage = false;
    }
  }
  
  else if (currentState == PREVIOUS)
  {
    if (prevMessage)
    {
      myDelay((unsigned long)1000);
      prevMessage = false;
      Write(254);
      Write(128 + 64 + 12);
      Print("Back");
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
      Write('|');
      Write('-');
      Print("Use buttons to  set RGB display!");
      myDelay((unsigned long)1500);
      Write('|');
      Write('-');
      Print("Or shake for a  random color!");
      myDelay((unsigned long)1500);
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
  myDelay(1000);
  char setupOutput[32] = "";
  char numDice_s[10];
  char diceValue_s[10];
  char diceLine[16] = "";
  const char menuLine[16] = " -d   Roll   +d ";
  R = G = B = false;

  itoa(numDice, numDice_s, 10);
  strcat(diceLine, "      ");
  strcat(diceLine, numDice_s);
  strcat(diceLine, "d");
  itoa(dice[diceIndex],diceValue_s, 10);
  strcat(diceLine, diceValue_s);

  int diceLineLength = strlen(diceLine);
  if (diceLineLength < 16)
  {
    for (unsigned int i = 0; i < 16 -diceLineLength; i++)
    {
      strcat(diceLine, " ");
    }
  }

  strcpy(setupOutput, diceLine);
  strcat(setupOutput, menuLine);

  Write('|'); //Setting character
  Write('-'); //Clear display
  Write(254);
  Write(128 + 0 + 0);
  // Print ndm dice and menu options
  Print(setupOutput);
  enableAllInterrupts();
}

void printRolledDice();
{
  disableAllInterrupts();
  char diceVal[5] = "";
  int line1Length = 0;
  int line2Length = 0;
  int i = 0;
  char line1[16] = "";
  char line2[16] = "";

  for (i = 0; i < 5; i++)
  {
    if (rolledDice[i] != 0)
    {
      itoa(rolledDice[i], diceVal, 10);
      strcat(line1, diceVal);
      strcat(line1, " "); 
    }
  }
  for (i = 5; i < 10; i++)
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
    for (i = 0; i < 16 - line1Length; i++)
    {
      strcat(line1, " ");
    }
  }
  line2Length = strlen(line2);
  if (line2Length < 16)
  {
    for (i = 0; i < 16 - line2Length; i++)
    {
      strcat(line2, " ");
    }
  }
  Write('|'); //Setting character
  Write('-'); //Clear display
  Write(254);
  Write(128 + 0 + 0);
  Print(line1);
  Print(line2);
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

  if (numDice == 1 && rolledDice[0] == 20 && dice[diceIndex] == 20)
    natural20();
  else if (numDice == 1 && rolledDice[0] == 1 && dice[diceIndex] == 20)
    critical1();

  Write(254);
  Write(128 + 0 + 0);
  Write('|'); //Setting character
  Write('-'); //Clear display
  tiltCount = 0;
  canTilt = false;
  pntRoll = true;
}

void printRoll()
{
  disableAllInterrupts();
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

    Write('|'); //Setting character
    Write('-'); //Clear display
    Write(254);
    Write(128 + 0 + 0);
    // Print ndm dice and menu options
    Print(output);
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

  Write('|'); //Setting character
  Write('-'); //Clear display
  Write(254);
  Write(128 + 0 + 0);
  // Print ndm dice and menu options
  Print(output);

  Write(254);
  Write(128 + 64 + 1);
  Write('|');
  Write(setCustomChar(Rval - 128));
  
  Write(254);
  Write(128 + 64 + 8);
  Write('|');
  Write(setCustomChar(Gval - 158));
  
  Write(254);
  Write(128 + 64 + 15);
  Write('|');
  Write(setCustomChar(Bval - 188));

  // set background
  Write(124); 
  Write(Rval); 
  Write(124);
  Write(Gval); 
  Write(124); 
  Write(Bval); 
  enableAllInterrupts();
}

void clearChar(int row, int col)
{
  row = row * 64;
  Write(254);
  Write(128 + row + col);
  Print(" ");
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
    Write('|');
    Write('-');
    Print("   NATURAL 20                   ");
    Write('|');
    Write(128);
    Write('|');
    Write(187);
    Write('|');
    Write(188);
    myDelay(250);
    Write('|');
    Write('-');
    Print("                   NATURAL 20   ");
    Write('|');
    Write(128);
    Write('|');
    Write(158);
    Write('|');
    Write(217);
    myDelay(250);
  }
  Write('|');
  Write(Rval);
  Write('|');
  Write(Gval);
  Write('|');
  Write(Bval);
}

void critical1()
{
  for (int x = 0; x < 3; x++) 
  {
    // clear screen
    Write('|');
    Write('-');
    Print("   CRITICAL 1                   ");
    Write('|');
    Write(157);
    Write('|');
    Write(158);
    Write('|');
    Write(188);
    myDelay((unsigned long)250);
    Write('|');
    Write('-');
    Print("                   CRITICAL 1   ");
    Write('|');
    Write(157);
    Write('|');
    Write(158);
    Write('|');
    Write(200);
    myDelay((unsigned long)250);
  }
  Write('|');
  Write(Rval);
  Write('|');
  Write(Gval);
  Write('|');
  Write(Bval);
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
        R = false;
        G = true;
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
        Write('|');
        Rval = Entropy.random(128,157);
        Write(Rval);
        Write('|');
        Gval = Entropy.random(158,187);
        Write(Gval);
        Write('|');
        Bval = Entropy.random(188,217);
        Write(Bval);
        pntDisplay = true;
        tiltCount = 0;
      }
    }
    else if (currentState == SETUP)
    {
      if (tiltCount % 150 == 0)
      {
        Write('|');
        Write(Entropy.random(128,157));
        Write('|');
        Write(Entropy.random(158,187));
        Write('|');
        Write(Entropy.random(188,217));
      }
      if (tiltCount > 1000)
      {
        disableInterrupt(TILTSWITCH);
        Write('|');
        Write(Rval);
        Write('|');
        Write(Gval);
        Write('|');
        Write(Bval);
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
