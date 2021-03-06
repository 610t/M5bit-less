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

# TODO
## 実装可能だか未実装の項目
以下の項目は実装可能ですが、まだ実装していないものです。
- ラベルとデータを使ったメッセージのやりとり
- ピンへの入出力(GPIO, PWM, ADC, Servoなど)

## (標準の)M5Stackで足りない機能
以下の項目は、M5Stackに対応するセンサーなどが標準では無いため、実装できていません。
- 明るさセンサーの対応: Wio Terminalでは実装
- 磁力に関する機能(北からの角度などを含む): M5Stack Grayでは実装可能
- 音声入力((標準では)マイクが無いため): M5StickC/PlusやCore2などのマイク入力のある機種では実装可能


## 動画/発表資料
### M5bit Less: M5Stack x Scratch3 = So Fun!!; for [M5Stack Japan Creativity Contest 2021](https://protopedia.net/event/22)
- [発表資料](https://protopedia.net/prototype/2395)

[![M5bit Less: M5Stack x Scratch3 = So Fun!!動画](https://img.youtube.com/vi/-Nfu87CjvBU/0.jpg)](https://youtu.be/-Nfu87CjvBU)

### [Maker Faire Kyoto 2021](https://makezine.jp/event/mfk2021/)
#### M5bit Less: M5StackでScratchを使おう!!;「教育」カテゴリ応募作品
[![M5bit Less: M5StackでScratchを使おう!!](https://img.youtube.com/vi/sNwNkEHScCE/0.jpg)](https://www.youtube.com/watch?v=sNwNkEHScCE)

#### 京都観光案内 with M5bit Less - あかんターレで決めターレ; 「京都」カテゴリ応募作品
[![京都観光案内 with M5bit Less - あかんターレで決めターレ](https://img.youtube.com/vi/7ue7GZlBH6Y/0.jpg)](https://www.youtube.com/watch?v=7ue7GZlBH6Y)
