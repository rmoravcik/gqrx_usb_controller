#include <Keyboard.h>
#include <Mouse.h>
#include <Keypad.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns

// Define the Keymap
char keys[ROWS][COLS] = {
  {'<','.','>'},
  {'a','s','c'},
  {'n','d','+'},
  {'`','~','-'}
};

char keys_a[2] = { 'a', 'A' };
char keys_s[2] = { 's', 'S' };
char keys_c[2] = { 'c', 'C' };
char keys_n[4] = { 'n', 'w', 'W', 'o' };

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

bool tunning_mode = true;
KeypadEvent prev_key = 0;

void timerIsr() {
  encoder->service();
}

void setup() {
  // open the serial port:
  Serial.begin(9600);

  keypad.addEventListener(keypadEvent);

  // initialize control over the keyboard and mouse
  Keyboard.begin();
  Mouse.begin();

  encoder = new ClickEncoder(ENC_A, ENC_B, ENC_BUTTON, 4);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  last = -1;
}

void keypadEvent(KeypadEvent key) {
  static KeyState prev_state = IDLE;
  static uint8_t index = 0;
  KeypadEvent new_key = key;
  KeyState new_state = keypad.getState();

  if (new_state == RELEASED) {
    if (prev_state == PRESSED) {
      if (prev_key != key) {
        index = 1;
      } else {
        switch (key) {
          case 'a': new_key = keys_a[index]; index++; if (index >= sizeof(keys_a)) index = 0; break;
          case 's': new_key = keys_s[index]; index++; if (index >= sizeof(keys_s)) index = 0; break;
          case 'c': new_key = keys_c[index]; index++; if (index >= sizeof(keys_c)) index = 0; break;
          case 'n': new_key = keys_n[index]; index++; if (index >= sizeof(keys_n)) index = 0; break;
          case 'd': Keyboard.press(KEY_LEFT_CTRL); Keyboard.write('d'); Keyboard.release(KEY_LEFT_CTRL); return;
          default:  break;
        }
      }

      if (new_key == '+') {
        new_key = KEY_VOLUME_UP;
      } else if (new_key == '-') {
        new_key = KEY_VOLUME_DOWN;
      }

      Keyboard.write(new_key);
      prev_key = key;
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

  if ((prev_key == '+') || (prev_key == '-') || (key == '+') || (key == '-')) {
    if (key) {
        prev_key = key;
    }

    if (keypad.getState() == HOLD) {
      uint8_t key_to_send;
      if (prev_key == '+') {
        key_to_send = KEY_VOLUME_UP;
      } else if (prev_key == '-') {
        key_to_send = KEY_VOLUME_DOWN;
      }
      Keyboard.write(key_to_send);
      delay(50);
      return;
    }
  }

  if (b == ClickEncoder::DoubleClicked) {
    if (tunning_mode) {
      tunning_mode = false;
      Keyboard.write('f');      
    } else {
      tunning_mode = true;
      Keyboard.write('f');
      // move mouse cursor over waterfall
      Mouse.move(0, 127);
      Mouse.move(0, 127);
    }
  } else {
    value += encoder->getValue();
    if (value != last) {
      if (last < value) {
        if (keypad.getState() == HOLD) {
          Keyboard.write(KEY_TAB);
        } else {
          if (tunning_mode) {
            if (digitalRead(ENC_BUTTON) == LOW) {
              Mouse.move(0, 0, 10);
            } else {
              Mouse.move(0, 0, 1);
            }
          } else {
            if (digitalRead(ENC_BUTTON) == LOW) {
              Keyboard.write(KEY_RIGHT_ARROW);
            } else {
              Keyboard.write(KEY_UP_ARROW);
            }
          }
        }
      } else {
        if (keypad.getState() == HOLD) {
          Keyboard.press(KEY_LEFT_SHIFT);
          Keyboard.write(KEY_TAB);
          Keyboard.release(KEY_LEFT_SHIFT);
        } else {
          if (tunning_mode) {
            if (digitalRead(ENC_BUTTON) == LOW) {
              Mouse.move(0, 0, -10);
            } else {
              Mouse.move(0, 0, -1);
            }
          } else {
            if (digitalRead(ENC_BUTTON) == LOW) {
              Keyboard.write(KEY_LEFT_ARROW);
            } else {
              Keyboard.write(KEY_DOWN_ARROW);
            }
          }
        }
      }
      last = value;
    }
  }

  delay(10);
}
