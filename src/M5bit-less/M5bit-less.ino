#if !defined(ARDUINO_WIO_TERMINAL)
#include <M5Unified.h>
m5::board_t myBoard = m5gfx::board_unknown;

// PortB A/D, GPIO input
int pin0;
int pin1;
#endif

#include <Wire.h>

// For Stack-chan
bool stackchan_mode = false;

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
#include <FastLED.h>
#define NUM_LEDS 25
#define LED_DATA_PIN 27
CRGB leds[NUM_LEDS];
#endif

#if defined(ARDUINO_WIO_TERMINAL)
#include "WioTerminal_utils.h"
// Display
#include "TFT_eSPI.h"
TFT_eSPI tft;

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
#include <BLEDevice.h>
#include <BLEUtils.h>
#endif
#include <BLEServer.h>
#include <BLE2902.h>

// Mic for M5StickC/Plus
#if !defined(ARDUINO_WIO_TERMINAL)
#include <driver/i2s.h>

#define PIN_CLK 0
#define PIN_DATA 34
#define READ_LEN (2 * 256)
#define SAMPLING_RATE 11025
uint8_t BUFFER[READ_LEN] = { 0 };

int16_t *adcBuffer = NULL;
int soundLevel = 0;

void i2sInit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate = SAMPLING_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
  };

  i2s_pin_config_t pin_config;
  pin_config.bck_io_num = I2S_PIN_NO_CHANGE;
  pin_config.ws_io_num = PIN_CLK;
  pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  pin_config.data_in_num = PIN_DATA;

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLING_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void mic_record_task(void *arg) {
  size_t bytesread;
  while (1) {
    int total = 0;
    i2s_read(I2S_NUM_0, (char *)BUFFER, READ_LEN, &bytesread, (100 / portTICK_RATE_MS));
    adcBuffer = (int16_t *)BUFFER;
    for (int i = 0; i < READ_LEN / 2; i++) {
      total += abs(adcBuffer[i]);
    }
    soundLevel = total / (READ_LEN / 2);
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}
#endif

#define MBIT_MORE_SERVICE "0b50f3e4-607f-4151-9091-7d008d6ffc5c"
#define MBIT_MORE_CH_COMMAND "0b500100-607f-4151-9091-7d008d6ffc5c"       // R&W(20byte)
#define MBIT_MORE_CH_STATE "0b500101-607f-4151-9091-7d008d6ffc5c"         // R(7byte)
#define MBIT_MORE_CH_MOTION "0b500102-607f-4151-9091-7d008d6ffc5c"        // R(18byte)    :pitch,roll,accel,and gyro
#define MBIT_MORE_CH_PIN_EVENT "0b500110-607f-4151-9091-7d008d6ffc5c"     // R&N
#define MBIT_MORE_CH_ACTION_EVENT "0b500111-607f-4151-9091-7d008d6ffc5c"  // R&N(20byte)  :Buttons with timestamp
#define MBIT_MORE_CH_ANALOG_IN_P0 "0b500120-607f-4151-9091-7d008d6ffc5c"  // R
#define MBIT_MORE_CH_ANALOG_IN_P1 "0b500121-607f-4151-9091-7d008d6ffc5c"  // R
#define MBIT_MORE_CH_ANALOG_IN_P2 "0b500122-607f-4151-9091-7d008d6ffc5c"  // R
#define MBIT_MORE_CH_MESSAGE "0b500130-607f-4151-9091-7d008d6ffc5c"       // R : only for v2
#define ADVERTISING_STRING "BBC micro:bit [m5scr]"

// COMMAND CH 20byte
uint8_t cmd[] = { 0x02,  // microbit version (v1:0x01, v2:0x02)
                  0x02,  // protocol 0x02 only
                  0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00 };

// STATE CH 7byte
uint8_t state[] = {
  0x00, 0x00, 0x00, 0x00,  // GPIO 0-3
  0x00,                    // lightlevel
  0x00,                    // temperature(+128)
  0x00                     // soundlevel
};

// MOTION CH 18 byte
uint8_t motion[] = {
  0x00, 0x00,  // pitch
  0x00, 0x00,  // roll
  0xff, 0xff,  // ax
  0xff, 0x00,  // ay
  0x00, 0xff,  // az
  0x00, 0x00,  // gx
  0x00, 0x00,  // gy
  0x00, 0x00,  // gz
  0x00, 0x00   // ??
};

// ACTION CH 20 byte
uint8_t action[] = {
  0x01,                    // BUTTON cmd; BUTTON:0x01, GESTURE: 0x02
  0x01, 0x00,              // Button Name;1:A,2:B,100:P0,101:P1,102:P2,121:LOGO
  0x00,                    // Event Name;1:DOWN, 2:UP, 3:CLICK, 4:LONG_CLICK, 5:HOLD, 6:DOUBLE_CLICK
  0x00, 0x00, 0x00, 0x00,  // Timestamp
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00,
  0x12  // ACTION Event
};

// ANALOG PIN 2 byte
uint8_t analog[] = { 0x00, 0x00 };

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic[9] = { 0 };
bool deviceConnected = false;

// for pixel pattern
#define TEXT_SPACE 30
uint16_t pixel[5][5] = { 0 };

void drawPixel(int x, int y, int c) {
#if defined(ARDUINO_WIO_TERMINAL)
  int w = 320;
  int h = 240;
#else
  int w = M5.Lcd.width();
  int h = M5.Lcd.height();
#endif
  int ps = (w < (h - TEXT_SPACE)) ? w / 5 : (h - TEXT_SPACE) / 5;  // Pixel size

#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillRect(x * ps, y * ps + TEXT_SPACE, ps, ps, c);
#else
  M5.Lcd.fillRect(x * ps, y * ps + TEXT_SPACE, ps, ps, c);
#if !defined(CONFIG_IDF_TARGET_ESP32S3)
  if (myBoard == m5gfx::board_M5Atom) {
    if (c == TFT_BLACK) {
      leds[x + y * 5] = CRGB::Black;
    } else if (c == TFT_RED) {
      leds[x + y * 5] = CRGB::Red;
    } else if (c == TFT_BLUE) {
      leds[x + y * 5] = CRGB::Blue;
    }
    FastLED.show();
  }
#endif
#endif
};

void displayShowPixel() {
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      log_i("%1d", pixel[y][x] & 0b1);
      if (pixel[y][x] & 0b1) {
        drawPixel(x, y, TFT_RED);
      } else {
        drawPixel(x, y, TFT_BLACK);
      }
    }
  }
};

