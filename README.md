# M5bit-less
日本語/[English](README_en.md)

![M5bit Less](https://i.gyazo.com/37711fdbdec359e2834c6fbac1eb5bff.png)

[M5bit Less](https://scrapbox.io/M5S/M5bit_Less)は、[Microbit More](https://microbit-more.github.io/)をM5Stackで使うためのM5Stack用プログラムです。
Arduino IDEで作成しています。

## サポートしている機種
以下の機種で動くことを確認していますが、全てで同じ機能がサポートされているわけではありません。
詳しくは、サポートしている機能の表を参照してください。
- M5Stack Basic
- M5Stack Gray
- M5Stack Core2
- M5StickC
- M5StickC Plus
- ATOM Matrix
- Wio Terminal

## サポートしている機能
- ジェスチャー
- つながったときイベントブロック
- 加速度の入力(IMUを持つM5Stack Gray, M5StickC、MPU6886ユニット([M5Stack用6軸IMUユニット](https://www.switch-science.com/catalog/6623/)など)が必要です)
- パターンの表示
- 文字列の表示
- 音を鳴らす
- 音の大きさ
- AとBボタンのクリックされたとき、下がったとき、上がったとき
- ロゴ(Cボタン)のタップされたとき、触れられたとき、離されたとき
- 明るさ
- 温度

|機種/device|ジェスチャー/Gesture|加速度入力/acceleration|パターン表示/display pattern|文字列表示/display text|音を鳴らす/play tone|音の大きさ/sound level|ボタン/button input|明るさ/light intensity|温度/temperature|備考/Note|
|---|---|---|---|---|---|---|---|---|---|---|
|M5Stack Basic|-(IMU)|△別途IMU必要|o|o|o|x|A,B,C(LOGO)|x|△別途IMU必要||
|M5Stack Gray |-|o|o|o|o|x|A,B,C(LOGO)|x|o||
|M5Stack Core2|o|o|o|o|o|x|A,B,C(LOGO)|x|o||
|M5StickC|o|o|o|o|x|o(マイク付きモデル)|A,B|x|o||
|M5StickC Plus|o|o|o|o|o|o|A,B|x|o||
|ATOM Matrix|o|o|o|△|x|x|A|x|o|電波が弱い|
|Wio Terminal|o|o|o|o|o|o|A,B,C(LOGO)|o|o| |
- △:対応可能だがまだ実装できていないもの。
- -:まだ動作確認できてないもの

# どうやって使うの?
## M5Stack側の準備
[src/M5bit-less/M5bit-less.ino](src/M5bit-less/M5bit-less.ino)をArduino IDEからコンパイルして、M5Stackにダウンロードします。

### M5Stack Core2に関する注意事項
M5Stack Core2では、正常にコンパイルが終了してダウンロードできても、初期画面が表示されず動作しない時があります。
その場合、ツール->PSRAMを無効(Disable)にしてみて下さい。

## Scratch側の準備
Microbit Moreは、[stretch3](https://stretch3.github.io/)などで拡張機能として使うことができます。

![拡張機能](https://i.gyazo.com/208ad9cd788d453555267d8901b4050b.png)
一番左下のボタンから拡張機能の追加を行います。

![Microbit Moreの選択](https://i.gyazo.com/4780d7b0da3a260f7e709db4b16334c3.png)

Microbit Moreの拡張機能を追加します。

![デバイスの選択](https://i.gyazo.com/be6c3374e86301eb7874fa0d1ba9575d.png)

接続するデバイスを選択します。

あとは、通常のMicrobit Moreのように利用することができますが、まだ実装していない機能があるので注意してください。

# label & data拡張
label & data拡張は、ScratchとM5Stackの間で、文字列labelで、値がdata(文字列か数値)の特別なデータを送受信できます。

## ScratchからM5Stack方向の予約語
今のところ、画面描画のためのコマンドが実装されています。

### 予約されている変数
|ラベル名|意味|値|
|----|----|----|
|label|ラベルとデータの表示|0:表示しない(デフォルト)、それ以外:表示する|
|led|LEDのオンオフ|'on':オン, それ以外:オフ|
|x0,y0,x1,y1,x2,y2|座標(x,y)|整数|
|w,h|幅と高さ|整数|
|r|半径|整数|
|c|色(24bit)|整数|
|xc,yc|文字列表示位置|整数|
|str|表示する文字列|文字列|
|size|文字サイズ|整数(1-7)|
|tc|文字色(16bit)|整数|
|cmd|描画コマンド|文字列(下表参照)|

#### 予約されているコマンド
以下のような画面描画のためのコマンドが利用可能です。
|cmdのデータ|意味|実際のコマンド|
|----|----|----|
|drawPixel|点を描画|M5.Lcd.drawPixel(x0, y0, c)|
|drawLine|線を描画|M5.Lcd.drawLine(x0, y0, x1, y1, c)|
|drawRect|四角を描画|M5.Lcd.drawRect(x0, y0, w, h, c)|
|drawTriangl|三角を描画|M5.Lcd.drawTriangle(x0, y0, x1, y1, x2, y2, c)|
|drawRoundRe|角丸四角を描画|M5.Lcd.drawRoundRect(x0, y0, w, h, r, c)|
|fillScreen|全画面を単色で塗る|M5.Lcd.fillScreen(c)|
|fillRect|塗った四角を描画|M5.Lcd.fillRect(x0, y0, w, h, c)|
|fillCircle|塗った円を描画|M5.Lcd.fillCircle(x0, y0, r, c)|
|fillTriangl|塗った三角を描画|M5.Lcd.fillTriangle(x0, y0, x1, y1, x2, y2, c)|
|fillRoundRe|塗った角丸四角を描画|M5.Lcd.fillRoundRect(x0, y0, w, h, r, c)|
|print|文字列を描画|M5.Lcd.setCursor(xc, yc);M5.Lcd.setTextColor(tc);M5.Lcd.setTextSize(size);M5.Lcd.print(str)|

## M5StackからScratch方向の予約語
今のところ、以下のようなキーボード入力が利用可能です。

### 予約されている変数
以下のような変数が予約されています。
|ラベル|意味|実際の値|
|----|----|----|
|Key|Grayのキーボードで入力された文字コード|'a', 'b', 'A', ...|
|a|ランダムな'a'-'z'の文字を返す|'a', 'b', 'c', ...|

# TODO
## 実装可能だか未実装の項目
以下の項目は実装可能ですが、まだ実装していないものです。
- ピンへの入出力(GPIO, PWM, ADC, Servoなど)

## (標準の)M5Stackで足りない機能
以下の項目は、M5Stackに対応するセンサーなどが標準では無いため、実装できていません。
- 明るさセンサーの対応: Wio Terminalでは実装
- 磁力に関する機能(北からの角度などを含む): M5Stack Grayでは実装可能
- 音声入力((標準では)マイクが無いため): M5StickC/PlusやCore2などのマイク入力のある機種では実装可能


## 動画/発表資料
### M5StackとScratchのより良い関係:覚醒編; for  [IoT縛りの勉強会! IoTLT vol.85](https://iotlt.connpass.com/event/239891/)
- [発表資料](https://scrapbox.io/M5S/M5Stack%E3%81%A8Scratch%E3%81%AE%E3%82%88%E3%82%8A%E8%89%AF%E3%81%84%E9%96%A2%E4%BF%82:%E8%A6%9A%E9%86%92%E7%B7%A8)

[![M5StackとScratchのより良い関係:覚醒編動画1](https://img.youtube.com/vi/Vk8FoH25KJg/0.jpg)](https://youtu.be/Vk8FoH25KJg)

[![M5StackとScratchのより良い関係:覚醒編動画2](https://img.youtube.com/vi/FYaJ7QR_N-g/0.jpg)](https://youtu.be/FYaJ7QR_N-g)

### M5bit Less: M5Stack x Scratch3 = So Fun!!; for [M5Stack Japan Creativity Contest 2021](https://protopedia.net/event/22)
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
