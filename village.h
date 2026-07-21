#ifndef VILLAGE_H
#define VILLAGE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "picoPwmAudio.h"
#include "charKatakana.h" // カタカナ描画ライブラリ
#include "musics.h"



// 外部音源データの宣言
extern const NoteEvent midi_score_title[];
extern const size_t MIDI_SCORE_LENGTH_title;
extern const NoteEvent midi_score_playing[];
extern const size_t MIDI_SCORE_LENGTH_playing;
extern const NoteEvent midi_score_gameover[];
extern const size_t MIDI_SCORE_LENGTH_gameover;

// 定数定義
#define NUM_VILLAGES 4
#define PERSONS_PER_VILLAGE 10

// ゲーム全体の状態
enum GameState {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_GAMEOVER
};

// メニューの状態（階層管理）
enum MenuState {
    MENU_CLOSED,
    MENU_MAIN,
    MENU_TRADE,
    MENU_VIRUS,
    MENU_TIME
};

// ウイルスのパラメータ構造体 (レベル1~5)
struct VirusStatus {
    uint8_t infectivity; // 感染力 (1~5)
    uint8_t lethality;   // 致死率 (1~5)
    uint8_t incubation;  // 潜伏期間 (1~5)
    uint8_t sequelae;    // 後遺症発症率 (1~5)
};

// 住民データ
struct Person {
    float x, y;
    float vx, vy;
    uint8_t state;        // 0:健康, 1:潜伏, 2:発症, 3:回復, 4:死亡, 5:後遺症
    uint16_t timer;
};

// 村データ
struct VillageData {
    bool isOpen;          // 開放(true) / 閉鎖(false)
    uint32_t closedTicks; // 閉鎖継続時間 (交易不足判定用)
    Person persons[PERSONS_PER_VILLAGE];
    bool inCrisis;        // 緊急事態フラグ
};

class Village {
public:
    Village();
    ~Village();

    /**
     * @brief メインエントリーポイント（ブロッキング呼び出し）
     */
    void updateAudio();
    void play(uint8_t buzzerPin, uint8_t selectPin, uint8_t leftPin, uint8_t rightPin, Adafruit_SSD1306* display);

private:
    uint8_t _buzzerPin, _selectPin, _leftPin, _rightPin;
    Adafruit_SSD1306* _display;

    GameState _state;
    MenuState _menuState;
    int _menuCursor;

    // シミュレーションデータ
    VillageData _villages[NUM_VILLAGES];
    VirusStatus _virus;
    uint8_t _currentViewVillage; // 現在画面表示している村 (0~3)
    uint16_t _tickIntervalMs;    // 時間の進み方 (100ms~500ms)
    uint8_t _timeMultiplier;     // 倍速レベル (1~5)
    unsigned long _lastTickTime;

    // --- オーディオ制御 ---
    const NoteEvent* _currentScore;
    size_t _currentScoreLength;
    size_t _bgmIndex;
    unsigned long _lastBgmTime;

    int _alertStep;
    unsigned long _lastAlertTime;

    void setBGM(const NoteEvent* score, size_t length);
    void playButtonSE();   // 1kHz 割り込み音
    void triggerAlertSE(); // 1.5kHz 3回割り込み音

    // --- ロジック・メニュー・描画 ---
    void initGame();
    void processInput();
    void updateSimulation();
    void checkVillageCrisis();
    void render();

    // メニュー制御
    void handleMainMenuInput();
    void handleTradeMenuInput();
    void handleVirusMenuInput();
    void handleTimeMenuInput();

    void drawMenu();
    bool isButtonPressed(uint8_t pin);
    // --- 追加メンバ変数 ---
    unsigned long _gameStartTime; // 生き残った時間（スコア）計算用
    uint32_t _finalScore;        // ゲームオーバー時のスコア保持用
    bool _isFirstGameStart;      // タイトルからゲームに入った直後かの判定用
};
// village.h の末尾などに追加
void updateAudioCore1();
#endif // VILLAGE_H