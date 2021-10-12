#include "gesture.h"

extern uint8_t action[];
extern int16_t iax, iay, iaz;
extern BLECharacteristic* pCharacteristic[];

void sendGesture(uint8_t gesture)
{
  log_i("Gesture:%d\n", gesture);

  memset((char *)(action), 0, 20); // clear action buffer

  //// Gesture
  action[0] = 0x02;
  action[19] = 0x12; // ACTION_EVENT

  action[1] = gesture & 0xff;

  // Set TimeStamp (Little Endian)
  uint32_t time = (uint32_t)millis();
  action[2] = (time & 0xff);
  action[3] = (time >> 8) & 0xff;
  action[4] = (time >> 16) & 0xff;
  action[5] = (time >> 24) & 0xff;

  pCharacteristic[4]->setValue(action, 20);
  pCharacteristic[4]->notify();
}

uint32_t instantaneousAccelerationSquared()
{
  // Use pythagoras theorem to determine the combined force acting on the device.
  return (uint32_t)iax * (uint32_t)iax + (uint32_t)iay * (uint32_t)iay + (uint32_t)iaz * (uint32_t)iaz;
}

int instantaneousPosture()
{
  bool shakeDetected = false;

  // Test for shake events.
  // We detect a shake by measuring zero crossings in each axis. In other words, if we see a strong acceleration to the left followed by
  // a strong acceleration to the right, then we can infer a shake_ Similarly, we can do this for each axis (left / right, up / down, in / out).
  //
  // If we see enough zero crossings in succession (MICROBIT_ACCELEROMETER_SHAKE_COUNT_THRESHOLD), then we decide that the device
  // has been shaken.
  if ((iax < -MICROBIT_ACCELEROMETER_SHAKE_TOLERANCE && shake_x) || (iax > MICROBIT_ACCELEROMETER_SHAKE_TOLERANCE && !shake_x)) {
    shakeDetected = true;
    shake_x = !shake_x;
  }

  if ((iay < -MICROBIT_ACCELEROMETER_SHAKE_TOLERANCE && shake_y) || (iay > MICROBIT_ACCELEROMETER_SHAKE_TOLERANCE && !shake_y)) {
    shakeDetected = true;
    shake_y = !shake_y;
  }

  if ((iaz < -MICROBIT_ACCELEROMETER_SHAKE_TOLERANCE && shake_z) || (iaz > MICROBIT_ACCELEROMETER_SHAKE_TOLERANCE && !shake_z)) {
    shakeDetected = true;
    shake_z = !shake_z;
  }

  // If we detected a zero crossing in this sample period, count this.
  if (shakeDetected && shake_count < MICROBIT_ACCELEROMETER_SHAKE_COUNT_THRESHOLD) {
    shake_count++;

    if (shake_count == 1)
      shake_timer = 0;

    if (shake_count == MICROBIT_ACCELEROMETER_SHAKE_COUNT_THRESHOLD) {
      shake_shaken = 1;
      shake_timer = 0;
      return MICROBIT_ACCELEROMETER_EVT_SHAKE;
    }
  }

  // measure how long we have been detecting a SHAKE event.
  if (shake_count > 0) {
    shake_timer++;

    // If we've issued a SHAKE event already, and sufficient time has assed, allow another SHAKE event to be issued.
    if (shake_shaken && shake_timer >= MICROBIT_ACCELEROMETER_SHAKE_RTX) {
      shake_shaken = 0;
      shake_timer = 0;
      shake_count = 0;
    }

    // Decay our count of zero crossings over time. We don't want them to accumulate if the user performs slow moving motions.
    else if (!shake_shaken && shake_timer >= MICROBIT_ACCELEROMETER_SHAKE_DAMPING)
    {
      shake_timer = 0;
      if (shake_count > 0)
        shake_count--;
    }
  }

  uint32_t force = instantaneousAccelerationSquared();
  if (force < MICROBIT_ACCELEROMETER_FREEFALL_THRESHOLD)
    return MICROBIT_ACCELEROMETER_EVT_FREEFALL;

  // Determine our posture.
  if (iax < (-500 + MICROBIT_ACCELEROMETER_TILT_TOLERANCE))
    return MICROBIT_ACCELEROMETER_EVT_TILT_RIGHT;

  if (iax > (500 - MICROBIT_ACCELEROMETER_TILT_TOLERANCE))
    return MICROBIT_ACCELEROMETER_EVT_TILT_LEFT;

  if (iay < (-500 + MICROBIT_ACCELEROMETER_TILT_TOLERANCE))
#if defined(ARDUINO_M5Stack_ATOM)
    return MICROBIT_ACCELEROMETER_EVT_TILT_UP;
#else
    return MICROBIT_ACCELEROMETER_EVT_TILT_DOWN;
#endif

  if (iay > (500 - MICROBIT_ACCELEROMETER_TILT_TOLERANCE))
#if defined(ARDUINO_M5Stack_ATOM)
    return MICROBIT_ACCELEROMETER_EVT_TILT_DOWN;
#else
    return MICROBIT_ACCELEROMETER_EVT_TILT_UP;
#endif

  if (iaz < (-500 + MICROBIT_ACCELEROMETER_TILT_TOLERANCE))
#if defined(ARDUINO_WIO_TERMINAL) || defined(ARDUINO_M5Stack_ATOM)
    return MICROBIT_ACCELEROMETER_EVT_FACE_UP;
#else
    return MICROBIT_ACCELEROMETER_EVT_FACE_DOWN;
#endif

  if (iaz > (500 - MICROBIT_ACCELEROMETER_TILT_TOLERANCE))
#if defined(ARDUINO_WIO_TERMINAL) || defined(ARDUINO_M5Stack_ATOM)
    return MICROBIT_ACCELEROMETER_EVT_FACE_DOWN;
#else
    return MICROBIT_ACCELEROMETER_EVT_FACE_UP;
#endif

  return MICROBIT_ACCELEROMETER_EVT_NONE;
}

