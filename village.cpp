#include "village.h"
#include "charKatakana.h"

// main.cppで定義されているグローバルオーディオインスタンスを参照
extern PicoPwmAudio audio;
extern charKatakana kana;

Village::Village() 
    : _buzzerPin(0), _selectPin(0), _leftPin(0), _rightPin(0), _display(nullptr),
      _state(STATE_TITLE), _menuState(MENU_CLOSED), _menuCursor(0),
      _currentViewVillage(0), _tickIntervalMs(300), _timeMultiplier(3), _lastTickTime(0),
      _currentScore(nullptr), _currentScoreLength(0), _bgmIndex(0), _lastBgmTime(0),
      _alertStep(0), _lastAlertTime(0) {
}

Village::~Village() {
}

// --- 変更・追加する箇所 ---

// 現在動作中の Village インスタンスを保持するポインタ
static Village* g_currentVillage = nullptr;

// Core1 から呼び出されるグローバル関数
void updateAudioCore1() {
    if (g_currentVillage != nullptr) {
        g_currentVillage->updateAudio(); // 音声更新のみCore1で実行
    }
}

void Village::play(uint8_t buzzerPin, uint8_t selectPin, uint8_t leftPin, uint8_t rightPin, Adafruit_SSD1306* display) {
    _buzzerPin = buzzerPin;
    _selectPin = selectPin;
    _leftPin   = leftPin;
    _rightPin  = rightPin;
    _display   = display;

    // Core1 が参照できるようにポインタを設定
    g_currentVillage = this;

    _state = STATE_TITLE;
    setBGM(midi_score_title, MIDI_SCORE_LENGTH_title);

    while (true) {
        // 【変更点】 updateAudio(); の呼び出しを削除（Core1におまかせ）

        // 1. 入力処理
        processInput();

        // 2. シミュレーション更新
        if (_state == STATE_PLAYING && _menuState == MENU_CLOSED) {
            unsigned long now = millis();
            if (now - _lastTickTime >= _tickIntervalMs) {
                _lastTickTime = now;
                updateSimulation();
                checkVillageCrisis();
            }
        }

        // 3. 画面描画（重い処理）
        render();

        // // [SELECT + LEFT] 長押しで戻る
        // if (!digitalRead(_selectPin) && !digitalRead(_leftPin)) {
        //     audio.clearTone();
        //     g_currentVillage = nullptr; // ポインタ解除
        //     delay(300);
        //     break;
        // }

        delay(20);
    }
}

// -------------------------------------------------------------
// オーディオ・割り込みSE制御
// -------------------------------------------------------------
void Village::setBGM(const NoteEvent* score, size_t length) {
    _currentScore = score;
    _currentScoreLength = length;
    _bgmIndex = 0;
    _lastBgmTime = millis();
}

void Village::updateAudio() {
    unsigned long currentMillis = millis();

    // BGM進行
    if (_currentScore && _currentScoreLength > 0) {
        uint32_t waitTime = _currentScore[_bgmIndex].delay_ms;
        if (currentMillis - _lastBgmTime >= waitTime) {
            uint16_t freq = _currentScore[_bgmIndex].freq;
            uint32_t sus = _currentScore[_bgmIndex].sustain_ms;
            if (freq > 0) {
                audio.writeTone(freq, sus);
            }
            _lastBgmTime = currentMillis;
            _bgmIndex = (_bgmIndex + 1) % _currentScoreLength;
        }
    }

    // 村のアラート割り込み (1.5kHz 3回鳴動)
    if (_alertStep > 0) {
        if (currentMillis - _lastAlertTime >= 120) {
            _lastAlertTime = currentMillis;
            if (_alertStep % 2 != 0) {
                audio.writeTone(1500, 80);
            }
            _alertStep--;
        }
    }
}

void Village::playButtonSE() {
    audio.writeTone(1000, 50); // 1kHz ボタン音割り込み
}

void Village::triggerAlertSE() {
    _alertStep = 6; // 3回点滅鳴動
    _lastAlertTime = millis();
}

bool Village::isButtonPressed(uint8_t pin) {
    return (digitalRead(pin) == LOW);
}

