#if defined(ARDUINO_M5Stack_Core_ESP32)
#define M5STACK_MPU6886
#include <M5Stack.h>
#elif defined(ARDUINO_M5STACK_Core2)
#include <M5Core2.h>
#elif defined(ARDUINO_M5Stick_C)
#include <M5StickC.h>
#elif defined(ARDUINO_M5Stick_C_Plus)
#include <M5StickCPlus.h>
#elif defined(ARDUINO_M5Stack_ATOM)
#include <M5Atom.h>
// Colours
#define WHITE CRGB::White
#define BLACK CRGB::Black
#define RED   CRGB::Red
#define GREEN CRGB::Green
#define BLUE  CRGB::Blue
#elif defined(ARDUINO_WIO_TERMINAL)
#include "WioTerminal_utils.h"
// Display
#include"TFT_eSPI.h"
TFT_eSPI tft;
// Colours
#define WHITE TFT_WHITE
#define BLACK TFT_BLACK
#define RED   TFT_RED
#define GREEN TFT_GREEN
#define BLUE  TFT_BLUE
// For IMU
#include "LIS3DHTR.h"
#include <SPI.h>
LIS3DHTR<TwoWire> lis;
// Tone
SPEAKER Beep;
#endif
#if defined(ARDUINO_WIO_TERMINAL)
// Dirty hack avoid conflicting min/max macro and function
#undef min
#undef max
#include <rpcBLEDevice.h>
#else
#include "utility/MahonyAHRS.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#endif
#include <BLEServer.h>
#include <BLE2902.h>

#if !defined(ARDUINO_WIO_TERMINAL)
//// GPIO
// for PortB
#define PIN0_INPUT GPIO_NUM_36 // analog input
#define PIN1_INPUT GPIO_NUM_26
#endif

// Mic for M5StickC/Plus
#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
#include <driver/i2s.h>

#define PIN_CLK  0
#define PIN_DATA 34
#define READ_LEN (2 * 256)
#define SAMPLING_RATE 11025
uint8_t BUFFER[READ_LEN] = {0};

int16_t *adcBuffer = NULL;
int soundLevel = 0;

void i2sInit()
{
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  SAMPLING_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
  };

  i2s_pin_config_t pin_config;
  pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
  pin_config.ws_io_num    = PIN_CLK;
  pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  pin_config.data_in_num  = PIN_DATA;

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLING_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void mic_record_task (void* arg)
{
  size_t bytesread;
  while (1) {
    int total = 0;
    i2s_read(I2S_NUM_0, (char*) BUFFER, READ_LEN, &bytesread, (100 / portTICK_RATE_MS));
    adcBuffer = (int16_t *)BUFFER;
    for (int i = 0; i < READ_LEN / 2; i++) {
      total += abs(adcBuffer[i]);
    }
    soundLevel = total / (READ_LEN / 2);
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}
#endif

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
uint8_t cmd[] = {0x02, // microbit version (v1:0x01, v2:0x02)
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

// ANALOG PIN 2 byte
uint8_t analog[] = {0x00, 0x00};

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic[9] = {0};
bool deviceConnected = false;

// for pixel pattern
#define TEXT_SPACE 30
uint16_t pixel[5][5] = {0};

void drawPixel(int x, int y, int c) {
#if !defined(ARDUINO_WIO_TERMINAL) && !defined(ARDUINO_M5Stack_ATOM)
  int w = M5.Lcd.width();
  int h = M5.Lcd.height();
#elif defined(ARDUINO_WIO_TERMINAL)
  int w = 320;
  int h = 240;
#endif
#if !defined(ARDUINO_M5Stack_ATOM)
  int ps = (w < (h - TEXT_SPACE)) ? w / 5 : (h - TEXT_SPACE) / 5; // Pixel size
#endif

#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillRect(x * ps, y * ps + TEXT_SPACE, ps, ps, c);
#elif !defined(ARDUINO_M5Stack_ATOM)
  M5.Lcd.fillRect(x * ps, y * ps + TEXT_SPACE, ps, ps, c);
#else
  M5.dis.drawpix(x, y, c);
#endif
};

void displayShowPixel() {
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      log_i("%1d", pixel[y][x] & 0b1);
      if (pixel[y][x] & 0b1) {
        drawPixel(x, y, RED);
      } else {
        drawPixel(x, y, BLACK);
      }
    }
  }
};

