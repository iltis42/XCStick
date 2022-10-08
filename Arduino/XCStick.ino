
#include <USB.h>
#include <USBHIDKeyboard.h>
#include "esp32-hal-cpu.h"

// remote control for XCSoar, emulates a keyboard and mouse
// hardware is just pushbuttons connected between GPIO pins and GND.
// for each button press a keystroke or mouse action is sent

const int key_repeat_interval{25};
const int key_holddown_interval{8};
int button_pressed{-1};
int button_released{-1};
int altMode{0};
int qmMode{0};
long unsigned time_pressed{0};
long unsigned time_released{0};
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

byte buttons[]={  // mapping array from definitions to set up the pins     button
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

#define NUMBUTTONS sizeof(buttons)//gives size of array *helps for adding buttons

int button_states[NUMBUTTONS+1];
int button_holddown[NUMBUTTONS+1];

void setup() {
  for (byte set=0;set<=NUMBUTTONS;set++){  //setup of button pin hardware
    modeS2F = false;
    button_states[set] = 1;
    button_holddown[set] = 0;
    pinMode(buttons[set],INPUT_PULLUP);
    digitalWrite(buttons[set],HIGH);
  }
  // Wait a second since the HID drivers need a bit of time to re-mount
  delay(1000);
  Keyboard.begin();
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  delay(1000);
  Serial.printf("Pushbutton Bounce library test: CPU: %f Mhz\n", (float)getCpuFrequencyMhz());
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
    if (button_pressed == JOY_UP) 
       keyPressRepeat( JOY_UP, KEY_UP_ARROW );
    else if (button_pressed == JOY_DOWN) 
       keyPressRepeat( JOY_DOWN, KEY_DOWN_ARROW );
    else if (button_pressed == JOY_RIGHT)
       keyPressRepeat( JOY_RIGHT, KEY_RIGHT_ARROW );
    else if (button_pressed == JOY_LEFT)
       keyPressRepeat( JOY_LEFT, KEY_LEFT_ARROW );     

    else if (button_pressed == RH_MIDDLE){  // Round robin, first Alternates, second Flarm Radar
        if( altMode == 0 ){
          keyPressRelase(KEY_F6);  // Alternates
          altMode = 1;
        } else if ( altMode == 1 ) {
          keyPressRelase(KEY_ESC);
          altMode = 2;
        } else if ( altMode == 2 ) {
          keyPressRelase(KEY_F4);  // Flarm Radar
          altMode = 3;
        }
        else if ( altMode == 3 ) {
          keyPressRelase(KEY_ESC);
          altMode = 0;
        }
    }
    else if (button_pressed == TOP_CENTER) {
        if( qmMode == 0 ){
          keyPressRelase(KEY_F1);  // Quick Menu
          qmMode = 1;
        } else if ( qmMode == 1 ) {
          keyPressRelase(KEY_ESC);
          qmMode = 0;
        } 
          // keyPressRelase('M');
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
    }
    else if (button_pressed == JOY_PRESS) {
       keyPressRelase( KEY_RETURN );
    }
}


void loop() {
   // Serial.printf("Button Loop\n" );
   for (int num=0;num<NUMBUTTONS;num++) {
        // Serial.printf("Button %d loop\n" );
        int button_pressed{-1};
        int button_released{-1};
        // bouncer[num].update();
        if (digitalRead(buttons[num]) == 0) {  // returns true if button is pressed
          button_pressed = num;
          if (button_holddown[num] == 0) {    
             Serial.printf("P-In: %d\n", button_pressed );
             handleButton(button_pressed, button_released);
             // Serial.printf("P-Out: %d\n", button_pressed );
          } else {
            if( button_holddown[num] )
               button_holddown[num]--;
          }
          if (button_states[num] == 1) {
            button_states[num] = 0;
            button_holddown[num] = key_holddown_interval;
          }
        }
        else {
          if (button_states[num] == 0) {
             button_released = num;
             button_states[num] = 1;
             button_holddown[num] = 0;
             //Serial.printf("R: %d\n", button_released);
             handleButton(button_pressed, button_released);
          }
        }
  }
  delay(key_repeat_interval);
}
