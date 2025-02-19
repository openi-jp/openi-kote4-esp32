# openi-kote3-esp32
コテ打ち自動機のESP32バージョン4です。スタッカーを効率的に改良しました。

## GPIO
|GPIOピン割り当て|ピンの用途|
|---|---|
|GA1|stepper1 pls|
|GA2|stepper1 dir|
|GA3|stepper2 pls|
|GB1|stacker_stepper1 pls|
|GB2|stepper2 dir|
|GB3|stacker_stepper1 dir|
|GC1|stacker_stepper2 pls|
|GC2|stacker_stepper2 dir|
|GC3|stacker_arm pls|
|MOSI|stacker_arm dir|
|MISO|stacker_stepper4 pls|
|SCK|stacker_stepper4 dir|

## I2C通信

### 出力4枚
|I2Cアドレス|GPIO0|GPIO1|GPIO2|
|---|---|---|---|
|x20|ポール1-4|ポール2|ポール3-5|
|x21|ポール6|ローラー2|リフト|
|x22|カッター|バキューム|ブロー|
|x23|ヒーター1|ヒーター2|スタッカープレス|

### 入力3枚
|I2Cアドレス|GPIO0|GPIO1|GPIO2|
|---|---|---|---|
|0x25|非常停止|布検知センサー|反転信号|
|0x26|非常停止2|シリンダー上センサー|シリンダー下センサー|
|0x27|スタッカー原点センサー|スタッカー布端センサー||
