#ifndef _WIOTERMINAL_UTILS_H_
#define _WIOTERMINAL_UTILS_H_

#include "Arduino.h"

class SPEAKER {
  public:
    SPEAKER(void);

    void mute();
    void setVolume(uint8_t volume);
    void tone(uint16_t frequency);
    void tone(uint16_t frequency, uint32_t duration);
    void update();

  private:
    uint32_t _count;
    uint8_t _volume;
    uint16_t _freq;
    bool _forever;
    bool speaker_on;
};
#endif