void updateGesture()
{
  // Check for High/Low G force events - typically impulses, impacts etc.
  // Again, during such spikes, these event take priority of the posture of the device.
  // For these events, we don't perform any low pass filtering.
  uint32_t force = instantaneousAccelerationSquared();

  log_i("updateGesture:%d,%d,%d\n", iax, iay, iaz);

  if (force > MICROBIT_ACCELEROMETER_3G_THRESHOLD) {
    if (force > MICROBIT_ACCELEROMETER_3G_THRESHOLD && !shake_impulse_3) {
      sendGesture(MICROBIT_ACCELEROMETER_EVT_3G);
      shake_impulse_3 = 1;
    }
    if (force > MICROBIT_ACCELEROMETER_6G_THRESHOLD && !shake_impulse_6) {
      sendGesture(MICROBIT_ACCELEROMETER_EVT_6G);
      shake_impulse_6 = 1;
    }
    if (force > MICROBIT_ACCELEROMETER_8G_THRESHOLD && !shake_impulse_8) {
      sendGesture(MICROBIT_ACCELEROMETER_EVT_8G);
      shake_impulse_8 = 1;
    }

    impulseSigma = 0;
  }

  // Reset the impulse event onve the acceleration has subsided.
  if (impulseSigma < MICROBIT_ACCELEROMETER_GESTURE_DAMPING) {
    impulseSigma++;
  } else {
    shake_impulse_3 = shake_impulse_6 = shake_impulse_8 = 0;
  }

  // Determine what it looks like we're doing based on the latest sample_..
  uint16_t g = instantaneousPosture();

  if (g == MICROBIT_ACCELEROMETER_EVT_SHAKE) {
    lastGesture = MICROBIT_ACCELEROMETER_EVT_SHAKE;
    sendGesture(MICROBIT_ACCELEROMETER_EVT_SHAKE);
    return;
  }

  // Perform some low pass filtering to reduce jitter from any detected effects
  if (g == currentGesture) {
    if (sigma < MICROBIT_ACCELEROMETER_GESTURE_DAMPING) {
      sigma++;
    }
  } else {
    currentGesture = g;
    sigma = 0;
  }

  // If we've reached threshold, update our record and raise the relevant event...
  if (currentGesture != lastGesture && sigma >= MICROBIT_ACCELEROMETER_GESTURE_DAMPING) {
    lastGesture = currentGesture;
    sendGesture(lastGesture);
  }
}
