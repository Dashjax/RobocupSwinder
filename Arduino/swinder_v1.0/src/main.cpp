#include <Arduino.h>
#include <Solenoid.hpp>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>

// Debug mode
#define DEBUG true

// Solonoid spin motor
#define SS_STEP_PIN 36
#define SS_DIR_PIN 35
#define SS_FAULT_PIN 30
#define SS_SLEEP_PIN 37
// Set SS direction
#define SS_DIR_SET 0

// Carriage control motor
#define CC_STEP_PIN 39
#define CC_DIR_PIN 38
#define CC_FAULT_PIN 29
#define CC_SLEEP_PIN 40
// Set CC direction
#define CC_DIR_SET 0

// Rotary encoder
#define RE_BUTTON_PIN 21
#define RE_A_PIN 22
#define RE_B_PIN 23

// Limit switches
#define LS_START_PIN 7
#define LS_END_PIN 8

// Misc constants
#define VERSION "V1.0"
#define BUTTON_DELAY 200

// Variables
uint8_t task = 0;
uint32_t length = 500; // cm * 100
uint32_t inductance = 4000; // mH * 100
uint32_t radius = 50; // cm * 100
uint32_t numTurns = 0;

// Define LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define Rotary Encoder
Encoder encoder(RE_A_PIN, RE_B_PIN);


// Function definition
void choosePreset();
void valSelect();
String formatVal(uint32_t, uint32_t);
uint32_t calcTurns();
void valEditor(uint32_t*, uint32_t);

void setup() {
  #if DEBUG 
    Serial.begin(9600);
    Serial.println("Swinder v1.0 - Debug Mode");
  #endif

  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello World!");

  // Initialize Rotary Encoder
  encoder.write(0);
  pinMode(RE_BUTTON_PIN, INPUT);

  // Initialize Limit Switches
  pinMode(LS_START_PIN, INPUT);
  pinMode(LS_END_PIN, INPUT);

  // Initialize CC Motor
  pinMode(CC_DIR_PIN, OUTPUT);
  pinMode(CC_STEP_PIN, OUTPUT);
  pinMode(CC_SLEEP_PIN, OUTPUT);
  pinMode(CC_FAULT_PIN, INPUT);
  digitalWrite(CC_DIR_PIN, CC_DIR_SET);
  digitalWrite(CC_SLEEP_PIN, LOW);


  // Initialize SS Motor
  pinMode(SS_DIR_PIN, OUTPUT);
  pinMode(SS_STEP_PIN, OUTPUT);
  pinMode(SS_SLEEP_PIN, OUTPUT);
  pinMode(SS_FAULT_PIN, INPUT);
  digitalWrite(SS_DIR_PIN, SS_DIR_SET);
  digitalWrite(SS_SLEEP_PIN, LOW);

  #if !DEBUG
    startupAnimation();
  #endif
}

void loop() {
  /*
  Usual Task Progression:
  -Choose preset
  -Edit Values
  -Confirmation
  -Spin
  -End
  -Restart
  */
  switch (task) {
    case 0:
      choosePreset();
      break;
    case 1:
      valSelect();
      break;
    default:
      task = 0;
  }
}


