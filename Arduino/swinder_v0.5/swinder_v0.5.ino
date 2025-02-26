  #include <LiquidCrystal_I2C.h>
  #include <DRV8825.h>
  #include <Encoder.h>

//Pins setup
const int coil_motor_step = 36, coil_motor_dir = 37, coil_motor_fault = 29, coil_motor_sleep = 35;
const int feed_motor_step = 38, feed_motor_dir = 39, feed_motor_fault = 30, feed_motor_sleep = 40;
const int rbutton = 23, ra = 22, rb = 21;
const int limit_switch_start = 7, limit_switch_end = 8;

//Define LCD
//Pins: SDA 18, SCL 19
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Define RotaryEncoder
Encoder encoder(ra, rb);

//Const vals
const float pi = 3.14159;
const float mu = 4 * pi * pow(10, -7);
const int BUTTON_DELAY = 200;
const int MOTOR_DELAY = 1;
const float MAX_LENGTH = 20;              //cm
const float MAX_INDUCTANCE = 10000;       //mH
const float MAX_RADIUS = 5;               //cm
const float WIRE_DIAMETER = 0.0511;       //cm
const int FULL_ROTATION = 200;            //steps
const float FEED_PER_FULL_ROTATION = 0.004 * FULL_ROTATION; //cm
const float FEED_PER_STEP = FEED_PER_FULL_ROTATION / (FULL_ROTATION * 2);
const float FEED_STEP_PER_FULL_ROTATION = WIRE_DIAMETER / FEED_PER_STEP;
const int COIL_STEP_PER_FEED_STEP = round(FULL_ROTATION / FEED_STEP_PER_FULL_ROTATION);
const float OFFSET = 0.5;                   //cm
const float PADDING = 0.1;                  //cm

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
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello World!");

  //Setup button inputs
  pinMode(rbutton, INPUT);

  //Setup encoder
  encoder.write(0);
  
  //Setup limit switch
  pinMode(limit_switch, INPUT);

  //Setup motor pins
  pinMode(coil_motor_step, OUTPUT);
  pinMode(coil_motor_dir, OUTPUT);
  pinMode(coil_motor_sleep, OUTPUT);
  pinMode(feed_motor_step, OUTPUT);
  pinMode(feed_motor_dir, OUTPUT);
  pinMode(feed_motor_sleep, OUTPUT);
  pinMode(coil_motor_fault, INPUT);
  pinMode(feed_motor_fault, INPUT);
  digitalWrite(coil_motor_dir, LOW);
  digitalWrite(feed_motor_dir, LOW);
  digitalWrite(coil_motor_sleep, LOW);
  digitalWrite(feed_motor_sleep, LOW);
}

//Main program loop
void loop() {
  startup_animation();
  choose_preset();
}

/*
Basic animation to run on startup
*/
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
  lcd.print("V0.5");
  delay(500);
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
  long oldPosition = encoder.read() / 4;

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
      delay(BUTTON_DELAY);
      
      //Assign presets
      switch (cursor_idx) {
        case 1:
          inductance = 40;
          length = 5;
          radius = 0.5;
          break;
        case 3:
          inductance = 80;
          length = 5;
          radius = 0.5;
          break;
        case 5:
          inductance = 40;
          length = 3;
          radius = 0.5;
          break;
        case 7:
          inductance = 0.1;
          length = 1;
          radius = 1;
          break;
        default:
          inductance = 0;
          length = 0;
          radius = 0;
      }
      
      //Reset and go to val select
      lcd.noCursor();
      lcd.noBlink();
      val_select();
    }

    //Read Encoder
    long newPosition = encoder.read() / 4;
    int dir = newPosition - oldPosition;
    if (dir > 0 && cursor_idx < 9) {
      cursor_idx += 2;
      oldPosition = newPosition;
    } else if (dir < 0 && cursor_idx > 1) {
      cursor_idx -= 2;
      oldPosition = newPosition;
    } else if (dir != 0) {
      oldPosition = newPosition;
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
  bool screen_change = true;
  long oldPosition = encoder.read() / 4;

  //Screen setup
  lcd.clear();

  //Selection loop
  while(true) {

    //Screens
    if (screen_change) {
      switch (screen_idx) {
        case 0: //Inductance (mH)
          lcd.setCursor(0, 0);
          lcd.print("Inductance (mH)");
          lcd.setCursor(0, 1);
          lcd.print(format_val(inductance, MAX_INDUCTANCE));
          break;
        case 1: //Length (cm)
          lcd.setCursor(0, 0);
          lcd.print("Length (cm)");
          lcd.setCursor(0, 1);
          lcd.print(format_val(length, MAX_LENGTH));
          break;
        case 2: //Radius (cm)
          lcd.setCursor(0, 0);
          lcd.print("Radius (cm)");
          lcd.setCursor(0, 1);
          lcd.print(format_val(radius, MAX_RADIUS));
          break;
        case 3: //Confirmation Screen
          num_turns = calc_turns(inductance, length, radius);
          lcd.setCursor(0, 0);
          lcd.print("Turns: ");
          lcd.print(num_turns);
          lcd.setCursor(0, 1);
          lcd.print("Confirm");
          break;
      }
      screen_change = false;
    }

    //Read Button
    if (digitalRead(rbutton) == LOW) {
      delay(BUTTON_DELAY);
      switch (screen_idx) {
        case 0:
          val_editor(&inductance, MAX_INDUCTANCE);
          break;
        case 1:
          val_editor(&length, MAX_LENGTH);
          break;
        case 2:
          val_editor(&radius, MAX_RADIUS);
          break;
        case 3:
          confirm_screen();
        break;
      }
      oldPosition = encoder.read() / 4;
      screen_change = true;
      lcd.clear();
    }

    //Read Encoder
    long newPosition = encoder.read() / 4;
    int dir = newPosition - oldPosition;
    if (dir > 0 && screen_idx < 3) {
      screen_idx += 1;
      oldPosition = newPosition;
      screen_change = true;
      lcd.clear();
    } else if (dir < 0 && screen_idx > 0) {
      screen_idx -= 1;
      oldPosition = newPosition;
      screen_change = true;
      lcd.clear();
    } else if (dir != 0) {
      oldPosition = newPosition;
    }

    //Stability Delay
    delay(2);
  }
}