void fillScreen(int c) {
#if !defined(ARDUINO_M5Stack_ATOM) && !defined(ARDUINO_WIO_TERMINAL)
  M5.Lcd.fillScreen(c);
#elif defined(ARDUINO_M5Stack_ATOM)
  for (int x = 0; x < 5; x++) {
    for (int y = 0; y < 5; y++) {
      drawPixel(x, y, c);
    }
  }
#else // ARDUINO_WIO_TERMINAL
  tft.fillScreen(c);
#endif
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer * pServer) {
      log_i("connect\n");
      deviceConnected = true;
      fillScreen(WHITE);
    };

    void onDisconnect(BLEServer * pServer) {
      log_i("disconnect\n");
      deviceConnected = false;
#if !defined(ARDUINO_WIO_TERMINAL)
      ESP.restart();
#else
      setup();
#endif
    }
};

// dummy callback
class DummyCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      log_i("DUMMY Read\n");
    }
    void onWrite(BLECharacteristic * pCharacteristic) {
      log_i("DUMMY Write\n");
    }
};

// for cmd

class CmdCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      log_i("CMD read\n");
      pCharacteristic->setValue(cmd, 20);
    }

    void onWrite(BLECharacteristic * pCharacteristic) {
      log_i("CMD write\n");
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
      log_i("CMD len:%d\n", value.length());
      log_i("%s\n", value.c_str());
      const char *cmd_str = value.c_str();
      log_i("%s\n", cmd_str);
      char cmd = (cmd_str[0] >> 5);
      if (cmd == 0x02) {
        //// CMD_DISPLAY  0x02
        log_i("CMD display\n");
        char cmd_display = cmd_str[0] & 0b11111;
        if (cmd_display == 0x00) {
          // CLEAR    0x00
          log_i(">> clear\n");
          fillScreen(BLACK);
        } else if (cmd_display == 0x01) {
          // TEXT     0x01
          log_i(">> text\n");
          log_i("%s\n", &(cmd_str[1]));
#if !defined(ARDUINO_M5Stack_ATOM) && !defined(ARDUINO_WIO_TERMINAL)
          M5.Lcd.fillRect(0, 0, M5.Lcd.width(), TEXT_SPACE - 1, BLACK);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println(&(cmd_str[1]));
#elif defined(ARDUINO_WIO_TERMINAL)
          tft.fillRect(0, 0, 320, TEXT_SPACE - 1, BLACK);
          tft.drawString(String(&(cmd_str[1])), 0, 0);
#else
          // Not implemented yet for ATOM Matrix
#endif
        } else if (cmd_display == 0x02) {
          // PIXELS_0 0x02
          log_i(">> pixel0\n");
          for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 5; x++) {
              pixel[y][x] = (cmd_str[y * 5 + (x + 1)] & 0xb);
            }
          }
        } else if (cmd_display == 0x03) {
          // PIXELS_1 0x03
          log_i(">> pixel1\n");
          for (int y = 3; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
              pixel[y][x] = (cmd_str[(y - 3) * 5 + (x + 1)] & 0xb);
            }
          }
          displayShowPixel();
        }
      } else if (cmd == 0x03) {
        //// CMD_AUDIO  0x03
        log_i("CMD audio\n");
        char cmd_audio = cmd_str[0] & 0b11111;
        if (cmd_audio == 0x00) {
          // STOP_TONE  0x00
          log_i(">> Stop tone\n");
#if defined(ARDUINO_M5Stack_Core_ESP32)
          M5.Speaker.mute();
#elif defined(ARDUINO_M5Stick_C_Plus)
          M5.Beep.mute();
#elif defined(ARDUINO_WIO_TERMINAL)
          Beep.mute();
#endif
        } else if (cmd_audio == 0x01) {
          // PLAY_TONE  0x01
          const uint8_t max_volume = 5;
          log_i(">> Play tone\n");
          uint32_t duration = (cmd_str[4] & 0xff) << 24
                              | (cmd_str[3] & 0xff) << 16
                              | (cmd_str[2] & 0xff) << 8
                              | (cmd_str[1] & 0xff);
          uint16_t freq = 1000000 / duration;
          uint8_t volume = map(cmd_str[5], 0, 255, 0, max_volume);
          log_i("Volume:%d\n", volume);
          log_i("Duration:%d\n", duration);
          log_i("Freq:%d\n", freq);
#if defined(ARDUINO_M5Stack_Core_ESP32)
          M5.Speaker.setVolume(volume);
          M5.Speaker.tone(freq);
#elif defined(ARDUINO_M5Stick_C_Plus)
          M5.Beep.setVolume(volume);
          M5.Beep.tone(freq);
#elif defined(ARDUINO_WIO_TERMINAL)
          Beep.setVolume(volume);
          Beep.tone(freq);
#endif
        }
      }
    }
};