void fillScreen(int c) {
#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillScreen(c);
#else
  if (myBoard == m5gfx::board_M5Atom) {
    for (int x = 0; x < 5; x++) {
      for (int y = 0; y < 5; y++) {
        drawPixel(x, y, c);
      }
    }
  }
  M5.Lcd.fillScreen(c);
#endif
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    log_i("connect\n");
    deviceConnected = true;
    fillScreen(TFT_BLACK);
  };

  void onDisconnect(BLEServer *pServer) {
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
class DummyCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    log_i("DUMMY Read\n");
  }
  void onWrite(BLECharacteristic *pCharacteristic) {
    log_i("DUMMY Write\n");
  }
};

// for cmd
// Global variable for drawing graphics using data & label.
uint32_t label_flag = 0;
uint32_t x_0, y_0, x_1, y_1, x_2, y_2 = 0;  // (x,y) axis
uint32_t x_c, y_c = 0;                      // position of cursor for text
String str = "";                            // String for text output
uint32_t size = 1;                          // text size
uint32_t tc = 0;                            // text color
uint32_t w, h = 0;                          // width & height
uint32_t r = 0;                             // radius for circle
uint32_t c = 0;                             // color

void getLabelDataValue(char *var_name, String label_str, uint32_t *var, int data_val) {
  if (!label_str.compareTo(var_name)) {
    *var = data_val;
  }
}

// Stackchan Draw command
#if !defined(ARDUINO_WIO_TERMINAL)
int norm_x(int x) {
  return (int(x / 320.0 * M5.Lcd.width()));
}

int norm_y(int y) {
  return (int(y / 240.0 * M5.Lcd.height()));
}
#endif

void clear_eyes() {
#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillRect(0, 0, 320, 120, TFT_BLACK);
#else
  M5.Lcd.fillRect(norm_x(0), norm_y(0), norm_x(320), norm_y(120), TFT_BLACK);
#endif
}

void clear_mouth() {
#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillRect(0, 120, 320, 120, TFT_BLACK);
#else
  M5.Lcd.fillRect(norm_x(0), norm_y(120), norm_x(320), norm_y(120), TFT_BLACK);
#endif
}

void draw_eye() {
  clear_eyes();
#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillCircle(90, 93, 8, TFT_WHITE);
  tft.fillCircle(230, 96, 8, TFT_WHITE);
#else
  M5.Lcd.fillCircle(norm_x(90), norm_y(93), norm_y(8), TFT_WHITE);
  M5.Lcd.fillCircle(norm_x(230), norm_y(96), norm_y(8), TFT_WHITE);
#endif
}

void draw_closeeye() {
  clear_eyes();
#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillRect(82, 93, 16, 4, TFT_WHITE);
  tft.fillRect(222, 93, 16, 4, TFT_WHITE);
#else
  M5.Lcd.fillRect(norm_x(82), norm_y(93), norm_x(16), norm_y(4), TFT_WHITE);
  M5.Lcd.fillRect(norm_x(222), norm_y(93), norm_x(16), norm_y(4), TFT_WHITE);
#endif
}

void draw_mouth() {
  clear_mouth();
#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillRect(163 - 45, 148, 90, 4, TFT_WHITE);
#else
  M5.Lcd.fillRect(norm_x(163 - 45), norm_y(148), norm_x(90), norm_y(4), TFT_WHITE);
#endif
}

void draw_openmouth() {
  clear_mouth();
#if defined(ARDUINO_WIO_TERMINAL)
  tft.fillRect(140, 130, 40, 40, TFT_WHITE);
#else
  M5.Lcd.fillRect(norm_x(140), norm_y(130), norm_x(40), norm_y(40), TFT_WHITE);
#endif
}

