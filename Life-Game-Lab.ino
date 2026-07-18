#include <Wire.h>
#include "Display.h"
#include "Dinosaur.h"
#include "life.h"
#include "grow_up.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define SSD1306_DEVICE_ID 0x3c
#define select_start_pin 15
#define left_pin         14
#define right_pin        11
#define buzzer_pin       23

Display display;
Dinosaur dinosaur;
Life_game life;
grow_up gup;
Adafruit_SSD1306 ssdisplay(128, 64, &Wire);

void setup()
{
    Wire.end();
    Wire.setSCL(1);
    Wire.setSDA(0);
    Wire.begin();
    EEPROM.begin(64);
    pinMode(select_start_pin, INPUT_PULLUP);
    pinMode(left_pin, INPUT_PULLUP);
    pinMode(right_pin, INPUT_PULLUP);
    pinMode(buzzer_pin, OUTPUT);
    ssdisplay.begin(SSD1306_SWITCHCAPVCC, SSD1306_DEVICE_ID);
    ssdisplay.setTextColor( SSD1306_WHITE, SSD1306_BLACK );
    ssdisplay.clearDisplay();
    ssdisplay.display();
    ssdisplay.setTextSize(2);
    ssdisplay.setCursor(0, 0);
    ssdisplay.print("Life");
    ssdisplay.setCursor(20, 25);
    ssdisplay.print("Game");
    ssdisplay.setCursor(60, 45);
    ssdisplay.print("Lab.");
    ssdisplay.display();
    buzzer();
    buzzer();
    if (!digitalRead(select_start_pin)){
        EEPROM.write(0, 0);
        EEPROM.write(1, 0);
        EEPROM.commit();
    }
    if (!digitalRead(left_pin)){
        grow_up::clearSave();
    }
    delay(2000);
    while(!digitalRead(select_start_pin) || !digitalRead(left_pin) || !digitalRead(right_pin)){delay(10);}
    ssdisplay.clearDisplay();
    ssdisplay.display();
    // ゲーム番号
    // 0 = ライフ
    // 1 = dino
    // 2 = キッチンタイマー
    // 3 = grow_up
    int gameNo = 0;
    uint8_t maxDino = EEPROM.read(0);
    uint8_t maxDino2 = EEPROM.read(1);
    uint16_t maxScore = (maxDino << 8) | maxDino2;
    // メニュー
    while(digitalRead(select_start_pin)){
        if (!digitalRead(left_pin)){
            buzzer();
            if (gameNo < 3){
                gameNo++;
            } else if(gameNo == 3){
                gameNo = 0;
            }
            while(!digitalRead(left_pin) || !digitalRead(right_pin)){};
        } else if(!digitalRead(right_pin)){
            buzzer();
            if (gameNo > 0){
                gameNo--;
            } else if(gameNo == 0){
                gameNo = 3;
            }
            while(!digitalRead(left_pin) || !digitalRead(right_pin)){};
        }
        // 以下描画処理
        // SSD1306用
        ssdisplay.clearDisplay();
        ssdisplay.setCursor(1, 1);
        ssdisplay.setTextSize(3);
        if (gameNo == 0){
            ssdisplay.print("Life");
        } else if(gameNo == 1) {
            ssdisplay.print("Dino");
            ssdisplay.setCursor(0, 30);
            ssdisplay.setTextSize(2);
            ssdisplay.print("maxScore:");
            ssdisplay.setCursor(0,50);
            ssdisplay.print(String(maxScore));
        } else if(gameNo == 2){
            ssdisplay.print("Timer");
        } else if(gameNo == 3){
            ssdisplay.print("Pet");
            if (EEPROM.read(grow_up::SAVE_ADDR) == grow_up::SAVE_MAGIC){
                ssdisplay.setCursor(0, 30);
                ssdisplay.setTextSize(2);
                ssdisplay.print("CONTINUE");
            }
        }
        ssdisplay.display();
    }
    tone(buzzer_pin, 1000);
    delay(40);
    noTone(buzzer_pin);
    delay(20);
    tone(buzzer_pin, 1500);
    delay(40);
    noTone(buzzer_pin);
    while(!digitalRead(select_start_pin)) {}
    if (gameNo == 0){
        lifeGame();
    } else if(gameNo == 1) {
        dino_play();
    } else if(gameNo == 2){
        timer();
    } else if(gameNo == 3){
        petGame();
    }
}

void loop()
{
}

void dino_play(){
    display.initialize(SSD1306_DEVICE_ID);
    dinosaur.initialize(&display);
    dinosaur.setPin(left_pin, right_pin, select_start_pin);
    display.clear(true);
    display.flush();
    delay(10);
    display.clear(false);
    display.flush();
    delay(10);
    for(;;){
        dinosaur.update();
        sound_manager(dinosaur.getSound());
        delay(16);
    }
}

void lifeGame(){
    life.setLife(buzzer_pin, select_start_pin, left_pin, right_pin, &ssdisplay);
}

void petGame(){
    gup.play(buzzer_pin, select_start_pin, left_pin, right_pin, &ssdisplay);
}