// for state
class StateCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      float temp = 0;
#if !defined(ARDUINO_WIO_TERMINAL)
      M5.IMU.getTempData(&temp); // get temperature from IMU
      state[4] = (random(256) & 0xff); // Random sensor value for lightlevel
#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
      state[6] = ((int)map(soundLevel, 0, 1024, 0, 255) & 0xff); // Random sensor value for soundlevel
#else
      state[6] = (random(256) & 0xff); // Random sensor value for soundlevel
#endif
#else
      temp = lis.getTemperature();
      int light = (int)map(analogRead(WIO_LIGHT), 0, 511, 0, 255);
      log_i(">> Light Level " + String(light));
      state[4] = (light & 0xff); // lightlevel
      int mic = (int)map(analogRead(WIO_MIC), 0, 511, 0, 255);
      state[6] = (mic & 0xff); // soundlevel
      log_i(">> sound Level " + String(mic));
#endif
      state[5] = ((int)(temp + 128) & 0xff); // temperature(+128)
      log_i("STATE read %s", (char *)state);
      pCharacteristic->setValue(state, 7);
    }
};

// for accelerometer related values
#define ACC_MULT 512
#define RAD_TO_DEG 57.324
float ax, ay, az;
int16_t iax, iay, iaz;
int16_t gx, gy, gz;
float pitch , roll, yaw;

void updateIMU() {
#if !defined(ARDUINO_WIO_TERMINAL)
  M5.IMU.getAccelData(&ax, &ay, &az); // get accel
  M5.IMU.getGyroAdc(&gx, &gy, &gz);   // get gyro
  MahonyAHRSupdateIMU(gx, gy, gz, ax, ay, az, &pitch, &roll, &yaw);
#else
  lis.getAcceleration(&ay, &ax, &az);
  pitch = atan(-ax / sqrtf(ay * ay + az * az)) * RAD_TO_DEG;
  roll = atan(ay / az) * RAD_TO_DEG;
#endif
  iax = (int16_t)(ax * ACC_MULT);
  iay = (int16_t)(ay * ACC_MULT);
  iaz = (int16_t)(az * ACC_MULT);
}

class MotionCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      updateIMU();

      motion[0] = ((int)(pitch * ACC_MULT) & 0xff);
      motion[1] = (((int)(pitch * ACC_MULT) >> 8 ) & 0xff);
      motion[2] = ((int)(roll * ACC_MULT) & 0xff);
      motion[3] = (((int)(roll * ACC_MULT) >> 8 ) & 0xff);
      motion[4] = (iax & 0xff);
      motion[5] = ((iax >> 8 ) & 0xff);
      motion[6] = (iay & 0xff);
      motion[7] = ((iay >> 8 ) & 0xff);
      motion[8] = (-iaz & 0xff);
      motion[9] = ((-iaz >> 8 ) & 0xff);
      pCharacteristic->setValue(motion, 20);

      // debug print
      char msg[256] = {0};
      for (int i = 0; i < sizeof(motion); i++) {
        sprintf(&msg[i * 3], "%02x,", motion[i], sizeof(motion) * 3 - 3 * i);
      }
      log_i("MOTION read: %s\n", msg);
    }
};

