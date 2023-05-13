#include <USB.h>
#include <USBHIDKeyboard.h>
#include "esp32-hal-cpu.h"
#include <esp_task_wdt.h>
#include "esp32-hal-tinyusb.h"

// remote control for XCSoar, emulates a keyboard and mouse
// hardware is just pushbuttons connected between GPIO pins and GND.
// for each button press a keystroke or mouse action is sent

const int key_repeat_interval {
  20
};    // key repeat = 30 ms plus USBHID delay = 100 mS = 130 mS
const int very_long_press_timeout {
  20
};  // 5 s for very long press, e.g. for restart
int very_long_press_counter{0};
int altMode{0};
int qmMode{0};
#define WDT_TIMEOUT 5

boolean modeS2F{false};

USBHIDKeyboard Keyboard;

enum e_button {
  TOP_CENTER,
  RH_MIDDLE,
  RH_LOWER,
  STF,
  JOY_UP,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_DOWN,
  JOY_PRESS
};

byte buttons[] = { // mapping array from definitions to set up the pins          button
  38,   // Button TOP_CENTER = upper middle button (QM)                         0
  10,   // Button RH_MIDDLE = upper RH button (ALT) Alternates & Flarm Radar    1
  12,   // Button RH_LOWER = lower right hand button                            2
  37,   // Button STF = STF switch, external to PCB                             3
  35,   // Button JOY_UP = joystick up                                          4
  33,   // Button JOY_LEFT = joystick left                                      5
  34,   // Button JOY_RIGHT = joystick right                                    6
  36,   // Button JOY_DOWN = joystick down                                      7
  0     // Button JOY_PRESS = joystick press                                    8
};

byte unused_buttons[] = { // mapping array from definitions to set up the pins of all unused I/O
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14
};

#define NUMBUTTONS sizeof(buttons)  //gives size of array *helps for adding buttons
#define NUM_UNUSED_BUTTONS sizeof(unused_buttons)

int button_states[NUMBUTTONS + 1];

void setup() {
  modeS2F = false;
  for (byte set = 0; set <= NUMBUTTONS; set++) { //setup of button pin hardware
    button_states[set] = 1;
    pinMode(buttons[set], INPUT_PULLUP);
  }
  for (byte set = 0; set <= NUM_UNUSED_BUTTONS; set++) { //setup of button pin hardware
    pinMode(buttons[set], INPUT_PULLUP);
  }
  // Wait a second since the HID drivers need a bit of time to re-mount
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  setCpuFrequencyMhz(80);
  Keyboard.begin();
  // delay(1000);
  // Serial.printf("XCStick start: CPU: %f Mhz\n", (float)getCpuFrequencyMhz());
}

void keyPressRepeat( int button, int key ) {
  // Serial.printf("Key button=%d, key=%d\n",button,key);
  Keyboard.press(key);
  Keyboard.release(key);
}

void keyPressRelase( int key ) {
  Keyboard.press(key);
  Keyboard.release(key);
}