void sound_manager(int sound_no){
    switch(sound_no){
        case 0:
            noTone(buzzer_pin);
            break;
        case 1:
            tone(buzzer_pin, 2200);
            delay(50);
            noTone(buzzer_pin);
            break;
        case 2:
            tone(buzzer_pin, 800);
            delay(50);
            noTone(buzzer_pin);
            break;
        case 3:
            for (int i = 0;i <= 1;i++){
                tone(buzzer_pin, 500);
                delay(50);
                noTone(buzzer_pin);
                delay(50);
            }
            break;
    }
}
int m = 0;
int s = 0;
void printOut(){
    ssdisplay.clearDisplay();
    String dispOut = "";
    ssdisplay.setCursor(0, 0);
    ssdisplay.setTextSize(4);
    if (m < 10){
        dispOut += "0";
    }
    dispOut += m;
    dispOut += ":";
    if (s < 10){
        dispOut += "0";
    }
    dispOut += s;
    ssdisplay.print(dispOut);
    ssdisplay.display();
}
void timer(){
    int setM = 0;
    int setS = 0;
    int condx = 2; // 0=countUp; 1=countDown; 2=setting
    bool isContinue = true;
    unsigned long currentMillis = millis();
    unsigned long cMil2 = millis();
    unsigned long cMil3 = millis();
    int count_up_m = 0;
    int count_up_s = 0;
    printOut();
    while(1){
        delay(20);
        switch(condx){
            case 0:
                if (millis() - currentMillis >= 1000){
                    currentMillis = millis();
                    if (s < 59){
                        s++;
                    } else if(s == 59){
                        s = 0;
                        m++;
                    }
                    if (m > 99){
                        condx = 2;
                    }
                    printOut();
                }
                if (!digitalRead(select_start_pin) && digitalRead(left_pin) && digitalRead(right_pin)){
                    buzzer();
                    isContinue = true;
                    condx = 2;
                    printOut();
                    while(!digitalRead(select_start_pin) && digitalRead(left_pin) && digitalRead(right_pin)){delay(10);}
                }
                else if(!digitalRead(left_pin) && !digitalRead(right_pin)){
                    buzzer();
                    m = 0;
                    s = 0;
                    setM = 0;
                    setS = 0;
                    isContinue = false;
                    condx = 2;
                    printOut();
                    while(!digitalRead(left_pin) && !digitalRead(right_pin)){delay(10);}
                }
                break;

            case 1:
                if (millis() - currentMillis >= 1000){
                    currentMillis = millis();
                    if (s > 0){
                        s--;
                    } else if (s == 0){
                        if (m == 0){
                            timeUp();
                            isContinue = false;
                            condx = 2;
                            m = setM;
                            s = setS;
                            // Timed UP!
                        } else {
                            m--;
                            s = 59;
                        }
                    }
                    printOut();
                }
                if (!digitalRead(select_start_pin) && digitalRead(left_pin) && digitalRead(right_pin)){
                    buzzer();
                    isContinue = true;
                    condx = 2;
                    printOut();
                    while(!digitalRead(select_start_pin) && digitalRead(left_pin) && digitalRead(right_pin)){delay(10);}
                }
                else if(!digitalRead(left_pin) && !digitalRead(right_pin)){
                    buzzer();
                    m = 0;
                    s = 0;
                    setM = 0;
                    setS = 0;
                    isContinue = false;
                    condx = 2;
                    printOut();
                    while(!digitalRead(left_pin) && !digitalRead(right_pin)){delay(10);}
                }
                break;
            case 2:
                bool selstart = !digitalRead(select_start_pin);
                bool left = !digitalRead(left_pin);
                bool right = !digitalRead(right_pin);
                if (selstart){
                    buzzer();
                    currentMillis = millis();
                    if (isContinue){
                        condx = (setM == 0 && setS == 0) ? 0 : 1; 
                    } else {
                        if (m == 0 && s == 0){
                            condx = 0;
                        } else {
                            condx = 1;
                            setM = m;
                            setS = s;
                        }
                    }
                    printOut();
                    delay(300);
                    break;
                }

                if (left || right){
                    isContinue = false;

                    if (left && !right){
                        if (millis() - cMil2 > 200){
                            cMil2 = millis();
                            if (count_up_m > 3){
                                m = (m + 1) % 100;
                                buzzer();
                            } else if(count_up_m == 0){
                                m = (m + 1) % 100;
                                buzzer();
                            }
                            count_up_m++;
                            printOut();
                        }
                    } else {
                        count_up_m = 0;
                    }
                    if (right && !left){
                        if (millis() - cMil3 > 200){
                            cMil3 = millis();
                            if (count_up_s > 3){
                                s = (s + 1) % 60;
                                buzzer();
                            } else if(count_up_s == 0){
                                s = (s + 1) % 60;
                                buzzer();
                            }
                            count_up_s++;
                            printOut();
                        }
                    } else {
                        count_up_s = 0;
                    }
                    if (left && right){
                        buzzer();
                        m = 0;
                        s = 0;
                        setM = 0;
                        setS = 0;
                        isContinue = false;
                        count_up_m = 0;
                        count_up_s = 0;
                        condx = 2;
                        printOut();
                        delay(300);
                    }
                } else {
                    count_up_m = 0;
                    count_up_s = 0;
                }
                break;
        }
    }
}

void buzzer(){
    tone(buzzer_pin, 2000);
    delay(20);
    noTone(buzzer_pin);
    delay(20);
}

void timeUp(){
    int ring = 0;
    unsigned long countUp = millis();
    unsigned long countUp2 = millis();
    bool buzzerState = false;
    bool buzzerState2 = true;
    while(ring < 20 && (digitalRead(select_start_pin) && digitalRead(left_pin) && digitalRead(right_pin))){
        if (millis() - countUp > 500){
            countUp = millis();
            buzzerState2 = !buzzerState2;
            noTone(buzzer_pin);
            ring++;
        }
        if (millis() - countUp2 > 40 && buzzerState2){
            countUp2 = millis();
            if (buzzerState){
                tone(buzzer_pin, 5000);
            } else {
                noTone(buzzer_pin);
            }
            buzzerState = !buzzerState;
        }
    }
    noTone(buzzer_pin);
    buzzer();
}

void setup1(){
}
void loop1(){
}