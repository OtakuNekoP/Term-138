// need to use old libmaple based Arduino_STM32 to build this code
// https://github.com/rogerclarkmelbourne/Arduino_STM32

#include <Adafruit_GFX.h> 
// needs to be Adafruit GFX Library v1.1.4, check/change your installed version
// otherwise you will get a black screen or compiler errors

#include "src/TFTLib/Adafruit_TFTLCD_8bit_STM32.h"

// TFT display constants
#define PORTRAIT     0
#define LANDSCAPE     1

Adafruit_TFTLCD_8bit_STM32 tft;

#define LED_ON  digitalWrite(PA15, LOW)
#define LED_OFF digitalWrite(PA15, HIGH)

// ==================================================================================================
// Setup
// ==================================================================================================

bool run = true;
bool baudsel = true;
int crlfsel = 0;

void setup() {
  afio_cfg_debug_ports(AFIO_DEBUG_NONE); //added to disable the debug port. My stock DSO-138 won't allow the screen to work without this
  // see http://www.stm32duino.com/viewtopic.php?t=1130#p13919 for more info
  
  // init gpio
  pinMode(PA15, OUTPUT); // Trigger LED
  pinMode(PB15, INPUT); // BTN1 OK
  pinMode(PB14, INPUT); // BTN2 +
  pinMode(PB13, INPUT); // BTN3 -
  pinMode(PB12, INPUT); // BTN4 SEL
  
  LED_ON;
  
  Serial.begin(115200); // USB CDC as debug serial
  Serial1.begin(115200);
  
  // init the IL9341 display
  initDisplay();

  //Serial1.println("Term-138 Ready!");
  delay(1000);
  Serial.print("Term-138 Ready!");
  
  LED_OFF;
  delay(1000);
}

// ==================================================================================================
// Loop
// ==================================================================================================

void loop() {
  if (run){
    // If a new char is available on serial link, Display the char.
    if (Serial1.available())
    {
      char c=Serial1.read();
      tftPutchar(c);
  
      // Uncomment for echoing
      //Serial.print(c);
    } 
  }
  
  btnHandler();
}


// ==================================================================================================
// Button handler
// ==================================================================================================

void btnHandler() {
    // Press BTN OK to clear screen
  if (digitalRead(PB15) == 0){
    delay(50);
    if (digitalRead(PB15) == 0){
      tftClearTerminal();
      tftDisplayStatus();
      while (digitalRead(PB15) == 0);
    }
  }

  // Press BTN + to change baud rate
  if (digitalRead(PB14) == 0){
    delay(50);
    if (digitalRead(PB14) == 0){
      baudsel = !baudsel;
      while (digitalRead(PB14) == 0);
      Serial1.end();
      if (baudsel) {
        Serial1.begin(115200);
      } else {
        Serial1.begin(9600);
      }
      tftDisplayStatus();
    }
  }

  // Press BTN - to change new line mode
  if (digitalRead(PB13) == 0){
    delay(50);
    if (digitalRead(PB13) == 0){
      crlfsel++;
      if (crlfsel == 3){
        crlfsel = 0;
      }
      while (digitalRead(PB13) == 0);
      tftDisplayStatus();
    }
  }
  
  // Press BTN SEL to pause
  if (digitalRead(PB12) == 0){
    delay(50);
    if (digitalRead(PB12) == 0){
      run = !run;
      tftDisplayStatus();
      while (digitalRead(PB12) == 0);
    }
  }
}

// ==================================================================================================
// Display handler
// ==================================================================================================

// ------------------------
void initDisplay()  {
// ------------------------
  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(LANDSCAPE);
  tft.fillScreen(ILI9341_BLACK);

  // Display header and clear terminal
  banner();
  delay(4000);

  tftClearTerminal();
  tftDisplayStatus();
}


// ------------------------
void banner() {
// ------------------------
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(110, 30);
  tft.print("Term138");
  tft.drawRect(100, 25, 100, 25, ILI9341_WHITE);

  tft.setTextSize(1);
  tft.setCursor(30, 70);
  tft.print("Little Terminal with TFT Display");

  tft.setCursor(30, 95);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.print("https://github.com/otakunekop/term-138");

  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(30, 120);
  tft.print("DSO-138 hardware by JYE-Tech");
  
  tft.setCursor(30, 145);
  tft.print("Firmware version: v0");

  tft.setTextSize(1);
  tft.setCursor(30, 200);
  tft.print("GNU GENERAL PUBLIC LICENSE Version 3");
}

// ==================================================================================================
// Terminal handler
// ==================================================================================================

// Coordinates of the cursor
uint16 x=0;
uint16 y=0;

// Clear terminal
void tftClearTerminal()
{    
  // Display a black rectangle in the terminal body 
  tft.fillScreen(ILI9341_BLACK);  
  x=0;
  y=1;
}

// Increase Y (new line)
// If cursor is at the bottom of the display, 
// Clear terminal to start a new page
void increase_Y()
{
  // Increase Y and check if last line
  if (++y>29)
    {
      // Clear terminal (new page)
      tftClearTerminal();
      tftDisplayStatus();
      x=0;
      y=1;      
    }
}

// Increase Y (next char)
// If cursor is a end of line, 
// jump to a new line
void increase_X()
{
  // Increase X and check if end of line
  if (++x>52) 
  {
    // New line
    x=0;
    increase_Y();
  }
}

// Display a char
void tftPutchar(char c)
{
  LED_ON;
  // Create an string for displaying the char
  char string[2];
  string[0]=c;
  string[1]=0; 

  // New line ?
  if (checkNewLine(c))
  {
    x=0;
    increase_Y();
  }
  else
  {
    // Display char
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(1);
    tft.setCursor(x*6, y*8);
    tft.print(string);
    
    increase_X();
  }
  LED_OFF;  
}

bool checkNewLine(char c)
{
  if (crlfsel == 0){
    // CR or LF
    if (c=='\r' || c=='\n'){
      return 1;
    } else {
      return 0;
    }
  } else if (crlfsel == 1) {
    // CR  \r
    if (c=='\r'){
      return 1;
    } else {
      return 0;
    }
  } else if (crlfsel == 2) {
    // LF  \n
    if (c=='\n'){
      return 1;
    } else {
      return 0;
    }
  }
}

// Display a toolbar on top
void tftDisplayStatus()
{
  tft.setTextColor(ILI9341_RED, ILI9341_WHITE);
  tft.setTextSize(1);
  tft.setCursor(6, 0);
  tft.print(" | Term-138 | ");

  if (baudsel){
    tft.print("BAUD 115200");
  } else {
    tft.print("BAUD  9600 ");
  }

  tft.print(" | ");

  if (crlfsel == 0){
    tft.print(" CR or LF ");
  } else if (crlfsel == 1) {
    tft.print("  CR  \\r  ");
  } else if (crlfsel == 2) {
    tft.print("  LF  \\n  ");
  }

  tft.print(" | ");

  if (run){
    tft.print("RUNNING");
  } else {
    tft.print(" PAUSE ");
  }

  tft.print(" | ");
}