void handleButton(int button_pressed, int button_released) {
  // Serial.printf("pressed: %d released %d\n", button_pressed, button_released );
  if (button_pressed == JOY_UP)
    keyPressRepeat( JOY_UP, KEY_UP_ARROW );
  else if (button_pressed == JOY_DOWN)
    keyPressRepeat( JOY_DOWN, KEY_DOWN_ARROW );
  else if (button_pressed == JOY_RIGHT)
    keyPressRepeat( JOY_RIGHT, KEY_RIGHT_ARROW );
  else if (button_pressed == JOY_LEFT)
    keyPressRepeat( JOY_LEFT, KEY_LEFT_ARROW );
  else if (button_pressed == RH_MIDDLE) { // Round robin, first Alternates, second Flarm Radar
    if ( altMode == 0 ) {
      if ( qmMode != 0 ) {
        keyPressRelase(KEY_ESC);
        qmMode = 0;
      }
      keyPressRelase(KEY_F6);  // Alternates
      altMode = 1;
    } else if ( altMode == 1 ) {
      keyPressRelase(KEY_ESC);
      altMode = 2;
      qmMode = 0;
    } else if ( altMode == 2 ) {
      keyPressRelase(KEY_F4);  // Flarm Radar
      altMode = 3;
    }
    else if ( altMode == 3 ) {
      keyPressRelase(KEY_ESC);
      altMode = 0;
      qmMode = 0;
    }
  }
  else if (button_pressed == TOP_CENTER) {
    if ( qmMode == 0 ) {
      if ( altMode ) {
        keyPressRelase(KEY_ESC);
        altMode = 0;
      }
      keyPressRelase(KEY_F1);  // Quick Menu
      qmMode = 1;
    } else if ( qmMode != 0) {
      keyPressRelase(KEY_ESC);
      qmMode = 0;
      altMode = 0;
    }
  }
  else if (button_pressed == STF) {
    if (modeS2F) {
      modeS2F = false;#include <USB.h>
#include <USBHIDKeyboard.h>
#include "esp32-hal-cpu.h"
#include <esp_task_wdt.h>
#include "esp32-hal-tinyusb.h"

// remote control for XCSoar, emulates a keyboard and mouse
// hardware is just pushbuttons connected between GPIO pins and GND.
// for each button press a keystroke or mouse action is sent

const int key_repeat_interval {
  20
};    // key repeat = 30 ms plus USBHID delay = 100 mS = 130 mS
const int very_long_press_timeout {
  20
};  // 5 s for very long press, e.g. for restart
int very_long_press_counter{0};
int altMode{0};
int qmMode{0};
#define WDT_TIMEOUT 5

boolean modeS2F{false};

USBHIDKeyboard Keyboard;

enum e_button {
  TOP_CENTER,
  RH_MIDDLE,
  RH_LOWER,
  STF,
  JOY_UP,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_DOWN,
  JOY_PRESS
};

byte buttons[] = { // mapping array from definitions to set up the pins          button
  38,   // Button TOP_CENTER = upper middle button (QM)                         0
  10,   // Button RH_MIDDLE = upper RH button (ALT) Alternates & Flarm Radar    1
  12,   // Button RH_LOWER = lower right hand button                            2
  37,   // Button STF = STF switch, external to PCB                             3
  35,   // Button JOY_UP = joystick up                                          4
  33,   // Button JOY_LEFT = joystick left                                      5
  34,   // Button JOY_RIGHT = joystick right                                    6
  36,   // Button JOY_DOWN = joystick down                                      7
  0     // Button JOY_PRESS = joystick press                                    8
};

byte unused_buttons[] = { // mapping array from definitions to set up the pins of all unused I/O
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14
};

#define NUMBUTTONS sizeof(buttons)  //gives size of array *helps for adding buttons
#define NUM_UNUSED_BUTTONS sizeof(unused_buttons)

int button_states[NUMBUTTONS + 1];

void setup() {
  modeS2F = false;
  for (byte set = 0; set <= NUMBUTTONS; set++) { //setup of button pin hardware
    button_states[set] = 1;
    pinMode(buttons[set], INPUT_PULLUP);
  }
  for (byte set = 0; set <= NUM_UNUSED_BUTTONS; set++) { //setup of button pin hardware
    pinMode(buttons[set], INPUT_PULLUP);
  }
  // Wait a second since the HID drivers need a bit of time to re-mount
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  setCpuFrequencyMhz(80);
  Keyboard.begin();
  // delay(1000);
  // Serial.printf("XCStick start: CPU: %f Mhz\n", (float)getCpuFrequencyMhz());
}

void keyPressRepeat( int button, int key ) {
  // Serial.printf("Key button=%d, key=%d\n",button,key);
  Keyboard.press(key);
  Keyboard.release(key);
}

void keyPressRelase( int key ) {
  Keyboard.press(key);
  Keyboard.release(key);
}

void handleButton(int button_pressed, int button_released) {
  // Serial.printf("pressed: %d released %d\n", button_pressed, button_released );
  if (button_pressed == JOY_UP)
    keyPressRepeat( JOY_UP, KEY_UP_ARROW );
  else if (button_pressed == JOY_DOWN)
    keyPressRepeat( JOY_DOWN, KEY_DOWN_ARROW );
  else if (button_pressed == JOY_RIGHT)
    keyPressRepeat( JOY_RIGHT, KEY_RIGHT_ARROW );
  else if (button_pressed == JOY_LEFT)
    keyPressRepeat( JOY_LEFT, KEY_LEFT_ARROW );
  else if (button_pressed == RH_MIDDLE) { // Round robin, first Alternates, second Flarm Radar
    if ( altMode == 0 ) {
      if ( qmMode != 0 ) {
        keyPressRelase(KEY_ESC);
        qmMode = 0;
      }
      keyPressRelase(KEY_F6);  // Alternates
      altMode = 1;
    } else if ( altMode == 1 ) {
      keyPressRelase(KEY_ESC);
      altMode = 2;
      qmMode = 0;
    } else if ( altMode == 2 ) {
      keyPressRelase(KEY_F4);  // Flarm Radar
      altMode = 3;
    }
    else if ( altMode == 3 ) {
      keyPressRelase(KEY_ESC);
      altMode = 0;
      qmMode = 0;
    }
  }
  else if (button_pressed == TOP_CENTER) {
    if ( qmMode == 0 ) {
      if ( altMode ) {
        keyPressRelase(KEY_ESC);
        altMode = 0;
      }
      keyPressRelase(KEY_F1);  // Quick Menu
      qmMode = 1;
    } else if ( qmMode != 0) {
      keyPressRelase(KEY_ESC);
      qmMode = 0;
      altMode = 0;
    }
  }
  else if (button_pressed == STF) {
    if (modeS2F) {
      modeS2F = false;
      keyPressRelase('V');
    }
    else {
      modeS2F = true;
      keyPressRelase('S');
    }
  }
  else if (button_pressed == RH_LOWER) {
    keyPressRelase(KEY_ESC);
    Keyboard.releaseAll();
    keyPressRelase(KEY_ESC);
    altMode = 0;
    qmMode = 0;
  }
  else if (button_pressed == JOY_PRESS) {
    keyPressRelase( KEY_RETURN );
  }
}

void loop() {
  for (int num = 0; num < NUMBUTTONS; num++) {
    int button_pressed{ -1};
    int button_released{ -1};
    if (digitalRead(buttons[num]) == 0) {  // returns true if button is pressed
      button_pressed = num;
      // Serial.printf("Button num: %d\n", button_pressed );
      if ( num == JOY_PRESS ) {
        very_long_press_counter++;
        // Serial.printf("very long press %d\n", very_long_press_counter );
        if ( very_long_press_counter > very_long_press_timeout ) {
          esp_restart();
        }
      }
      // Serial.printf("Pressed: %d\n", button_pressed );
      if (button_states[num] == 1) {
        button_states[num] = 0;
      }
      handleButton(button_pressed, button_released);
    }
    else {        // key not pressed
      if ( num == JOY_PRESS ) {
        very_long_press_counter = 0;
      }
      if (button_states[num] == 0) {
        button_released = num;
        button_states[num] = 1;
        // Serial.printf("R: %d\n", button_released);
        handleButton(button_pressed, button_released);
      }
    }
  }
  delay(key_repeat_interval);
  if ( very_long_press_counter < very_long_press_timeout ) { // last resort, if restart doesn't trigger, second chance is WDT
     esp_task_wdt_reset();
  }
}
      keyPressRelase('V');
    }
    else {
      modeS2F = true;
      keyPressRelase('S');
    }
  }
  else if (button_pressed == RH_LOWER) {
    keyPressRelase(KEY_ESC);
    Keyboard.releaseAll();
    keyPressRelase(KEY_ESC);
    altMode = 0;
    qmMode = 0;
  }
  else if (button_pressed == JOY_PRESS) {
    keyPressRelase( KEY_RETURN );
  }
}

void loop() {
  for (int num = 0; num < NUMBUTTONS; num++) {
    int button_pressed{ -1};
    int button_released{ -1};
    if (digitalRead(buttons[num]) == 0) {  // returns true if button is pressed
      button_pressed = num;
      // Serial.printf("Button num: %d\n", button_pressed );
      if ( num == JOY_PRESS ) {
        very_long_press_counter++;
        // Serial.printf("very long press %d\n", very_long_press_counter );
        if ( very_long_press_counter > very_long_press_timeout ) {
          esp_restart();
        }
      }
      // Serial.printf("Pressed: %d\n", button_pressed );
      if (button_states[num] == 1) {
        button_states[num] = 0;
      }
      handleButton(button_pressed, button_released);
    }
    else {        // key not pressed
      if ( num == JOY_PRESS ) {
        very_long_press_counter = 0;
      }
      if (button_states[num] == 0) {
        button_released = num;
        button_states[num] = 1;
        // Serial.printf("R: %d\n", button_released);
        handleButton(button_pressed, button_released);
      }
    }
  }
  delay(key_repeat_interval);
  if ( very_long_press_counter < very_long_press_timeout ) { // last resort, if restart doesn't trigger, second chance is WDT
     esp_task_wdt_reset();
  }
}