/*
Edit value currently on screen
val: Value to be edited
max: Maximum of related value
-Rotate Clockwise: Move right/Increase value depending on editing_val
-Rotate Counterclockwise: Move left/Decrease value depending on editing_val
-Press: Start editing digit/Finish editing digit/Exit
*/
void val_editor(float* val, float max) {
  //Local vars
  int num_length = String(int(trunc(max))).length() + 3;  //Does include "."
  int cursor_idx = num_length - 1;
  float pwr_factor = 0.01;  //Power of 10 to add/subtract depending on cursor location
  bool editing_digit = false;
  bool screen_change = true;
  long oldPosition = encoder.read() / 4;

  //Screen setup
  lcd.setCursor(11, 1);
  lcd.print("Done");

  //Enable cursor at end of num
  lcd.cursor();
  lcd.noBlink();
  lcd.setCursor(cursor_idx, 1);
  
  //Editing loop
  while(true) {
    //Read Button
    if (digitalRead(rbutton) == LOW) {
      delay(BUTTON_DELAY);
      if (cursor_idx == 11) {
        lcd.noCursor();
        lcd.noBlink();
        return; //Exit editor
      } else {
        editing_digit = !editing_digit; //Begin editing a digit
        if (editing_digit) {
          lcd.blink();
        } else {
          lcd.noBlink();
        }
      }
    }

    //Read Encoder
    long newPosition = encoder.read() / 4;
    int dir = newPosition - oldPosition;
    if (!editing_digit) { //Selecting digit
      if (dir < 0 && cursor_idx > 0) {
        cursor_idx--;
        if (cursor_idx == num_length - 3) { //Jump decimal
          cursor_idx--;
        } 
        if (cursor_idx == 10) { //Jump back from done
          cursor_idx = num_length - 1;
        }
        pwr_factor *= 10;
        screen_change = true;
        oldPosition = newPosition;
      } else if (dir > 0 && cursor_idx <= num_length - 1) {
        cursor_idx++;
        if (cursor_idx == num_length - 3) { //Jump Decimal
          cursor_idx++;
        }
        if (cursor_idx == num_length) { //Jump to done
          cursor_idx = 11;
        }
        pwr_factor /= 10;
        screen_change = true;
        oldPosition = newPosition;
      } else if (dir != 0) {
        oldPosition = newPosition;
      }
    } else { //Editing Digit
      if (dir > 0 && *val + pwr_factor <= max) {
        *val += pwr_factor;
        screen_change = true;
        oldPosition = newPosition;
      } else if (dir < 0 && *val - pwr_factor >= 0) {
        *val -= pwr_factor;
        screen_change = true;
        oldPosition = newPosition;
      } else if (dir != 0) {
        oldPosition = newPosition;
      }
    }
    
    //Update Screen
    if (screen_change) {
      lcd.setCursor(0, 1);
      lcd.print(format_val(*val, max));
      lcd.setCursor(cursor_idx, 1);
      screen_change = false;
    }
    
    //Stability Delay
    delay(2);
  }
}

