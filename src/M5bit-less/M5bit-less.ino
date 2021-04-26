#if defined(ARDUINO_M5Stack_Core_ESP32)
#define M5STACK_MPU6886
#include <M5Stack.h>
#elif defined(ARDUINO_M5Stick_C)
#include <M5StickC.h>
#elif defined(ARDUINO_M5Stack_ATOM)
#include <M5Atom.h>
// Colours
#define WHITE CRGB::White
#define BLACK CRGB::Black
#define RED   CRGB::Red
#define GREEN CRGB::Green
#define BLUE  CRGB::Blue
#endif
#include "utility/MahonyAHRS.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define MSG(msg)  {Serial.print(msg);}
#define MSGLN(msg)  {Serial.println(msg);}
#define MSGF(msg)  {Serial.printf(msg);}

#define MBIT_MORE_SERVICE          "0b50f3e4-607f-4151-9091-7d008d6ffc5c"
#define MBIT_MORE_CH_COMMAND       "0b500100-607f-4151-9091-7d008d6ffc5c" // R&W(20byte)
#define MBIT_MORE_CH_STATE         "0b500101-607f-4151-9091-7d008d6ffc5c" // R(7byte)
#define MBIT_MORE_CH_MOTION        "0b500102-607f-4151-9091-7d008d6ffc5c" // R(18byte)    :pitch,roll,accel,and gyro 
#define MBIT_MORE_CH_PIN_EVENT     "0b500110-607f-4151-9091-7d008d6ffc5c" // R&N
#define MBIT_MORE_CH_ACTION_EVENT  "0b500111-607f-4151-9091-7d008d6ffc5c" // R&N(20byte)  :Buttons with timestamp 
#define MBIT_MORE_CH_ANALOG_IN_P0  "0b500120-607f-4151-9091-7d008d6ffc5c" // R
#define MBIT_MORE_CH_ANALOG_IN_P1  "0b500121-607f-4151-9091-7d008d6ffc5c" // R
#define MBIT_MORE_CH_ANALOG_IN_P2  "0b500122-607f-4151-9091-7d008d6ffc5c" // R
#define MBIT_MORE_CH_MESSAGE       "0b500130-607f-4151-9091-7d008d6ffc5c" // R : only for v2
#define ADVERTISING_STRING         "BBC micro:bit [m5scr]"

// COMMAND CH 20byte
uint8_t cmd[] = {0x01, // microbit version (v1:0x01, v2:0x02)
                 0x02, // protocol 0x02 only
                 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00
                };

// STATE CH 7byte
uint8_t state[] = {0x00, 0x00, 0x00, 0x00, // GPIO 0-3
                   0x00, // lightlevel
                   0x00, // temperature(+128)
                   0x00  // soundlevel
                  };

// MOTION CH 18 byte
uint8_t motion[] = {0x00, 0x00, // pitch
                    0x00, 0x00, // roll
                    0xff, 0xff, // ax
                    0xff, 0x00, // ay
                    0x00, 0xff, // az
                    0x00, 0x00, // gx
                    0x00, 0x00, // gy
                    0x00, 0x00, // gz
                    0x00, 0x00 // ??
                   };

// ACTION CH 20 byte
uint8_t action[] = {0x01, // BUTTON cmd; BUTTON:0x01, GESTURE: 0x02
                    0x01, 0x00, // Button Name;1:A,2:B,100:P0,101:P1,102:P2,121:LOGO
                    0x00, // Event Name;1:DOWN, 2:UP, 3:CLICK, 4:LONG_CLICK, 5:HOLD, 6:DOUBLE_CLICK
                    0x00, 0x00, 0x00, 0x00, // Timestamp
                    0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00,
                    0x12 // ACTION Event
                   };

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic[9] = {0};
bool deviceConnected = false;

// for pixel pattern
#define TEXT_SPACE 30
uint16_t pixel[5][5] = {0};

void drawPixel(int x, int y, int c) {
#if !defined(ARDUINO_M5Stack_ATOM)
  int w = M5.Lcd.width();
  int h = M5.Lcd.height();
  int ps = (w < (h - TEXT_SPACE)) ? w / 5 : (h - TEXT_SPACE) / 5; // Pixel size

  M5.Lcd.fillRect(x * ps, y * ps + TEXT_SPACE, ps, ps, c);
#else
  M5.dis.drawpix(x, y, c);
#endif
};

void displayShowPixel() {
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      MSG(pixel[y][x] & 0b1);
      if (pixel[y][x] & 0b1) {
        drawPixel(x, y, RED);
      } else {
        drawPixel(x, y, BLACK);
      }
    }
  }
};

