#define M5STACK_MPU6886
#include <M5Stack.h>
#include "utility/MahonyAHRS.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define MSG(msg)  {M5.Lcd.println(msg);Serial.println(msg);}

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

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer * pServer) {
      MSG("connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer * pServer) {
      MSG("disconnect");
      deviceConnected = false;
    }
};

// for cmd
class CmdCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      MSG("CMD read");
      pCharacteristic->setValue(cmd, 20);
    }

    void onWrite(BLECharacteristic * pCharacteristic) {
      MSG("CMD write");
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
      //// CMD_DISPLAY  0x02
      // CLEAR    0x00
      // TEXT     0x01
      // PIXELS_0 0x02
      // PIXELS_1 0x03
      //// CMD_AUDIO  0x03
      // STOP_TONE  0x00
      // PLAY_TONE  0x01
      //// CMD_DATA (only v2) 0x04

      std::string value = pCharacteristic->getValue();
      MSG("CMD len:" + String(value.length()));
      MSG(value.c_str());
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
      MSG("STATE read " + String((char *)state));
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
      MSG("MOTION read " + String((char *)motion));
      motion[0] = ((int)(pitch * ACC_MULT) & 0xff);
      motion[1] = (((int)(pitch * ACC_MULT) >> 8 ) & 0xff);
      motion[2] = ((int)(roll * ACC_MULT) & 0xff);
      motion[3] = (((int)(roll * ACC_MULT) >> 8 ) & 0xff);
      motion[4] = ((int)(ax * ACC_MULT) & 0xff);
      motion[5] = (((int)(ax * ACC_MULT) >> 8 ) & 0xff);
      motion[6] = ((int)(ay * ACC_MULT) & 0xff);
      motion[7] = (((int)(ay * ACC_MULT) >> 8 ) & 0xff);
      motion[8] = ((int)(az * ACC_MULT) & 0xff);
      motion[9] = (((int)(az * ACC_MULT) >> 8 ) & 0xff);
      pCharacteristic->setValue(motion, 20);
    }
};

// for button
class ActionCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      MSG("BTN read");
      pCharacteristic->setValue("Read me!!"); // dummy data
    }
};

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.IMU.Init(); // IMU for temperature, accel and gyro

  MSG("BLE start.");
  m5.Speaker.mute();

  BLEDevice::init("BBC micro:bit [m5scr]");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(MBIT_MORE_SERVICE);

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
  pCharacteristic[6] = pService->createCharacteristic(
                         MBIT_MORE_CH_ANALOG_IN_P1,
                         BLECharacteristic::PROPERTY_READ
                       );
  pCharacteristic[7] = pService->createCharacteristic(
                         MBIT_MORE_CH_ANALOG_IN_P2,
                         BLECharacteristic::PROPERTY_READ
                       );

  // MESSAGE (only for v2)
  pCharacteristic[8] = pService->createCharacteristic(
                         MBIT_MORE_CH_MESSAGE,
                         BLECharacteristic::PROPERTY_READ
                       );

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  uint32_t time = (uint32_t)millis();
  // Set TimeStamp (Little Endian)
  action[4] = (time & 0xff);
  action[5] = (time >> 8) & 0xff;
  action[6] = (time >> 16) & 0xff;
  action[7] = (time >> 24) & 0xff;

  if (deviceConnected) {
    // Send notify data for button A & B.
    // Now support click only
    if (M5.BtnA.wasPressed()) {
      MSG("Button A clicked!");
      action[1] = 0x01; // Button A
      action[3] = 0x03; // Button CLICK
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    }
    if (M5.BtnB.wasPressed()) {
      MSG("Button B clicked!");
      action[1] = 0x02; // Button B
      action[3] = 0x03; // Button CLICK
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
    }
  }
  M5.update();
}
