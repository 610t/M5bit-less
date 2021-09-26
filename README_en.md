# M5bit-less
English/[日本語](README.md)

![M5bit Less](https://i.gyazo.com/37711fdbdec359e2834c6fbac1eb5bff.png)

[M5bit Less](https://scrapbox.io/M5S/M5bit_Less) is a program which use [Microbit More](https://microbit-more.github.io/) from M5Stack. 
It is written by the Arduino IDE.

## Supported devices
I check devices work below, but not all of them support same Microbit More functions.
Please check a supported funciton table.
- M5Stack Basic
- M5Stack Gray
- M5Stack Core2
- M5StickC
- M5StickC Plus
- ATOM Matrix
- Wio Terminal

## Supported functions at Microbit More
- Gesture
- Connected and disconnected event block
- Acceleration input (need IMU)
- Display pattern
- Display text
- Play tone
- Sound level
- Button A, B event
- LOGO(Button C) event
- Light intensity
- Temperature

|device|gesture|acceleration|display pattern|display text|play tone|sound level|button input|light intensity|temperature|Note|
|---|---|---|---|---|---|---|---|---|---|---|
|M5Stack Basic|-(IMU)|△IMU|o|o|o|x|A,B,C(LOGO)|x|△IMU||
|M5Stack Gray |-|o|o|o|o|x|A,B,C(LOGO)|x|o||
|M5Stack Core2|o|o|o|o|o|x|A,B,C(LOGO)|x|o||
|M5StickC|o|o|o|o|x|o(With mic model)|A,B|x|o||
|M5StickC Plus|o|o|o|o|o|o|A,B|x|o||
|ATOM Matrix|-|o|o|△|x|x|A|x|o|Too weak BLE|
|Wio Terminal|o|o|o|o|o|o|A,B,C(LOGO)|o|o| |
- △:Implementable
- -:Not check yet

# How to use
## Prepare for the M5Stack
Compile & download [src/M5bit-less/M5bit-less.ino](src/M5bit-less/M5bit-less.ino).

### Notes on M5Stack Core2
In M5Stack Core2, even if the compilation is successfully completed and downloaded, the initial screen may not be displayed and the program may not work.
In this case, please try disabling PSRAM in Tools->PSRAM.

## Prepare for the Scratch
Microbit More can be used as an extension in [stretch3](https://stretch3.github.io/).

![Extensions](https://i.gyazo.com/208ad9cd788d453555267d8901b4050b.png)
Click on the bottom left button to add the extension.

![Select Microbit More](https://i.gyazo.com/4780d7b0da3a260f7e709db4b16334c3.png)

Add the Microbit More extension.

![Deveice selection](https://i.gyazo.com/be6c3374e86301eb7874fa0d1ba9575d.png)

Select a device to connect.

You can use like a Microbit More for micro:bit.

# TODO
## Implementable but not implement yet
The following items can be implemented, but have not yet been implemented.
- Message exchange using labels and data.
- Pin 0-2 I/O(GPIO, PWM, ADC, Servo, ...)

## Features missing at the (standard) M5Stack
The following items have not been implemented because there are no sensors compatible with the M5Stack.
- Light sensor (Implemented at Wio Terminal)
- Functions related to magnetic force (include heading, ...): (Implementable at M5Stack Gray)
- Mic input (Inplemantable at M5StickC/Plus and Core2)

## Movies/Other materials
### M5bit Less: M5Stack x Scratch3 = So Fun!!; for [M5Stack Japan Creativity Contest 2021](https://protopedia.net/event/22)
- [Description in Japanese](https://protopedia.net/prototype/2395)

[![M5bit Less: M5Stack x Scratch3 = So Fun!!動画](https://img.youtube.com/vi/-Nfu87CjvBU/0.jpg)](https://youtu.be/-Nfu87CjvBU)

### [Maker Faire Kyoto 2021](https://makezine.jp/event/mfk2021/)
#### M5bit Less: M5StackでScratchを使おう!!;「教育」カテゴリ応募作品
[![M5bit Less: M5StackでScratchを使おう!!](https://img.youtube.com/vi/sNwNkEHScCE/0.jpg)](https://www.youtube.com/watch?v=sNwNkEHScCE)

#### 京都観光案内 with M5bit Less - あかんターレで決めターレ; 「京都」カテゴリ応募作品
[![京都観光案内 with M5bit Less - あかんターレで決めターレ](https://img.youtube.com/vi/7ue7GZlBH6Y/0.jpg)](https://www.youtube.com/watch?v=7ue7GZlBH6Y)