void fillScreen(int c) {
#if !defined(ARDUINO_M5Stack_ATOM)
  M5.Lcd.fillScreen(c);
#else
  for (int x = 0; x < 5; x++) {
    for (int y = 0; y < 5; y++) {
      drawPixel(x, y, c);
    }
  }
#endif
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer * pServer) {
      MSGLN("connect");
      deviceConnected = true;
      fillScreen(WHITE);
    };

    void onDisconnect(BLEServer * pServer) {
      MSGLN("disconnect");
      deviceConnected = false;
      setup();
    }
};

// dummy callback
class DummyCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      MSGLN("DUMMY Read");
    }
    void onWrite(BLECharacteristic * pCharacteristic) {
      MSGLN("DUMMY Write");
    }
};

// for cmd

class CmdCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      MSGLN("CMD read");
      pCharacteristic->setValue(cmd, 20);
    }

    void onWrite(BLECharacteristic * pCharacteristic) {
      MSGLN("CMD write");
      ////// MUST implement!!
      //// CMD_CONFIG 0x00
      // MIC    0x01
      // TOUCH  0x02
      //// CMD_PIN  0x01
      // SET_OUTPUT 0x01
      // SET_PWM    0x02
      // SET_SERVO  0x03
      // SET_PULL   0x04
      // SET_EVENT  0x05
      //// CMD_DATA (only v2) 0x04

      std::string value = pCharacteristic->getValue();
      MSGLN("CMD len:" + String(value.length()));
      MSGLN(value.c_str());
      const char *cmd_str = value.c_str();
      MSGLN(cmd_str);
      char cmd = (cmd_str[0] >> 5);
      if (cmd == 0x02) {
        //// CMD_DISPLAY  0x02
        MSGLN("CMD display");
        char cmd_display = cmd_str[0] & 0b11111;
        if (cmd_display == 0x00) {
          // CLEAR    0x00
          MSGLN(">> clear");
          fillScreen(BLACK);
        } else if (cmd_display == 0x01) {
          // TEXT     0x01
          MSGLN(">> text");
          MSGLN(&(cmd_str[1]));
#if !defined(ARDUINO_M5Stack_ATOM)
          M5.Lcd.fillRect(0, 0, M5.Lcd.width(), TEXT_SPACE - 1, BLACK);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println(&(cmd_str[1]));
#else
          // Not implemented yet
#endif
        } else if (cmd_display == 0x02) {
          // PIXELS_0 0x02
          MSGLN(">> pixel0");
          for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 5; x++) {
              pixel[y][x] = (cmd_str[y * 5 + (x + 1)] & 0xb);
            }
          }
        } else if (cmd_display == 0x03) {
          // PIXELS_1 0x03
          MSGLN(">> pixel1");
          for (int y = 3; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
              pixel[y][x] = (cmd_str[(y - 3) * 5 + (x + 1)] & 0xb);
            }
          }
          displayShowPixel();
        }
      } else if (cmd == 0x03) {
        //// CMD_AUDIO  0x03
        MSGLN("CMD audio");
        char cmd_audio = cmd_str[0] & 0b11111;
        if (cmd_audio == 0x00) {
          // STOP_TONE  0x00
          MSGLN(">> Stop tone");
#if defined(ARDUINO_M5Stack_Core_ESP32)
          M5.Speaker.mute();
#endif
        } else if (cmd_audio == 0x01) {
          // PLAY_TONE  0x01
          const uint8_t max_volume = 5;
          MSGLN(">> Play tone");
          uint32_t duration = (cmd_str[4] & 0xff) << 24
                              | (cmd_str[3] & 0xff) << 16
                              | (cmd_str[2] & 0xff) << 8
                              | (cmd_str[1] & 0xff);
          uint16_t freq = 1000000 / duration;
          uint8_t volume = (uint8_t)(max_volume * cmd_str[5] / 255.0);
          MSGLN("Volume:" + String(volume));
          MSGLN("Duration:" + String(duration));
          MSGLN("Freq:" + String(freq));
#if defined(ARDUINO_M5Stack_Core_ESP32)
          M5.Speaker.setVolume(volume);
          M5.Speaker.tone(freq);
#endif
        }
      }
    }
};

// for state
class StateCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      float temp = 0;
      M5.IMU.getTempData(&temp); // get temperature from IMU

      // Now send random sensor values, lightlevel & soundlevel.
      state[4] = (random(256) & 0xff); // lightlevel
      state[5] = ((int)(temp + 128) & 0xff); // temperature(+128)
      state[6] = (random(256) & 0xff); // soundlevel
      MSGLN("STATE read " + String((char *)state));
      pCharacteristic->setValue(state, 7);
    }
};

