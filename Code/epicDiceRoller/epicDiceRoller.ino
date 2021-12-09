/*
   epicDiceRoller
   by Team 2
   Braden Harwood, Stephen Short, Michael Weston, Drew Seidel
*/

#include <AltSoftSerial.h>
#include <EnableInterrupt.h>
#include <RotaryEncoder.h>
#include <Entropy.h>

#define NUM_POSSIBLE_DICE 9 //array size for number of dice options. 9 Current options {2, 3, 4, 6, 8, 10, 12, 20, 100}. Changeable
#define MAX_NUM_DICE      10  // Max number of dice per roll. 
#define MIN_NUM_DICE      1   //cannot roll 0 dice 

// Define IO Pins With Descriptive Names
#define ROTARYPINA        3   
#define ROTARYPINB        2   
#define LEFTMENU          5   
#define MIDDLEMENU        6   
#define TILTSWITCH        7
#define LCDSCREENRX       9
#define ROTARYBTN         A0
#define RIGHTMENU         A1
#define TOGGLEUP          A2
#define TOGGLEDOWN        A3

// Interrupt Functions
void leftMenuFunc();                                              //handles presses to left button 
void middleMenuFunc();                                            //handles presses to middle menu
void rightMenuFunc();                                             //handles presses to right menu
void tiltFunc();                                                  //handles tilt detections
void toggleUpFunc();                                              //handles toggle switch flipped up
void toggleDownFunc();                                            //handles toggle switch flipped down
void encoderUpdate();                                             //handles input to rotary encoder
void encoderBtn();                                                //handles input to rotary encoder button


// Run time Functions
void printDice();                                                 //sets up print dice screen. Allows user to see options to increase or decrement number of dice, and the current selection (i.e "3d6")
void printRoll();                                                 //handles printing the dice roll when a roll is deteced. Allows user to go see dice, home, or redo. 
void printRolledDice();                                           //prints rolled dice, called if user selects dice from above screen.
void printDisplay();                                              //prints LDC background change screen to LCD, as well as cursor position for user selection
void changeDice(int pos);                                         //changes dice size based on rotary positon
void changeColor(int pos);                                        //changes RGB color levels based on rotary position
void custom32Message(char *stringInput, unsigned long delayTime); //clears scree, sets cursor at 0,0, prints custom message
void myDelay(unsigned long delayTime);                            //used millis for better delay handling
int setCustomChar(int val);                                       //converts inputs to display custom characters 0-6 
void critical1();                                                 //function to handle extra printing for critical 1 roll
void natural20();                                                 //function to handle extra printing for natural 20 roll
void yahtzee();                                                   //function to handle extra printing for yahtzee roll
void setColors(int Red, int Green, int Blue);                     //set colors uses writeS to change colors based on user input in display mode. Inital values defined below
void Print(char *string);                                         //Prints string to LCD
void Write(int val);                                              //writes byte/character to LCD screen
void WriteS(int val);                                             //writes a setting to LCD screen. Writes "124" (command to put LCD in setting mode), followed by setting value. 
void moveCursor(int row, int col);                                //moves LCD cursor

// Object definitions for encoder and LCD screen
RotaryEncoder encoder(ROTARYPINA, ROTARYPINB);

AltSoftSerial OpenLCD(7, LCDSCREENRX); //RX, TX

// Enum to determine what state we are in, used in global interrupts
typedef enum _device_state_e {SETUP, ROLL, PREVIOUS, DISP} state_t;  //states of operation
state_t currentState = SETUP;

// Contrast definition for the LCD Screen
byte contrast = 1; //Lower is more contrast. 0 to 5 works for most displays.

// Globals
const int dice[NUM_POSSIBLE_DICE] = {2, 3, 4, 6, 8, 10, 12, 20, 100}; //d2, d3, d6...etc. 
int rolledDice[10] = {0};                                       //array of rolled dice numbers
int diceIndex = 7;                                              //dice index
int numDice = 1;                                                //initial num dice
int diceRoll = 0;                                               //random value of dice roll given size of the die
int previousDiceRoll = diceRoll;                                //keeps track of previous dice roll
int tiltCount = 0;                                              //counter to make sure enough shaking occurs to trigger a roll, or to set random color display in display mode
int Rval = 157, Gval = 187, Bval = 217;                         //initial backlight setting
bool rlDice = false;                                            //detects if roll was iniated. Go to rollDice
bool pntRoll = false;                                           //determines if printRoll should be called
bool pntDice = false;                                           //detects if printDice (dice option screen) should be printed to LCD
bool pntDisplay = false;                                        //determimes if printDisplay should be called
bool pntRolledDice = false;                                     //determines if user selected "Dice" using left button, from printDice screen.
bool canTilt = false;                                           //set if user has activated tilt mode with toggle switch
bool canTiltMessage = false;                                    //if high, print "Shake Mode Activated", set tilt mode interrupt activated
bool randomMessage = false;                                     //in display state, message will be given if toggle is fliped
bool loaded = false;                                            //false unless in display mode, then true. 
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
  Write(contrast);  //set initial contrast to 
  WriteS(6);
  WriteS(4);

  //Initial background lighting settings. 
  Rval = 157;
  Gval = 187;
  Bval = 217;
  
  // set background
  setColors(Rval, Bval, Gval);
  
  // Record custom characters: 
  //Documentation: https://learn.sparkfun.com/tutorials/avr-based-serial-enabled-lcds-hookup-guide/firmware-overview
  WriteS(27); //character 0
  Write(B00000);  
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B00000);

  WriteS(28);   //character 1
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  WriteS(29);   //character 2
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  WriteS(30);   //character 3
  Write(B00000);
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  WriteS(31);   //character 4
  Write(B00000);
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  WriteS(32);  //character 5
  Write(B00000);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);
  
  WriteS(33);  //character 6
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B11111);
  Write(B00000);

  delay(2000);  //delay 3 seconds
  
  // Opening Message
  custom32Message("EPIC DICE ROLLER May the rolls  ", 1500);
  custom32Message("EPIC DICE ROLLER be with you!   ", 1500);
  // Initialize Screen
  printDice(); //go to main screen where user has option to roll dice, change dice size, and number of dice. 
}

// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine of the tilt switch.
// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine of the tilt switch.
void loop() {
  // for some reason fewer errors when nesting functions in interrupt handlers less...
  
  //check tilt
  if (canTilt)
    enableInterrupt(TILTSWITCH, tiltFunc, CHANGE); //if can tilt is high, (set if shake mode is actived and printed to LCD). 
  else
    disableInterrupt(TILTSWITCH);  //do not detect shake
  
  
  //what action with printing or rolling
  if (rlDice) 
  {
    rlDice = false;  //stop roll
    rollDice();     //handle roll procedure
  }
  else if (pntDice)  //else, roll was not processed, and user not trying to change screen
  {
    pntDice = false;
    printDice();      //Therefore, print screen where user can roll, change die size, number of dice, or go to backlight setup
  }
  else if (pntRolledDice) //if user rolls, and selects dice on print roll screen, go to printRolledDice
  {
    pntRolledDice = false;
    printRolledDice();      //prints rolled dice
  }
  else if (pntRoll)   //turned on in rollDice procedure (if roll was detected and sucsesfully proccesed).
  {
    pntRoll = false;
    printRoll();    //print, roll procedure. Gives user choice for dice (see dice), home, or redo. 
  }
  else if (pntDisplay)    //activated by using switching toggle switch down while in home screen
  {
    pntDisplay = false;
    printDisplay();
  }

  
  //state checks
  //setup state
  if (currentState == SETUP)
  {
    // Check Rotary Encoder
    changeDice();   //tests for user inputing changes to die size
    
    if (canTiltMessage)
    {
      custom32Message("  Shaking Mode    Activated!", 750);
      canTilt = true;
      canTiltMessage = false;
    }
  }

  //display state
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

//prints "Home Screen" 
//allow user to change die size, number of dice, roll the dice, go into display mode with toggle switch, or turn on shaking mode with toggle switch. 
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


//prints rolled dice. Called if user pressed "DICE" in print roll screen
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


//function handles the random number generation for the actual dice roll
//disables interrupts
//Takes number of dice, and size, and generates a random number for each die
//checks for natural 20, critical 1's, and yahteez and if true sends to the corresponding function for a fun extra print
//if no special condition, sets tilt interrupt detection off, and turns on pnt roll, renables interrupts, return to main loop
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
    if (numDice > 1 && rolledDice[i-1] == rolledDice[i])
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
  else if (numDice == 5 && same == 4)
    yahtzee();

  tiltCount = 0;
  canTilt = false;
  pntRoll = true;
  enableAllInterrupts();
}


//Prints inital roll screen after dice roll iniated. 
//Allows user to see Dice roll, Go Home (setup screen), or Redo the roll
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


//Prints the display screen in display mode. 
//Display mode allows users to change backlight settings
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

//Change dice polles updates from rotary encoder interrupt updates
//in setup mode 
//to detect and change the number of dice the user wants to roll
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

//Change dice polles updates from rotary encoder interrupt updates
//in display mode 
//to detect the color changes the user wants to make
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


//custom32Message clears the screen, moves the curor home, 
//then writes new message to screen. calls delay time
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

//my delay uses millis for better time delay 
//functionaltiy 
void myDelay(unsigned long delayTime)
{
  unsigned long current = millis();
  while ((millis() - current) < delayTime)
  {
  }
}


//converts display values for LCD screen
//to custom characters 0-6 that were made in setup
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

//extra print for natural 20 condtion. Checked for in rollDice. Returns to rollDice.
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

//extra print for critcal 1 condition. Checked for in rollDice. Returns to rollDice. 
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

//extra print for yahtzee. Check for in rollDice. Returns to rollDice. 
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


//set color values 
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

//moves cursor to row and column
void moveCursor(int row, int col)
{
  Write(254);
  Write(128 + (row*64) + col);
}


// Interrupt Function Code

//detects changes to left button, updates booleans given the state of operation.Changes state for rolling
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


//detects changes to middle button, updates booleans given state of operation.Changes state for rolling
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

//detects changes to right button, updates booleans given state of operation. Changes state if necessary
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


//detects changes to toggle switch switched up, updates booleans given state of operation. Changes state
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

//detects changes to toggle switch swithced down, updates booleans given state of operation. Changes state
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

//detects changes to toggle switch, and keeps track of interrupt count to avoid mayhem.
//If in Display mode, and tilt count is 400, set random color scheme
//If in setup mode, and tilt count is greater than 1000, roll the dice. 
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

// encoder update calls state to check state of encoder
void encoderUpdate() {
  encoder.tick(); // just call tick() to check the state.
}

//detects changes to encoder button, updates booleans given state of operation. Changes state if necessary
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


 //function disables all interrupt
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


// Enable the interrupts for the initial state
void enableAllInterrupts()
{
  
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
