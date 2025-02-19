#include "homing.h"

#define SENSOR_PIN 0 

/**
 * 原点復帰します。
 */
void homing()
{
    Serial.println(F("原点復帰を開始しました。"));

    if (in27.digitalRead(SENSOR_PIN) == LOW) {
        Serial.println(F("行き過ぎているので0.5秒逆回転します。"));
        stacker_arm1->setSpeedInHz(100);
        stacker_arm1->setAcceleration(100);
        stacker_arm1->runBackward();
        unsigned long startTime = millis();
        while (millis() - startTime < 500) {
            yield();
        }
        stacker_arm1->stopMove();
    }

    Serial.println(F("センサーがLOWになるまでゆっくり正回転します。"));
    stacker_arm1->setSpeedInHz(30);
    stacker_arm1->setAcceleration(30);
    stacker_arm1->runForward();

    while (in27.digitalRead(SENSOR_PIN) == HIGH) {
        yield();
    }
    stacker_arm1->stopMove();

    Serial.println(F("10ステップ逆回転してここを原点とします。"));
    stacker_arm1->move(-10);
    while (stacker_arm1->isRunning()) {
        yield();
    }

    Serial.println(F("原点復帰が完了しました。"));
    stacker_arm1->setCurrentPosition(0);  // 現在の位置を0と設定
}

/**
 * 原点復帰を行います。
 * 1. in27.digitalRead(0)で原点センサの状態を取得
 * 2. in27.digitalRead(0)がLOWなら行き過ぎてるのでstepper3を逆回転で１秒間回転
 * 3. in27.digitalRead(0)がLOWになるまでstepper3を正回転
 * 4. stepper3を20ステップ逆回転してここを原点とする
 */