// for accelerometer related values
#define ACC_MULT 1000
class MotionCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      float ax, ay, az;
      int16_t gx, gy, gz;
      float pitch , roll, yaw;
      M5.IMU.getAccelData(&ax, &ay, &az); // get accel
      M5.IMU.getGyroAdc(&gx, &gy, &gz);   // get gyro
      MahonyAHRSupdateIMU(gx, gy, gz, ax, ay, az, &pitch, &roll, &yaw);

      // Now send fixed accelerometer related values
      MSG("MOTION read:");
      motion[0] = ((int)(pitch * ACC_MULT) & 0xff);
      motion[1] = (((int)(pitch * ACC_MULT) >> 8 ) & 0xff);
      motion[2] = ((int)(roll * ACC_MULT) & 0xff);
      motion[3] = (((int)(roll * ACC_MULT) >> 8 ) & 0xff);
      motion[4] = ((int)(ax * ACC_MULT) & 0xff);
      motion[5] = (((int)(ax * ACC_MULT) >> 8 ) & 0xff);
      motion[6] = ((int)(ay * ACC_MULT) & 0xff);
      motion[7] = (((int)(ay * ACC_MULT) >> 8 ) & 0xff);
      motion[8] = ((int)(-az * ACC_MULT) & 0xff);
      motion[9] = (((int)(-az * ACC_MULT) >> 8 ) & 0xff);
      pCharacteristic->setValue(motion, 20);

      // debug print
      char msg[256] = {0};
      for (int i = 0; i < sizeof(motion); i++) {
        sprintf(&msg[i * 3], "%02x,", motion[i], sizeof(motion) * 3 - 3 * i);
      }
      MSGLN(msg);
    }
};

// for button
class ActionCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      MSGLN("BTN read");
      pCharacteristic->setValue("Read me!!"); // dummy data
    }
};

