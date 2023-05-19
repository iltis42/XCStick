#include <USB.h>
#include <USBHIDKeyboard.h>
#include "esp32-hal-cpu.h"
#include <esp_task_wdt.h>
#include "esp32-hal-tinyusb.h"
#include <AceButton.h>

// OTA includes
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include "Update.h"
#include <USB.h>
#include <USBHIDKeyboard.h>
#include "esp32-hal-cpu.h"
#include <esp_task_wdt.h>

using namespace ace_button;

// remote control for XCSoar, emulates a keyboard
// hardware is just pushbuttons connected between GPIO pins and GND.
// for each button press a keystroke is sent

#define WDT_TIMEOUT 5

boolean modeS2F{ false };
bool joy_pressed{ false };

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

enum e_button_table { 
  PIN,
  ID,
  FIRST,
  SECOND,
  THIRD,
  FOUR,
  LONG 
};

const byte button_table[][7] = {
//  I/O Button      key first press   second    third   fourth press, keylongPress
  { 38, TOP_CENTER, KEY_F1,           KEY_ESC,  0,      0,            KEY_ESC },
  { 10, RH_MIDDLE,  KEY_F6,           KEY_ESC,  KEY_F4, KEY_ESC,      KEY_ESC },
  { 12, RH_LOWER,   KEY_ESC,          0,        0,      0,            KEY_ESC },
  { 37, STF,        'S',              'V',      0,      0,            0 },
  { 35, JOY_UP,     KEY_UP_ARROW,     0,        0,      0,            0 },
  { 33, JOY_LEFT,   KEY_LEFT_ARROW,   0,        0,      0,            0 },
  { 34, JOY_RIGHT,  KEY_RIGHT_ARROW,  0,        0,      0,            0 },
  { 36, JOY_DOWN,   KEY_DOWN_ARROW,   0,        0,      0,            0 },
  { 0, JOY_PRESS,   KEY_RETURN,       0,        0,      0,            0 }
};

byte unused_buttons[] = {  // mapping array from definitions to set up the pins of all unused I/O
  1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 14, 21, 45, 46
};

#define NUMBUTTONS 9  //gives size of array *helps for adding buttons
#define NUM_UNUSED_BUTTONS sizeof(unused_buttons)

AceButton buttons[NUMBUTTONS];
unsigned int activations[NUMBUTTONS];
void handleButtonEvent(AceButton*, uint8_t, uint8_t);
bool OTAmode{false};

const char* ssid = "XCStick20";
const char* password = "xcstick20";

WebServer server(80);

const char* indexHtml =
    "<body style='font-family: Verdana,sans-serif; font-size: 14px;'>"
    "<meta name='viewport' content='width=>device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no' />"
    "<div style='width:320px;padding:20px;border-radius:10px;border:solid 2px #e0e0e0;margin:auto;margin-top:20px;'>"
    "<div style='width:100%;text-align:center;font-size:18px;font-weight:bold;margin-bottom:12px;'>XCStick OTA SW Update</div>"
       "<form method='POST' action='#' enctype='multipart/form-data' id='upload-form' style='width:100%;margin-bottom:8px;'>"
         "<input type='file' name='update'>"
         "<input type='submit' value='Update' style='float:right;'>"
       "</form>"
    "<div style='width:100%;background-color:#e0e0e0;border-radius:8px;'>"
    "<div id='prg' style='width:0%;background-color:#2196F3;padding:2px;border-radius:8px;color:white;text-align:center;'>0%</div>"
    "</div>"
    "</div>"
    "</body>"
    "<script>"
    "var prg = document.getElementById('prg');"
    "var form = document.getElementById('upload-form');"
    "form.addEventListener('submit', e=>{"
         "e.preventDefault();"
         "var data = new FormData(form);"
         "var req = new XMLHttpRequest();"
         "req.open('POST', '/');"  
         "req.upload.addEventListener('progress', p=>{"
              "let w = Math.round((p.loaded / p.total)*100) + '%';"
              "if(p.lengthComputable){"
                   "prg.innerHTML = w;"
                   "prg.style.width = w;"
              "}"
              "if(w == '100%'){ prg.style.backgroundColor = '#04AA6D'; }" 
         "});"
         "req.send(data);"
     "});"
    "</script>";

void ESPS2OTA(WebServer *server){
  static WebServer *_server = server;
  //Returns index.html page
  _server->on("/", HTTP_GET, [&]() {
    _server->sendHeader("Connection", "close");
    _server->send(200, "text/html", indexHtml);
  });

  /*handling uploading firmware file */
  _server->on("/", HTTP_POST, [&]() {
    _server->sendHeader("Connection", "close");
    _server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  }, [&]() {
    HTTPUpload& upload = _server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        delay(1000);
        ESP.restart();
      } else {
        Update.printError(Serial);
      }
    }
  });
}