// for button
class ActionCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
      log_i("BTN read\n");
      pCharacteristic->setValue("Read me!!"); // dummy data
    }
};

// for Analog pin
class AnalogPinCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic * pCharacteristic) {
#if !defined(ARDUINO_WIO_TERMINAL)
      int r = map(analogRead(PIN0_INPUT), 0, 4095, 0, 1023);
#else
      int r = analogRead(0);
#endif
      log_i("Analog Pin0 Read:%d\n", r);

      analog[0] = (r & 0xff);
      analog[1] = ((r >> 8 ) & 0xff);

      pCharacteristic->setValue(analog, 2);
    }
};

void setup() {
  Serial.begin(115200);
#if !defined(ARDUINO_WIO_TERMINAL)
#if !defined(ARDUINO_M5Stack_ATOM)
  M5.begin();
#else
  M5.begin(true, false, true);
#endif
#endif

#if defined(ARDUINO_M5Stick_C_Plus)
  // Disable Pin25 to use Pin36.
  gpio_pulldown_dis(GPIO_NUM_25);
  gpio_pullup_dis(GPIO_NUM_25);
#endif

#if !defined(ARDUINO_WIO_TERMINAL)
  M5.IMU.Init(); // IMU for temperature, accel and gyro
#else
  // Display
  tft.begin();
  tft.setRotation(3);

  // IMU
  lis.begin(Wire1);
  delay(100);
  lis.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G);
  lis.openTemp();
  //// Button
  // 5 way switch
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  // 3 Configurable Button
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  // Light sensor
  pinMode(WIO_LIGHT, INPUT);
  // microphone
  pinMode(WIO_MIC, INPUT);
  // LED
  pinMode(LED_BUILTIN, OUTPUT);
#endif

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
  // for Mic input
  i2sInit();
  xTaskCreate(mic_record_task, "mic_record_task", 2048, NULL, 1, NULL);
#endif

  // Create MAC address base fixed ID
  uint8_t mac0[6] = {0};
#if !defined(ARDUINO_WIO_TERMINAL)
  esp_efuse_mac_get_default(mac0);
#else
  // Create random mac address for avoid conflict ID.
  randomSeed(analogRead(A0));
  for (int i = 0; i < sizeof(mac0); i++) {
    mac0[i] = random(256);
  }
#endif
  String ID;
  for (int i = 0; i < 6; i++) {
    char ID_char = (((mac0[i] - 0x61) & 0b0011110) >> 1) + 0x61;
    ID += ID_char;
  }
  log_i("ID char:%s\n", ID.c_str());
  char adv_str[32] = {0};
  String("BBC micro:bit [" + ID + "]").toCharArray(adv_str, sizeof(adv_str));

#if !defined(ARDUINO_M5Stack_ATOM) && !defined(ARDUINO_WIO_TERMINAL)
  M5.Lcd.begin();
  M5.Lcd.fillScreen(BLACK);
#endif

  // Start up screen
  fillScreen(BLUE);
#if defined(ARDUINO_WIO_TERMINAL)
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.print("Welcome to\nM5bit Less!!\n\nPlease connect to\n");
  tft.println(adv_str);
#else
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
#endif

  log_i("BLE start.\n");
  log_i("%s\n", adv_str);
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
  pCharacteristic[5]->setCallbacks(new AnalogPinCallbacks());
  pCharacteristic[5]->addDescriptor(new BLE2902());

  pCharacteristic[6] = pService->createCharacteristic(
                         MBIT_MORE_CH_ANALOG_IN_P1,
                         BLECharacteristic::PROPERTY_READ
                       );
  pCharacteristic[6]->setCallbacks(new AnalogPinCallbacks());
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

