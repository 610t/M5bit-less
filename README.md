# M5bit-less
![M5bit Less](https://i.gyazo.com/37711fdbdec359e2834c6fbac1eb5bff.png)

[M5bit Less](https://scrapbox.io/M5S/M5bit_Less)は、[Microbit More](https://microbit-more.github.io/)をM5Stackで使うためのM5Stack用プログラムです。
Arduino IDEで作成しています。

## サポートしている機種
以下の機種で動くことを確認していますが、全てで同じ機能がサポートされているわけではありません。
詳しくは、サポートしている機能の表を参照してください。
- M5Stack Basic
- M5Stack Gray
- M5StickC
- M5StickC Plus

## サポートしている機能
- つながったときイベントブロック
- AとBボタンのクリック、下がったとき、上がったとき
- 文字列の表示
- パターンの表示
- 音の出力
- 加速度の入力(IMUを持つM5Stack Gray, M5StickC、MPU6886ユニット([M5Stack用6軸IMUユニット](https://www.switch-science.com/catalog/6623/)など)が必要です)

|機種|加速度入力|音出力|ボタン|
|---|---|---|---|
|M5Stack Basic|別途IMU必要|o|A,B,C(LOGO)|
|M5Stack Gray |o|o|A,B,C(LOGO)|
|M5StickC/Plus|o|x|A,B|

# どうやって使うの?
## M5Stack側の準備
[src/M5bit-less/M5bit-less.ino](src/M5bit-less/M5bit-less.ino)をArduino IDEからコンパイルして、M5Stackにダウンロードします。

## Scratch側の準備
Microbit Moreは、[stretch3](https://stretch3.github.io/)などで拡張機能として使うことができます。

M5bit Lessでは、画面に表示される"BBC micro:bit[hogehoge]"に接続します。

あとは、通常のMicrobit Moreのように利用することができますが、まだ実装していない機能があるので注意してください。

# TODO
## 実装可能だか未実装の項目
以下の項目は実装可能ですが、まだ実装していないものです。
- ラベルとデータを使ったメッセージのやりとり
- ピンへの入出力(GPIO, PWM, ADC, Servoなど)

## (標準の)M5Stackで足りない機能
以下の項目は、M5Stackに対応するセンサーなどが標準では無いため、実装できていません。
- 明るさセンサーの対応
- 磁力に関する機能(北からの角度などを含む): M5Stack Grayでは実装可能
- 音声入力((標準では)マイクが無いため): Core2などのマイク入力のある機種では実装可能
