#include <Arduino.h>
#include <ArduinoJson.h>
#include <FastAccelStepper.h>
#include <string>
#include <WebSocketsServer.h>

#include <openi_in.h>
#include <openi_out.h>
#include "constants.h"
#include "kote.h"
// #include "stacker.h"

openi_out out20;
openi_out out21;
openi_out out22;
openi_out out23;

openi_in in25;
openi_in in26;
openi_in in27;

WebSocketsServer webSocket = WebSocketsServer(WEBSOCKET_PORT);

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper1 = nullptr;
FastAccelStepper *stepper2 = nullptr;
FastAccelStepper *stacker_stepper1 = nullptr;
FastAccelStepper *stacker_stepper2 = nullptr;
FastAccelStepper *stacker_arm1 = nullptr;
FastAccelStepper *stacker_stepper4 = nullptr;

bool emst = false; // 非常停止

/**
 * WebSocketのイベント処理
 *
 * @param num クライアント番号
 * @param type イベントタイプ
 * @param payload ペイロード
 * @param length ペイロードの長さ
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_CONNECTED:
    Serial.println(F("タブレットと接続されました。"));
    break;

  case WStype_DISCONNECTED:
    Serial.println(F("タブレットと接続が切断されました。"));
    break;

  case WStype_TEXT:
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error)
    {
      Serial.print(F("コマンドのシリアライズに失敗しました。: "));
      Serial.println(error.c_str());
      return;
    }

    JsonObject cmd = doc.as<JsonObject>();
    kote_command(num, cmd, webSocket);
    
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Openi Kote3 Start"));

  // WiFiの接続
  WiFi.config(IPAddress(LOCAL_IP), IPAddress(GATEWAY), IPAddress(SUBNET));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  bool done = true;
  while (done)
  {
    Serial.print("WiFi に接続中");

    auto last = millis();
    while (WiFi.status() != WL_CONNECTED && last + 5000 > millis())
    {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      done = false;
    }
    else
    {
      Serial.println("再接続します。");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }

  Serial.println("");
  Serial.println(F("WiFiに接続しました。"));

  // WebSocketサーバー開始
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  out20.begin(0x20);
  out21.begin(0x21);
  out22.begin(0x22);
  out23.begin(0x23);

  in25.begin(0x25);
  in26.begin(0x26);
  in27.begin(0x27);

  // ステッパーモーターの接続と設定
  engine.init();
  stepper1 = engine.stepperConnectToPin(GA1);
  if (stepper1)
  {
    stepper1->setDirectionPin(GA2);
  } else {
    Serial.println("stepper1 is null");
  }
  stepper2 = engine.stepperConnectToPin(GA3);
  if (stepper2)
  {
    stepper2->setDirectionPin(GB2);
  } else {
    Serial.println("stepper2 is null");
  }
  stacker_stepper1 = engine.stepperConnectToPin(GB1);
  if (stacker_stepper1)
  {
    stacker_stepper1->setDirectionPin(GB3);
  }
  stacker_stepper2 = engine.stepperConnectToPin(GC1);
  if (stacker_stepper2)
  {
    stacker_stepper2->setDirectionPin(GC2);
  } else {
    Serial.println("stacker_stepper2 is null");
  }

  stacker_arm1 = engine.stepperConnectToPin(GC3);
  if (stacker_arm1)
  {
    stacker_arm1->setDirectionPin(MOSI);
  }
  stacker_stepper4 = engine.stepperConnectToPin(MISO);
  if (stacker_stepper4)
  {
    stacker_stepper4->setDirectionPin(SCKL);
  }
}

void loop()
{
  kote_loop();
  stacker_loop();
  webSocket.loop();
}