// Microbit More Command handling
class CmdCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    log_i("CMD read\n");
    pCharacteristic->setValue(cmd, 20);
  }

  void cmd_pin(const char *cmd_str) {
    char pin_cmd = (cmd_str[0] & 0x0f);
    switch (pin_cmd) {
      case 0x01:
        // OUTPUT
        log_i("CMD_PIN OUTPUT\n");
        log_i(" pin:%d, dat:%d\n", cmd_str[1], cmd_str[2]);
        if (cmd_str[1] == 0) {
          pinMode(pin0, OUTPUT);
          digitalWrite(pin0, cmd_str[2]);
        } else if (cmd_str[1] == 1) {
          pinMode(pin1, OUTPUT);
          digitalWrite(pin1, cmd_str[2]);
        }
        break;
      case 0x02:
        // PWM
        log_i("CMD_PIN PWM\n");
        log_i(" pin:%d, duty:%d %%\n", cmd_str[1], cmd_str[2]);
        break;
      case 0x03:
        // SERVO
        log_i("CMD_PIN SERVO\n");
        log_i(" pin:%d, degree:%d\n", cmd_str[1], cmd_str[2]);
        break;
      case 0x04:
        // PULL
        log_i("CMD_PIN PULL\n");
        break;
      case 0x05:
        // EVENT
        log_i("CMD_PIN EVENT\n");
        break;
    }
  }

  void display_text(const char *text) {
    log_i(">> text\n");
    log_i("%s\n", text);
#if defined(ARDUINO_WIO_TERMINAL)
    tft.fillRect(0, 0, 320, TEXT_SPACE - 1, TFT_BLACK);
    if (stackchan_mode) {
      // Draw fukidashi
      tft.fillEllipse(0, 0, 320, TEXT_SPACE, TFT_WHITE);
      tft.fillTriangle(320 / 2 - 320 * 0.1, TEXT_SPACE * 0.8,
                       320 / 2, TEXT_SPACE * 1.5,
                       320 / 2 + 320 * 0.1, TEXT_SPACE * 0.5, TFT_WHITE);
      tft.setTextColor(TFT_BLACK);
    } else {
      tft.setTextColor(TFT_WHITE);
    }
    tft.drawString(String(&(cmd_str[1])), 0, 0);
#else
    M5.Lcd.fillRect(0, 0, M5.Lcd.width(), TEXT_SPACE - 1, TFT_BLACK);
    if (stackchan_mode) {
      // Draw fukidashi
      M5.Lcd.fillEllipse(0, 0, M5.Lcd.width(), TEXT_SPACE, TFT_WHITE);
      M5.Lcd.fillTriangle(M5.Lcd.width() / 2 - M5.Lcd.width() * 0.1, TEXT_SPACE * 0.8,
                          M5.Lcd.width() / 2, TEXT_SPACE * 1.5,
                          M5.Lcd.width() / 2 + M5.Lcd.width() * 0.1, TEXT_SPACE * 0.5, TFT_WHITE);
      M5.Lcd.setTextColor(TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_WHITE);
    }
    M5.Lcd.setCursor(0, 0);
    if (myBoard == m5gfx::board_M5Stack || myBoard == m5gfx::board_M5StackCore2) {
      M5.Lcd.setTextSize(4);
    } else if (myBoard == m5gfx::board_M5StickC) {
      M5.Lcd.setTextSize(2);
    } else if (myBoard == m5gfx::board_M5StickCPlus || myBoard == m5gfx::board_M5StickCPlus2) {
      M5.Lcd.setTextSize(3);
    }
    M5.Lcd.println(text);
#endif
  }

  void cmd_display(const char *cmd_str) {
    char cmd_display = cmd_str[0] & 0b11111;
    switch (cmd_display) {
      case 0x00:
        // CLEAR
        log_i(">> clear\n");
        fillScreen(TFT_BLACK);
        break;
      case 0x01:
        // TEXT
        display_text(&(cmd_str[1]));
        break;
      case 0x02:
        // PIXELS_0
        log_i(">> pixel0\n");
        for (int y = 0; y < 3; y++) {
          for (int x = 0; x < 5; x++) {
            pixel[y][x] = (cmd_str[y * 5 + (x + 1)] & 0xb);
          }
        }
        break;
      case 0x03:
        // PIXELS_1
        log_i(">> pixel1\n");
        for (int y = 3; y < 5; y++) {
          for (int x = 0; x < 5; x++) {
            pixel[y][x] = (cmd_str[(y - 3) * 5 + (x + 1)] & 0xb);
          }
        }
        displayShowPixel();
        break;
    }
  }

  void cmd_audio(const char *cmd_str) {
    char cmd_audio = cmd_str[0] & 0b11111;
    switch (cmd_audio) {
      case 0x00:
        // STOP_TONE  0x00
        log_i(">> Stop tone\n");

#if defined(ARDUINO_WIO_TERMINAL)
        Beep.mute();
#else
        M5.Speaker.stop();
#endif
        break;
      case 0x01:
        // PLAY_TONE  0x01
        const uint8_t max_volume = 255;
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
#if defined(ARDUINO_WIO_TERMINAL)
        Beep.setVolume(volume);
        Beep.tone(freq);
#else
        M5.Speaker.setVolume(volume);
        M5.Speaker.tone(freq);
#endif
        break;
    }
  }

  void display_label_data(char *label, char *data, float data_val) {
    int label_location;
#if !defined(ARDUINO_WIO_TERMINAL)
    if (myBoard == m5gfx::board_M5StickC || myBoard == m5gfx::board_M5StickCPlus || myBoard == m5gfx::board_M5StickCPlus2) {
      if (myBoard == m5gfx::board_M5StickC) {
        label_location = 110;
      } else if (myBoard == m5gfx::board_M5StickCPlus || myBoard == m5gfx::board_M5StickCPlus2) {
        label_location = 170;
      }

      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.fillRect(0, label_location, M5.Lcd.width(), M5.Lcd.height() - label_location, TFT_BLACK);
      M5.Lcd.setCursor(0, label_location);
      M5.Lcd.printf("Label:%s\n", label);
      M5.Lcd.printf("Data:%s\n", data);
      M5.Lcd.printf(" val:");
      if (data_val < 100000) {
        M5.Lcd.printf("%8.2f", data_val);
      } else {
        M5.Lcd.printf("too big");
      }
    } else if (myBoard == m5gfx::board_M5Stack || myBoard == m5gfx::board_M5StackCore2) {
      int label_location_x = 210;
      int label_location_y = 40;
      int font_height = 20;
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.fillRect(label_location_x, label_location_y, M5.Lcd.width() - label_location_x, M5.Lcd.height(), TFT_BLACK);
      M5.Lcd.setCursor(label_location_x, label_location_y);
      M5.Lcd.printf("Label:");
      M5.Lcd.setCursor(label_location_x, label_location_y + font_height * 1);
      M5.Lcd.printf("%s", label);
      M5.Lcd.setCursor(label_location_x, label_location_y + font_height * 2);
      M5.Lcd.printf("Data :");
      M5.Lcd.setCursor(label_location_x, label_location_y + font_height * 3);
      M5.Lcd.printf("%s", data);
      M5.Lcd.setCursor(label_location_x, label_location_y + font_height * 4);
      M5.Lcd.printf(" val:");
      M5.Lcd.setCursor(label_location_x, label_location_y + font_height * 5);
      if (data_val < 100000) {
        M5.Lcd.printf("%8.2f", data_val);
      } else {
        M5.Lcd.printf("too big");
      }
    }
#endif
  }

  void
  set_variables(String label_str, float data_val, String data_str) {
    // Display label & data?
    getLabelDataValue("label", label_str, &label_flag, data_val);
    // Store variables
    getLabelDataValue("x0", label_str, &x_0, data_val);
    getLabelDataValue("y0", label_str, &y_0, data_val);
    getLabelDataValue("x1", label_str, &x_1, data_val);
    getLabelDataValue("y1", label_str, &y_1, data_val);
    getLabelDataValue("x2", label_str, &x_2, data_val);
    getLabelDataValue("y2", label_str, &y_2, data_val);
    getLabelDataValue("xc", label_str, &x_c, data_val);
    getLabelDataValue("yc", label_str, &y_c, data_val);
    if (!label_str.compareTo("str")) {
      str = data_str;
    }
    getLabelDataValue("size", label_str, &size, data_val);
    getLabelDataValue("tc", label_str, &tc, data_val);
    getLabelDataValue("w", label_str, &w, data_val);
    getLabelDataValue("h", label_str, &h, data_val);
    getLabelDataValue("r", label_str, &r, data_val);
    getLabelDataValue("c", label_str, &c, data_val);
  }

  void label_draw_cmd(String label_str, String data_str) {
    if (!label_str.compareTo("cmd")) {
      if (!data_str.compareTo("drawPixel")) {
#if defined(ARDUINO_WIO_TERMINAL)
        tft.drawPixel(x_0, y_0, c);
#else
        M5.Lcd.drawPixel(x_0, y_0, c);
#endif
      } else if (!data_str.compareTo("drawLine")) {
#if defined(ARDUINO_WIO_TERMINAL)
        tft.drawLine(x_0, y_0, x_1, y_1, c);
#else
        M5.Lcd.drawLine(x_0, y_0, x_1, y_1, c);
#endif
      } else if (!data_str.compareTo("drawRect")) {
#if defined(ARDUINO_WIO_TERMINAL)
        tft.drawRect(x_0, y_0, w, h, c);
#else
        M5.Lcd.drawRect(x_0, y_0, w, h, c);
#endif
      } else if (!data_str.compareTo("drawTriangl")) {  // "drawTriangle" is over data length limit.
#if defined(ARDUINO_WIO_TERMINAL)
        tft.drawTriangle(x_0, y_0, x_1, y_1, x_2, y_2, c);
#else
        M5.Lcd.drawTriangle(x_0, y_0, x_1, y_1, x_2, y_2, c);
#endif
      } else if (!data_str.compareTo("drawRoundRe")) {  // "drawRoundRect" is over data length limit.
#if defined(ARDUINO_WIO_TERMINAL)
        tft.drawRoundRect(x_0, y_0, w, h, r, c);
#else
        M5.Lcd.drawRoundRect(x_0, y_0, w, h, r, c);
#endif
      } else if (!data_str.compareTo("fillScreen")) {
#if defined(ARDUINO_WIO_TERMINAL)
        tft.fillScreen(c);
#else
        M5.Lcd.fillScreen(c);
#endif
      } else if (!data_str.compareTo("fillRect")) {
#if defined(ARDUINO_WIO_TERMINAL)
        tft.fillRect(x_0, y_0, w, h, c);
#else
        M5.Lcd.fillRect(x_0, y_0, w, h, c);
#endif
      } else if (!data_str.compareTo("fillCircle")) {
#if defined(ARDUINO_WIO_TERMINAL)
        tft.fillCircle(x_0, y_0, r, c);
#else
        M5.Lcd.fillCircle(x_0, y_0, r, c);
#endif
      } else if (!data_str.compareTo("fillTriangl")) {  // "fillTriangle" is over data length limit.
#if defined(ARDUINO_WIO_TERMINAL)
        tft.fillTriangle(x_0, y_0, x_1, y_1, x_2, y_2, c);
#else
        M5.Lcd.fillTriangle(x_0, y_0, x_1, y_1, x_2, y_2, c);
#endif
      } else if (!data_str.compareTo("fillRoundRe")) {  // "fillRoundRect" is over data length limit.
#if defined(ARDUINO_WIO_TERMINAL)
        tft.fillRoundRect(x_0, y_0, w, h, r, c);
#else
        M5.Lcd.fillRoundRect(x_0, y_0, w, h, r, c);
#endif
      } else if (!data_str.compareTo("print")) {
#if defined(ARDUINO_WIO_TERMINAL)
        tft.setTextColor(tc);
        tft.setTextSize(size);
        tft.drawString(str, x_c, y_c);
#else
        M5.Lcd.setCursor(x_c, y_c);
        M5.Lcd.setTextColor(tc);
        M5.Lcd.setTextSize(size);
        M5.Lcd.print(str);
#endif
      }
    }
  }

  void label_stackchan_cmd(String label_str, String data_str) {
    if (!label_str.compareTo("stack")) {

      if (!data_str.compareTo("eye")) {
        draw_eye();
      } else if (!data_str.compareTo("closeeye")) {
        draw_closeeye();
      } else if (!data_str.compareTo("mouth")) {
        draw_mouth();
      } else if (!data_str.compareTo("openmouth")) {
        draw_openmouth();
      } else if (!data_str.compareTo("say")) {
        draw_openmouth();
        delay(10);
        draw_mouth();
        delay(10);
      } else if (!data_str.compareTo("on")) {
        stackchan_mode = true;
      } else if (!data_str.compareTo("off")) {
        stackchan_mode = false;
      }
    }
  }

  void cmd_data(const char *cmd_str) {
    log_i("CMD DATA\n");

    // Show input data.
    log_i(">>> Data input:");
    for (int i = 0; i <= 20; i++) {
      log_i("(%d)%02x%c:", i, cmd_str[i], cmd_str[i]);
    }
    log_i("\n");

    // Convert from input data to label & data.
    char label[9] = { 0 };
    strncpy(label, &cmd_str[1], sizeof(label) - 1);
    String label_str = String(label);

    char data[12] = { 0 };
    strncpy(data, &cmd_str[9], sizeof(data) - 1);
    String data_str = String(data);

    // Convert from 8bit uint8_t x 4 to 32bit float with little endian.
    static union {
      uint32_t i;
      uint8_t b[sizeof(float)];
      float f;
    } conv_data;
    conv_data.b[0] = cmd_str[9];
    conv_data.b[1] = cmd_str[10];
    conv_data.b[2] = cmd_str[11];
    conv_data.b[3] = cmd_str[12];
    float data_val = conv_data.f;

    log_i("Label str:%s, Data str:%s, Data value:%f.\n", label_str, data_str, data_val);

    // Can't get correct command for number=0x13 and text=0x14. Why?
    char cmd_data = cmd_str[20];
    if (cmd_data == 0x13) {
      log_i("Data is Number.\n");
    } else if (cmd_data == 0x14) {
      log_i("Data is Text.\n");
    } else {
      log_i("Data is Unknown:%02x.\n", cmd_data);
    }

    // Show label & data at display.
    if (label_flag != 0) {
      display_label_data(label, data, data_val);
    }

#if !defined(ARDUINO_WIO_TERMINAL)
    // On and off LED at M5StickC family.
    if (myBoard == m5gfx::board_M5StickC || myBoard == m5gfx::board_M5StickCPlus || myBoard == m5gfx::board_M5StickCPlus2) {
      // Change the LED brightness level to an integer value labeled "led".
      if (strcmp(label, "led") == 0) {
        M5.Power.setLed(constrain(data_val, 0, 255));
      }
    }
#endif

    // Set variables for drawing object.
    set_variables(label_str, data_val, data_str);

    // Do command: for drawing and stackchan
    label_draw_cmd(label_str, data_str);
    label_stackchan_cmd(label_str, data_str);
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
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

    std::string value = pCharacteristic->getValue();
    log_i("CMD len:%d\n", value.length());
    log_i("%s\n", value.c_str());
    const char *cmd_str = value.c_str();
    log_i("%s\n", cmd_str);
    char cmd = (cmd_str[0] >> 5);
    switch (cmd) {
      case 0x01:
        // CMD_PIN
        log_i("CMD pin\n");
        cmd_pin(cmd_str);
        break;
      case 0x02:
        //// CMD_DISPLAY
        log_i("CMD display\n");
        cmd_display(cmd_str);
        break;
      case 0x03:
        //// CMD_AUDIO
        log_i("CMD audio\n");
        cmd_audio(cmd_str);
        break;
      case 0x04:
        //// CMD_DATA (only v2)
        log_i("CMD data\n");
        cmd_data(cmd_str);
        break;
    }
  }
};

