#pragma once
#ifndef PICO_PWM_AUDIO_H
#define PICO_PWM_AUDIO_H

#include <Arduino.h>
#include "hardware/pwm.h"

// 最大同時発音数
#define MAX_POLYPHONY 6
// サンプリング周波数 (Hz)
#define AUDIO_SAMPLE_RATE 22050
// PWMの分解能 (8bit: 0~255)
#define PWM_TOP_VAL 255

// 各発音（ボイス）の管理構造体
struct Voice {
    bool active;               // 再生中フラグ
    float frequency;           // 周波数 (Hz)
    float phase;               // 現在の位相 (0.0 ~ 1.0)
    float phase_increment;     // 1サンプルごとの位相進み量
    float volume;              // 現在の音量 (1.0 ~ 0.0)
    float decay_rate;          // 1サンプルあたりの音量減衰量
    uint32_t sustain_samples;  // 保持する残りのサンプル数
};

class PicoPwmAudio {
public:
    PicoPwmAudio();
    ~PicoPwmAudio();

    /**
     * @brief PWMオーディオの初期化
     * @param pinNo 再生に使用するGPIOピン番号
     */
    void init(uint8_t pinNo);

    /**
     * @brief 合成波形（音）の追加
     * @param audio_frequency 追加する音の周波数 (Hz)
     * @param sustain_ms 音量を維持する持続時間 (ミリ秒、デフォルト: 0)
     * @note sustain_ms 経過後、1秒かけてエンベロープ（フェードアウト）します。
     */
    void writeTone(uint16_t audio_frequency, uint32_t sustain_ms = 0);

    /**
     * @brief 現在登録されているすべてのトーンを消去して無音にします
     */
    void clearTone();

    /**
     * @brief 音の再生開始（タイマーの開始）
     */
    void startTone();

    /**
     * @brief 音の再生終了（すべてのボイスを消音しタイマーを停止）
     */
    void stopTone();

    /**
     * @brief 周期ごとに呼ばれるオーディオ波形計算関数
     * @return PWMデューティ比 (0 ~ 255)
     */
    uint16_t tick();

private:
    uint8_t _pin;
    uint _slice_num;
    uint _channel;
    bool _is_playing;

    Voice _voices[MAX_POLYPHONY];
    struct repeating_timer _timer;

    // タイマー割り込みから呼び出す静的コールバック
    static bool timerCallback(struct repeating_timer *t);
};

#endif // PICO_PWM_AUDIO_H