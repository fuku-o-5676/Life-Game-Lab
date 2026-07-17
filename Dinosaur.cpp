#include "Dinosaur.h"
#include <EEPROM.h>

DinoStage::DinoStage()
{
    initialize();
}

void DinoStage::initialize()
{
    m_frameCount = 0;
    m_lastGenerateFrame = 0;
    m_speedPerFrame8 = 10;

    for (uint8_t i=0; i<BoxNumber; i++)
        m_boxList[i].enabled = false;
}

void DinoStage::generateBox()
{
    if ((m_frameCount - m_lastGenerateFrame) < GenerateMinFrameGap)
        return;
    
    if ( ((m_frameCount - m_lastGenerateFrame) > GenerateMaxFrameGap) ||
         (random(255) < m_generateRatio) ) {
        m_lastGenerateFrame = m_frameCount;

        for (uint8_t i=0; i<BoxNumber; i++) {
            if (!m_boxList[i].enabled) {
                m_boxList[i].y = random(12) + 4;
                m_boxList[i].x = 60 * 8;
                m_boxList[i].enabled = true;
                break;
            }
        }
    }
}

bool DinoStage::checkCollision(uint8_t x, uint8_t y)
{
    for (uint8_t i=0; i<BoxNumber; i++) {
        if (!m_boxList[i].enabled)
            continue;

        Box& box = m_boxList[i];
        uint8_t xL = x+2;
        uint8_t xR = x+6;
        uint8_t XL = box.x/8+1;
        uint8_t XR = box.x/8+2;

        uint8_t y1 = y+2;
        uint8_t y2 = y+9;
        uint8_t Y1 = Display::Height-box.y+1;
        uint8_t Y2 = Display::Height-box.y+2;

        if ((xL <= XR) && (xR >= XL) &&
            (y1 <= Y2) && (y2 >= Y1))
            return true;
    }
    return false;
}

void DinoStage::updateLevel(Display* m_display)
{
    // if (m_frameCount % 2000) // distance 500
    //     return;

    uint8_t speedLevel = (int)(m_frameCount / 1000);

    switch (speedLevel) {
    case 0:
        m_speedPerFrame8 = 11;
        break;
    case 1:
        m_speedPerFrame8 = 12;
        break;
    case 2:;
        m_speedPerFrame8 = 14;
        break;
    case 3:
        m_speedPerFrame8 = 18;
        break;
    case 4:
        m_speedPerFrame8 = 22;
        break;
    default:
        m_speedPerFrame8 = 24;
    }
    bool wb = (bool)((m_frameCount / 2000) % 2);
    m_display->wbChange(wb);
}

void DinoStage::update(Display* display)
{
    m_frameCount++;
    updateLevel(display);
    generateBox();
    updateBoxPositions(display);
    if (m_frameCount > maxFrameCount) {
        m_frameCount = 0;
        m_lastGenerateFrame = 0;
    }
}

void DinoStage::updateBoxPositions(Display* display)
{
    for (uint8_t i=0; i<BoxNumber; i++) {
        if (!m_boxList[i].enabled)
            continue;
        display->putSprite(m_boxList[i].x/8, Display::Height-m_boxList[i].y, const_cast<uint8_t*>(PIX_BOX4), true);
        
        m_boxList[i].x -= m_speedPerFrame8;
        if (m_boxList[i].x < 0)
            m_boxList[i].enabled = false;
        else
            display->putSprite(m_boxList[i].x/8, Display::Height-m_boxList[i].y, const_cast<uint8_t*>(PIX_BOX4));
    }
}


void Dinosaur::initialize(Display* display)
{
    m_distance = 0;
    m_altitude = 0;
    m_onJump = false;
    m_ducking = false;
    m_display = display;
    m_style = RunA;
}