// for state
class StateCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    float temp = 0;

#if defined(ARDUINO_WIO_TERMINAL)
    temp = lis.getTemperature();
    int light = (int)map(analogRead(WIO_LIGHT), 0, 511, 0, 255);
    log_i(">> Light Level " + String(light));
    state[4] = (light & 0xff);  // lightlevel
    int mic = (int)map(analogRead(WIO_MIC), 0, 511, 0, 255);
    state[6] = (mic & 0xff);  // soundlevel
    log_i(">> sound Level " + String(mic));
#else
    // GPIO input from PIN0 & PIN1.
    int r0 = analogRead(pin0);
    int r1 = analogRead(pin1);
    state[0] = 0;
    if (r0 >= 2048) {
      state[0] |= 0b01;
    }
    if (r1 >= 2048) {
      state[0] |= 0b10;
    }

    if (myBoard == m5gfx::board_M5StickC || myBoard == m5gfx::board_M5StickCPlus || myBoard == m5gfx::board_M5StickCPlus2) {
      state[6] = ((int)map(soundLevel, 0, 1024, 0, 255) & 0xff);
    } else {
      state[6] = (random(256) & 0xff);  // Random sensor value for soundlevel
    }

    M5.Imu.getTemp(&temp);            // get temperature from IMU
    state[4] = (random(256) & 0xff);  // Random sensor value for lightlevel
