# M5bit-less
![M5bit Less](https://i.gyazo.com/37711fdbdec359e2834c6fbac1eb5bff.png)

[M5bit Less](https://scrapbox.io/M5S/M5bit_Less)は、[Microbit More](https://microbit-more.github.io/)をM5Stackで使うためのM5Stack用プログラムです。
Arduino IDEで作成しています。

## デモ動画
### M5bit Less: M5StackでScratchを使おう!!; Maker Fair Kyoto 2021「教育」カテゴリ応募作品
[![M5bit Less: M5StackでScratchを使おう!!](https://img.youtube.com/vi/sNwNkEHScCE/0.jpg)](https://www.youtube.com/watch?v=sNwNkEHScCE)

### 京都観光案内 with M5bit Less - あかんターレで決めターレ ; Maker Fair Kyoto 2021「京都」カテゴリ応募作品
[![京都観光案内 with M5bit Less - あかんターレで決めターレ](https://img.youtube.com/vi/7ue7GZlBH6Y/0.jpg)](https://www.youtube.com/watch?v=7ue7GZlBH6Y)

## サポートしている機種
以下の機種で動くことを確認していますが、全てで同じ機能がサポートされているわけではありません。
詳しくは、サポートしている機能の表を参照してください。
- M5Stack Basic
- M5Stack Gray
- M5StickC
- M5StickC Plus
- Wio Terminal
- (作業中: ATOM Matrix)


## サポートしている機能
- つながったときイベントブロック
- 加速度の入力(IMUを持つM5Stack Gray, M5StickC、MPU6886ユニット([M5Stack用6軸IMUユニット](https://www.switch-science.com/catalog/6623/)など)が必要です)
- パターンの表示
- 文字列の表示
- 音を鳴らす
- 音の大きさ
- AとBボタンのクリック、下がったとき、上がったとき
- 明るさ
- 温度

|機種|加速度入力|パターン表示|文字列表示|音を鳴らす|音の大きさ|ボタン|明るさ|温度|備考|
|---|---|---|---|---|---|---|---|---|---|
|M5Stack Basic|△別途IMU必要|o|o|o|x|A,B,C(LOGO)|x|△別途IMU必要||
|M5Stack Gray |o|o|o|o|x|A,B,C(LOGO)|x|o||
|M5StickC|o|o|o|x|△(マイク付きモデル)|A,B|x|o||
|M5StickC Plus|o|o|o|o|△|A,B|x|o||
|Wio Terminal|o|o|o|△|o|A,B,C(LOGO)|o|o| |
|(ATOM Matrix)|o|o|△|x|x|A|x|x|作業中|
- △:対応可能だがまだ実装できていないもの。

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
- 明るさセンサーの対応: Wio Terminalでは実装可能
- 磁力に関する機能(北からの角度などを含む): M5Stack Grayでは実装可能
- 音声入力((標準では)マイクが無いため): Core2などのマイク入力のある機種では実装可能
