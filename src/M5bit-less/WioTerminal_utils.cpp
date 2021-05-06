#if defined(ARDUINO_WIO_TERMINAL)
#include "WioTerminal_utils.h"

SPEAKER::SPEAKER(void) {
  _volume = 5;
  _forever = false;
  _freq;
}

void SPEAKER::tone(uint16_t frequency) {
  _forever = true;
  _freq = frequency;
}

void SPEAKER::tone(uint16_t frequency, uint32_t duration) {
  tone(frequency);
  _count = millis() + duration;
  speaker_on = 1;
  _forever = false;
}

void SPEAKER::setVolume(uint8_t volume) {
  _volume = (int)map(volume, 0, 5, 0, 255);
}

void SPEAKER::mute() {
  //digitalWrite(WIO_BUZZER, LOW);
  analogWrite(WIO_BUZZER, 0);
  _forever = false;
}

void SPEAKER::update() {
  if (_forever) {
    //digitalWrite(WIO_BUZZER, HIGH);
    analogWrite(WIO_BUZZER, _volume);
    delayMicroseconds(_freq);
    //digitalWrite(WIO_BUZZER, LOW);
    analogWrite(WIO_BUZZER, 0);
    delayMicroseconds(_freq);
  } else {
    if (speaker_on) {
      if (millis() > _count) {
        speaker_on = 0;
        mute();
      }
    }
  }
}
#endif
