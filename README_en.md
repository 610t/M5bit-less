# M5bit-less
English/[日本語](README.md)

![M5bit Less](https://i.gyazo.com/37711fdbdec359e2834c6fbac1eb5bff.png)

[M5bit Less](https://scrapbox.io/M5S/M5bit_Less) is a program which use [Microbit More](https://microbit-more.github.io/) from M5Stack. 
It is written by the Arduino IDE.

## Supported devices
I check devices work below, but not all of them support same Microbit More functions.
I use [M5Unified](https://github.com/m5stack/M5Unified), so almost all M5Stack family can use.
Please check a supported funciton table.
- M5Stack Basic
- M5Stack Gray
- M5Stack Core2
- M5StickC
- M5StickC Plus
- ATOM Matrix
- Cardputer
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
|ATOM Matrix|o|o|o|△|x|x|A|x|o|Too weak BLE|
|CardPuter|x|x|o|o|o|o|A|x|x||
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

# label & data extension
M5bitLess label & data extension can send and receive special data between Scratch and M5Stack.

## Reserved words from Scratch to M5Stack

For now, commands for screen drawing are implemented.

### Reserved variables
|label|meanings|value|
|----|----|----|
|label|show label & data|0:not show(default)、other:show|
|led|on/off of LED|'on':on, other:off|
|x0,y0,x1,y1,x2,y2|location(x,y)|int|
|w,h|width & height|int|
|r|radius|int|
|c|color(24bit)|int|
|xc,yc|location of string|int|
|str|string to show|string|
|size|size of character|int(1-7)|
|tc|color of string(16bit)|int|
|cmd|draw command (show below)|string|

#### Reserved commands.
The following commands for screen drawing are available.

|value of cmd|meanings|command|
|----|----|----|
|drawPixel|draw point|M5.Lcd.drawPixel(x0, y0, c)|
|drawLine|draw line|M5.Lcd.drawLine(x0, y0, x1, y1, c)|
|drawRect|draw rectangle|M5.Lcd.drawRect(x0, y0, w, h, c)|
|drawTriangl|draw triangle|M5.Lcd.drawTriangle(x0, y0, x1, y1, x2, y2, c)|
|drawRoundRe|draw corner rounded rectangle|M5.Lcd.drawRoundRect(x0, y0, w, h, r, c)|
|fillScreen|fill full screen with a color|M5.Lcd.fillScreen(c)|
|fillRect|draw filled rectangle|M5.Lcd.fillRect(x0, y0, w, h, c)|
|fillCircle|draw filled circle|M5.Lcd.fillCircle(x0, y0, r, c)|
|fillTriangl|draw filled triangle|M5.Lcd.fillTriangle(x0, y0, x1, y1, x2, y2, c)|
|fillRoundRe|draw filled corner rounded rectangle|M5.Lcd.fillRoundRect(x0, y0, w, h, r, c)|
|print|draw string|M5.Lcd.setCursor(xc, yc);M5.Lcd.setTextColor(tc);M5.Lcd.setTextSize(size);M5.Lcd.print(str)|

## Reserved words from M5Stack to Scratch
For now, the following keyboard inputs are available.

### Reserved variables
These value is reserved.
|label|meanings|value|
|----|----|----|
|Key|character code with Faces keyboard|'a', 'b', 'A', ...|
|a|Random string 'a'-'z'|'a', 'b', 'c', ...|

# TODO
## Implementable but not implement yet
The following items can be implemented, but have not yet been implemented.
- Pin 0-2 I/O(GPIO, PWM, ADC, Servo, ...)

## Features missing at the (standard) M5Stack
The following items have not been implemented because there are no sensors compatible with the M5Stack.
- Light sensor (Implemented at Wio Terminal)
- Functions related to magnetic force (include heading, ...): (Implementable at M5Stack Gray)
- Mic input (Inplemantable at M5StickC/Plus and Core2)

## Movies/Other materials
### M5StackとScratchのより良い関係:覚醒編; for  [IoT縛りの勉強会! IoTLT vol.85](https://iotlt.connpass.com/event/239891/)
- [Description in Japanese](https://scrapbox.io/M5S/M5Stack%E3%81%A8Scratch%E3%81%AE%E3%82%88%E3%82%8A%E8%89%AF%E3%81%84%E9%96%A2%E4%BF%82:%E8%A6%9A%E9%86%92%E7%B7%A8)

[![M5StackとScratchのより良い関係:覚醒編動画1](https://img.youtube.com/vi/Vk8FoH25KJg/0.jpg)](https://youtu.be/Vk8FoH25KJg)

[![M5StackとScratchのより良い関係:覚醒編動画2](https://img.youtube.com/vi/FYaJ7QR_N-g/0.jpg)](https://youtu.be/FYaJ7QR_N-g)

### M5bit Less: M5Stack x Scratch3 = So Fun!!; for [M5Stack Japan Creativity Contest 2021](https://protopedia.net/event/22)
#### M5bitLess label & data extension for [M5Stack Japan Creativity Contest 2022](https://protopedia.net/event/m5stack2022)
- [Description in Japanese](https://protopedia.net/prototype/3224)

[![M5bitLess label & data extension movie](https://gyazo.com/878db35177a5c89e2c7c3f08fcb09fc6.png)](https://youtu.be/1stCapqBokw)

#### M5bit Less: M5Stack x Scratch3 = So Fun!!; for [M5Stack Japan Creativity Contest 2021](https://protopedia.net/event/22)
- [Description in Japanese](https://protopedia.net/prototype/2395)

[![M5bit Less: M5Stack x Scratch3 = So Fun!! movie](https://img.youtube.com/vi/-Nfu87CjvBU/0.jpg)](https://youtu.be/-Nfu87CjvBU)

### [Maker Faire Kyoto 2021](https://makezine.jp/event/mfk2021/)
#### M5bit Less: M5StackでScratchを使おう!!;「教育」カテゴリ応募作品
[![M5bit Less: M5StackでScratchを使おう!!](https://img.youtube.com/vi/sNwNkEHScCE/0.jpg)](https://www.youtube.com/watch?v=sNwNkEHScCE)

#### 京都観光案内 with M5bit Less - あかんターレで決めターレ; 「京都」カテゴリ応募作品
[![京都観光案内 with M5bit Less - あかんターレで決めターレ](https://img.youtube.com/vi/7ue7GZlBH6Y/0.jpg)](https://www.youtube.com/watch?v=7ue7GZlBH6Y)



## 動画/発表資料
### M5Stack Japan Creativity Contest
#### M5bitLess label & data extension for [M5Stack Japan Creativity Contest 2022](https://protopedia.net/event/m5stack2022)
- [発表資料](https://protopedia.net/prototype/3224)

[![M5bitLess label & data extension動画](https://gyazo.com/878db35177a5c89e2c7c3f08fcb09fc6.png)](https://youtu.be/1stCapqBokw)

#### M5bit Less: M5Stack x Scratch3 = So Fun!!; for [M5Stack Japan Creativity Contest 2021](https://protopedia.net/event/22)
- [発表資料](https://protopedia.net/prototype/2395)

[![M5bit Less: M5Stack x Scratch3 = So Fun!!動画](https://img.youtube.com/vi/-Nfu87CjvBU/0.jpg)](https://youtu.be/-Nfu87CjvBU)

### [Maker Faire Kyoto 2021](https://makezine.jp/event/mfk2021/)
#### M5bit Less: M5StackでScratchを使おう!!;「教育」カテゴリ応募作品
[![M5bit Less: M5StackでScratchを使おう!!](https://img.youtube.com/vi/sNwNkEHScCE/0.jpg)](https://www.youtube.com/watch?v=sNwNkEHScCE)

#### 京都観光案内 with M5bit Less - あかんターレで決めターレ; 「京都」カテゴリ応募作品
[![京都観光案内 with M5bit Less - あかんターレで決めターレ](https://img.youtube.com/vi/7ue7GZlBH6Y/0.jpg)](https://www.youtube.com/watch?v=7ue7GZlBH6Y)
