#include "grow_up.h"
#include <EEPROM.h>

// ---- スプライトデータ (drawBitmap形式: 1行=横8bit単位, MSBが左) ----

// たまご (ゆれは描画位置のオフセットで表現)
static const uint8_t SPR_EGG[] = {
    0b00000111, 0b11100000,
    0b00001111, 0b11110000,
    0b00011111, 0b11111000,
    0b00111111, 0b11111100,
    0b00111011, 0b11011100,
    0b01111111, 0b11111110,
    0b01111101, 0b11101110,
    0b01111111, 0b11111110,
    0b01111111, 0b11111110,
    0b01111011, 0b11011110,
    0b01111111, 0b11111110,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b00001111, 0b11110000,
    0b00000111, 0b11100000,
};

// ベビー (はねるアニメは描画位置のオフセットで表現)
static const uint8_t SPR_BABY[] = {
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000111, 0b11100000,
    0b00001111, 0b11110000,
    0b00011111, 0b11111000,
    0b00011011, 0b11011000,
    0b00011111, 0b11111000,
    0b00011110, 0b01111000,
    0b00001111, 0b11110000,
    0b00000111, 0b11100000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

static const uint8_t SPR_CHILD_A[] = {
    0b00000000, 0b00000000,
    0b00001100, 0b00110000,
    0b00001111, 0b11110000,
    0b00011111, 0b11111000,
    0b00111111, 0b11111100,
    0b00110011, 0b11001100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111000, 0b00011100,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b00001111, 0b11110000,
    0b00001100, 0b00110000,
    0b00011000, 0b00011000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

static const uint8_t SPR_CHILD_B[] = {
    0b00000000, 0b00000000,
    0b00001100, 0b00110000,
    0b00001111, 0b11110000,
    0b00011111, 0b11111000,
    0b00111111, 0b11111100,
    0b00110011, 0b11001100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111000, 0b00011100,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b00001111, 0b11110000,
    0b00000110, 0b01100000,
    0b00001100, 0b00110000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// アダルト (よい姿)
static const uint8_t SPR_ADULT_GOOD_A[] = {
    0b00011111, 0b11111000,
    0b00111111, 0b11111100,
    0b01111111, 0b11111110,
    0b01100111, 0b11100110,
    0b01111111, 0b11111110,
    0b01111000, 0b00011110,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b00001111, 0b11110000,
    0b01101111, 0b11110110,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00111000, 0b00011100,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

static const uint8_t SPR_ADULT_GOOD_B[] = {
    0b00011111, 0b11111000,
    0b00111111, 0b11111100,
    0b01111111, 0b11111110,
    0b01100111, 0b11100110,
    0b01111111, 0b11111110,
    0b01111000, 0b00011110,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b01101111, 0b11110110,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00001110, 0b01110000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// アダルト (ふつうの姿: うさぎ耳)
static const uint8_t SPR_ADULT_NORMAL_A[] = {
    0b00011000, 0b00011000,
    0b00011000, 0b00011000,
    0b00111111, 0b11111100,
    0b01111111, 0b11111110,
    0b01100111, 0b11100110,
    0b01111111, 0b11111110,
    0b01111100, 0b00111110,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b01111111, 0b11111110,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00111000, 0b00011100,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

static const uint8_t SPR_ADULT_NORMAL_B[] = {
    0b00110000, 0b00001100,
    0b00011000, 0b00011000,
    0b00111111, 0b11111100,
    0b01111111, 0b11111110,
    0b01100111, 0b11100110,
    0b01111111, 0b11111110,
    0b01111100, 0b00111110,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b01111111, 0b11111110,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00001110, 0b01110000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// アダルト (わるい姿)
static const uint8_t SPR_ADULT_BAD_A[] = {
    0b00000000, 0b00000000,
    0b00100100, 0b00100100,
    0b00111111, 0b11111100,
    0b01100110, 0b01100110,
    0b01111111, 0b11111110,
    0b01100111, 0b11100110,
    0b01111111, 0b11111110,
    0b01111001, 0b10011110,
    0b01111111, 0b11111110,
    0b01111111, 0b11111110,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00011000, 0b00011000,
    0b00110000, 0b00001100,
    0b00000000, 0b00000000,
};

static const uint8_t SPR_ADULT_BAD_B[] = {
    0b00000000, 0b00000000,
    0b00100100, 0b00100100,
    0b00111111, 0b11111100,
    0b01100110, 0b01100110,
    0b01111111, 0b11111110,
    0b01100111, 0b11100110,
    0b01111111, 0b11111110,
    0b01111001, 0b10011110,
    0b01111111, 0b11111110,
    0b01111111, 0b11111110,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00001100, 0b00110000,
    0b00000110, 0b01100000,
    0b00000000, 0b00000000,
};

// アダルト (ひみつの姿: ほし)
static const uint8_t SPR_ADULT_SECRET_A[] = {
    0b00000001, 0b10000000,
    0b00000001, 0b10000000,
    0b00000011, 0b11000000,
    0b00000011, 0b11000000,
    0b00000111, 0b11100000,
    0b01111111, 0b11111110,
    0b11111111, 0b11111111,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b00011011, 0b11011000,
    0b00011111, 0b11111000,
    0b00011110, 0b01111000,
    0b00111111, 0b11111100,
    0b00111100, 0b00111100,
    0b01110000, 0b00001110,
    0b01100000, 0b00000110,
};

static const uint8_t SPR_ADULT_SECRET_B[] = {
    0b00000001, 0b10000000,
    0b00000001, 0b10000000,
    0b00000011, 0b11000000,
    0b00000011, 0b11000000,
    0b00000111, 0b11100000,
    0b01111111, 0b11111110,
    0b11111111, 0b11111111,
    0b00111111, 0b11111100,
    0b00011111, 0b11111000,
    0b00011011, 0b11011000,
    0b00011111, 0b11111000,
    0b00011110, 0b01111000,
    0b00111111, 0b11111100,
    0b00111100, 0b00111100,
    0b00011000, 0b00011000,
    0b00111000, 0b00011100,
};

static const uint8_t SPR_TOMB[] = {
    0b00000111, 0b11100000,
    0b00001111, 0b11110000,
    0b00011111, 0b11111000,
    0b00011110, 0b01111000,
    0b00011000, 0b00011000,
    0b00011110, 0b01111000,
    0b00011110, 0b01111000,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00011111, 0b11111000,
    0b00111111, 0b11111100,
    0b01111111, 0b11111110,
    0b11111111, 0b11111111,
};

static const uint8_t SPR_SKULL[] = {
    0b00111100,
    0b01111110,
    0b11011011,
    0b11111111,
    0b01011010,
    0b00111100,
    0b00100100,
    0b00111100,
};

static const uint8_t SPR_POOP[] = {
    0b00000000,
    0b00010000,
    0b00111000,
    0b00011100,
    0b00111110,
    0b01111100,
    0b11111110,
    0b01111100,
};

static const uint8_t SPR_MOON[] = {
    0b00111000,
    0b01110000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b01110000,
    0b00111000,
};

static const uint8_t SPR_STAR[] = {
    0b00010000,
    0b01010100,
    0b00111000,
    0b11111110,
    0b00111000,
    0b01010100,
    0b00010000,
    0b00000000,
};

static const uint8_t SPR_GIFT[] = {
    0b01100110,
    0b00011000,
    0b11111111,
    0b10011001,
    0b10011001,
    0b10011001,
    0b10011001,
    0b11111111,
};

static const uint8_t SPR_HEART[] = {
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000,
};

// アイコン (ごはん・でんき・あそぶ・くすり・そうじ・しかる・ステータス)
static const uint8_t ICON_BMP_FEED[] = {
    0b00011000,
    0b00111100,
    0b00111100,
    0b01111110,
    0b01111110,
    0b11111111,
    0b11111111,
    0b01111110,
};

static const uint8_t ICON_BMP_LIGHT[] = {
    0b00111100,
    0b01111110,
    0b01111110,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00111100,
    0b00011000,
};

static const uint8_t ICON_BMP_PLAY[] = {
    0b00111100,
    0b01111110,
    0b11111111,
    0b11100111,
    0b11100111,
    0b11111111,
    0b01111110,
    0b00111100,
};

static const uint8_t ICON_BMP_MED[] = {
    0b00000000,
    0b00011000,
    0b00011000,
    0b01111110,
    0b01111110,
    0b00011000,
    0b00011000,
    0b00000000,
};

static const uint8_t ICON_BMP_CLEAN[] = {
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b01110000,
    0b11111000,
    0b11111000,
    0b01110000,
};

static const uint8_t ICON_BMP_SCOLD[] = {
    0b00000010,
    0b00001110,
    0b00111110,
    0b11111110,
    0b11111110,
    0b00111110,
    0b00001110,
    0b00000010,
};

static const uint8_t ICON_BMP_STATUS[] = {
    0b00111100,
    0b01000010,
    0b10011001,
    0b10100101,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100,
};

static const uint8_t* const ICON_BMPS[] = {
    ICON_BMP_FEED, ICON_BMP_LIGHT, ICON_BMP_PLAY, ICON_BMP_MED,
    ICON_BMP_CLEAN, ICON_BMP_SCOLD, ICON_BMP_STATUS
};

// うんちの表示位置 (2倍描画なので16x16)
static const int16_t POOP_X[] = { 96, 112, 96, 112 };
static const int16_t POOP_Y[] = { 46, 46, 30, 30 };

static int16_t clampStat(int16_t v)
{
    if (v < 0) return 0;
    if (v > 100) return 100;
    return v;
}

static int16_t clampWeight(int16_t v)
{
    if (v < 5) return 5;
    if (v > 99) return 99;
    return v;
}

// ---- 本体 ----

void grow_up::play(byte buzzer, byte select, byte left, byte right, Adafruit_SSD1306* display)
{
    m_buzzerPin = buzzer;
    m_selectPin = select;
    m_leftPin = left;
    m_rightPin = right;
    m_display = display;

    randomSeed(micros()); // メニュー操作のタイミングで毎回変わる
    m_display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);

    loadRecords();
    if (!loadState())
        resetPet();
    m_wasSleeping = isSleeping(); // 起動直後に「朝」の演出が出ないように

    m_mode = NORMAL;
    m_icon = ICON_FEED;
    m_feedCursor = 0;
    m_playCursor = 0;
    m_animFrame = false;
    m_blink = false;
    m_attention = false;
    for (uint8_t i = 0; i < 3; i++) {
        m_lastLevel[i] = true;
        m_lastEdgeMs[i] = 0;
    }
    waitAllRelease();
    m_lastTickMs = millis();
    m_lastAnimMs = millis();

    for (;;) {
        // 経過した秒数ぶんゲームを進める (演出などで遅れてもここで追いつく)
        while (millis() - m_lastTickMs >= 1000) {
            m_lastTickMs += 1000;
            if (m_stage != DEAD)
                tick();
        }
        if (millis() - m_lastAnimMs >= 500) {
            m_lastAnimMs = millis();
            m_animFrame = !m_animFrame;
            m_blink = !m_blink;
            movePet();
        }
        handleInput();
        if (m_stage == DEAD) {
            renderDead();
        } else {
            static const char* feedItems[] = { "MEAL", "SNACK", "BACK" };
            static const char* playItems[] = { "GUESS", "STOP!", "BACK" };
            switch (m_mode) {
            case NORMAL:    renderNormal();                        break;
            case FEED_MENU: renderMenu(feedItems, 3, m_feedCursor); break;
            case PLAY_MENU: renderMenu(playItems, 3, m_playCursor); break;
            case STATUS:    renderStatus();                        break;
            }
        }
        delay(30);
    }
}

void grow_up::resetPet()
{
    m_stage = EGG;
    m_adultForm = FORM_NORMAL;
    m_ageSec = 0;
    m_hunger = 100;
    m_happy = 100;
    m_weight = 10;
    m_discipline = 0;
    m_sick = false;
    m_sulking = false;
    m_lightOff = false;
    m_poop = 0;
    m_careMistakes = 0;
    m_hungerAcc = 0;
    m_happyAcc = 0;
    m_checkAcc = 0;
    m_mistakeAcc = 0;
    m_sulkSec = 0;
    m_poopCountdown = -1;
    m_lastSaveSec = 0;
    m_wasSleeping = false;
    m_attention = false;
    m_petX = 24;
    m_petDir = 1;
}

bool grow_up::isSleeping()
{
    if (m_stage == EGG || m_stage == DEAD)
        return false;
    return (m_ageSec % SleepCycleSec) >= SleepStartSec;
}

// ---- 1秒ごとのゲーム進行 ----

void grow_up::tick()
{
    m_ageSec++;

    if (m_stage == EGG) {
        if (m_ageSec >= HatchSec) {
            m_stage = BABY;
            melodyEvolve();
            saveState();
        }
        return;
    }

    bool sleeping = isSleeping();

    // 朝になったら電気を自動でつけて起きる
    if (m_wasSleeping && !sleeping) {
        m_lightOff = false;
        melodyMorning();
    }
    m_wasSleeping = sleeping;
    if (sleeping)
        m_sulking = false; // 寝たらきげんは直る

    // 満腹度・ごきげんの減少 (病気で倍速・睡眠中は半分)
    uint16_t hungerInterval = 20;
    uint16_t happyInterval = 30;
    if (m_sick) {
        hungerInterval /= 2;
        happyInterval /= 2;
    }
    if (sleeping) {
        hungerInterval *= 2;
        happyInterval *= 2;
    }
    if (++m_hungerAcc >= hungerInterval) {
        m_hungerAcc = 0;
        if (m_hunger > 0) m_hunger--;
    }
    if (++m_happyAcc >= happyInterval) {
        m_happyAcc = 0;
        if (m_happy > 0) m_happy--;
    }

    // うんち (寝ている間は止まる)
    if (!sleeping) {
        if (m_poopCountdown > 0) {
            m_poopCountdown--;
            if (m_poopCountdown == 0) {
                if (m_poop < MaxPoop) m_poop++;
                m_poopCountdown = -1;
            }
        }
        if (m_poopCountdown < 0)
            m_poopCountdown = 240 + random(120);
    }

    // すねたまま放置するとお世話ミス
    if (m_sulking) {
        m_sulkSec++;
        if (m_sulkSec >= SulkTimeoutSec) {
            m_sulking = false;
            if (m_careMistakes < 255) m_careMistakes++;
        }
    }

    // 1分ごとの判定
    if (++m_checkAcc >= 60) {
        m_checkAcc = 0;
        if (!sleeping) {
            // 病気 (うんち放置・空腹・不機嫌・太りすぎでかかりやすい)
            if (!m_sick) {
                int16_t chance = 2 + m_poop * 8;
                if (m_hunger == 0) chance += 15;
                if (m_happy == 0) chance += 10;
                if (m_weight >= FatWeight) chance += 15;
                if (chance > 70) chance = 70;
                if (random(100) < chance)
                    m_sick = true;
            }
            // すねる (キッズ以降。しかってしつけるチャンス)
            if (!m_sick && !m_sulking && m_stage >= CHILD && random(100) < SulkChancePct) {
                m_sulking = true;
                m_sulkSec = 0;
                melodyAngry();
            }
            // ラッキーイベント
            else if (!m_sick && !m_sulking && !m_lightOff && m_mode == NORMAL &&
                     random(100) < EventChancePct) {
                doEvent(random(3));
            }
        }
    }

    // お世話ミス (空腹0・ごきげん0・病気・寝ているのに電気つけっぱなしで60秒ごとに1回)
    if (m_hunger == 0 || m_happy == 0 || m_sick || (sleeping && !m_lightOff)) {
        if (++m_mistakeAcc >= 60) {
            m_mistakeAcc = 0;
            if (m_careMistakes < 255) m_careMistakes++;
        }
    } else {
        m_mistakeAcc = 0;
    }

    // 進化
    if (m_stage == BABY && m_ageSec >= ChildAgeSec) {
        m_stage = CHILD;
        melodyEvolve();
        saveState();
    } else if (m_stage == CHILD && m_ageSec >= AdultAgeSec) {
        m_stage = ADULT;
        // お世話ミス・しつけ・体重で姿が決まる
        if (m_careMistakes == 0 && m_discipline >= 50 && m_weight <= 30)
            m_adultForm = FORM_SECRET;
        else if (m_careMistakes <= 3 && m_discipline >= 25)
            m_adultForm = FORM_GOOD;
        else if (m_careMistakes <= 6)
            m_adultForm = FORM_NORMAL;
        else
            m_adultForm = FORM_BAD;
        melodyEvolve();
        saveState();
    }

    // 死亡 (お世話ミスが多すぎる or 寿命)
    if (m_careMistakes >= DeathMistakes || (m_stage == ADULT && m_ageSec >= OldAgeSec)) {
        die();
        return;
    }

    // お知らせ (状態が悪くなった瞬間に鳴らす)
    bool attention = (m_hunger <= 20) || (m_happy <= 20) || m_sick || (m_poop >= 3) ||
                     m_sulking || (sleeping && !m_lightOff);
    if (attention && !m_attention)
        melodyAttention();
    m_attention = attention;

    if (m_ageSec - m_lastSaveSec >= AutoSaveSec)
        saveState();
}

void grow_up::die()
{
    m_stage = DEAD;
    m_mode = NORMAL;
    melodyDeath();
    uint32_t ageMin = m_ageSec / 60;
    if (ageMin > 65535) ageMin = 65535;
    if (ageMin > m_bestAgeMin)
        m_bestAgeMin = ageMin;
    EEPROM.write(SAVE_ADDR, 0); // ペットのセーブだけ消す (記録は残す)
    saveRecords();              // ここでcommitされる
}

// ---- セーブ・ロード ----
// ペット: [8]magic [9]version [10]stage [11]form [12]hunger [13]happy [14]flags
//         [15]poop [16]mistakes [17-18]age(分) [19]weight [20]discipline [21]checksum
// 記録:   [24]magic [25]generation [26-27]bestAge(分) [28]checksum

void grow_up::saveState()
{
    uint32_t ageMin = m_ageSec / 60;
    if (ageMin > 65535) ageMin = 65535;
    uint8_t flags = (m_sick ? 1 : 0) | (m_sulking ? 2 : 0) | (m_lightOff ? 4 : 0);
    uint8_t d[11];
    d[0] = m_stage;
    d[1] = m_adultForm;
    d[2] = (uint8_t)m_hunger;
    d[3] = (uint8_t)m_happy;
    d[4] = flags;
    d[5] = m_poop;
    d[6] = m_careMistakes;
    d[7] = (uint8_t)(ageMin >> 8);
    d[8] = (uint8_t)(ageMin & 0xff);
    d[9] = (uint8_t)m_weight;
    d[10] = (uint8_t)m_discipline;

    uint8_t sum = 0;
    EEPROM.write(SAVE_ADDR, SAVE_MAGIC);
    EEPROM.write(SAVE_ADDR + 1, SAVE_VERSION);
    for (uint8_t i = 0; i < 11; i++) {
        EEPROM.write(SAVE_ADDR + 2 + i, d[i]);
        sum ^= d[i];
    }
    EEPROM.write(SAVE_ADDR + 13, sum);
    EEPROM.commit();
    m_lastSaveSec = m_ageSec;
}

bool grow_up::loadState()
{
    if (EEPROM.read(SAVE_ADDR) != SAVE_MAGIC)
        return false;
    if (EEPROM.read(SAVE_ADDR + 1) != SAVE_VERSION)
        return false;

    uint8_t d[11];
    uint8_t sum = 0;
    for (uint8_t i = 0; i < 11; i++) {
        d[i] = EEPROM.read(SAVE_ADDR + 2 + i);
        sum ^= d[i];
    }
    if (EEPROM.read(SAVE_ADDR + 13) != sum)
        return false;
    if (d[0] < BABY || d[0] > ADULT) // たまごと死亡状態は保存されない
        return false;

    resetPet();
    m_stage = (Stage)d[0];
    m_adultForm = (d[1] > FORM_SECRET) ? FORM_NORMAL : d[1];
    m_hunger = clampStat(d[2]);
    m_happy = clampStat(d[3]);
    m_sick = (d[4] & 1) != 0;
    m_sulking = (d[4] & 2) != 0;
    m_lightOff = (d[4] & 4) != 0;
    m_poop = (d[5] > MaxPoop) ? MaxPoop : d[5];
    m_careMistakes = d[6];
    m_ageSec = ((uint32_t)d[7] << 8 | d[8]) * 60UL;
    m_weight = clampWeight(d[9]);
    m_discipline = clampStat(d[10]);
    m_lastSaveSec = m_ageSec;
    return true;
}

void grow_up::loadRecords()
{
    m_generation = 1;
    m_bestAgeMin = 0;
    if (EEPROM.read(RECORD_ADDR) != RECORD_MAGIC)
        return;
    uint8_t gen = EEPROM.read(RECORD_ADDR + 1);
    uint8_t hi = EEPROM.read(RECORD_ADDR + 2);
    uint8_t lo = EEPROM.read(RECORD_ADDR + 3);
    if (EEPROM.read(RECORD_ADDR + 4) != (uint8_t)(gen ^ hi ^ lo))
        return;
    m_generation = (gen == 0) ? 1 : gen;
    m_bestAgeMin = ((uint16_t)hi << 8) | lo;
}

void grow_up::saveRecords()
{
    uint8_t hi = (uint8_t)(m_bestAgeMin >> 8);
    uint8_t lo = (uint8_t)(m_bestAgeMin & 0xff);
    EEPROM.write(RECORD_ADDR, RECORD_MAGIC);
    EEPROM.write(RECORD_ADDR + 1, m_generation);
    EEPROM.write(RECORD_ADDR + 2, hi);
    EEPROM.write(RECORD_ADDR + 3, lo);
    EEPROM.write(RECORD_ADDR + 4, (uint8_t)(m_generation ^ hi ^ lo));
    EEPROM.commit();
}

void grow_up::clearSave()
{
    EEPROM.write(SAVE_ADDR, 0);
    EEPROM.write(RECORD_ADDR, 0);
}

// ---- ボタン ----

byte grow_up::pinOf(uint8_t index)
{
    switch (index) {
    case 0:  return m_selectPin;
    case 1:  return m_leftPin;
    default: return m_rightPin;
    }
}

bool grow_up::buttonPressed(uint8_t index)
{
    bool level = digitalRead(pinOf(index)); // HIGH=離している
    if (level != m_lastLevel[index] && millis() - m_lastEdgeMs[index] > 30) {
        m_lastEdgeMs[index] = millis();
        m_lastLevel[index] = level;
        if (!level)
            return true; // 押した瞬間
    }
    return false;
}

void grow_up::waitAllRelease()
{
    while (!digitalRead(m_selectPin) || !digitalRead(m_leftPin) || !digitalRead(m_rightPin)) {
        delay(10);
    }
    for (uint8_t i = 0; i < 3; i++)
        m_lastLevel[i] = true;
}

int8_t grow_up::waitLeftRight(uint32_t timeoutMs)
{
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (buttonPressed(1)) return 1;
        if (buttonPressed(2)) return 2;
        delay(5);
    }
    return -1;
}

// ---- 入力処理 ----

void grow_up::handleInput()
{
    bool sel = buttonPressed(0);
    bool left = buttonPressed(1);
    bool right = buttonPressed(2);
    if (!(sel || left || right))
        return;

    if (m_stage == DEAD) {
        if (sel) {
            beep(2000, 20);
            m_generation++;
            saveRecords();
            resetPet();
            m_mode = NORMAL;
        }
        return;
    }

    switch (m_mode) {
    case NORMAL:
        if (left) {
            beep(2000, 20);
            m_icon = (m_icon + 1) % ICON_COUNT;
        } else if (right) {
            beep(2000, 20);
            m_icon = (m_icon + ICON_COUNT - 1) % ICON_COUNT;
        } else if (sel) {
            executeIcon();
        }
        break;

    case FEED_MENU:
        if (left) {
            beep(2000, 20);
            m_feedCursor = (m_feedCursor + 1) % 3;
        } else if (right) {
            beep(2000, 20);
            m_feedCursor = (m_feedCursor + 2) % 3;
        } else if (sel) {
            beep(2000, 20);
            if (m_feedCursor == 0)
                doFeed(false);
            else if (m_feedCursor == 1)
                doFeed(true);
            m_mode = NORMAL;
        }
        break;

    case PLAY_MENU:
        if (left) {
            beep(2000, 20);
            m_playCursor = (m_playCursor + 1) % 3;
        } else if (right) {
            beep(2000, 20);
            m_playCursor = (m_playCursor + 2) % 3;
        } else if (sel) {
            if (m_playCursor == 0)
                doPlayGuess();
            else if (m_playCursor == 1)
                doPlayTiming();
            else
                beep(2000, 20);
            m_mode = NORMAL;
        }
        break;

    case STATUS:
        beep(2000, 20);
        m_mode = NORMAL;
        break;
    }
}

void grow_up::executeIcon()
{
    bool sleeping = isSleeping();
    switch (m_icon) {
    case ICON_FEED:
        if (m_stage == EGG || sleeping || m_sulking || m_lightOff) { melodyRefuse(); return; }
        beep(2000, 20);
        m_feedCursor = 0;
        m_mode = FEED_MENU;
        break;
    case ICON_LIGHT:
        beep(2000, 20);
        m_lightOff = !m_lightOff;
        break;
    case ICON_PLAY:
        if (m_stage == EGG || m_sick || sleeping || m_sulking || m_lightOff) { melodyRefuse(); return; }
        beep(2000, 20);
        m_playCursor = 0;
        m_mode = PLAY_MENU;
        break;
    case ICON_MEDICINE:
        if (!m_sick) { melodyRefuse(); return; }
        doMedicine();
        break;
    case ICON_CLEAN:
        if (m_poop == 0) { melodyRefuse(); return; }
        doClean();
        break;
    case ICON_SCOLD:
        if (m_stage == EGG || sleeping) { melodyRefuse(); return; }
        doScold();
        break;
    case ICON_STATUS:
        beep(2000, 20);
        m_mode = STATUS;
        break;
    }
}

// ---- アクション ----

void grow_up::doFeed(bool snack)
{
    if (!snack && m_hunger >= 100) {
        refuseAnim(); // おなかいっぱい
        return;
    }

    m_petX = 48; // 画面中央で食べる
    for (int8_t r = 6; r >= 2; r -= 2) {
        drawScene();
        m_display->fillCircle(40, 50, r, SSD1306_WHITE);
        m_display->display();
        beep(600, 30);
        delay(300);
    }
    drawScene();
    m_display->display();

    if (snack) {
        m_happy = clampStat(m_happy + 15);
        m_hunger = clampStat(m_hunger + 10);
        m_weight = clampWeight(m_weight + 3);
    } else {
        m_hunger = clampStat(m_hunger + 35);
        m_weight = clampWeight(m_weight + 2);
    }
    // 食後しばらくするとうんちをする
    if (m_poopCountdown < 0 || m_poopCountdown > 120)
        m_poopCountdown = 60 + random(60);
    melodyHappy();
}

void grow_up::doPlayGuess()
{
    // 左右当てゲーム: ペットがどちらを向くか当てる。5回中3回で勝ち
    beep(2000, 20);
    waitAllRelease();
    uint8_t wins = 0;
    for (uint8_t round = 0; round < 5; round++) {
        m_display->clearDisplay();
        drawSprite2x(48, 16, petBitmap(), 16, 16);
        m_display->setTextSize(1);
        m_display->setCursor(0, 0);
        m_display->print(round + 1);
        m_display->print("/5");
        m_display->setCursor(28, 54);
        m_display->print("LEFT or RIGHT?");
        m_display->display();

        int8_t press = waitLeftRight(10000);
        if (press < 0)
            break; // 放置されたら中断

        uint8_t answer = random(2); // 0=左 1=右
        bool correct = (press == 1 && answer == 0) || (press == 2 && answer == 1);

        m_display->clearDisplay();
        drawSprite2x(48, 16, petBitmap(), 16, 16);
        if (answer == 0)
            m_display->fillTriangle(30, 24, 30, 40, 12, 32, SSD1306_WHITE);
        else
            m_display->fillTriangle(98, 24, 98, 40, 116, 32, SSD1306_WHITE);
        m_display->setTextSize(2);
        m_display->setCursor(58, 0);
        m_display->print(correct ? "O" : "X");
        m_display->display();
        if (correct) {
            wins++;
            beep(2200, 60);
        } else {
            beep(300, 120);
        }
        delay(700);
    }

    m_display->clearDisplay();
    m_display->setTextSize(2);
    m_display->setCursor(16, 24);
    m_display->print("WIN ");
    m_display->print(wins);
    m_display->print("/5");
    m_display->display();
    if (wins >= 3) {
        m_happy = clampStat(m_happy + 20);
        melodyHappy();
    } else {
        m_happy = clampStat(m_happy + 5);
        beep(600, 80);
    }
    m_hunger = clampStat(m_hunger - 4); // 運動でおなかがすく
    m_weight = clampWeight(m_weight - 2);
    delay(800);
    waitAllRelease();
}

void grow_up::doPlayTiming()
{
    // タイミングゲーム: 動くマーカーをゾーン内で止める。3回中2回で勝ち
    beep(2000, 20);
    waitAllRelease();
    uint8_t hits = 0;
    for (uint8_t round = 0; round < 3; round++) {
        int16_t zoneX = 30 + random(60);
        int16_t x = 12;
        int8_t dir = 1;
        uint32_t start = millis();
        bool pressed = false;

        while (millis() - start < 10000) {
            if (buttonPressed(0)) {
                pressed = true;
                break;
            }
            x += dir * 3;
            if (x <= 12) { x = 12; dir = 1; }
            if (x >= 114) { x = 114; dir = -1; }

            m_display->clearDisplay();
            m_display->setTextSize(1);
            m_display->setCursor(0, 0);
            m_display->print(round + 1);
            m_display->print("/3  STOP!");
            m_display->drawRect(10, 26, 108, 12, SSD1306_WHITE);
            m_display->fillRect(zoneX, 28, 18, 8, SSD1306_WHITE);
            m_display->drawFastVLine(x, 22, 20, SSD1306_WHITE);
            m_display->drawBitmap(56, 46, petBitmap(), 16, 16, SSD1306_WHITE);
            m_display->display();
            delay(15);
        }

        bool hit = pressed && (x >= zoneX) && (x < zoneX + 18);
        m_display->setTextSize(2);
        m_display->setCursor(58, 46);
        m_display->print(hit ? "O" : "X");
        m_display->display();
        if (hit) {
            hits++;
            beep(2200, 60);
        } else {
            beep(300, 120);
        }
        delay(600);
        if (!pressed)
            break; // 放置されたら中断
    }

    m_display->clearDisplay();
    m_display->setTextSize(2);
    m_display->setCursor(16, 24);
    m_display->print("HIT ");
    m_display->print(hits);
    m_display->print("/3");
    m_display->display();
    if (hits >= 2) {
        m_happy = clampStat(m_happy + 20);
        melodyHappy();
    } else {
        m_happy = clampStat(m_happy + 5);
        beep(600, 80);
    }
    m_hunger = clampStat(m_hunger - 4);
    m_weight = clampWeight(m_weight - 2);
    delay(800);
    waitAllRelease();
}

void grow_up::doClean()
{
    beep(2000, 20);
    for (int16_t off = 0; off <= 40; off += 8) {
        drawScene(0, off); // うんちを右へ掃き出す
        m_display->display();
        delay(80);
    }
    m_poop = 0;
    m_happy = clampStat(m_happy + 5);
    melodyHappy();
}

void grow_up::doMedicine()
{
    beep(2000, 20);
    for (uint8_t i = 0; i < 2; i++) {
        m_display->invertDisplay(true);
        delay(120);
        m_display->invertDisplay(false);
        delay(120);
    }
    if (random(100) < 60) { // ときどき2回目のくすりが必要
        m_sick = false;
        melodyHappy();
    } else {
        melodyRefuse();
    }
}

void grow_up::doScold()
{
    if (m_sulking) {
        // すねているときにしかると、しつけが身につく
        beep(400, 80);
        delay(40);
        beep(300, 120);
        m_sulking = false;
        m_discipline = clampStat(m_discipline + 25);
        drawScene();
        m_display->display();
        delay(300);
        melodyHappy(); // 仲直り
    } else {
        // 何もしていないのにしかられて悲しむ
        melodyRefuse();
        m_happy = clampStat(m_happy - 5);
    }
}

void grow_up::doEvent(uint8_t kind)
{
    switch (kind) {
    case 0: // 流れ星
        for (int16_t x = 120; x > -10; x -= 10) {
            drawScene();
            m_display->drawBitmap(x, 4 + (120 - x) / 8, SPR_STAR, 8, 8, SSD1306_WHITE);
            m_display->display();
            delay(50);
        }
        m_happy = clampStat(m_happy + 10);
        melodyHappy();
        break;

    case 1: // ともだちが遊びにきた
        for (int16_t x = 112; x > m_petX + 34; x -= 6) {
            drawScene();
            m_display->drawBitmap(x, 46, SPR_BABY, 16, 16, SSD1306_WHITE);
            m_display->display();
            delay(70);
        }
        drawScene();
        m_display->drawBitmap(m_petX + 34, 46, SPR_BABY, 16, 16, SSD1306_WHITE);
        m_display->drawBitmap(m_petX + 20, 18, SPR_HEART, 8, 8, SSD1306_WHITE);
        m_display->display();
        melodyHappy();
        m_happy = clampStat(m_happy + 10);
        delay(600);
        break;

    case 2: // プレゼントが届いた
        for (int16_t y = 0; y <= 40; y += 8) {
            drawScene();
            m_display->drawBitmap(56, y, SPR_GIFT, 8, 8, SSD1306_WHITE);
            m_display->display();
            delay(80);
        }
        m_display->invertDisplay(true);
        delay(100);
        m_display->invertDisplay(false);
        m_happy = clampStat(m_happy + 5);
        m_hunger = clampStat(m_hunger + 5);
        melodyHappy();
        break;
    }
}

void grow_up::refuseAnim()
{
    for (uint8_t i = 0; i < 3; i++) {
        drawScene(-3);
        m_display->display();
        beep(250, 40);
        delay(90);
        drawScene(3);
        m_display->display();
        delay(90);
    }
    drawScene();
    m_display->display();
}

void grow_up::movePet()
{
    if (m_stage != BABY && m_stage != CHILD && m_stage != ADULT)
        return;
    if (m_sick || m_sulking || isSleeping())
        return; // 具合が悪い・すねている・寝ているときは動かない
    if (random(8) == 0)
        m_petDir = -m_petDir;
    m_petX += m_petDir * 4;
    if (m_petX < 4) {
        m_petX = 4;
        m_petDir = 1;
    }
    if (m_petX > 60) {
        m_petX = 60;
        m_petDir = -1;
    }
}

// ---- 描画 ----

void grow_up::drawSprite2x(int16_t x, int16_t y, const uint8_t* bmp, uint8_t w, uint8_t h)
{
    uint8_t bytesPerRow = (w + 7) / 8;
    for (uint8_t row = 0; row < h; row++) {
        for (uint8_t col = 0; col < w; col++) {
            if (bmp[row * bytesPerRow + col / 8] & (0x80 >> (col % 8)))
                m_display->fillRect(x + col * 2, y + row * 2, 2, 2, SSD1306_WHITE);
        }
    }
}

const uint8_t* grow_up::petBitmap()
{
    bool frameB = m_animFrame && !m_sick && !m_sulking && !isSleeping();
    switch (m_stage) {
    case BABY:
        return SPR_BABY;
    case CHILD:
        return frameB ? SPR_CHILD_B : SPR_CHILD_A;
    case ADULT:
        switch (m_adultForm) {
        case FORM_SECRET: return frameB ? SPR_ADULT_SECRET_B : SPR_ADULT_SECRET_A;
        case FORM_GOOD:   return frameB ? SPR_ADULT_GOOD_B : SPR_ADULT_GOOD_A;
        case FORM_NORMAL: return frameB ? SPR_ADULT_NORMAL_B : SPR_ADULT_NORMAL_A;
        default:          return frameB ? SPR_ADULT_BAD_B : SPR_ADULT_BAD_A;
        }
    default:
        return SPR_EGG;
    }
}

const char* grow_up::stageName()
{
    switch (m_stage) {
    case EGG:   return "EGG";
    case BABY:  return "BABY";
    case CHILD: return "CHILD";
    case ADULT: return "ADULT";
    default:    return "-";
    }
}

void grow_up::drawIconBar()
{
    for (uint8_t i = 0; i < ICON_COUNT; i++) {
        int16_t x = 3 + i * 13;
        m_display->drawBitmap(x, 2, ICON_BMPS[i], 8, 8, SSD1306_WHITE);
        if (i == m_icon)
            m_display->drawRect(x - 2, 0, 12, 12, SSD1306_WHITE);
    }
}

void grow_up::drawPoops(int16_t offset)
{
    for (uint8_t p = 0; p < m_poop; p++)
        drawSprite2x(POOP_X[p] + offset, POOP_Y[p], SPR_POOP, 8, 8);
}

void grow_up::drawScene(int8_t shake, int16_t poopOffset)
{
    m_display->clearDisplay();
    drawIconBar();
    bool sleeping = isSleeping();

    if (m_lightOff) {
        // 暗闇: 月と星だけが見える
        drawSprite2x(104, 16, SPR_MOON, 8, 8);
        m_display->drawPixel(20, 22, SSD1306_WHITE);
        m_display->drawPixel(44, 30, SSD1306_WHITE);
        m_display->drawPixel(70, 20, SSD1306_WHITE);
        m_display->drawPixel(88, 42, SSD1306_WHITE);
        m_display->drawPixel(32, 46, SSD1306_WHITE);
        if (sleeping) {
            m_display->setTextSize(1);
            m_display->setCursor(56, 34);
            m_display->print(m_blink ? "Zz" : "zZ");
        }
        m_display->drawFastHLine(0, 63, 128, SSD1306_WHITE);
        return;
    }

    m_display->drawFastHLine(0, 63, 128, SSD1306_WHITE); // 地面
    drawPoops(poopOffset);

    if (m_stage == EGG) {
        drawSprite2x(48 + shake + (m_animFrame ? 2 : -2), 30, SPR_EGG, 16, 16);
    } else {
        int16_t y = 30;
        if (m_stage == BABY && m_animFrame && !m_sick && !sleeping && !m_sulking)
            y -= 4; // ぴょんぴょんはねる
        drawSprite2x(m_petX + shake, y, petBitmap(), 16, 16);

        m_display->setTextSize(1);
        if (sleeping) {
            m_display->setCursor(m_petX + 34, 22);
            m_display->print(m_blink ? "Zz" : "zZ");
        } else if (m_sulking) {
            m_display->setCursor(m_petX + 34, 22); // 怒りマーク
            m_display->print("#");
        } else if (m_happy <= 20) {
            m_display->fillRect(m_petX + 7, 46, 2, 3, SSD1306_WHITE); // なみだ
        }
    }

    if (m_sick)
        m_display->drawBitmap(96, 2, SPR_SKULL, 8, 8, SSD1306_WHITE);
    if (m_attention && m_blink) {
        m_display->setTextSize(2);
        m_display->setCursor(116, 0);
        m_display->print("!");
    }
}

void grow_up::renderNormal()
{
    drawScene();
    m_display->display();
}

void grow_up::renderDead()
{
    m_display->clearDisplay();
    drawSprite2x(16, 24, SPR_TOMB, 16, 16);
    m_display->setTextSize(1);
    m_display->setCursor(64, 24);
    m_display->print("R.I.P.");
    m_display->setCursor(64, 36);
    m_display->print("AGE:");
    m_display->print(m_ageSec / 60);
    m_display->print("m");
    m_display->setCursor(64, 48);
    m_display->print("GEN:");
    m_display->print(m_generation);
    if (m_blink) {
        m_display->setCursor(2, 2);
        m_display->print("PUSH SELECT: NEW EGG");
    }
    m_display->display();
}

void grow_up::renderStatus()
{
    m_display->clearDisplay();
    m_display->setTextSize(1);
    m_display->setCursor(0, 0);
    m_display->print(stageName());
    m_display->print(" AGE:");
    m_display->print(m_ageSec / 60);
    m_display->print("m");

    m_display->setCursor(0, 12);
    m_display->print("FULL");
    drawBar(34, 10, m_hunger);
    m_display->setCursor(0, 24);
    m_display->print("HAPPY");
    drawBar(34, 22, m_happy);

    m_display->setCursor(0, 34);
    m_display->print("WT:");
    m_display->print(m_weight);
    if (m_weight >= FatWeight)
        m_display->print("!");
    m_display->setCursor(64, 34);
    m_display->print("DISC:");
    m_display->print(m_discipline);

    m_display->setCursor(0, 44);
    m_display->print("MISS:");
    m_display->print(m_careMistakes);
    if (m_sick) {
        m_display->setCursor(64, 44);
        m_display->print("SICK!");
    }

    m_display->setCursor(0, 54);
    m_display->print("GEN:");
    m_display->print(m_generation);
    m_display->setCursor(64, 54);
    m_display->print("BEST:");
    m_display->print(m_bestAgeMin);
    m_display->print("m");
    m_display->display();
}

void grow_up::renderMenu(const char* const* items, uint8_t count, uint8_t cursor)
{
    m_display->clearDisplay();
    m_display->setTextSize(2);
    for (uint8_t i = 0; i < count; i++) {
        int16_t y = i * 22;
        if (i == cursor) {
            m_display->setCursor(0, y);
            m_display->print(">");
        }
        m_display->setCursor(16, y);
        m_display->print(items[i]);
    }
    m_display->display();
}

void grow_up::drawBar(int16_t x, int16_t y, int16_t value)
{
    m_display->drawRect(x, y, 90, 10, SSD1306_WHITE);
    m_display->fillRect(x + 2, y + 2, (int16_t)(86L * value / 100), 6, SSD1306_WHITE);
}

// ---- サウンド ----

void grow_up::beep(uint16_t freq, uint16_t ms)
{
    tone(m_buzzerPin, freq);
    delay(ms);
    noTone(m_buzzerPin);
}

void grow_up::melodyHappy()
{
    beep(1300, 60);
    delay(30);
    beep(1700, 60);
    delay(30);
    beep(2100, 90);
}

void grow_up::melodyRefuse()
{
    beep(200, 120);
}

void grow_up::melodyAttention()
{
    beep(2600, 40);
    delay(40);
    beep(2600, 40);
}

void grow_up::melodyAngry()
{
    beep(180, 100);
    delay(40);
    beep(180, 100);
}

void grow_up::melodyMorning()
{
    beep(1500, 50);
    delay(30);
    beep(2000, 80);
}

void grow_up::melodyEvolve()
{
    beep(800, 80);
    delay(30);
    beep(1000, 80);
    delay(30);
    beep(1300, 80);
    delay(30);
    beep(1700, 120);
}

void grow_up::melodyDeath()
{
    beep(1000, 150);
    delay(50);
    beep(800, 150);
    delay(50);
    beep(600, 150);
    delay(50);
    beep(400, 300);
}