// -------------------------------------------------------------
// 初期化 & シミュレーション制御
// -------------------------------------------------------------
void Village::initGame() {
    _virus = {2, 2, 3, 1}; // 感染力2, 致死率2, 潜伏3, 後遺症1
    _timeMultiplier = 3;   // 3倍 (300ms)
    _tickIntervalMs = 300;
    _currentViewVillage = 0;
    _gameStartTime = millis();
    if (_isFirstGameStart) {
        _menuState = MENU_MAIN;
        _menuCursor = 0;
        _isFirstGameStart = false; // フラグのリセット
    }
    for (int v = 0; v < NUM_VILLAGES; v++) {
        _villages[v].isOpen = true;
        _villages[v].closedTicks = 0;
        _villages[v].inCrisis = false;

        for (int i = 0; i < PERSONS_PER_VILLAGE; i++) {
            _villages[v].persons[i].x = random(5, 120);
            _villages[v].persons[i].y = random(15, 55);
            _villages[v].persons[i].vx = (random(10, 25) / 10.0f) * (random(2) == 0 ? 1 : -1);
            _villages[v].persons[i].vy = (random(10, 25) / 10.0f) * (random(2) == 0 ? 1 : -1);
            _villages[v].persons[i].state = 0; // 健康
            _villages[v].persons[i].timer = 0;
        }
    }
    _villages[0].persons[0].state = 1; // 村1の最初の1人を感染者に設定

    setBGM(midi_score_playing, MIDI_SCORE_LENGTH_playing);
}

