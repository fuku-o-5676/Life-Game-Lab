#pragma once
#include <Arduino.h>
#include "SpriteData.h"


class Display {
public:
    static const uint8_t Width = 64;
    static const uint8_t Height = 32;

    static const uint8_t BufferCols = 64;
    static const uint8_t BufferAreaNumber = 8;
    static const uint8_t BufferRows = 4;

    static const uint8_t OLEDCols = 128;
    static const uint8_t OLEDRows = 8;
private:
    uint8_t m_deviceID;
    uint8_t m_buffer[BufferRows][BufferCols];
    bool m_dirtyArea[BufferRows][BufferAreaNumber];

    void sendCommand(uint8_t);
    uint8_t bufferToFragment(uint8_t segment, uint8_t col);
    void updateSegment(uint8_t segment, bool flush=false);

    void setDirtyBits(bool dirty);
    bool isDirty(uint8_t row, uint8_t col);
    
public:
    void initialize(uint8_t id);
    void clear(bool on=false);
    void setPixel(uint8_t x, uint8_t y, bool reset=false);
    void putSprite(uint8_t x, uint8_t y, uint8_t* sprite, bool reset=false);

    void showNumber(uint8_t x, uint8_t y, int16_t number);

    void wbChange(bool iswbinvert=false);
    void flush();
};
