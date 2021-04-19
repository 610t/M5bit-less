# M5bit-less
[M5bit Less](https://scrapbox.io/M5S/M5bit_Less)は、[Microbit More](https://microbit-more.github.io/)をM5Stackで使うためのM5Stack用プログラムです。
Arduino IDEで作成しています。

## サポートしている機能
- つながったときイベントブロック
- AとBボタンのクリック(下がったときと上がったときは未実装)
- 文字列の表示
- パターンの表示
- 音の出力
- 加速度の入力(IMUを持つM5Stack GrayやMPU6886ユニット(M5Stack用6軸IMUユニットなど)が必要です)

# どうやって使うの?
## M5Stack側の準備
[src/M5bit-less/M5bit-less.ino](src/M5bit-less/M5bit-less.ino)をArduino IDEからコンパイルして、M5Stackにダウンロードします。

## Scratch側の準備
Microbit Moreは、[stretch3](https://stretch3.github.io/)などで拡張機能として使うことができます。

現在のM5bit Lessでは、固定IDとして"BBC micro:bit[m5scr]"を使っているので、これに接続します。

あとは、通常のMicrobit Moreのように利用することができますが、まだ実装していない機能があるので注意してください。

# TODO
## 実装可能だか未実装の項目
以下の項目は実装可能ですが、まだ実装していないものです。
- ラベルとデータを使ったメッセージのやりとり
- ピンへの入出力(GPIO, PWM, ADC, Servoなど)

## (標準の)M5Stackで足りない機能
以下の項目は、M5Stackに対応するセンサーなどが標準では無いため、実装できていません。
- 明るさセンサーの対応
- 磁力に関する機能(北からの角度などを含む)
- 音声入力((標準では)マイクが無いため)
