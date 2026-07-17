#include "Display.h"
#include <Wire.h>

uint8_t *PIX_Numbers[] = { const_cast<uint8_t*>(PIX_0), const_cast<uint8_t*>(PIX_1), const_cast<uint8_t*>(PIX_2), const_cast<uint8_t*>(PIX_3), const_cast<uint8_t*>(PIX_4), const_cast<uint8_t*>(PIX_5), const_cast<uint8_t*>(PIX_6), const_cast<uint8_t*>(PIX_7), const_cast<uint8_t*>(PIX_8), const_cast<uint8_t*>(PIX_9)};


void Display::initialize(uint8_t id)
{
    clear();
    
    m_deviceID = id;
    Wire.end();
    Wire.begin();
    Wire.setClock(400000);

    sendCommand(0xae); // Display off
    sendCommand(0xd3); // Display offset
    sendCommand(0x00);
    sendCommand(0xA8); // Set multiplex ratio 63
    sendCommand(0x3f); 

    sendCommand(0x40); // start line

    // sendCommand(0xa0); // Remap addres0 -> seg0
    sendCommand(0xa1); // Remap addres0 -> seg127
    // sendCommand(0xc0); // Normal scan
    sendCommand(0xc8); //Reverse scan

    sendCommand(0x81); // contrast
    sendCommand(0x7f);

    sendCommand(0xa4); // Resume display

    sendCommand(0xa6); // Non-inverse display if reverse, put 0xa7

    sendCommand(0x8d); // Enable charge pump
    sendCommand(0x14);

    sendCommand(0xaf); // Display ON
}

void Display::sendCommand(uint8_t comm)
{
    Wire.beginTransmission(m_deviceID);
    Wire.write(0x00);
    Wire.write(comm);
    Wire.endTransmission();
}

void Display::clear(bool on)
{
    uint8_t fragment;
    if (on)
        fragment = 0xff;
    else
        fragment = 0x00;

    for (uint8_t row=0; row<BufferRows; row++)
        for (uint8_t col=0; col<BufferCols; col++)
            m_buffer[row][col] = fragment;

    setDirtyBits(true);
}

void Display::setPixel(uint8_t x, uint8_t y, bool reset)
{
    if ((x >= Width) || (y >= Height))
        return;
    
    uint8_t row = y / 8;
    if (reset)
        m_buffer[row][x] &= ~(0x01 << (y % 8));
    else
        m_buffer[row][x] |= 0x01 << (y % 8);

    m_dirtyArea[y/8][x/8] = true;
}

bool Display::isDirty(uint8_t row, uint8_t col)
{
    uint8_t bufferRow = row / 2;
    uint8_t bufferArea = (col >> 1) / 8;
    return m_dirtyArea[bufferRow][bufferArea];
}

void Display::setDirtyBits(bool dirty)
{
    for (uint8_t row=0; row<BufferRows; row++)
        for (uint8_t area=0; area<BufferAreaNumber; area++)
            m_dirtyArea[row][area] = dirty;
}

void Display::updateSegment(uint8_t row, bool flush)
{
    uint8_t col = 0;

    for (uint8_t col=0; col<OLEDCols; col+=8) {
        if (isDirty(row, col)) {
            sendCommand(0x22); // page addressing mode
            sendCommand(0xb0 | row);
            sendCommand(0x00 | (col & 0x0f));
            sendCommand(0x10 | (col >> 4));

            Wire.beginTransmission(m_deviceID);
            Wire.write(0x40);
            for (uint8_t i=0; i<8; i++)
                Wire.write(bufferToFragment(row, col+i));
            Wire.endTransmission();
        }
    }
}

void Display::flush()
{
    for (uint8_t row=0; row<OLEDRows; row++)
        updateSegment(row);
        
    setDirtyBits(false);
}

uint8_t Display::bufferToFragment(uint8_t row, uint8_t col)
{
    uint8_t bufferRow = row / 2;
    uint8_t bufferCol = col / 2;

    uint8_t bufferFragment = m_buffer[bufferRow][bufferCol];
    if (row % 2)
        bufferFragment = bufferFragment >> 4;

    uint8_t fragment = 0x00;
    if (bufferFragment & 0x01)
        fragment |= 0x03;
    if (bufferFragment & 0x02)
        fragment |= 0x0c;
    if (bufferFragment & 0x04)
        fragment |= 0x30;
    if (bufferFragment & 0x08)
        fragment |= 0xc0;

    return fragment;
}

void Display::putSprite(uint8_t x, uint8_t y, uint8_t* sprite, bool reset)
{
    uint8_t x_number = sprite[0] / 8 + ((sprite[0] % 8) ? 1 : 0);
    uint8_t index = 2;
    for (uint8_t sy=0; sy<sprite[1]; sy++) {
        for (uint8_t sx=0; sx<x_number; sx++) {
            setPixel(x + sx*8    , y + sy, !(sprite[index] & 0x80) || reset);
            setPixel(x + sx*8 + 1, y + sy, !(sprite[index] & 0x40) || reset);
            setPixel(x + sx*8 + 2, y + sy, !(sprite[index] & 0x20) || reset);
            setPixel(x + sx*8 + 3, y + sy, !(sprite[index] & 0x10) || reset);
            setPixel(x + sx*8 + 4, y + sy, !(sprite[index] & 0x08) || reset);
            setPixel(x + sx*8 + 5, y + sy, !(sprite[index] & 0x04) || reset);
            setPixel(x + sx*8 + 6, y + sy, !(sprite[index] & 0x02) || reset);
            setPixel(x + sx*8 + 7, y + sy, !(sprite[index] & 0x01) || reset);
            index++;
        }
    }
}

void Display::showNumber(uint8_t x, uint8_t y, int16_t number)
{
    for (int16_t base = 10000; base >= 1; base /= 10) {
        uint8_t num = number / base;
        number -= num * base;
        putSprite(x, y, PIX_Numbers[0], true);
        putSprite(x, y, PIX_Numbers[num]);
        x += 6;
    }
}

void Display::wbChange(bool iswbinvert){
    if (iswbinvert){
    sendCommand(0xa7);
    } else {
        sendCommand(0xa6);
    }
}