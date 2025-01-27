  #include <LiquidCrystal.h>
  #include <DRV8825.h>
  #include <RotaryEncoder.h>

//Pins setup
const int lcd_rs = 0, lcd_en = 1, lcd_d4 = 2, lcd_d5 = 3, lcd_d6 = 4, lcd_d7 = 5;
const int coil_motor_step = 36, coil_motor_dir = 37;
const int feed_motor_step = 38, feed_motor_dir = 39;
const int rbutton = 23, ra = 22, rb = 21;
const int limit_switch = 40;

//Define LCD
LiquidCrystal lcd(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7);

//Define RotaryEncoder
RotaryEncoder encoder(ra, rb, RotaryEncoder::LatchMode::FOUR3);

//Const vals
const float pi = 3.14159;
const float mu = 4 * pi * pow(10, -7);
const int BUTTON_DELAY = 200;
const float MAX_LENGTH = 20;              //cm
const float MAX_INDUCTANCE = 10000;       //mH
const float MAX_RADIUS = 5;               //cm
const float WIRE_DIAMETER = 0.0511;       //cm
const int FULL_ROTATION = 240;            //steps
const float FEED_PER_FULL_ROTATION = 0.8; //cm
const float FEED_PER_STEP = FEED_PER_FULL_ROTATION / FULL_ROTATION;
const float FEED_STEP_PER_FULL_ROTATION = WIRE_DIAMETER / FEED_PER_STEP;
const int COIL_STEP_PER_FEED_STEP = round(FULL_ROTATION / FEED_STEP_PER_FULL_ROTATION);

//Basic vals
float length = 5;       //In cm
float inductance = 40;  //In milihenrys
float radius = 0.5;     // In cm
int num_turns = 0;

void setup() {
  //Serial for debug
  Serial.begin(9600);
  Serial.println("Hello World!");

  //Setup lcd
  lcd.begin(16, 2);
  lcd.print("Hello World!");

  //Setup button inputs
  pinMode(rbutton, INPUT);
  

  //Setup limit switch
  pinMode(limit_switch, INPUT);

  //Setup motor pins
  pinMode(coil_motor_step, OUTPUT);
  pinMode(coil_motor_dir, OUTPUT);
  pinMode(feed_motor_step, OUTPUT);
  pinMode(feed_motor_dir, OUTPUT);
  digitalWrite(coil_motor_dir, LOW);
  digitalWrite(feed_motor_dir, LOW);
}

//Main program loop
void loop() {
  startup_animation();
  choose_preset();
  val_select();
}

void startup_animation() {
  int d = 100;
  String s = "Robojackets!";
  lcd.clear();
  lcd.setCursor(2, 0);
  for (unsigned int i = 0; i < s.length(); i++) {
    lcd.print(s.charAt(i));
    delay(d);
  }
  lcd.setCursor(5, 1);
  lcd.print("V0.3");
  delay(800);
}

/*
Select Preset Screen
-Rotate Clockwise: Move Cursor Right
-Rotate Counterclockwise: Move Cursor Left
-Press: Select Preset
*/
void choose_preset() {
  //Local vars
  int cursor_idx = 1;

  //Screen setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Presets:");
  lcd.setCursor(0, 1);
  lcd.print(" A B C D None");
  lcd.setCursor(cursor_idx, 1);
  lcd.cursor();
  lcd.blink();

  //Selection loop
  while (true) {
    //Read Button
    if (digitalRead(rbutton) == LOW) {
      Serial.println("Select!");
      delay(BUTTON_DELAY);
      break;
    }

    //Read Encoder
    encoder.tick();
    int dir = (int)(encoder.getDirection());
    if (dir > 0 && cursor_idx < 9) {
      cursor_idx += 2;
    } else if (dir < 0 && cursor_idx > 1) {
      cursor_idx -= 2;
    }

    //Update cursor
    lcd.setCursor(cursor_idx, 1);

    //Stability Delay
    delay(1);
  }
}

/*
Select which value to edit/confirm and start
Screens:
  1: Inductance
  2: Length
  3: Radius
  4: Confirmation
-Rotate Clockwise: Go down screen
-Rotate Counterclockwise: Go up screen
-Press: Begin editing number/confirm and exit
*/
void val_select() {
  //Local vars
  int screen_idx = 0;

  //Screen setup
  lcd.clear();

  //Selection loop
  while(true) {
    //Read Button
    if (digitalRead(rbutton) == LOW) {
      Serial.println("Select!");
      delay(BUTTON_DELAY);
      break;
    }

    //Read Encoder
    encoder.tick();
    int dir = (int)(encoder.getDirection());
    if (dir > 0 && screen_idx < 2) {
      screen_idx += 1;
    } else if (dir < 0 && screen_idx > 0) {
      screen_idx -= 1;
    }

    //Screens
    switch (screen_idx) {
      case 0: //Inductance (mH)
        lcd.setCursor(0, 0);
        lcd.print("Inductance (mH)");
        lcd.setCursor(0, 1);
        lcd.print("2");
        break;
      case 1: //Length (cm)
        lcd.setCursor(0, 0);
        lcd.print("Length (cm)");
        lcd.setCursor(0, 1);
        lcd.print("2");
        break;
      case 2: //Radius (cm)
        lcd.setCursor(0, 0);
        lcd.print("Radius (cm)");
        lcd.setCursor(0, 1);
        lcd.print("2");
        break;
    }
  }
}

void val_editor() {

}

void step_coil() {
  digitalWrite(coil_motor_step, HIGH);
  delay(1);
  digitalWrite(coil_motor_step, LOW);
  delay(1);
}

void step_feed() {
  digitalWrite(feed_motor_step, HIGH);
  delay(1);
  digitalWrite(feed_motor_step, LOW);
  delay(1);
}

/*
Formats numbers to have the right amount of leading zeros for display
num: number to be formatted
max: maximum value of num
*/
String format_val(float num, float max) {
  unsigned int length = String(int(trunc(max))).length() + 3;
  String s = "";
  String n = String(num);
  for (unsigned int i = 0; i < length; i++) {
    if (i < length - n.length()) {
      s += "0";
    } else {
      s += n[i - (length - n.length())];
    }
  }
  return s;
}
