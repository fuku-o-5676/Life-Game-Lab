#include "PicoPwmAudio.h"

PicoPwmAudio::PicoPwmAudio() 
    : _pin(0), _slice_num(0), _channel(0), _is_playing(false) {
    clearTone();
}

PicoPwmAudio::~PicoPwmAudio() {
    stopTone();
}

void PicoPwmAudio::init(uint8_t pinNo) {
    _pin = pinNo;

    // 1. GPIOをPWM出力モードに設定
    gpio_set_function(_pin, GPIO_FUNC_PWM);
    _slice_num = pwm_gpio_to_slice_num(_pin);
    _channel = pwm_gpio_to_channel(_pin);

    // 2. PWMのクロック・キャリア周波数の設定（約125kHz）
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_TOP_VAL);
    pwm_init(_slice_num, &config, true);

    // 初期状態はセンター（無音）を出力
    pwm_set_chan_level(_slice_num, _channel, PWM_TOP_VAL / 2);
}

void PicoPwmAudio::writeTone(uint16_t audio_frequency, uint32_t sustain_ms) {
    if (audio_frequency == 0) return;

    // ミリ秒からサンプル数への変換
    uint32_t sustain_smp = (uint32_t)((float)AUDIO_SAMPLE_RATE * (sustain_ms / 1000.0f));

    // 既存の同じ周波数が鳴っていればリセットしてパラメータ更新
    for (int i = 0; i < MAX_POLYPHONY; i++) {
        if (_voices[i].active && (uint16_t)_voices[i].frequency == audio_frequency) {
            _voices[i].phase = 0.0f;
            _voices[i].volume = 1.0f;
            _voices[i].sustain_samples = sustain_smp;
            return;
        }
    }

    // 空いているボイスを探して割り当て
    int target_index = -1;
    for (int i = 0; i < MAX_POLYPHONY; i++) {
        if (!_voices[i].active) {
            target_index = i;
            break;
        }
    }

    // 空きがない場合は一番音量の小さい（消えかかっている）ボイスを上書き
    if (target_index == -1) {
        float min_vol = 1.0f;
        target_index = 0;
        for (int i = 0; i < MAX_POLYPHONY; i++) {
            if (_voices[i].volume < min_vol) {
                min_vol = _voices[i].volume;
                target_index = i;
            }
        }
    }

    // ボイスパラメータ設定
    Voice &v = _voices[target_index];
    v.frequency = (float)audio_frequency;
    v.phase = 0.0f;
    v.phase_increment = v.frequency / (float)AUDIO_SAMPLE_RATE;
    v.volume = 1.0f;
    v.sustain_samples = sustain_smp;
    
    // 1.0 秒で 1.0 から 0.0 になる減衰レート（1サンプルあたり）
    v.decay_rate = 1.0f / ((float)AUDIO_SAMPLE_RATE * 0.3f);
    
    v.active = true;
}

void PicoPwmAudio::clearTone() {
    // 全ボイスのフラグと変数をリセット
    for (int i = 0; i < MAX_POLYPHONY; i++) {
        _voices[i].active = false;
        _voices[i].frequency = 0.0f;
        _voices[i].phase = 0.0f;
        _voices[i].phase_increment = 0.0f;
        _voices[i].volume = 0.0f;
        _voices[i].decay_rate = 0.0f;
        _voices[i].sustain_samples = 0;
    }
}

void PicoPwmAudio::startTone() {
    if (_is_playing) return;

    // 約45.35μs (22050Hz) 周期でタイマー割り込みを設定
    int64_t interval_us = -(1000000 / AUDIO_SAMPLE_RATE);
    add_repeating_timer_us(interval_us, PicoPwmAudio::timerCallback, this, &_timer);
    _is_playing = true;
}

void PicoPwmAudio::stopTone() {
    if (!_is_playing) return;

    cancel_repeating_timer(&_timer);
    _is_playing = false;

    // 全ボイスをクリア
    clearTone();

    // 出力を無音（センター）に戻す
    pwm_set_chan_level(_slice_num, _channel, PWM_TOP_VAL / 2);
}

uint16_t PicoPwmAudio::tick() {
    float mixed_wave = 0.0f;
    int active_count = 0;

    for (int i = 0; i < MAX_POLYPHONY; i++) {
        Voice &v = _voices[i];
        if (!v.active) continue;

        // 1. 矩形波の計算 (-1.0 ~ 1.0)
        float wave = (v.phase < 0.5f) ? 1.0f : -1.0f;

        // 2. エンベロープ（音量）の適用
        mixed_wave += wave * v.volume;
        active_count++;

        // 3. 位相の更新
        v.phase += v.phase_increment;
        if (v.phase >= 1.0f) {
            v.phase -= 1.0f;
        }

        // 4. 持続時間と減衰（エンベロープ）の制御
        if (v.sustain_samples > 0) {
            // 持続時間が残っている間は音量を1.0に維持
            v.sustain_samples--;
        } else {
            // 持続時間経過後、1秒かけて減衰
            v.volume -= v.decay_rate;
            if (v.volume <= 0.0f) {
                v.volume = 0.0f;
                v.active = false; // 消音完了したらボイスを解放
            }
        }
    }

    // 発音中の音がなければ中心値（無音）を返す
    if (active_count == 0) {
        return PWM_TOP_VAL / 2;
    }

    // 鳴っている音数に合わせて正規化し、-1.0 ~ +1.0 の範囲に収める
    float normalized = mixed_wave / (float)MAX_POLYPHONY;
    
    // PWM出力値 (0 ~ 255) に変換
    uint16_t pwm_val = (uint16_t)((normalized + 1.0f) * 0.5f * PWM_TOP_VAL);
    return pwm_val;
}

// 静的タイマーコールバック関数
bool PicoPwmAudio::timerCallback(struct repeating_timer *t) {
    PicoPwmAudio *instance = static_cast<PicoPwmAudio*>(t->user_data);
    if (instance) {
        uint16_t pwm_val = instance->tick();
        pwm_set_chan_level(instance->_slice_num, instance->_channel, pwm_val);
    }
    return true;
}