#ifndef KOTE_H
#define KOTE_H

#include <ArduinoJson.h>
#include <FastAccelStepper.h>
#include <WebSocketsServer.h>

#include <openi_in.h>
#include <openi_out.h>
#include "homing.h"

extern WebSocketsServer webSocket;

extern FastAccelStepper *stepper1;
extern FastAccelStepper *stepper2;
extern FastAccelStepper *stacker_stepper1;
extern FastAccelStepper *stacker_stepper2;
extern FastAccelStepper *stacker_arm1;
extern FastAccelStepper *stacker_stepper4;

extern openi_out out20;
extern openi_out out21;
extern openi_out out22;
extern openi_out out23;

extern openi_in in25;
extern openi_in in26;
extern openi_in in27;

enum KoteState {
    KOTE_IDLE,              // 停止中です。
    KOTE_MOVE_STACKER,      // スタッかー連動の移動中です。
    KOTE_START_WITH_STACKER, // スタッカーと連動して移動を開始します。
    KOTE_MOVE_WITH_STACKER, // スタッカー連動の移動中です。
    KOTE_END_WITH_STACKER, // スタッカーと連動して移動を終了します。
    KOTE_END2_WITH_STACKER, // スタッカーと連動して移動を終了します。
    KOTE_END3_WITH_STACKER, // スタッカーと連動して移動を終了します。

};

enum StackerState {
    STACKER_IDLE,           // 停止中です。
    STACKER_RUN,            // 動作中です。
    STACKER_EXTEND_ARM,     // アームを伸ばします。
    STACKER_DROP,           // たらします。
    STACKER_RETRACT_ARM,    // アームを戻します。
    STACKER_DROP2,           // たらします。
    STACKER_FINAL           // 最終工程です。
};

void kote_command(uint8_t num, JsonObject& cmd, WebSocketsServer& webSocket);
void move(uint32_t speed, int32_t acceleration, int32_t distance);
void move_stacker(uint32_t speed, int32_t acceleration, int32_t distance, int32_t d);
void move1(uint32_t speed, int32_t acceleration, int32_t distance);
void move2(uint32_t speed, int32_t acceleration, int32_t distance);
void run1(uint32_t speed, int32_t acceleration, int32_t distance);
void run2(uint32_t speed, int32_t acceleration, int32_t distance);
void run2_stacker(uint32_t speed, int32_t acceleration, int32_t distance, int32_t d);
void kote_loop();

void stacker_command(uint8_t num, JsonObject& cmd, WebSocketsServer& webSocket);
void stacker_run(uint32_t speed, int32_t acceleration, int32_t distance, int32_t distance2);
void stacker_motor1_2(uint32_t speed, int32_t acceleration, int32_t distance);
void stacker_motor4(uint32_t speed, int32_t acceleration, int32_t distance);
void stacker_move(uint32_t speed, int32_t acceleration, int32_t distance);
void stacker_pause();
void stacker_reset();
void stacker_resume();
void stacker_stop();
void stacker_loop();

#endif