void setup() {
  Serial.begin(115200);
  for (byte b = 0; b < NUMBUTTONS; b++) {  //setup of button pin hardware
    pinMode(button_table[b][PIN], INPUT_PULLUP);
  }
  if( digitalRead( button_table[TOP_CENTER][PIN] ) == 0 ){
    delay(1000);
    setCpuFrequencyMhz(160);      
    Serial.println("OTA mode startup");
    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_2dBm);
    WiFi.softAP(ssid, password);
    delay(1000);
    IPAddress IP = IPAddress (10, 10, 10, 1);
    IPAddress NMask = IPAddress (255, 255, 255, 0);
    WiFi.softAPConfig(IP, IP, NMask);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("OTA AP IP address: ");
    Serial.println(myIP); 
    /* SETUP YOR WEB OWN ENTRY POINTS */
    server.on("/test", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", "Hello test 123!");
    });
    ESPS2OTA(&server);
    server.begin();
    Keyboard.begin();
    Serial.printf("XCStick OTA mode started: CPU: %f Mhz restart cause %d\n", (float)getCpuFrequencyMhz(), esp_reset_reason() );
    esp_task_wdt_init(WDT_TIMEOUT, true);  // enable panic so ESP32 restarts
    esp_task_wdt_add(NULL);                // add current thread to WDT watch
    OTAmode = true;
  }
  else {
    delay(1000);
    Serial.println("Keyboard Mode");
    setCpuFrequencyMhz(80);                // save energy, its far enough
    modeS2F = false;
    ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(handleButtonEvent);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    for (byte b = 0; b < NUMBUTTONS; b++) {  //setup of button pin hardware
      buttons[b].init(button_table[b][PIN], HIGH, b);
    }
    // Serial.println("Init unsed buttons");
    for (byte b = 0; b < NUM_UNUSED_BUTTONS; b++) {  //setup of button pin hardware
      pinMode(unused_buttons[b], INPUT_PULLUP);
    }
    Keyboard.begin();
    delay(1000);
    Serial.printf("XCStick kbd started: CPU: %f Mhz restart cause %d\n", (float)getCpuFrequencyMhz(), esp_reset_reason() );
    esp_task_wdt_init(WDT_TIMEOUT, true);  // enable panic so ESP32 restarts
    esp_task_wdt_add(NULL);                // add current thread to WDT watch
  }
}

void loop() {
  if( OTAmode ){
    server.handleClient();
  }else{
    for (int i = 0; i < NUMBUTTONS; i++) {
      buttons[i].check();
    }
  }
  delay(4);
  if ( digitalRead( button_table[RH_LOWER][PIN] ) ){  // ESC button long press for watchdog expiry and restart
      esp_task_wdt_reset();
  }
}

void release(byte key1, byte key2, byte key3, byte key4, byte keyL=0) {
  Keyboard.release(key1);
  if (key2)                 // is there a long press key ?
    Keyboard.release(key2);  // yep -> release
  if (key3)                 // is there a long press key ?
    Keyboard.release(key3);  // yep -> release
  if (key4)                 // is there a long press key ?
    Keyboard.release(key4);  // yep -> release
  if (keyL) 
    Keyboard.release(keyL);
}

void handleButtonEvent(AceButton* button, uint8_t eventType, uint8_t state) {
  Serial.printf("Button: %d evt: %d\n", button->getId(), eventType);
  int id = button->getId();
  byte key1 = button_table[id][FIRST];
  byte key2 = button_table[id][SECOND];
  byte key3 = button_table[id][THIRD];
  byte key4 = button_table[id][FOUR];
  byte keyL = button_table[id][LONG];

  int modul = 1;
  if (key2)
    modul++;
  if (key3)
    modul++;
  if (key4)
    modul++;

  switch (eventType) {
    case AceButton::kEventPressed:
      if (button_table[id][ID] == JOY_PRESS)
        joy_pressed = true;
      
      if (activations[id] % modul == 0) {
        Keyboard.press(key1);
      } else if (key2 && activations[id] % modul == 1) {
        Keyboard.press(key2);
      } else if (key3 && activations[id] % modul == 2) {
        Keyboard.press(key3);
      } else if (key4 && activations[id] % modul == 3) {
        Keyboard.press(key4);
      }
      activations[id]++;
      if (activations[id] >= modul)
        activations[id] = 0;
      break;

    case AceButton::kEventReleased:
      if (button_table[id][ID] == JOY_PRESS)
        joy_pressed = false;
      release(key1,key2,key3,key4,keyL);
      if (button_table[id][FIRST] == KEY_ESC){
        for (byte b = 0; b < NUMBUTTONS; b++) {
           activations[b] = 0;
         }
      }
      break;

    case AceButton::kEventLongPressed:
      if (keyL) {
        release(key1,key2,key3,key4);
        Keyboard.press(KEY_ESC);    // return from previous screen
        Keyboard.release(KEY_ESC);  // release ESC key
        Keyboard.press(keyL);       // key for long press action is pressed
      }
      break;
  }
}
