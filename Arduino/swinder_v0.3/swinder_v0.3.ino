#include <LiquidCrystal.h>
#include <DRV8825.h>

//Pins setup
const int lcd_rs = 0, lcd_en = 1, lcd_d4 = 2, lcd_d5 = 3, lcd_d6 = 4, lcd_d7 = 5;
const int coil_motor_step = 24, coil_motor_dir = 25;
const int feed_motor_step = 26, feed_motor_dir = 27;
const int up_btn = 35, down_btn = 36, mid_btn = 37, left_btn = 38, right_btn = 39;
const int limit_switch = 40;

//Define motor vars
DRV8825 coil_motor;
DRV8825 wire_motor;

//Define LCD
LiquidCrystal lcd(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7);

//Const vals
const float pi = 3.14159;
const float mu = 4 * pi * pow(10, -7);
const int BUTTON_DELAY = 200;
const float MAX_LENGTH = 20;           //cm
const float MAX_INDUCTANCE = 10000;    //mH
const float MAX_RADIUS = 5;            //cm
const float WIRE_DIAMETER = 0.0511;    //cm
const int FULL_ROTATION = 240;         //steps
const float FEED_PER_FULL_ROTATION = 0.8; //cm
const float FEED_PER_STEP = FEED_PER_FULL_ROTATION / FULL_ROTATION;
const float FEED_STEP_PER_FULL_ROTATION = WIRE_DIAMETER / FEED_PER_STEP;
const int COIL_STEP_PER_FEED_STEP = round(FULL_ROTATION / FEED_STEP_PER_FULL_ROTATION);

//Basic vals
float length = 5;       //In cm
float inductance = 40;  //In milihenrys
float radius = 0.5;     // In cm
int num_turns = 0;
bool running = false;

//Input Screen
int screen_idx = 0;
String screen_up[] = { "Swinder V0.3", "Length (cm) ", "Radius (cm)", "Inductance (mH)", "Num Turns ", "Start" };
String screen_dn[] = { "Down to start", "", "", "", "", ""};

void setup() {
  //Serial for debug
  Serial.begin(9600);
  Serial.println("Hello World!");
  //Setup lcd
  lcd.begin(16, 2);
  lcd.print("Hello World!");

  //Setup coil motor
  coil_motor.begin(coil_motor_step, coil_motor_dir);

  //Setup wire guide motor
  wire_motor.begin(feed_motor_step, feed_motor_dir);

  //Setup button inputs
  pinMode(up_btn, INPUT);
  pinMode(down_btn, INPUT);
  pinMode(left_btn, INPUT);
  pinMode(right_btn, INPUT);
  pinMode(middle_btn, INPUT);

  //Setup limit switch
  pinMode(limit_switch, INPUT);

  //Startup animation
  start_animation();

  //Initial Screen
  update_screen(0);
}

