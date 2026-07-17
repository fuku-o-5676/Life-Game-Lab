#pragma once
#include "Display.h"


class DinoStage {
    struct Box {
        int16_t x;
        uint8_t y;
        bool enabled = true;

        Box() { enabled = false; }
    };

    static const uint8_t BoxNumber = 16;
    static const int16_t GenerateMinFrameGap = 32;
    static const int16_t GenerateMaxFrameGap = 128;

    Box m_boxList[BoxNumber];
    uint32_t m_frameCount = 0;
    uint32_t maxFrameCount = 4000000000UL;
    uint32_t m_lastGenerateFrame = 0;
    int8_t m_speedPerFrame8 = 10;
    uint8_t m_generateRatio = 128;
    
    void updateLevel(Display* m_display);
    void generateBox();
    void updateBoxPositions(Display* display);
public:
    DinoStage();
    void initialize();
    void update(Display* display);
    bool checkCollision(uint8_t x, uint8_t altitude);
};


class Dinosaur {
    const int16_t AccelG = 10;
    const uint8_t DinosaurX = 4;
    enum Style {RunA, RunB, DuckA, DuckB };
    
    uint16_t m_distance;
    int16_t m_altitude;
    int16_t m_frameCount;
    int16_t m_velocity;
    Style m_style;

    bool m_onJump;
    bool m_ducking;
    Display* m_display;

    DinoStage m_stage;

    byte jump_pin;
    byte duck_pin;
    byte restart_pin;
    bool dino_gram = false;
    bool dino_jump = false;
    bool dino_gram_sound_readed = true;
    bool dino_jump_sound_readed = true;
    bool dino_gameover_sound_readed = true;

    void checkButton();
    void updateDisplay();
    void updateStyle();
    uint8_t* styleToSprite();
    
public:
    void initialize(Display* display);
    void update();
    void gameOver();
    void setPin(byte jump, byte duck, byte restart);
    int  getSound();
};
