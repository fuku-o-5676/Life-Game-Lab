#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// たまごっち風育成ゲーム
// 操作: left = アイコン送り / right = アイコン戻し / select_start = 決定
class Tamagotchi {
public:
    // EEPROMの0-1番地はDinoのハイスコアが使用しているため8番地以降を使う
    static const int SAVE_ADDR = 8;
    static const uint8_t SAVE_MAGIC = 0xA7;

    void play(byte buzzer, byte select, byte left, byte right, Adafruit_SSD1306* display); // 戻らない
    static void clearSave(); // 世代記録も含めて全消去。EEPROM.commit()は呼び出し側で行う

private:
    enum Stage : uint8_t { EGG = 0, BABY, CHILD, ADULT, DEAD };
    enum Form : uint8_t { FORM_BAD = 0, FORM_NORMAL, FORM_GOOD, FORM_SECRET };
    enum Mode : uint8_t { NORMAL, FEED_MENU, PLAY_MENU, STATUS };
    enum { ICON_FEED = 0, ICON_LIGHT, ICON_PLAY, ICON_MEDICINE, ICON_CLEAN, ICON_SCOLD, ICON_STATUS, ICON_COUNT };

    static const uint8_t SAVE_VERSION = 2;
    static const int RECORD_ADDR = 24; // 世代・最長記録 (死んでも消えない)
    static const uint8_t RECORD_MAGIC = 0xB3;

    static const uint16_t HatchSec     = 20;   // たまごがかえるまでの秒数
    static const uint16_t ChildAgeSec  = 180;  // ベビー -> キッズ
    static const uint16_t AdultAgeSec  = 600;  // キッズ -> アダルト
    static const uint32_t OldAgeSec    = 3600; // 寿命
    static const uint8_t  DeathMistakes = 10;  // お世話ミスがこの回数で死亡
    static const uint8_t  MaxPoop      = 4;
    static const uint16_t AutoSaveSec  = 300;
    static const uint16_t SleepCycleSec = 900; // 12分活動 + 3分睡眠の繰り返し
    static const uint16_t SleepStartSec = 720;
    static const uint16_t SulkTimeoutSec = 180; // すねたまま放置でお世話ミス
    static const uint8_t  SulkChancePct = 30;   // 1分ごとにすねる確率(%)
    static const uint8_t  EventChancePct = 12;  // 1分ごとのラッキーイベント確率(%)
    static const int16_t  FatWeight = 50;       // これ以上は太りすぎで病気になりやすい

    byte m_buzzerPin;
    byte m_selectPin;
    byte m_leftPin;
    byte m_rightPin;
    Adafruit_SSD1306* m_display;

    // ペットの状態
    Stage m_stage;
    uint8_t m_adultForm;     // Form (キッズ->アダルト進化時に決定)
    uint32_t m_ageSec;
    int16_t m_hunger;        // 満腹度 0-100
    int16_t m_happy;         // ごきげん 0-100
    int16_t m_weight;        // 体重 5-99
    int16_t m_discipline;    // しつけ 0-100
    bool m_sick;
    bool m_sulking;          // すねている
    bool m_lightOff;         // 部屋の電気
    uint8_t m_poop;
    uint8_t m_careMistakes;

    // 世代記録 (死んでも引き継ぐ)
    uint8_t m_generation;
    uint16_t m_bestAgeMin;

    // タイマー類 (秒カウンタ)
    uint32_t m_lastTickMs;
    uint16_t m_hungerAcc;
    uint16_t m_happyAcc;
    uint16_t m_checkAcc;     // 1分ごとの判定用
    uint16_t m_mistakeAcc;   // お世話ミス判定用
    uint16_t m_sulkSec;
    int32_t m_poopCountdown; // 次のうんちまでの秒数 (負なら未予約)
    uint32_t m_lastSaveSec;
    bool m_wasSleeping;

    // UIの状態
    Mode m_mode;
    uint8_t m_icon;
    uint8_t m_feedCursor;
    uint8_t m_playCursor;
    bool m_animFrame;
    bool m_blink;
    uint32_t m_lastAnimMs;
    int16_t m_petX;
    int8_t m_petDir;
    bool m_attention;

    // ボタン (0=select 1=left 2=right)
    bool m_lastLevel[3];
    uint32_t m_lastEdgeMs[3];

    void resetPet();
    bool loadState();
    void saveState();
    void loadRecords();
    void saveRecords();

    void tick(); // 1秒ぶんのゲーム進行
    void die();
    bool isSleeping();

    byte pinOf(uint8_t index);
    bool buttonPressed(uint8_t index);
    void waitAllRelease();
    int8_t waitLeftRight(uint32_t timeoutMs);

    void handleInput();
    void executeIcon();
    void doFeed(bool snack);
    void doPlayGuess();
    void doPlayTiming();
    void doClean();
    void doMedicine();
    void doScold();
    void doEvent(uint8_t kind);
    void refuseAnim();
    void movePet();

    void renderNormal();
    void renderDead();
    void renderStatus();
    void renderMenu(const char* const* items, uint8_t count, uint8_t cursor);
    void drawScene(int8_t shake = 0, int16_t poopOffset = 0);
    void drawIconBar();
    void drawPoops(int16_t offset);
    void drawBar(int16_t x, int16_t y, int16_t value);
    void drawSprite2x(int16_t x, int16_t y, const uint8_t* bmp, uint8_t w, uint8_t h);
    const uint8_t* petBitmap();
    const char* stageName();

    void beep(uint16_t freq, uint16_t ms);
    void melodyHappy();
    void melodyRefuse();
    void melodyAttention();
    void melodyAngry();
    void melodyMorning();
    void melodyEvolve();
    void melodyDeath();
};