#endif
    state[5] = ((int)(temp + 128) & 0xff);  // temperature(+128)
    log_i("STATE read %s", (char *)state);
    pCharacteristic->setValue(state, 7);
  }
};

// for accelerometer related values
#define ACC_MULT 512
#if !defined(RAD_TO_DEG)
#define RAD_TO_DEG 57.324
#endif
float ax, ay, az;
int16_t iax, iay, iaz;
float gx, gy, gz;
float pitch, roll, yaw;

void updateIMU() {
#if defined(ARDUINO_WIO_TERMINAL)
  lis.getAcceleration(&ay, &ax, &az);
  pitch = atan(-ax / sqrtf(ay * ay + az * az)) * RAD_TO_DEG;
  roll = atan(ay / az) * RAD_TO_DEG;
#else
  M5.Imu.getAccel(&ax, &ay, &az);     // get accel
  M5.Imu.getGyro(&gx, &gy, &gz);      // get gyro
#endif
  iax = (int16_t)(ax * ACC_MULT);
  iay = (int16_t)(ay * ACC_MULT);
  iaz = (int16_t)(az * ACC_MULT);
}

class MotionCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    updateIMU();

    motion[0] = ((int)(pitch * ACC_MULT) & 0xff);
    motion[1] = (((int)(pitch * ACC_MULT) >> 8) & 0xff);
    motion[2] = ((int)(roll * ACC_MULT) & 0xff);
    motion[3] = (((int)(roll * ACC_MULT) >> 8) & 0xff);
    motion[4] = (iax & 0xff);
    motion[5] = ((iax >> 8) & 0xff);
    motion[6] = (iay & 0xff);
    motion[7] = ((iay >> 8) & 0xff);
    motion[8] = (-iaz & 0xff);
    motion[9] = ((-iaz >> 8) & 0xff);
    pCharacteristic->setValue(motion, 20);

    // debug print
    char msg[256] = { 0 };
    for (int i = 0; i < sizeof(motion); i++) {
      sprintf(&msg[i * 3], "%02x,", motion[i], sizeof(motion) * 3 - 3 * i);
    }
    log_i("MOTION read: %s\n", msg);
  }
};