void setup() {
  Serial.begin(115200);
#if !defined(ARDUINO_M5Stack_ATOM)
  M5.begin();
#else
  M5.begin(true, false, true);
#endif
  M5.IMU.Init(); // IMU for temperature, accel and gyro

  // Create MAC address base fixed ID
  uint8_t mac0[6];
  esp_efuse_mac_get_default(mac0);
  String ID;
  for (int i = 0; i < 6; i++) {
    char ID_char = (((mac0[i] - 0x61) & 0b0011110) >> 1) + 0x61;
    ID += ID_char;
  }
  MSGLN("ID char:" + ID);
  char adv_str[32] = {0};
  String("BBC micro:bit [" + ID + "]").toCharArray(adv_str, sizeof(adv_str));

#if !defined(ARDUINO_M5Stack_ATOM)
  M5.Lcd.begin();
  M5.Lcd.fillScreen(BLACK);
#endif

  // Start up screen
  fillScreen(BLUE);
#if !defined(ARDUINO_M5Stack_ATOM)
#if defined(ARDUINO_M5Stack_Core_ESP32)
  M5.Lcd.setTextSize(2);
#else
  M5.Lcd.setTextSize(1);
#endif
  M5.Lcd.print("Welcome to\nM5bit Less!!\n\nPlease connect to\n");
  M5.Lcd.println(adv_str);
#if defined(ARDUINO_M5Stack_Core_ESP32)
  M5.Lcd.setTextSize(4);
#else
  M5.Lcd.setTextSize(2);
#endif
#endif

  MSGLN("BLE start.");
  MSGLN(adv_str);
#if defined(ARDUINO_M5Stack_Core_ESP32)
  m5.Speaker.mute();
#endif
  BLEDevice::init(adv_str);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(BLEUUID(MBIT_MORE_SERVICE), 27);

  // CMD
  pCharacteristic[0] = pService->createCharacteristic(
                         MBIT_MORE_CH_COMMAND,
                         BLECharacteristic::PROPERTY_READ |
                         BLECharacteristic::PROPERTY_WRITE |
                         BLECharacteristic::PROPERTY_WRITE_NR
                       );
  pCharacteristic[0]->setCallbacks(new CmdCallbacks());
  pCharacteristic[0]->addDescriptor(new BLE2902());

  // STATE
  pCharacteristic[1] = pService->createCharacteristic(
                         MBIT_MORE_CH_STATE,
                         BLECharacteristic::PROPERTY_READ
                       );
  pCharacteristic[1]->setCallbacks(new StateCallbacks());
  pCharacteristic[1]->addDescriptor(new BLE2902());

  // MOTION
  pCharacteristic[2] = pService->createCharacteristic(
                         MBIT_MORE_CH_MOTION,
                         BLECharacteristic::PROPERTY_READ
                       );
  pCharacteristic[2]->setCallbacks(new MotionCallbacks());
  pCharacteristic[2]->addDescriptor(new BLE2902());

  pCharacteristic[3] = pService->createCharacteristic(
                         MBIT_MORE_CH_PIN_EVENT,
                         BLECharacteristic::PROPERTY_READ |
                         BLECharacteristic::PROPERTY_NOTIFY
                       );
  pCharacteristic[3]->setCallbacks(new DummyCallbacks());
  pCharacteristic[3]->addDescriptor(new BLE2902());

  // ACTION
  pCharacteristic[4] = pService->createCharacteristic(
                         MBIT_MORE_CH_ACTION_EVENT,
                         BLECharacteristic::PROPERTY_READ |
                         BLECharacteristic::PROPERTY_NOTIFY
                       );
  pCharacteristic[4]->setCallbacks(new ActionCallbacks());
  pCharacteristic[4]->addDescriptor(new BLE2902());

  // PINS
  pCharacteristic[5] = pService->createCharacteristic(
                         MBIT_MORE_CH_ANALOG_IN_P0,
                         BLECharacteristic::PROPERTY_READ
                       );
  pCharacteristic[5]->setCallbacks(new DummyCallbacks());
  pCharacteristic[5]->addDescriptor(new BLE2902());

  pCharacteristic[6] = pService->createCharacteristic(
                         MBIT_MORE_CH_ANALOG_IN_P1,
                         BLECharacteristic::PROPERTY_READ
                       );
  pCharacteristic[6]->setCallbacks(new DummyCallbacks());
  pCharacteristic[6]->addDescriptor(new BLE2902());


  pCharacteristic[7] = pService->createCharacteristic(
                         MBIT_MORE_CH_ANALOG_IN_P2,
                         BLECharacteristic::PROPERTY_READ
                       );
  pCharacteristic[7]->setCallbacks(new DummyCallbacks());
  pCharacteristic[7]->addDescriptor(new BLE2902());


  // MESSAGE (only for v2)
  pCharacteristic[8] = pService->createCharacteristic(
                         MBIT_MORE_CH_MESSAGE,
                         BLECharacteristic::PROPERTY_READ
                       );
  pCharacteristic[8]->setCallbacks(new DummyCallbacks());
  pCharacteristic[8]->addDescriptor(new BLE2902());


  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

// Previous button state
uint8_t prevA = 0, prevB = 0, prevC = 0;

void loop() {
  uint32_t time = (uint32_t)millis();
  // Set TimeStamp (Little Endian)
  action[4] = (time & 0xff);
  action[5] = (time >> 8) & 0xff;
  action[6] = (time >> 16) & 0xff;
  action[7] = (time >> 24) & 0xff;

  if (deviceConnected) {
    // Send notify data for button A, B and C(LOGO).

    //// Button A
    action[1] = 0x01;
#if !defined(ARDUINO_M5Stack_ATOM)
    uint8_t btnA = M5.BtnA.wasPressed();
    uint8_t btn_status = M5.BtnA.isPressed();
#else
    uint8_t btnA = M5.Btn.wasPressed();
    uint8_t btn_status = M5.Btn.isPressed();
#endif
    if (btnA) {
      // Button CLICK
      MSGLN("Button A clicked!");
      action[3] = 0x03;
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    }
    if (btn_status == 0 && prevA == 1) {
      // Button Up
      MSGLN("Button A up!");
      action[3] = 0x02;
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    } else if (btn_status == 1 && prevA == 0) {
      // Button Down
      MSGLN("Button A down!");
      action[3] = 0x01;
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    }
    prevA = btn_status;

#if !defined(ARDUINO_M5Stack_ATOM)
    //// Button B
    action[1] = 0x02;
    if (M5.BtnB.wasPressed()) {
      // Button CLICK
      MSGLN("Button B clicked!");
      action[3] = 0x03;
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    }
    btn_status = M5.BtnB.isPressed();
    if (btn_status == 0 && prevB == 1) {
      // Button UP
      MSGLN("Button B up!");
      action[3] = 0x02;
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    } else if (btn_status == 1 && prevB == 0) {
      // Button Down
      MSGLN("Button B down!");
      action[3] = 0x01;
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    }
    prevB = btn_status;


#if defined(ARDUINO_M5Stack_Core_ESP32)
    //// Button C (LOGO)
    action[1] = 121; // LOGO 121
    if (M5.BtnC.wasPressed()) {
      MSGLN("Button C (LOGO) clicked!");
      action[3] = 0x03; // Button CLICK
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    }
#endif
#endif
  }
  M5.update();
}
