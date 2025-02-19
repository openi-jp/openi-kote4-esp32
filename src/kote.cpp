#include "kote.h"

#define ROLLER_TO_STACKER_RATIO 1.256 // ローラーとスタッカーの比率
#define ACCELERATION_TIME_SECONDS 2 // 加速時間

uint8_t webSocketNum = 0;

// コテの状態
KoteState koteState = KOTE_IDLE;
int32_t koteDistance = 0;
int32_t koteCurrentPosition = 0;
int32_t koteD = 0;
bool alreadyPause = false; 
unsigned long riftUpStartTime = 0; // リフトアップするスタート時間を記録
const unsigned long LIFT_UP_INTERVAL = 800; // リフトアップするミリ秒
bool isRiftUp = true;       // すでにリフトアップしたかどうかのフラグ
int32_t stacker_end_Distance1 = 0;
int32_t stacker_end_Distance2 = 0;
int32_t stacker_end_current_position = 0;


// スタッカーの状態
StackerState stackerState = STACKER_IDLE;
bool isPause = false;

const int32_t ARM_DISTANCE = 1705;  // アームの移動距離


uint32_t _speed = 1000; // スピード
int32_t _acceleration = 1000; // 加速度
int32_t _distance1 = 0; // ドロップする距離
int32_t _distance2 = 0;
int32_t currentPos1 = 0;

/**
 * アームを移動します。
 * 
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void stacker_arm(uint32_t speed, int32_t acceleration, int32_t distance) {
    Serial.println(F("armを移動します。"));
    Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);

    stacker_arm1->setSpeedInHz(speed);
    stacker_arm1->setAcceleration(acceleration * 3.0);
    stacker_arm1->moveTo(-distance);
}

/**
 * コマンドを処理します。
 *
 * @param num クライアント番号
 * @param cmd コマンド
 * @param webSocket WebSocketサーバー
 */