// for button
class ActionCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    log_i("BTN read\n");
    pCharacteristic->setValue("Read me!!");  // dummy data
  }
};

// for Analog pin
class AnalogPinCallback0 : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
#if !defined(ARDUINO_WIO_TERMINAL)
    int r = map(analogRead(pin0), 0, 4095, 0, 1023);
#else
    int r = analogRead(0);
#endif
    log_i("Analog Pin0 Read:%d\n", r);

    analog[0] = (r & 0xff);
    analog[1] = ((r >> 8) & 0xff);

    pCharacteristic->setValue(analog, 2);
  }
};

class AnalogPinCallback1 : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
#if !defined(ARDUINO_WIO_TERMINAL)
    int r = map(analogRead(pin1), 0, 4095, 0, 1023);
#else
    int r = analogRead(0);
#endif
    log_i("Analog Pin1 Read:%d\n", r);

    analog[0] = (r & 0xff);
    analog[1] = ((r >> 8) & 0xff);

    pCharacteristic->setValue(analog, 2);
  }
};

void setup_M5Stack() {
#if !defined(ARDUINO_WIO_TERMINAL)  // M5Stack
  // Init M5Stack.
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.init();

  // Init speaker.
  auto spk_cfg = M5.Speaker.config();
  M5.Speaker.config(spk_cfg);
  M5.Speaker.begin();
  myBoard = M5.getBoard();

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
  // Init FastLED(NeoPixel).
  if (myBoard == m5gfx::board_M5Atom) {
    FastLED.addLeds<WS2811, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(20);
  }
#endif

  // for Mic input
  if (myBoard == m5gfx::board_M5StickC || myBoard == m5gfx::board_M5StickCPlus || myBoard == m5gfx::board_M5StickCPlus2) {
    i2sInit();
    xTaskCreate(mic_record_task, "mic_record_task", 2048, NULL, 1, NULL);
  }
#endif
}