void sendBtn(uint8_t btnID, uint8_t btn, uint8_t btn_status, uint8_t prev) {
  memset((char *)(action), 0, 20); // clear action buffer

  action[0] = 0x01; // for Button event
  action[19] = 0x12; // ACTION_EVENT

  action[1] = btnID; // btnID 0x01:BtnA, 0x02:BtnB, 121:BtnC(LOGO)

  // Set TimeStamp (Little Endian)
  uint32_t time = (uint32_t)millis();
  action[4] = (time & 0xff);
  action[5] = (time >> 8) & 0xff;
  action[6] = (time >> 16) & 0xff;
  action[7] = (time >> 24) & 0xff;

  if (btn) {
    // Button CLICK
    log_i(" button clicked!\n");
    action[3] = 0x03;
    pCharacteristic[4]->setValue(action, 20);
    pCharacteristic[4]->notify();
  }
  if (btn_status == 0 && prev == 1) {
    // Button Up
    log_i(" button up!\n");
    action[3] = 0x02;
    pCharacteristic[4]->setValue(action, 20);
    pCharacteristic[4]->notify();
  } else if (btn_status == 1 && prev == 0) {
    // Button Down
    log_i(" button down!\n");
    action[3] = 0x01;
    pCharacteristic[4]->setValue(action, 20);
    pCharacteristic[4]->notify();
  }
}

// Previous button state
uint8_t prevA = 0, prevB = 0, prevC = 0;
uint32_t old_label_time = 0;

void loop() {
  if (deviceConnected) {
    // Send notify data for button A, B and C(LOGO).
    uint8_t btnA = 0, btnB = 0, btnC = 0,
            btn_statusA = 0, btn_statusB = 0, btn_statusC = 0;

    // Get all button status
#if defined(ARDUINO_WIO_TERMINAL)
    if (digitalRead(WIO_KEY_A) == LOW) {
      btnA = 1;
      btn_statusA = 1;
    }
    if (digitalRead(WIO_KEY_B) == LOW) {
      btnB = 1;
      btn_statusB = 1;
    }
    if (digitalRead(WIO_KEY_C) == LOW) {
      btnC = 1;
      btn_statusC = 1;
    }
#else
#if defined(ARDUINO_M5Stack_ATOM)
    btnA = M5.Btn.wasPressed();
    btn_statusA = M5.Btn.isPressed();
#else
    btnA = M5.BtnA.wasPressed();
    btn_statusA = M5.BtnA.isPressed();
    btnB = M5.BtnB.wasPressed();
    btn_statusB = M5.BtnB.isPressed();
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
    btnC = M5.BtnC.wasPressed();
    btn_statusC = M5.BtnC.isPressed();
#endif
#endif
#endif

    //// Button A
    action[1] = 0x01;
    sendBtn(0x01, btnA, btn_statusA, prevA);
    prevA = btn_statusA;

    //// Button B
    action[1] = 0x02;
    sendBtn(0x02, btnB, btn_statusB, prevB);
    prevB = btn_statusB;

    //// Button C (LOGO)
    action[1] = 121; // LOGO 121
    sendBtn(121, btnC, btn_statusC, prevC);
    prevC = btn_statusC;

    updateGesture();

    // Send dummy data label='a' data=random('a'-'z') every 50ms
    uint32_t label_time = (uint32_t)millis();
    if (label_time - old_label_time > 50) {
      memset((char *)(action), 0, 20); // clear action buffer
      action[19] = 0x14; // DATA_TEXT
      action[0] = 0x61; // 'a'
      action[1] = 0;
      action[8] = 0x61 + random(26); // 'a-z'
      action[9] = 0;
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
      old_label_time = label_time;
    }
  }

#if !defined(ARDUINO_WIO_TERMINAL)
  M5.update();
#endif
#if defined(ARDUINO_WIO_TERMINAL)
  Beep.update();
#endif
}