void Village::updateSimulation() {
    int totalInfected = 0;
    for (int v = 0; v < NUM_VILLAGES; v++) {
        if (!_villages[v].isOpen) {
            _villages[v].closedTicks++;
        } else {
            _villages[v].closedTicks = 0;
        }

        for (int i = 0; i < PERSONS_PER_VILLAGE; i++) {
            Person &p = _villages[v].persons[i];
            if (p.state == 4 || p.state == 6) continue; // 死亡
            // 感染者（潜伏State:1、発症State:2）のカウント
            if (p.state == 1 || p.state == 2) {
                totalInfected++;
            }
            p.x += p.vx;
            p.y += p.vy;

            // if (p.x <= 2 || p.x >= 125) p.vx *= -1;
            if (p.x <= 2) {
                int leftNeighbor = (v + NUM_VILLAGES - 1) % NUM_VILLAGES;
                // 自村と移動先の村の両方が OPEN であれば隣の村へ移動
                if (_villages[v].isOpen && _villages[leftNeighbor].isOpen) {
                    _villages[leftNeighbor].persons[i] = p; // 隣の村へデータをコピー
                    _villages[leftNeighbor].persons[i].x = 124; // 反対側の端へ配置
                    p.state = 6; // 移動元の元の枠は移動として無効化（あるいは初期化）
                } else {
                    p.vx *= -1; // CLOSE の場合は跳ね返る
                    p.x = 3;
                }
            }
            // 右端に到達した場合
            else if (p.x >= 125) {
                int rightNeighbor = (v + 1) % NUM_VILLAGES;
                if (_villages[v].isOpen && _villages[rightNeighbor].isOpen) {
                    _villages[rightNeighbor].persons[i] = p;
                    _villages[rightNeighbor].persons[i].x = 3;
                    p.state = 6;
                } else {
                    p.vx *= -1;
                    p.x = 124;
                }
            }
            if (p.y <= 12 || p.y >= 61) p.vy *= -1;

            if (p.state == 1 || p.state == 2) p.timer++;

            // 潜伏 -> 発症
            if (p.state == 1 && p.timer > (30 - _virus.incubation * 4)) {
                p.state = 2;
                p.timer = 0;
            }

            // 発症後の生死判定
            if (p.state == 2 && p.timer > 40) {
                if (random(100) < (_virus.lethality * 15)) {
                    p.state = 4; // 死亡
                } else if (random(100) < (_virus.sequelae * 12)) {
                    p.state = 5; // 後遺症
                    p.vx *= 0.3f;
                    p.vy *= 0.3f;
                } else {
                    p.state = 3; // 回復
                }
            }

            // 接触感染
            if (p.state == 0) {
                for (int j = 0; j < PERSONS_PER_VILLAGE; j++) {
                    if (i == j) continue;
                    Person &other = _villages[v].persons[j];
                    if (other.state == 1 || other.state == 2) {
                        float dx = p.x - other.x;
                        float dy = p.y - other.y;
                        if (sqrt(dx * dx + dy * dy) < (3.0f + _virus.infectivity * 1.2f)) {
                            p.state = 1;
                            p.timer = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
    // if (totalInfected == 0) {
    //     _finalScore = (millis() - _gameStartTime) / 1000; // 生き残った秒数をスコアにする
    //     _state = STATE_GAMEOVER;
    //     setBGM(midi_score_gameover, MIDI_SCORE_LENGTH_gameover);
    // }
}

// -------------------------------------------------------------
// 緊急事態（人口激減・交易不足）チェック & 自動表示切替
// -------------------------------------------------------------
void Village::checkVillageCrisis() {
    for (int v = 0; v < NUM_VILLAGES; v++) {
        int aliveCount = 0;
        for (int i = 0; i < PERSONS_PER_VILLAGE; i++) {
            if (_villages[v].persons[i].state != 4) aliveCount++;
        }

        bool crisisNow = (aliveCount <= 2 && aliveCount > 0) || (_villages[v].closedTicks > 100);

        if (crisisNow && !_villages[v].inCrisis) {
            _villages[v].inCrisis = true;
            _currentViewVillage = v; // 該当の村へ画面を強制切替
            triggerAlertSE();        // アラート鳴動
            break;
        } else if (!crisisNow) {
            _villages[v].inCrisis = false;
        }
    }
}

// -------------------------------------------------------------
// 入力 & メニュー処理
// -------------------------------------------------------------
void Village::processInput() {
    if (_state == STATE_TITLE) {
        if (isButtonPressed(_selectPin)) {
            playButtonSE();
            _state = STATE_PLAYING;
            _isFirstGameStart = true;
            initGame();
            delay(200);
        }
        return;
    }

    if (_state == STATE_GAMEOVER) {
        if (isButtonPressed(_selectPin) || isButtonPressed(_leftPin) || isButtonPressed(_rightPin)) {
            playButtonSE();
            _state = STATE_TITLE;
            setBGM(midi_score_title, MIDI_SCORE_LENGTH_title);
            delay(200);
        }
        return;
    }

    if (_menuState == MENU_CLOSED) {
        if (isButtonPressed(_selectPin)) {
            playButtonSE();
            _menuState = MENU_MAIN;
            _menuCursor = 0;
            while(isButtonPressed(_selectPin)){delay(10);}
        } else if (isButtonPressed(_leftPin)) {
            playButtonSE();
            _currentViewVillage = (_currentViewVillage + 3) % NUM_VILLAGES;
            while(isButtonPressed(_leftPin)){delay(10);}
        } else if (isButtonPressed(_rightPin)) {
            playButtonSE();
            _currentViewVillage = (_currentViewVillage + 1) % NUM_VILLAGES;
            while(isButtonPressed(_rightPin)){delay(10);}

        }
    } else {
        switch (_menuState) {
            case MENU_MAIN:  handleMainMenuInput();  break;
            case MENU_TRADE: handleTradeMenuInput(); break;
            case MENU_VIRUS: handleVirusMenuInput(); break;
            case MENU_TIME:  handleTimeMenuInput();  break;
            default: break;
        }
    }
}

void Village::handleMainMenuInput() {
    if (isButtonPressed(_leftPin)) {
        playButtonSE();
        _menuCursor = (_menuCursor + 3) % 4;
        delay(150);
    } else if (isButtonPressed(_rightPin)) {
        playButtonSE();
        _menuCursor = (_menuCursor + 1) % 4;
        delay(150);
    } else if (isButtonPressed(_selectPin)) {
        playButtonSE();
        delay(150);
        if (_menuCursor == 0) _menuState = MENU_CLOSED;
        else if (_menuCursor == 1) { _menuState = MENU_TRADE; _menuCursor = 0; }
        else if (_menuCursor == 2) { _menuState = MENU_VIRUS; _menuCursor = 0; }
        else if (_menuCursor == 3) { _menuState = MENU_TIME;  _menuCursor = 0; }
    }
}

void Village::handleTradeMenuInput() {
    if (isButtonPressed(_leftPin)) {
        playButtonSE();
        _menuCursor = (_menuCursor + 4) % 5;
        delay(150);
    } else if (isButtonPressed(_rightPin)) {
        playButtonSE();
        _menuCursor = (_menuCursor + 1) % 5;
        delay(150);
    } else if (isButtonPressed(_selectPin)) {
        playButtonSE();
        delay(150);
        if (_menuCursor < 4) {
            _villages[_menuCursor].isOpen = !_villages[_menuCursor].isOpen;
        } else {
            _menuState = MENU_MAIN; // ﾓﾄﾞﾙ
            _menuCursor = 1;
        }
    }
}

void Village::handleVirusMenuInput() {
    if (isButtonPressed(_leftPin)) {
        playButtonSE();
        _menuCursor = (_menuCursor + 4) % 5;
        delay(150);
    } else if (isButtonPressed(_rightPin)) {
        playButtonSE();
        _menuCursor = (_menuCursor + 1) % 5;
        delay(150);
    } else if (isButtonPressed(_selectPin)) {
        playButtonSE();
        delay(150);
        if (_menuCursor == 0) _virus.infectivity = (_virus.infectivity % 5) + 1;
        else if (_menuCursor == 1) _virus.lethality = (_virus.lethality % 5) + 1;
        else if (_menuCursor == 2) _virus.incubation = (_virus.incubation % 5) + 1;
        else if (_menuCursor == 3) _virus.sequelae = (_virus.sequelae % 5) + 1;
        else {
            _menuState = MENU_MAIN; // ﾓﾄﾞﾙ
            _menuCursor = 2;
        }
    }
}

void Village::handleTimeMenuInput() {
    if (isButtonPressed(_leftPin)) {
        playButtonSE();
        _menuCursor = (_menuCursor + 1) % 2;
        delay(150);
    } else if (isButtonPressed(_rightPin)) {
        playButtonSE();
        _menuCursor = (_menuCursor + 1) % 2;
        delay(150);
    } else if (isButtonPressed(_selectPin)) {
        playButtonSE();
        delay(150);
        if (_menuCursor == 0) {
            _timeMultiplier = (_timeMultiplier % 5) + 1;
            _tickIntervalMs = 600 - (_timeMultiplier * 100);
        } else {
            _menuState = MENU_MAIN; // ﾓﾄﾞﾙ
            _menuCursor = 3;
        }
    }
}

// -------------------------------------------------------------
// 描画処理 (writeKatakana を使用)
// -------------------------------------------------------------
void Village::render() {
    if (!_display) return;
    _display->clearDisplay();

    if (_state == STATE_TITLE) {
        // kana.printKatakana(_display, 20, 15, "ｶﾝｾﾝｼｮｳ");
        // kana.printKatakana(_display, 16, 30, "ｼﾐｭﾚｰｼｮﾝ");
        // kana.printKatakana(_display, 10, 50, "SELECTﾃﾞ ｽﾀｰﾄ");
        kana.printKatakana(_display, 20, 15, "Influence");
        kana.printKatakana(_display, 16, 30, "Simulator");
        kana.printKatakana(_display, 10, 50, "PRESS SELECT");
        _display->display();
        return;
    }
    // --- 【追加】 ゲームオーバー画面の描画 ---
    if (_state == STATE_GAMEOVER) {
        kana.printKatakana(_display, 28, 10, "GAME OVER");
        
        char scoreStr[20];
        snprintf(scoreStr, sizeof(scoreStr), "SCORE: %lu sec", _finalScore);
        kana.printKatakana(_display, 10, 30, scoreStr);
        kana.printKatakana(_display, 10, 50, "PRESS BUTTON");
        
        _display->display();
        return;
    }

    // --- 村と住民の表示 ---
    VillageData &vData = _villages[_currentViewVillage];

    // ヘッダー表示
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:", _currentViewVillage + 1);
    kana.printKatakana(_display, 0, 0, buf);
    // kana.printKatakana(_display, 20, 0, vData.isOpen ? "ｶｲﾎｳ" : "ﾍｲｻ");
    kana.printKatakana(_display, 20, 0, vData.isOpen ? "OPEN" : "CLOSE");

    if (vData.inCrisis) {
        // kana.printKatakana(_display, 88, 0, "ﾋﾟﾝﾁ!");
        kana.printKatakana(_display, 88, 0, "WARN!");
    }

    _display->drawFastHLine(0, 10, 128, SSD1306_WHITE);

    // 住民ドット描画
    for (int i = 0; i < PERSONS_PER_VILLAGE; i++) {
        Person &p = vData.persons[i];
        int px = (int)p.x;
        int py = (int)p.y;

        if (p.state == 0) _display->drawPixel(px, py, SSD1306_WHITE);                  // 健康
        else if (p.state == 1) _display->drawRect(px-1, py-1, 3, 3, SSD1306_WHITE);    // 潜伏
        else if (p.state == 2) _display->fillRect(px-1, py-1, 3, 3, SSD1306_WHITE);    // 発症
        else if (p.state == 3) _display->drawCircle(px, py, 2, SSD1306_WHITE);         // 回復
        else if (p.state == 4) { _display->drawLine(px-1, py-1, px+1, py+1, SSD1306_WHITE); } // 死亡
        else if (p.state == 5) _display->drawFastHLine(px-1, py, 3, SSD1306_WHITE);    // 後遺症
    }

    // --- メニュー描画 ---
    if (_menuState != MENU_CLOSED) {
        drawMenu();
    }

    _display->display();
}

void Village::drawMenu() {
    _display->fillRect(10, 4, 108, 56, SSD1306_BLACK);
    _display->drawRect(10, 4, 108, 56, SSD1306_WHITE);

    int y = 8;

    if (_menuState == MENU_MAIN) {
        // const char* items[] = {"ﾄｼﾞﾙ", "ﾄﾚｰﾄﾞ", "ｳｲﾙｽ", "ｼﾞｶﾝ"};
        const char* items[] = {"close", "trade", "virus", "time"};
        for (int i = 0; i < 4; i++) {
            int curY = y + (i * 12);
            kana.printKatakana(_display, 14, curY, (_menuCursor == i) ? ">" : " ");
            kana.printKatakana(_display, 26, curY, items[i]);
        }
    } 
    else if (_menuState == MENU_TRADE) {
        for (int i = 0; i < 4; i++) {
            int curY = y + (i * 10);
            char numStr[4];
            snprintf(numStr, sizeof(numStr), "%d", i + 1);
            kana.printKatakana(_display, 14, curY, (_menuCursor == i) ? ">" : " ");
            kana.printKatakana(_display, 24, curY, numStr);
            // kana.printKatakana(_display, 36, curY, _villages[i].isOpen ? "ｶｲﾎｳ" : "ﾍｲｻ");
            kana.printKatakana(_display, 36, curY, _villages[i].isOpen ? "open" : "close");
        }
        int curY = y + 40;
        kana.printKatakana(_display, 14, curY, (_menuCursor == 4) ? ">" : " ");
        // kana.printKatakana(_display, 26, curY, "ﾓﾄﾞﾙ");
        kana.printKatakana(_display, 26, curY, "return");
    } 
    else if (_menuState == MENU_VIRUS) {
        // const char* labels[] = {"ｶﾝｾﾝﾘｮｸ", "ﾁｼﾘﾂ", "ｾﾝﾌﾟｸ", "ｺｳｲｼｮｳ"};
        const char* labels[] = {"infections", "fatality", "latent", "after"};
        uint8_t vals[] = {_virus.infectivity, _virus.lethality, _virus.incubation, _virus.sequelae};

        for (int i = 0; i < 4; i++) {
            int curY = y + (i * 10);
            char valStr[8];
            snprintf(valStr, sizeof(valStr), ":%d", vals[i]);
            kana.printKatakana(_display, 14, curY, (_menuCursor == i) ? ">" : " ");
            kana.printKatakana(_display, 24, curY, labels[i]);
            kana.printKatakana(_display, 80, curY, valStr);
        }
        int curY = y + 40;
        kana.printKatakana(_display, 14, curY, (_menuCursor == 4) ? ">" : " ");
        // kana.printKatakana(_display, 26, curY, "ﾓﾄﾞﾙ");
        kana.printKatakana(_display, 26, curY, "return");
    } 
    else if (_menuState == MENU_TIME) {
        char timeStr[16];
        // snprintf(timeStr, sizeof(timeStr), "%dﾊﾞｲ", _timeMultiplier);
        snprintf(timeStr, sizeof(timeStr), "x%d", _timeMultiplier);
        
        kana.printKatakana(_display, 14, y + 10, (_menuCursor == 0) ? ">" : " ");
        kana.printKatakana(_display, 26, y + 10, timeStr);

        kana.printKatakana(_display, 14, y + 30, (_menuCursor == 1) ? ">" : " ");
        // kana.printKatakana(_display, 26, y + 30, "ﾓﾄﾞﾙ");
        kana.printKatakana(_display, 26, y + 30, "return");
    }
}