/*
Final check before starting winding process
Y starts process
N returns to val select
-Rotate Clockwise: Move right
-Rotate Counterclockwise: Move left
-Press: Select choice
*/
void confirm_screen() {
  //Local vars
  int cursor_idx = 0;
  long oldPosition = encoder.read() / 4;

  //Screen setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Begin Process?");
  lcd.setCursor(0, 1);
  lcd.print("Y/N");
  lcd.cursor();
  lcd.blink();
  lcd.setCursor(0, 1);

  //Loop till confirmation
  while (true) {
    //Read Button
    if (digitalRead(rbutton) == LOW) {
      delay(BUTTON_DELAY);
      lcd.noBlink();
      lcd.noCursor();
      if (cursor_idx == 0) {
        spin(); //Go to spin
      } else {
        val_select(); //Return to val select
      }
    }

    //Read encoder
    long newPosition = encoder.read() / 4;
    int dir = newPosition - oldPosition;
    if (dir > 0 && cursor_idx == 0) {
      cursor_idx = 2;
      oldPosition = newPosition;
    } else if (dir < 0 && cursor_idx == 2) {
      cursor_idx = 0;
      oldPosition = newPosition;
    } else if (dir != 0) {
      oldPosition = newPosition;
    }

    //Update cursor
    lcd.setCursor(cursor_idx, 1);

    //Stability Delay
    delay(2);
  }
}

/*
Spin process, runs to completion
-Rotate Clockwise: None
-Rotate Counterclockwise: None
-Press: Goes to pause
*/
void spin() {
  //Local vals
  int num_step_coil_const = num_turns * FULL_ROTATION;
  int num_step_coil = num_step_coil_const;
  int percent_complete = (100 * (num_step_coil_const - num_step_coil)) / num_step_coil_const;
  int new_percent_complete = percent_complete;
  float carriage_pos = -(OFFSET + PADDING);
  int carriage_factor = 1;
  int step_count = 0;

  //Screen setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Zeroing:");
  lcd.setCursor(0, 1);
  lcd.print("Please Wait...");

  //Wake motors
  digitalWrite(coil_motor_sleep, HIGH);
  digitalWrite(feed_motor_sleep, HIGH);
  delay(500);

  //Set starting direction
  digitalWrite(coil_motor_dir, HIGH);
  digitalWrite(feed_motor_dir, LOW);

  //Zero carriage
  while (true) {
    //Check for motor fault
    if (digitalRead(coil_motor_fault) == LOW || digitalRead(feed_motor_fault) == LOW) {
      motor_fault();
    }
    if (digitalRead(limit_switch_start) == HIGH) {
      break;
    } else {
      step_feed();
    }
  }

  //Reverse carriage direction
  digitalWrite(feed_motor_dir, HIGH);
  delay(100);

  //Move to start position
  while (carriage_pos < 0) {
    //Check for motor fault
    if (digitalRead(coil_motor_fault) == LOW || digitalRead(feed_motor_fault) == LOW) {
      motor_fault();
    }
    step_feed();
    carriage_pos += FEED_PER_STEP;
  }
  carriage_pos = 0;

  //Process screen setup
  lcd.setCursor(0, 1);
  lcd.print("OK!             ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Percent Complete:");
  lcd.setCursor(0, 1);
  lcd.print(percent_complete);
  lcd.print("%");

  while (num_step_coil > 0) { //Process loop
    //Check for motor fault
    if (digitalRead(coil_motor_fault) == LOW || digitalRead(feed_motor_fault) == LOW) {
      motor_fault();
    }
    //Read Button
    if (digitalRead(rbutton) == LOW) {
      delay(BUTTON_DELAY);
      pause();
      delay(500);
      lcd.clear(); //Reset display after pause
      lcd.setCursor(0, 0);
      lcd.print("Percent Complete:");
      lcd.setCursor(0, 1);
      lcd.print(percent_complete);
      lcd.print("%");
    }

    //Reverse carriage
    if (carriage_pos > length - PADDING) {
      digitalWrite(feed_motor_dir, LOW);
      carriage_factor = -1;
    }
    if (carriage_pos < PADDING) {
      digitalWrite(feed_motor_dir, HIGH);
      carriage_factor = 1;
    }

    //Drive Carriage
    if (step_count % COIL_STEP_PER_FEED_STEP == 0) {
      step_both();
      carriage_pos += FEED_PER_STEP * carriage_factor;
    } else {
      //Drive coil
      step_coil();
    }

    //Decrement steps
    num_step_coil -= 1;
    //Increment step count
    step_count += 1;

    //Update Screen
    new_percent_complete = (100 * (num_step_coil_const - num_step_coil)) / num_step_coil_const;
    if (percent_complete != new_percent_complete) {
      percent_complete = new_percent_complete;
      lcd.setCursor(0, 1);
      lcd.print(percent_complete);
      lcd.print("%");
    }
  }

  //Goes to completion screen to prompt to restart
  done();
}