void kote_command(uint8_t num, JsonObject &cmd, WebSocketsServer &webSocket)
{
    webSocketNum = num;
    String command = cmd["command"].as<String>();
    Serial.printf("コマンド1: %s\n", command.c_str());

    if (command == "kote_blow")
    {
        webSocket.sendTXT(num, "done");

        String status = cmd["status"].as<String>();
        if (status == "on")
        {
            out22.digitalWrite(2, 1);
        }
        else if (status == "off")
        {
            out22.digitalWrite(2, 0);
        }
    }

    else if (command == "kote_cut")
    {
        webSocket.sendTXT(num, "done");

        // カッターを下に動かす
        out22.digitalWrite(0, 1);
        // in 0x26 2 がHIGHになるまで待つ
        while (in26.digitalRead(2) == 0)
        {
            delay(100);
        }
        // 0.2秒待つ
        delay(200);
        // カッターを上に動かす
        out22.digitalWrite(0, 0);
        // 0.4秒待つ
        delay(400);
        // リフターを下に動かす
        out21.digitalWrite(2, 1);
        // バキュームをOFF
        out22.digitalWrite(1, 1);
        // 0.5秒待つ
        delay(500);
    }

    else if (command == "kote_cutter")
    {
        webSocket.sendTXT(num, "done");

        String status = cmd["status"].as<String>();
        if (status == "up")
        {
            out22.digitalWrite(0, 0);
        }
        else if (status == "down")
        {
            out22.digitalWrite(0, 1);
        }
    }

    else if (command == "kote_emst")
    {
      webSocket.sendTXT(num, "done");
      String status = cmd["status"].as<String>();
      if (status == "on")
      {
        // emst = true;
        stepper1->stopMove();
        stepper2->stopMove();
      }
      else if (status == "off")
      {
        // emst = false;
      }
    }
        // スタッカーと連動して移動を終了します。
    else if (command == "kote_end_with_stacker")
    {
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t distance1 = cmd["distance1"].as<int32_t>();
        int32_t distance2 = cmd["distance2"].as<int32_t>();
        end_with_stacker(speed, distance1, distance2);

        // webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_heater1")
    {
        webSocket.sendTXT(num, "done");
        String status = cmd["status"].as<String>();
        if (status == "on")
        {
            out23.digitalWrite(0, 1);
        }
        else if (status == "off")
        {
            out23.digitalWrite(0, 0);
        }
    }

    else if (command == "kote_heater2")
    {
        webSocket.sendTXT(num, "done");
        String status = cmd["status"].as<String>();
        if (status == "on")
        {
            out23.digitalWrite(1, 1);
        }
        else if (status == "off")
        {
            out23.digitalWrite(1, 0);
        }
    }

    else if (command == "kote_lift")
    {
        webSocket.sendTXT(num, "done");
        String status = cmd["status"].as<String>();
        if (status == "up")
        {
            out21.digitalWrite(2, 0);
        }
        else if (status == "down")
        {
            out21.digitalWrite(2, 1);
        }
    }

    else if (command == "kote_move")
    {
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        move(speed, acceleration, distance);
        webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_move_stacker")
    {
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        int32_t d = cmd["d"].as<int32_t>();
        move_stacker(speed, acceleration, distance, d);
        // webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_move_with_stacker")
    {
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        move_with_stacker(speed, distance);
        // webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_move1")
    {
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        move1(speed, acceleration, distance);
        webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_move2")
    {
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        move2(speed, acceleration, distance);
        webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_pole1")
    {
        webSocket.sendTXT(num, "done");
        String status = cmd["status"].as<String>();
        if (status == "on")
        {
            out20.digitalWrite(0, 1);
        }
        else if (status == "off")
        {
            out20.digitalWrite(0, 0);
        }
    }

    else if (command == "kote_pole2")
    {
        webSocket.sendTXT(num, "done");
        String status = cmd["status"].as<String>();
        if (status == "on")
        {
            out20.digitalWrite(1, 1);
        }
        else if (status == "off")
        {
            out20.digitalWrite(1, 0);
        }
    }

    else if (command == "kote_pole3")
    {
        webSocket.sendTXT(num, "done");
        String status = cmd["status"].as<String>();
        if (status == "on")
        {
            out20.digitalWrite(2, 1);
        }
        else if (status == "off")
        {
            out20.digitalWrite(2, 0);
        }
    }

    else if (command == "kote_pole6")
    {
        webSocket.sendTXT(num, "done");
        String status = cmd["status"].as<String>();
        if (status == "on")
        {
            out21.digitalWrite(0, 1);
        }
        else if (status == "off")
        {
            out21.digitalWrite(0, 0);
        }
    }

    else if (command == "kote_roller2")
    {
        webSocket.sendTXT(num, "done");
        String status = cmd["status"].as<String>();
        if (status == "up")
        {
            out21.digitalWrite(1, 0);
        }
        else if (status == "down")
        {
            out21.digitalWrite(1, 1);
        }
    }

    else if (command == "kote_run1")
    {
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        run1(speed, acceleration, distance);
        webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_run2")
    {
        // バキュームをOFF
        out22.digitalWrite(1, 0);
        // ブローをON
        out22.digitalWrite(2, 1);
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        run2(speed, acceleration, distance);
        webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_run2_stacker")
    {
        // バキュームをOFF
        out22.digitalWrite(1, 0);
        // ブローをON
        out22.digitalWrite(2, 1);
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        int32_t d = cmd["d"].as<int32_t>();
        run2_stacker(speed, acceleration, distance, d);

        // webSocket.sendTXT(num, "done");

    }

    // スタッカーと連動して移動を開始します。
    else if (command == "kote_start_with_stacker")
    {
        // バキュームをOFF
        out22.digitalWrite(1, 0);
        // ブローをON
        out22.digitalWrite(2, 1);
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        start_with_stacker(speed, distance);

        // webSocket.sendTXT(num, "done");
    }

    else if (command == "kote_sensor1")
    {
        Serial.printf("kote_sensor1実行開始します.\n");
        uint8_t sensor1 = in25.digitalRead(1); // 布検知センサー
        Serial.printf("sensor1: %d\n", sensor1);
        // 0だったらOFF, 1だったらON
        if (sensor1 == 0)
        {
            webSocket.sendTXT(num, "OFF");
        }
        else
        {
            webSocket.sendTXT(num, "ON");
        }
    }

    else if (command == "kote_set") 
    {
        Serial.printf("kote_set実行開始します.\n");
        // ローラー2を下に動かす
        out21.digitalWrite(1, 1);
        // リフトを上に動かす
        out21.digitalWrite(2, 0);
        // カッターを上に動かす
        out22.digitalWrite(0, 0);
        // 1.5秒待つ
        delay(1500);

        webSocket.sendTXT(num, "done");
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();

        Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);
        move(speed, acceleration, distance);
    }

    else if (command == "kote_vacuum")
    {
        webSocket.sendTXT(num, "done");

        String status = cmd["status"].as<String>();
        if (status == "on")
        {
            out22.digitalWrite(1, 1);
        }
        else if (status == "off")
        {
            out22.digitalWrite(1, 0);
        }
    }

    // ホーミング
    else if (command == "stacker_homing"){
        webSocket.sendTXT(num, "done");
        homing();
    }

    // アームの移動
    else if (command == "stacker_arm"){
        webSocket.sendTXT(num, "done");
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        stacker_arm(speed, acceleration, distance);
    }

    // 実行
    else if (command == "stacker_run"){
        webSocket.sendTXT(num, "done");
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance1 = cmd["distance1"].as<int32_t>();
        int32_t distance2 = cmd["distance2"].as<int32_t>();
        stacker_run(speed, acceleration, distance1, distance2); 
    }

    // 移動
    else if (command == "stacker_motor1_2"){
        webSocket.sendTXT(num, "done");
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        stacker_motor1_2(speed, acceleration, distance);
    }

    // 移動
    else if (command == "stacker_motor4"){
        webSocket.sendTXT(num, "done");
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        stacker_motor4(speed, acceleration, distance);
    }

    // 移動
    else if (command == "stacker_move"){
        webSocket.sendTXT(num, "done");
        uint32_t speed = cmd["speed"].as<uint32_t>();
        int32_t acceleration = cmd["acceleration"].as<int32_t>();
        int32_t distance = cmd["distance"].as<int32_t>();
        stacker_move(speed, acceleration, distance);
    }

    // 一時停止
    else if (command == "stacker_pause"){
        webSocket.sendTXT(num, "done");
        stacker_pause();
    }

    // スタッカーをリセット
    else if (command == "stacker_reset"){
        webSocket.sendTXT(num, "done");
        stacker_reset();
    }

    // 再開
    else if (command == "stacker_resume"){
        webSocket.sendTXT(num, "done");
        stacker_resume();
    }



    else if (command == "stacker_stop"){
        webSocket.sendTXT(num, "done");
        stacker_stop();
    }
    // プレスの上昇
    else if (command == "stacker_press"){
        webSocket.sendTXT(num, "done");
        
        String status = cmd["status"].as<String>();
        if (status == "up") {
            out23.digitalWrite(2, 0);
        } else if (status == "down") {
            out23.digitalWrite(2, 1);
        }
    }

    // テスト
    else if (command == "stacker_hello"){
        webSocket.sendTXT(num, "done");
    }
}

/**
 * スタッカーと連動して移動する一番最初です。
 *
 * @param speed 速度
 * @param distance 移動距離
 */
void end_with_stacker(uint32_t speed, int32_t distance1, int32_t distance2)
{
    uint32_t stackerSpeed = static_cast<uint32_t>(speed * ROLLER_TO_STACKER_RATIO); // スタッカーの速度
    int32_t acceleration = speed / ACCELERATION_TIME_SECONDS; // 加速度
    int32_t stackerAcceleration = stackerSpeed / ACCELERATION_TIME_SECONDS; // スタッカーの加速度
    stacker_end_Distance1 = static_cast<int32_t>(distance1 * ROLLER_TO_STACKER_RATIO); // スタッカーの移動距離1
    stacker_end_Distance2 = static_cast<int32_t>(distance2 * ROLLER_TO_STACKER_RATIO); // スタッカーの移動距離2

    // stepper1->setSpeedInHz(speed);
    // stepper2->setSpeedInHz(speed);
    stacker_stepper1->setSpeedInHz(stackerSpeed);
    stacker_stepper2->setSpeedInHz(stackerSpeed);
    stacker_stepper4->setSpeedInHz(stackerSpeed);

    // stepper1->setAcceleration(acceleration);
    // stepper2->setAcceleration(acceleration);
    stacker_stepper1->setAcceleration(stackerAcceleration);
    stacker_stepper2->setAcceleration(stackerAcceleration);
    stacker_stepper4->setAcceleration(stackerAcceleration);

    stacker_stepper1->runForward();
    stacker_stepper2->runBackward();
    stacker_stepper4->runForward();

    stacker_end_current_position = stacker_stepper1->getCurrentPosition();

    koteState = KOTE_END_WITH_STACKER;
}

/**
 * モーターを移動します。
 *
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void move(uint32_t speed, int32_t acceleration, int32_t distance)
{
    Serial.println(F("移動します。"));
    Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);

    stepper1->setSpeedInHz(speed * 1.05);
    stepper2->setSpeedInHz(speed);

    stepper1->setAcceleration(acceleration);
    stepper2->setAcceleration(acceleration);

    stepper1->move(distance * 1.05);
    stepper2->move(distance);

    while (stepper1->isRunning() || stepper2->isRunning())
    {
        // yield();
    }
}

/**
 * モーターを移動します。
 *
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void move_stacker(uint32_t speed, int32_t acceleration, int32_t distance, int32_t d)
{
    // Serial.println(F("スタッカーと連動して移動します。"));

    koteCurrentPosition = stepper1->getCurrentPosition();
    koteDistance = distance;
    koteD = d;
    // Serial.printf("CurrentPosition: %d, Distance: %d\n", koteCurrentPosition, distance);

    stepper1->setSpeedInHz(speed);
    stepper2->setSpeedInHz(speed);

    stepper1->setAcceleration(acceleration);
    stepper2->setAcceleration(acceleration);

    stepper1->move(distance);
    stepper2->move(distance);

    stacker_resume();

    koteState = KOTE_MOVE_STACKER;

    // while (stepper1->isRunning() || stepper2->isRunning()) {
        // stacker_loop();

        // 目標距離までの残り距離を計算
        // int32_t remainingDistance = abs(distance - (stepper1->getCurrentPosition() - koteCurrentPosition));
        
        // remainingDistanceを表示
        // Serial.printf("remainingDistance: %d, distance: %d, CurrentPosition: %d, koteCurrentPosition: %d\n", remainingDistance, distance, stepper1->getCurrentPosition(), koteCurrentPosition);

        // 残り距離が300未満になったらスタッカーを停止
        // if (remainingDistance < d) {
            // Serial.println("スタッカーを停止します。");
            // stacker_pause();
            // break;
        // }
    // } 
}

/**
 * スタッカーと連動して移動します。
 *
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void move_with_stacker(uint32_t speed, int32_t distance)
{
    uint32_t stackerSpeed = static_cast<uint32_t>(speed * ROLLER_TO_STACKER_RATIO); // スタッカーの速度
    int32_t acceleration = speed / ACCELERATION_TIME_SECONDS; // 加速度
    int32_t stackerAcceleration = stackerSpeed / ACCELERATION_TIME_SECONDS; // スタッカーの加速度
    int32_t stackerDistance = static_cast<int32_t>(distance * ROLLER_TO_STACKER_RATIO); // スタッカーの移動距離

    stepper1->setSpeedInHz(speed);
    stepper2->setSpeedInHz(speed);
    stacker_stepper1->setSpeedInHz(stackerSpeed);
    stacker_stepper2->setSpeedInHz(stackerSpeed);
    stacker_stepper4->setSpeedInHz(stackerSpeed);

    stepper1->setAcceleration(acceleration);
    stepper2->setAcceleration(acceleration);
    stacker_stepper1->setAcceleration(stackerAcceleration);
    stacker_stepper2->setAcceleration(stackerAcceleration);
    stacker_stepper4->setAcceleration(stackerAcceleration);

    stepper1->move(distance);
    stepper2->move(distance);
    stacker_stepper1->move(stackerDistance);
    stacker_stepper2->move(-stackerDistance);
    stacker_stepper4->move(stackerDistance);

    koteState = KOTE_MOVE_WITH_STACKER;
}

/**
 * モーター1を移動します。
 *
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void move1(uint32_t speed, int32_t acceleration, int32_t distance)
{
    Serial.println(F("モーター1を移動します。"));
    Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);

    stepper1->setSpeedInHz(speed);
    stepper1->setAcceleration(acceleration);
    stepper1->move(distance);
}

/**
 * モーター2を移動します。
 *
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void move2(uint32_t speed, int32_t acceleration, int32_t distance)
{
    Serial.println(F("モーター2を移動します。"));
    Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);

    stepper2->setSpeedInHz(speed);
    stepper2->setAcceleration(acceleration);
    stepper2->move(distance);

}

/**
 * モーターを実行します。
 *
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void run1(uint32_t speed, int32_t acceleration, int32_t distance)
{
    Serial.println(F("実行1します。"));
    Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);

    stepper1->setSpeedInHz(speed);
    stepper2->setSpeedInHz(speed);

    stepper1->setAcceleration(acceleration);
    stepper2->setAcceleration(acceleration);

    stepper1->move(distance);
    stepper2->move(distance);

    while (stepper1->isRunning() || stepper2->isRunning())
    {
        // yield();
    }
}

/**
 * モーターを実行します。
 *
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void run2(uint32_t speed, int32_t acceleration, int32_t distance)
{
    Serial.println(F("実行1します。"));
    Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);

    stepper1->setSpeedInHz(speed);
    stepper2->setSpeedInHz(speed);

    stepper1->setAcceleration(acceleration);
    stepper2->setAcceleration(acceleration);

    stepper1->move(distance);
    stepper2->move(distance);

    unsigned long startTime = millis(); // スタート時間を記録
    unsigned long interval = 300;       // 0.3秒をミリ秒で表記
    bool alreadyExecuted = false;       // コマンド実行フラグ

    while (stepper1->isRunning() || stepper2->isRunning())
    {
        if (!alreadyExecuted && millis() - startTime >= interval)
        {
            out21.digitalWrite(2, 0); // リフトを上に動かす
            out22.digitalWrite(2, 0); // ブローをOFF
            alreadyExecuted = true;   // コマンドを実行したのでフラグを立てる
        }
        // yield();
    }
}


/**
 * モーターを実行します。
 *
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void run2_stacker(uint32_t speed, int32_t acceleration, int32_t distance, int32_t d)
{
    // Serial.println(F("run2_stacker 実行1します。"));
    koteCurrentPosition = stepper1->getCurrentPosition();
    koteDistance = distance;
    koteD = d;

    // Serial.printf("CurrentPosition: %d, distance: %d\n", koteCurrentPosition, distance);

    stepper1->setSpeedInHz(speed);
    stepper2->setSpeedInHz(speed);

    stepper1->setAcceleration(acceleration);
    stepper2->setAcceleration(acceleration);

    stepper1->move(distance);
    stepper2->move(distance);

    stacker_resume();

    riftUpStartTime = millis(); // スタート時間を記録
    isRiftUp = false; 

    koteState = KOTE_MOVE_STACKER;
    // unsigned long interval = 800;       // 0.3秒をミリ秒で表記
    // bool alreadyExecuted = false;       // コマンド実行フラグ
    /*
    while (stepper1->isRunning() || stepper2->isRunning())
    {
        stacker_loop();

        if (!alreadyExecuted && millis() - startTime >= interval)
        {
            out21.digitalWrite(2, 0); // リフトを上に動かす
            out22.digitalWrite(2, 0); // ブローをOFF
            alreadyExecuted = true;   // コマンドを実行したのでフラグを立てる
        }

        // 目標距離までの残り距離を計算
        int32_t remainingDistance = abs(distance - (stepper1->getCurrentPosition() - koteCurrentPosition));

        // 表示
        // Serial.printf("remainingDistance: %d, distance: %d, CurrentPosition: %d, koteCurrentPosition: %d\n", remainingDistance, distance, stepper1->getCurrentPosition(), koteCurrentPosition);

        //const int32_t MIN = 600;
                // 残り距離が300未満になったらスタッカーを停止
        if (remainingDistance < d) {
            // Serial.println("スタッカーを停止します。");
            stacker_pause();
            break;
        }

        // yield();
    }
    */
}

/**
 * スタッカーと連動して移動する一番最初です。
 *
 * @param speed 速度
 * @param distance 移動距離
 */
void start_with_stacker(uint32_t speed, int32_t distance)
{
    uint32_t stackerSpeed = static_cast<uint32_t>(speed * ROLLER_TO_STACKER_RATIO); // スタッカーの速度
    int32_t acceleration = speed / ACCELERATION_TIME_SECONDS; // 加速度
    int32_t stackerAcceleration = stackerSpeed / ACCELERATION_TIME_SECONDS; // スタッカーの加速度
    int32_t stackerDistance = static_cast<int32_t>(distance * ROLLER_TO_STACKER_RATIO); // スタッカーの移動距離

    stepper1->setSpeedInHz(speed);
    stepper2->setSpeedInHz(speed);
    stacker_stepper1->setSpeedInHz(stackerSpeed);
    stacker_stepper2->setSpeedInHz(stackerSpeed);
    stacker_stepper4->setSpeedInHz(stackerSpeed);

    stepper1->setAcceleration(acceleration);
    stepper2->setAcceleration(acceleration);
    stacker_stepper1->setAcceleration(stackerAcceleration);
    stacker_stepper2->setAcceleration(stackerAcceleration);
    
    stacker_stepper4->setAcceleration(stackerAcceleration);

    stepper1->move(distance);
    stepper2->move(distance);
    stacker_stepper1->move(stackerDistance);
    stacker_stepper2->move(-stackerDistance);
    // アームを伸ばす
    stacker_arm1->moveTo(-ARM_DISTANCE);
    stacker_stepper4->move(stackerDistance);

    riftUpStartTime = millis(); // スタート時間を記録
    isRiftUp = false; 

    koteState = KOTE_START_WITH_STACKER;
}

/**
 * スタッカーを動かします。
 * 
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance ドロップする距離
 */
void stacker_run(uint32_t speed, int32_t acceleration, int32_t distance1, int32_t distance2) {
    _speed = speed;
    _acceleration = acceleration;
    _distance1 = distance1;
    _distance2 = distance2;

    stacker_stepper1->setSpeedInHz(speed);
    stacker_stepper2->setSpeedInHz(speed);
    stacker_arm1->setSpeedInHz(speed);
    stacker_stepper4->setSpeedInHz(speed);

    stacker_stepper1->setAcceleration(acceleration);
    stacker_stepper2->setAcceleration(acceleration);
    stacker_arm1->setAcceleration(acceleration * 3.0);
    stacker_stepper4->setAcceleration(acceleration);

    stacker_stepper1->runForward();
    stacker_stepper2->runBackward();
    stacker_stepper4->runForward();

    stackerState = STACKER_RUN;
}

/**
 * スタッカーのモーター1, 2を動かします。
 * 
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void stacker_motor1_2(uint32_t speed, int32_t acceleration, int32_t distance) {
    Serial.println(F("motor1, motor2の試運転を行います。"));
    Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);

    stacker_stepper1->setSpeedInHz(speed);
    stacker_stepper2->setSpeedInHz(speed);

    stacker_stepper1->setAcceleration(acceleration);
    stacker_stepper2->setAcceleration(acceleration);

    stacker_stepper1->move(distance);
    stacker_stepper2->move(distance);
}

/**
 * スタッカーのモーター4を動かします。
 * 
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void stacker_motor4(uint32_t speed, int32_t acceleration, int32_t distance) {
    Serial.println(F("motor4の試運転を行います。"));
    Serial.printf("speed: %d, acceleration: %d, distance: %d\n", speed, acceleration, distance);

    stacker_stepper4->setSpeedInHz(speed);
    stacker_stepper4->setAcceleration(acceleration);
    stacker_stepper4->move(distance);
}

/**
 * スタッカーを移動します。
 * 
 * @param speed 速度
 * @param acceleration 加速度
 * @param distance 移動距離
 */
void stacker_move(uint32_t speed, int32_t acceleration, int32_t distance) {
        // プレスを上昇させます
    // out23.digitalWrite(2, 0);

    stacker_stepper1->setSpeedInHz(speed);
    stacker_stepper2->setSpeedInHz(speed);
    stacker_stepper4->setSpeedInHz(speed);

    stacker_stepper1->setAcceleration(acceleration);
    stacker_stepper2->setAcceleration(acceleration);
    stacker_stepper4->setAcceleration(acceleration);

    stacker_stepper1->move(distance);
    stacker_stepper2->move(distance);
    stacker_stepper4->move(distance);
}

/**
 * スタッカーを一時停止します。
 */
void stacker_pause() {
    // Serial.println("stacker_pause() スタッカーを一時停止します。");

    // Serial.printf("getCurrentPosition:  %d\n", stacker_stepper1->getCurrentPosition());
    stacker_stepper1->stopMove();
    stacker_arm1->stopMove();
    stacker_stepper2->stopMove();
    stacker_stepper4->stopMove();

    isPause = true;
}

/**
 * スタッカーをリセットします。
 */
void stacker_reset() {
    Serial.println("stacker_reset() スタッカーをリセットします。");
    // アームをホームに戻します
    stacker_arm1->moveTo(0);
    // 抑えを上昇させます
    out23.digitalWrite(2, 0);
    stackerState = STACKER_IDLE;
}

/**
 * スタッカーを再開します。   
 */
void stacker_resume() {
    // Serial.println("stacker_pause() スタッカーを再開します。");

    // Serial.printf("getCurrentPosition:  %d\n", stacker_stepper1->getCurrentPosition());
    stacker_stepper1->runForward();
    if (stackerState == STACKER_EXTEND_ARM) {
        stacker_arm1->moveTo(-ARM_DISTANCE);
    } else if (stackerState == STACKER_RETRACT_ARM) {
        stacker_arm1->moveTo(0);
    }
    stacker_stepper2->runBackward();
    stacker_stepper4->runForward();

    isPause = false;
}

/**
 * スタッカーを停止します。
 */
void stacker_stop() {

    stacker_stepper1->stopMove();
    stacker_stepper2->stopMove();
    stacker_arm1->stopMove();
    stacker_stepper4->stopMove();

    // stacker_arm1->moveTo(0); 
    // プレスを上昇させます
    // out23.digitalWrite(2, 0);

    isPause = false;
    stackerState = STACKER_IDLE;

}

/**
 * コテのループ処理
 */
void kote_loop() {
    switch (koteState){
        case KOTE_MOVE_STACKER:
            if (stepper1->isRunning() || stepper2->isRunning()) {
                // 目標距離までの残り距離を計算
                int32_t remainingDistance = abs(koteDistance - (stepper1->getCurrentPosition() - koteCurrentPosition));
                
                // remainingDistanceを表示
                // Serial.printf("remainingDistance: %d\n", remainingDistance);

                if (!isRiftUp && millis() - riftUpStartTime >= LIFT_UP_INTERVAL)
                {
                    out21.digitalWrite(2, 0); // リフトを上に動かす
                    out22.digitalWrite(2, 0); // ブローをOFF
                    isRiftUp = true;   // コマンドを実行したのでフラグを立てる
                }

                // 残り距離が300未満になったらスタッカーを停止
                if (!alreadyPause && remainingDistance < koteD) {
                    // Serial.println("スタッカーを停止します。");
                    stacker_pause();
                    alreadyPause = true;
                }
            } else {
                koteState = KOTE_IDLE;
                koteDistance = 0;
                koteCurrentPosition = 0;
                koteD = 0;
                alreadyPause = false;
                webSocket.sendTXT(webSocketNum, "done");
            }
            break;
        // スタッカーと連動して最初に移動する場合
        case KOTE_START_WITH_STACKER:
            if (!stepper1->isRunning() && !stepper2->isRunning() && !stacker_stepper1->isRunning() && !stacker_stepper2->isRunning() && !stacker_stepper4->isRunning()) {
                koteState = KOTE_IDLE;
                webSocket.sendTXT(webSocketNum, "done");
            } else {
                if (!isRiftUp && millis() - riftUpStartTime >= LIFT_UP_INTERVAL)
                    {
                        out21.digitalWrite(2, 0); // リフトを上に動かす
                        out22.digitalWrite(2, 0); // ブローをOFF
                        isRiftUp = true;   // コマンドを実行したのでフラグを立てる
                    }
            }
            break;
        // スタッカーと連動して移動する場合
        case KOTE_MOVE_WITH_STACKER:
            if (!stepper1->isRunning() && !stepper2->isRunning() && !stacker_stepper1->isRunning() && !stacker_stepper2->isRunning() && !stacker_stepper4->isRunning()) {
                koteState = KOTE_IDLE;
                webSocket.sendTXT(webSocketNum, "done");
            }
            break;

        case KOTE_END_WITH_STACKER:
            if (stacker_stepper1->getCurrentPosition() - stacker_end_current_position > stacker_end_Distance1) {
                stacker_arm1->moveTo(0); // アームを戻す
                koteState = KOTE_END2_WITH_STACKER;
            }
            break;

        case KOTE_END2_WITH_STACKER:
            if (!stacker_arm1->isRunning()) {
                stacker_end_current_position = stacker_stepper1->getCurrentPosition();
                out23.digitalWrite(2, 1); // プレスを下降させます
                koteState = KOTE_END3_WITH_STACKER;           
            } 
            break;

        case KOTE_END3_WITH_STACKER:
            if (stacker_stepper1->getCurrentPosition() - stacker_end_current_position > stacker_end_Distance2) {
                // アームを伸ばす
                stacker_arm1->moveTo(-ARM_DISTANCE);
                koteState = KOTE_IDLE;
                webSocket.sendTXT(webSocketNum, "done");
            }
            break;


    }
}


/**
 * スタッカーのループ処理
 */
void stacker_loop() {
    if (stackerState == STACKER_IDLE || isPause) {
        return; // IDLEまたは一時停止中は何もしない
    }

    switch (stackerState) {
        case STACKER_RUN:
            if (in27.digitalRead(1) == HIGH) {
                Serial.println("布端センサーがONになりました。");
                stacker_stepper4->setSpeedInHz(_speed * 0.5);
                stacker_arm1->moveTo(-ARM_DISTANCE);

                 // アームを伸ばすモードへ。
                stackerState = STACKER_EXTEND_ARM;
            }
            break;

        case STACKER_EXTEND_ARM:
            if (!stacker_arm1->isRunning()) {
                Serial.println("ドロップします。");

                stacker_stepper4->setSpeedInHz(_speed);
                // ここでstepper1の現在位置を保存
                currentPos1 = stacker_stepper1->getCurrentPosition();

                stackerState = STACKER_DROP;
            }
            break;

        case STACKER_DROP:
            if (stacker_stepper1->getCurrentPosition() - currentPos1 > _distance1) {
                Serial.printf("Final getCurrentPosition:  %d, currentPos1: %d, distance1: %d\n", stacker_stepper1->getCurrentPosition(), currentPos1, _distance1);
                // Serial.println(F("ドロップ完了でアームを戻します。"));
                stacker_stepper4->setSpeedInHz(_speed);
                stacker_arm1->moveTo(0); // アームを戻す
                
                stackerState = STACKER_RETRACT_ARM;
            }
            break;

        case STACKER_RETRACT_ARM:
            if (!stacker_arm1->isRunning()) {
                out23.digitalWrite(2, 1); // プレスを下降させます
                // Serial.println("アームを戻しました。");
                stacker_stepper4->setSpeedInHz(_speed);
                // ここでstepper1の現在位置を保存
                currentPos1 = stacker_stepper1->getCurrentPosition();

                stackerState = STACKER_DROP2;
            }
            break;

        case STACKER_DROP2:
        Serial.printf("getCurrentPosition:  %d, currentPos1: %d, distance2: %d\n", stacker_stepper1->getCurrentPosition(), currentPos1, _distance2);
    
            if (stacker_stepper1->getCurrentPosition() - currentPos1 > _distance2) {
                // Serial.println("ドロップ2完了でアームを戻します。");

                stackerState = STACKER_FINAL;
            }
            break;

        case STACKER_FINAL:
            if (in27.digitalRead(1) == LOW) {
                Serial.println("スタッカーを停止します。");
                out23.digitalWrite(2, 0); // プレスを上昇させます

                stackerState = STACKER_RUN;
            }
            break;
    }
}