/*
Select Preset Screen
-Rotate Clockwise: Move Cursor Right
-Rotate Counterclockwise: Move Cursor Left
-Press: Select Preset
*/
void choosePreset() {
  uint8_t cursorIndex = 1;
  long reOldPosition = encoder.read() / 4;

  // Screen setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Presets:");
  lcd.setCursor(0, 1);
  lcd.print(" A B C D None");
  lcd.setCursor(cursorIndex, 1);
  lcd.cursor();
  lcd.blink();

  // Selection loop
  while (true) {
    // Trigger selection on button press
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);

      #if DEBUG
        Serial.println("B!");
      #endif

      // Set preset
      switch (cursorIndex) {
        case 1:
          inductance = 4000;
          length = 500;
          radius = 50;
          break;
        case 3:
          inductance = 8000;
          length = 500;
          radius = 50;
          break;
        case 5:
          inductance = 4000;
          length = 300;
          radius = 50;
          break;
        case 7:
          inductance = 10;
          length = 100;
          radius = 100;
          break;
        default:
          inductance = 0;
          length = 0;
          radius = 0;
      }
      
      // Set task to val editing
      task = 1;

      // Reset lcd
      lcd.noCursor();
      lcd.noBlink();

      return;
    }

    // Read encoder
    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (dir > 0 && cursorIndex < 9) {
      cursorIndex += 2;
    } else if (dir < 0 && cursorIndex > 1) {
      cursorIndex -= 2;
    }
    reOldPosition = reNewPosition;

    // Update cursor
    lcd.setCursor(cursorIndex, 1);

    // Stability delay
    delay(1);
  }
}

void valSelect() {
  // Local vars
  uint8_t screenIndex = 0;
  bool screenChange = true;
  long reOldPosition = encoder.read() / 4;

  // Setup screen
  lcd.clear();

  while (true) {
    if (screenChange) {
      lcd.clear();
      switch (screenIndex) {
        case 0:
          lcd.setCursor(0, 0);
          lcd.print("Inductance (mH)");
          lcd.setCursor(0, 1);
          lcd.print(formatVal(inductance, MAX_INDUCTANCE));
          break;
        case 1: //Length (cm)
          lcd.setCursor(0, 0);
          lcd.print("Length (cm)");
          lcd.setCursor(0, 1);
          lcd.print(formatVal(length, MAX_LENGTH));
          break;
        case 2: //Radius (cm)
          lcd.setCursor(0, 0);
          lcd.print("Radius (cm)");
          lcd.setCursor(0, 1);
          lcd.print(formatVal(radius, MAX_RADIUS));
          break;
        case 3: //Confirmation Screen
          numTurns = calcTurns();
          lcd.setCursor(0, 0);
          lcd.print("Turns: ");
          lcd.print(numTurns);
          lcd.setCursor(0, 1);
          lcd.print("Confirm");
          break;
      }
      screenChange = false;
    }

    // Read button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);
      switch (screenIndex) {
        case 0:
          valEditor(&inductance, MAX_INDUCTANCE);
          break;
        case 1:
          valEditor(&length, MAX_LENGTH);
          break;
        case 2:
          valEditor(&radius, MAX_RADIUS);
          break;
        case 3:
          task = 2;
          return;
      }
    }

    // Read Encoder
    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (dir > 0 && screenIndex < 3) {
      screenIndex += 1;
      screenChange = true;
    } else if (dir < 0 && screenIndex > 0) {
      screenIndex -= 1;
      screenChange = true;
    }
    reOldPosition = reNewPosition;

    delay(1);
  }
}

void valEditor(uint32_t* val, uint32_t max) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TODO");
  while (true) {}
}

String formatVal(uint32_t num, uint32_t max) {
  uint8_t maxLength = String(max).length() + 1; // Cannot be greater than 10
  String returnString = "";
  String numberString = String(num);

  // Add leading zeros to match length
  for (size_t i = 0; i < maxLength - (numberString.length() + 1); i++) {
      returnString += "0";
  }
  
  // Add decimal point with 2 positions of precision
  returnString += numberString.substring(0, numberString.length() - 3);
  returnString +=  ".";
  returnString += numberString.substring(numberString.length() - 2);

  return returnString;
}

uint32_t calcTurns() {
  //TODO
 return 3;
}

/*
  Simple animation to play at startup
  Total delay: 1600ms
*/
void startupAnimation() {
  String s = "Robojackets!";
  lcd.clear();
  lcd.setCursor(2, 0);
  for (size_t i = 0; i < s.length(); i++) {
    lcd.print(s.charAt(i));
    delay(100);
  }
  lcd.setCursor(5, 1);
  lcd.print(VERSION);
  delay(500);
}