void pause() {
  //Local variables
  int cursor_idx = 0;
  long oldPosition = encoder.read() / 4;

  //Screen setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Paused");
  lcd.setCursor(0, 1);
  lcd.print("Resume  Restart");
  lcd.cursor();
  lcd.blink();
  lcd.setCursor(0, 1);

  //Sleep motors
  digitalWrite(coil_motor_sleep, LOW);
  digitalWrite(feed_motor_sleep, LOW);

  //Loop till confirmation
  while (true) {
    //Read Button
    if (digitalRead(rbutton) == LOW) {
      delay(BUTTON_DELAY);
      lcd.noBlink();
      lcd.noCursor();
      if (cursor_idx == 0) {
        digitalWrite(coil_motor_sleep, HIGH);
        digitalWrite(feed_motor_sleep, HIGH);
        return; //Go back to spin and continue
      } else {
        choose_preset(); //Restart program to return to preset screen
      }
    }

    //Read encoder
    long newPosition = encoder.read() / 4;
    int dir = newPosition - oldPosition;
    if (dir > 0 && cursor_idx == 0) {
      cursor_idx = 8;
      oldPosition = newPosition;
    } else if (dir < 0 && cursor_idx == 8) {
      cursor_idx = 0;
      oldPosition = newPosition;
    } else if (dir != 0) {
      oldPosition = newPosition;
    }

    //Update cursor
    lcd.setCursor(cursor_idx, 1);

    //Stability Delay
    delay(2);
  }
}

/*
Only called in case of motor fault from either of the driver fault pins.
Infinitly loops till a hard reset.
*/
void motor_fault() {
  //Stop motors
  digitalWrite(coil_motor_sleep, LOW);
  digitalWrite(feed_motor_sleep, LOW);
  
  //Setup Screen
  lcd.setCursor(0, 0);
  lcd.print("     ERROR!     ");
  lcd.setCursor(0, 1);
  lcd.print("  MOTOR FAULT!  ");

  //Infinitely loop till forced reset
  while(true) {
    delay(2);
  }
}

void step_coil() {
  digitalWrite(coil_motor_step, HIGH);
  delay(MOTOR_DELAY);
  digitalWrite(coil_motor_step, LOW);
  delay(MOTOR_DELAY);
}

void step_feed() {
  digitalWrite(feed_motor_step, HIGH);
  delay(MOTOR_DELAY);
  digitalWrite(feed_motor_step, LOW);
  delay(MOTOR_DELAY);
}

void step_both() {
  digitalWrite(feed_motor_step, HIGH);
  digitalWrite(coil_motor_step, HIGH);
  delay(MOTOR_DELAY);
  digitalWrite(feed_motor_step, LOW);
  digitalWrite(coil_motor_step, LOW);
  delay(MOTOR_DELAY);
}

/*
Displays "Done" screen when winding has completed
- On button press, return to preset screen
- Has NO functionality for moving the cursor
*/
void done() {
  //Screen setup
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Completed!");
  lcd.setCursor(0,1);
  lcd.print("Press to Return");

  //Loop until confirmation
  while (true) {
    //Read button
    if(digitalRead(rbutton) == LOW) {
      delay(BUTTON_DELAY);
      choose_preset(); //Restart program and return to preset screen
    }

    //Stability delay
    delay(2);
  }
}

/*
Formats numbers to have the right amount of leading zeros for display
num: number to be formatted
max: maximum value of num
*/
String format_val(float num, float max) {
  //Local vars
  unsigned int length = String(int(trunc(max))).length() + 3; //Total length of val as str
  length = (length > 10 ? 10 : length); //Ensure length is never greater than 10
  String s = "";
  String n = String(num);

  //Adds leading zeros as needed to match expected length
  for (unsigned int i = 0; i < length; i++) {
    if (i < length - n.length()) {
      s += "0";
    } else {
      s += n[i - (length - n.length())]; //!! UPDATE TO FINISH EARLY !!
    }
  }

  return s;
}

/*
Calculates the number of turns needed for the given length, radius, and inductance
i: inductance of wanted solanoid
l: length of solanoid
r: radius of solanoid
*/
int calc_turns(float i, float l, float r) {
  //Value Correction
  l *= 0.01; //cm to m
  r *= 0.01; //cm to m
  i *= 0.001; //mH to H
  return round(sqrt((l * i) / ((pow(r, 2) * pi) * mu)));
}