void setup_pins() {
#if !defined(ARDUINO_WIO_TERMINAL)  // M5Stack
  //// GPIO
  // for PortB (ADC, GPIO input)
  // Default is for M5StickC/Plus, CoreInk
  pin0 = GPIO_NUM_33;
  pin1 = GPIO_NUM_32;

  switch (myBoard) {
    case m5gfx::board_M5Atom:
    case m5gfx::board_M5AtomU:
    case m5gfx::board_M5AtomPsram:
      pin0 = GPIO_NUM_32;
      pin1 = GPIO_NUM_26;
      break;

    case m5gfx::board_M5Stack:
    case m5gfx::board_M5StackCore2:
    case m5gfx::board_M5Tough:
      pin0 = GPIO_NUM_36;
      pin1 = GPIO_NUM_26;
      break;

    case m5gfx::board_M5Paper:
      pin0 = GPIO_NUM_33;
      pin1 = GPIO_NUM_26;
      break;

    case m5gfx::board_M5StackCoreS3:  // for portB
      pin0 = GPIO_NUM_8;
      pin1 = GPIO_NUM_9;
      break;

    case m5gfx::board_M5AtomS3:
    case m5gfx::board_M5Cardputer:
    case m5gfx::board_M5Dial:  // for portB
      pin0 = GPIO_NUM_1;
      pin1 = GPIO_NUM_2;
      break;

    default:
      break;
  }
#endif
}