void loop() {
  while (!running) {
    if (digitalRead(mid_btn) == HIGH) {
      switch (screen_idx) {
        case 1: //Length
          val_editor(&length, MAX_LENGTH);
          break;
        case 2: //Radius
          val_editor(&radius, MAX_RADIUS);
          break;
        case 3: //Inductance
          val_editor(&inductance, MAX_INDUCTANCE);
          break;
        case 5: //Start
          running = true;
          break;
      }
      delay(BUTTON_DELAY);
    } else if (digitalRead(down_btn) == HIGH) {
      screen_idx = (screen_idx + 1) % 6;
      update_screen(screen_idx);
      delay(BUTTON_DELAY);
    } else if (digitalRead(up_btn) == HIGH) {
      if (screen_idx == 0) {
        screen_idx = 5;
      } else {
        screen_idx = (screen_idx - 1) % 6;
      }
      update_screen(screen_idx);
      delay(BUTTON_DELAY);
    }
  }
  while (running) {
    //Var setup
    bool paused = false;
    int num_steps_coil = num_turns * FULL_ROTATION;
    int feed_idx = 0;

    //Set starting direction
    digitalWrite(coil_motor_dir, HIGH);
    digitalWrite(feed_motor_dir, LOW);

    //Zero carriage
    bool limit = false;
    while (!limit) {
      if (digitalRead(down_btn) == HIGH) {
        limit = true;
      } else {
        digitalWrite(feed_motor_step, HIGH);
        delay(1);
        digitalWrite(feed_motor_step, LOW);
        delay(1);
      }
    }
    float carriage_dist = 0;
    digitalWrite(feed_motor_dir, HIGH);
    
    //Coil Wire
    lcd.setCursor(0, 0);
    lcd.print("Turns Remaining:");

    while (num_steps_coil > 0) {
      if (digitalRead(mid_btn) == HIGH) {
        delay(BUTTON_DELAY);
        paused = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PAUSED");
        while (paused) {
          if (digitalRead(mid_btn) == HIGH) {
            paused = false;
            delay(BUTTON_DELAY);
          }
          delay(1);
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RESUMING");
        delay(1000);
        lcd.setCursor(0, 0);
        lcd.print("Turns Remaining:");
      }
      if (feed_idx % COIL_STEP_PER_FEED_STEP == 0) {
        digitalWrite(feed_motor_step, HIGH);
      }
      digitalWrite(coil_motor_step, HIGH);
      delay(1);
      digitalWrite(coil_motor_step, LOW);
      if (feed_idx % COIL_STEP_PER_FEED_STEP == 0) {
        digitalWrite(feed_motor_step, LOW);
        carriage_dist += FEED_PER_STEP;
      }
      lcd.setCursor(0, 1);
      lcd.print(String(num_steps_coil / FULL_ROTATION));
      num_steps_coil--;
      feed_idx++;
    }
  }
}

void update_screen(int idx) {
  num_turns = calc_turns(length, radius, inductance);
  screen_dn[1] = format_val(length, MAX_LENGTH);
  screen_dn[2] = format_val(radius, MAX_RADIUS);
  screen_dn[3] = format_val(inductance, MAX_INDUCTANCE);
  screen_dn[4] = String(num_turns);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(screen_up[idx]);
  lcd.setCursor(0, 1);
  lcd.print(screen_dn[idx]);
}

void val_editor(float* val, float max) {
  //Setup vars
  int num_length = String(int(trunc(max))).length() + 3;  //Does include "."
  int cursor_idx = num_length - 1;
  float pwr_factor = 0.01;  //Power of 10 to add/subtract depending on cursor location
  bool editing_num = true;  //True until mid_btn is pressed again
  //Enable cursor and blink at end of num
  lcd.cursor();
  lcd.blink();
  lcd.setCursor(cursor_idx, 1);
  delay(BUTTON_DELAY);
  
  while (editing_num) {
    if(digitalRead(mid_btn) == HIGH) { //Exit number editing
      editing_num = false;
      delay(BUTTON_DELAY);
    } else if (digitalRead(up_btn) == HIGH) {
      if (*val + pwr_factor <= max) {
        *val += pwr_factor;
      }
      delay(BUTTON_DELAY);
    } else if (digitalRead(down_btn) == HIGH) {
      if (*val - pwr_factor >= 0) {
        *val -= pwr_factor;
      }
      delay(BUTTON_DELAY);
    } else if (digitalRead(left_btn) == HIGH) {
      if (cursor_idx > 0) {
        cursor_idx--;
        if (cursor_idx == num_length - 3) {
          cursor_idx--;
        }
        pwr_factor *= 10;
      }
      delay(BUTTON_DELAY);
    } else if (digitalRead(right_btn) == HIGH) {
      if (cursor_idx < num_length - 1) {
        cursor_idx++;
        if (cursor_idx == num_length - 3) {
          cursor_idx++;
        }
        pwr_factor /= 10;
      }
      delay(BUTTON_DELAY);
    }
    //Update Screen
    lcd.setCursor(0, 1);
    lcd.print(format_val(*val, max));
    lcd.setCursor(cursor_idx, 1);
    delay(10);
  }

  //Hide cursor
  lcd.noBlink();
  lcd.noCursor();
}

//Formats numbers to have the right amount of leading zeros for display
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

//Calculates the number of turns needed
int calc_turns(float l, float r, float i) {
  //Value Correction
  l *= 0.01; //cm to m
  r *= 0.01; //cm to m
  i *= 0.001; //mH to H
  return round(sqrt((l * i) / ((pow(r, 2) * pi) * mu)));
}

void start_animation() {
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
  delay(1000);
}