void Dinosaur::gameOver()
{
    m_display->wbChange();
    dino_gameover_sound_readed = false;
    uint8_t maxDino = EEPROM.read(0);
    uint8_t maxDino2 = EEPROM.read(1);
    uint16_t maxScore = (maxDino << 8) | maxDino2;
    if (maxScore < m_distance){
        char upper = (char)(m_distance >> 8);
        char lower = (char)(m_distance & 0xFF);
        EEPROM.write(0, upper);
        EEPROM.write(1, lower);
        EEPROM.commit();
    }
    m_display->putSprite(2, 12, const_cast<uint8_t*>(PIX_G));
    delay(20);
    m_display->flush();
    m_display->putSprite(10, 12, const_cast<uint8_t*>(PIX_A));
    delay(20);
    m_display->flush();
    m_display->putSprite(18, 12, const_cast<uint8_t*>(PIX_M));
    delay(20);
    m_display->flush();
    m_display->putSprite(26, 12, const_cast<uint8_t*>(PIX_E)); 
    delay(20);
    m_display->flush();   
    m_display->putSprite(34, 12, const_cast<uint8_t*>(PIX_O));
    delay(20);
    m_display->flush();
    m_display->putSprite(42, 12, const_cast<uint8_t*>(PIX_V));
    delay(20);
    m_display->flush();
    m_display->putSprite(50, 12, const_cast<uint8_t*>(PIX_E));
    delay(20);
    m_display->flush();
    m_display->putSprite(58, 12, const_cast<uint8_t*>(PIX_R));  
    delay(20);  
    m_display->flush();
    delay(50);
    while (digitalRead(restart_pin) && digitalRead(duck_pin) && digitalRead(jump_pin)) { delay(10); }

    initialize(m_display);
    m_stage.initialize();
}

int16_t calcY(int16_t altitude) {
    int16_t y = 21 - altitude;

    if (y>21)
        y = 21;
    if (y<0)
        y = 0;
    return y;
}

void Dinosaur::updateStyle()
{
    if (m_style==RunA) {
        if (m_ducking)
            m_style=DuckB;
        else
            m_style=RunB;
    } else if (m_style==RunB) {
        if (m_ducking)
            m_style=DuckA;
        else
            m_style=RunA;
    } else if (m_style==DuckA) {
        if (m_ducking)
            m_style = DuckB;
        else
            m_style = RunA;
    } else if (m_style==DuckB) {
        if (m_ducking)
            m_style = DuckA;
        else
            m_style = RunB;
    }
}

uint8_t* Dinosaur::styleToSprite()
{
    switch (m_style) {
    case RunA: return const_cast<uint8_t*>(PIX_MANA);
    case RunB: return const_cast<uint8_t*>(PIX_MANB);
    case DuckA: return const_cast<uint8_t*>(PIX_MANC);
    case DuckB: return const_cast<uint8_t*>(PIX_MAND);
    default:
        m_style = RunA;
        return const_cast<uint8_t*>(PIX_MANA);
    }
}

void Dinosaur::updateDisplay()
{
    int16_t altitudeTmp = m_altitude;
    if (m_onJump) {
        m_altitude += m_velocity;
        if (m_frameCount % 2)
            m_velocity--;

        if (m_altitude<=0) {
            m_altitude = 0;
            m_velocity = 0;
            m_onJump = false;
        }
    }

    int16_t y = calcY(altitudeTmp);
    m_display->putSprite(DinosaurX, y, styleToSprite(), true);
    
    updateStyle();
    y = calcY(m_altitude);
    m_display->putSprite(DinosaurX, y, styleToSprite(), false);

    m_display->showNumber(Display::Width - 6*6, 0, m_distance);
    m_display->flush();
}

void Dinosaur::update()
{
    if ((m_frameCount % 4) == 0 )
        m_distance++;
    if (m_distance > 60000)
        m_distance = 60000;
    m_frameCount++;
    checkButton();

    m_stage.update(m_display);
    if (m_stage.checkCollision(DinosaurX, calcY(m_altitude) + (m_ducking ? 4 : 0))) {
        gameOver();
        m_display->clear();
        m_display->flush();
        delay(1000);
    }

    updateDisplay();
}

void Dinosaur::checkButton()
{
    if (!digitalRead(jump_pin) && !m_onJump) {
        m_velocity = 4;
        m_onJump = true;
        m_ducking = false;
        if (!dino_jump){
            dino_jump = true;
            dino_jump_sound_readed = false;
        }
    } else {
        if(dino_jump){
            dino_jump = false;
        }
    }

    if (!digitalRead(duck_pin) && !m_onJump) {
        m_ducking = true;
        if (!dino_gram){
            dino_gram = true;
            dino_gram_sound_readed = false;
        }
    } else {
        m_ducking = false;
        if (dino_gram){
            dino_gram = false;
        }
    }
}
void Dinosaur::setPin(byte jump, byte duck, byte restart){
    jump_pin = jump;
    duck_pin = duck;
    restart_pin = restart;
}
int Dinosaur::getSound(){
    if (!dino_jump_sound_readed){
        dino_jump_sound_readed = true;
        return 1;
    } else if(!dino_gram_sound_readed){
        dino_gram_sound_readed = true;
        return 2;
    } else if(!dino_gameover_sound_readed){
        dino_gameover_sound_readed = true;
        return 3;
    } else {
        return 0;
    }
}