void setup_BLE() {
  // Create MAC address base fixed ID
  uint8_t mac0[6] = { 0 };
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
  char adv_str[32] = { 0 };
  String("BBC micro:bit [" + ID + "]").toCharArray(adv_str, sizeof(adv_str));

  // Start up screen
  fillScreen(TFT_BLUE);
#if defined(ARDUINO_WIO_TERMINAL)
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.print("Welcome to\nM5bit Less!!\nPlease connect to\n");
  tft.println(adv_str);
#else
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("Welcome to\nM5bit Less!!\nPlease connect to\n");
  M5.Lcd.println(adv_str);
#endif

  log_i("BLE start.\n");
  log_i("%s\n", adv_str);
  BLEDevice::init(adv_str);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(BLEUUID(MBIT_MORE_SERVICE), 27);

  // CMD
  pCharacteristic[0] = pService->createCharacteristic(
    MBIT_MORE_CH_COMMAND,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  pCharacteristic[0]->setCallbacks(new CmdCallbacks());
  pCharacteristic[0]->addDescriptor(new BLE2902());

  // STATE
  pCharacteristic[1] = pService->createCharacteristic(
    MBIT_MORE_CH_STATE,
    BLECharacteristic::PROPERTY_READ);
  pCharacteristic[1]->setCallbacks(new StateCallbacks());
  pCharacteristic[1]->addDescriptor(new BLE2902());

  // MOTION
  pCharacteristic[2] = pService->createCharacteristic(
    MBIT_MORE_CH_MOTION,
    BLECharacteristic::PROPERTY_READ);
  pCharacteristic[2]->setCallbacks(new MotionCallbacks());
  pCharacteristic[2]->addDescriptor(new BLE2902());

  pCharacteristic[3] = pService->createCharacteristic(
    MBIT_MORE_CH_PIN_EVENT,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic[3]->setCallbacks(new DummyCallbacks());
  pCharacteristic[3]->addDescriptor(new BLE2902());

  // ACTION
  pCharacteristic[4] = pService->createCharacteristic(
    MBIT_MORE_CH_ACTION_EVENT,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic[4]->setCallbacks(new ActionCallbacks());
  pCharacteristic[4]->addDescriptor(new BLE2902());

  // PINS
  pCharacteristic[5] = pService->createCharacteristic(
    MBIT_MORE_CH_ANALOG_IN_P0,
    BLECharacteristic::PROPERTY_READ);
  pCharacteristic[5]->setCallbacks(new AnalogPinCallback0());
  pCharacteristic[5]->addDescriptor(new BLE2902());

  pCharacteristic[6] = pService->createCharacteristic(
    MBIT_MORE_CH_ANALOG_IN_P1,
    BLECharacteristic::PROPERTY_READ);
  pCharacteristic[6]->setCallbacks(new AnalogPinCallback1());
  pCharacteristic[6]->addDescriptor(new BLE2902());

  pCharacteristic[7] = pService->createCharacteristic(
    MBIT_MORE_CH_ANALOG_IN_P2,
    BLECharacteristic::PROPERTY_READ);
  pCharacteristic[7]->setCallbacks(new DummyCallbacks());
  pCharacteristic[7]->addDescriptor(new BLE2902());


  // MESSAGE (only for v2)
  pCharacteristic[8] = pService->createCharacteristic(
    MBIT_MORE_CH_MESSAGE,
    BLECharacteristic::PROPERTY_READ);
  pCharacteristic[8]->setCallbacks(new DummyCallbacks());
  pCharacteristic[8]->addDescriptor(new BLE2902());


  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void setup_WioTerminal() {
#if defined(ARDUINO_WIO_TERMINAL)
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
}

void setup() {
  Serial.begin(115200);

#if defined(ARDUINO_WIO_TERMINAL)
  setup_WioTerminal();
#else  // M5Stack
  setup_M5Stack();
  setup_pins();
#endif
  setup_BLE();
}

void sendBtn(uint8_t btnID, uint8_t btn, uint8_t btn_status, uint8_t prev) {
  memset((char *)(action), 0, 20);  // clear action buffer

  action[0] = 0x01;   // for Button event
  action[19] = 0x12;  // ACTION_EVENT

  action[1] = btnID;  // btnID 0x01:BtnA, 0x02:BtnB, 121:BtnC(LOGO)

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
#if !defined(ARDUINO_WIO_TERMINAL)
    M5.update();
#endif
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
    btnA = M5.BtnA.wasPressed();
    btn_statusA = M5.BtnA.isPressed();
    btnB = M5.BtnB.wasPressed();
    btn_statusB = M5.BtnB.isPressed();
    btnC = M5.BtnC.wasPressed();
    btn_statusC = M5.BtnC.isPressed();
#endif

#define BUTTON_DELAY 50

    //// Button A
    action[1] = 0x01;
    sendBtn(0x01, btnA, btn_statusA, prevA);
    prevA = btn_statusA;
    delay(BUTTON_DELAY);

    //// Button B
    action[1] = 0x02;
    sendBtn(0x02, btnB, btn_statusB, prevB);
    prevB = btn_statusB;
    delay(BUTTON_DELAY);

    //// Button C (LOGO)
    action[1] = 121;  // LOGO 121
    sendBtn(121, btnC, btn_statusC, prevC);
    prevC = btn_statusC;
    delay(BUTTON_DELAY);

    updateGesture();

    // Send dummy data label='a' data=random('a'-'z') every 50ms
    uint32_t label_time = (uint32_t)millis();
    if (label_time - old_label_time > 50) {
      memset((char *)(action), 0, 20);  // clear action buffer
      action[19] = 0x14;                // DATA_TEXT
      action[0] = 0x61;                 // 'a'
      action[1] = 0;
      action[8] = 0x61 + random(26);  // 'a-z'
      action[9] = 0;
      pCharacteristic[4]->setValue(action, 20);
      pCharacteristic[4]->notify();
#if !defined(ARDUINO_WIO_TERMINAL)
      if (myBoard == m5gfx::board_M5Stack) {
        // keyboard input for M5Stack Faces
        if (digitalRead(5) == LOW) {
          Wire.requestFrom(0x08, 1);  // 0x08 means FACES_KEYBOARD_I2C_ADDR.
          while (Wire.available()) {
            char c = Wire.read();             // receive a byte as character
            Serial.printf("Key:%c\n", c);     // print the character
            memset((char *)(action), 0, 20);  // clear action buffer
            action[19] = 0x14;                // DATA_TEXT
            action[0] = 0x4b;                 // 'K'
            action[1] = 0x65;                 // 'e'
            action[2] = 0x79;                 // 'y'
            action[3] = 0x00;
            action[8] = c;  // Key character
            action[9] = 0;
            delay(50);  // Wait 50ms
            pCharacteristic[4]->setValue(action, 20);
            pCharacteristic[4]->notify();
          }
        }
      }
#endif

      old_label_time = label_time;
    }
  }

#if defined(ARDUINO_WIO_TERMINAL)
  Beep.update();
#endif
};
