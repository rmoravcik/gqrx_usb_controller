#include <Keyboard.h>
#include <Keypad.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns

// Define the Keymap
char keys[ROWS][COLS] = {
  {'<','.','>'},
  {'a','s','c'},
  {'n','w','+'},
  {'`','~','-'}
};

// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 3, 8, 7, 5 };

// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 4, 2, 6 }; 

// Create the Keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

#define ENC_A      19
#define ENC_B      18
#define ENC_BUTTON 20

ClickEncoder *encoder;
int16_t last, value; // variables for current and last rotation value

void timerIsr() {
  encoder->service();
}

void setup() {
  // open the serial port:
  Serial.begin(9600);

  keypad.addEventListener(keypadEvent);

  // initialize control over the keyboard:
  Keyboard.begin();

  encoder = new ClickEncoder(ENC_A, ENC_B, ENC_BUTTON, 4);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  last = -1;
}

void keypadEvent(KeypadEvent key) {
  static KeypadEvent prev_key = 0;
  static KeyState prev_state = IDLE;
  KeyState new_state = keypad.getState();

  if (new_state == RELEASED) {
    if (prev_state == PRESSED) {
      // AM / AM_SYNC
      if ((prev_key == 'a') || (prev_key == 'A')) {
        if (prev_key == 'a') {
            key -= 0x20;
        }  
      }

      // LSB / USB
      if ((prev_key == 's') || (prev_key == 'S')) {
        if (prev_key == 's') {
            key -= 0x20;
        }
      }

      // CW LSB / CW USB
      if ((prev_key == 'c') || (prev_key == 'C')) {
        if (prev_key == 'c') {
            key -= 0x20;
        }
      }

      // NFM / WFM (mono) / WFM (stereo)
      if ((prev_key == 'n') || (prev_key == 'w') || (prev_key == 'W')) {
        if (prev_key == 'n') {
            key = 'w';
        } else if (prev_key == 'w') {
            key = prev_key - 0x20;
        }
      }
      prev_key = key;

      Keyboard.write(key);
    } else if (prev_state == HOLD) {
      if (key == '`') {
        Keyboard.release(KEY_LEFT_ALT);
      }
    }
  } else if (new_state == HOLD) {
    if (key == '`') {
      Keyboard.press(KEY_LEFT_ALT);
    }
  }

  prev_state = new_state;
}

void loop() {
  char key = keypad.getKey();
  ClickEncoder::Button b = encoder->getButton();

  if (b == ClickEncoder::DoubleClicked) {
    Keyboard.write('f');
  } else {
    value += encoder->getValue();
    if (value != last) {
      if (last < value) {
        if (keypad.getState() == HOLD) {
          Keyboard.write(KEY_TAB);
        } else if (digitalRead(ENC_BUTTON) == LOW) {
          Keyboard.write(KEY_RIGHT_ARROW);
        } else {
          Keyboard.write(KEY_UP_ARROW);
        }
      } else {
        if (keypad.getState() == HOLD) {
          Keyboard.press(KEY_LEFT_SHIFT);
          Keyboard.write(KEY_TAB);
          Keyboard.release(KEY_LEFT_SHIFT);
        } else if (digitalRead(ENC_BUTTON) == LOW) {
          Keyboard.write(KEY_LEFT_ARROW);
        } else {
          Keyboard.write(KEY_DOWN_ARROW);
        }
      }
      last = value;
    }
  }

  delay(10);
}
