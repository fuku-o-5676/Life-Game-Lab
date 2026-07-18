// 最小構成の動作確認用テストプログラム
// ディスプレイ表示とボタン押下でのブザー鳴動を確認する
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "WelcomeBitmap.h"

#define SSD1306_DEVICE_ID 0x3c
#define button_pin1        15
#define button_pin2        14
#define button_pin3        11

#define buzzer_pin         23

Adafruit_SSD1306 ssdisplay(128, 64, &Wire);

void showWelcome()
{
    ssdisplay.clearDisplay();
    ssdisplay.drawBitmap(0, 0, welcomeBitmap, welcomeBitmapWidth, welcomeBitmapHeight, SSD1306_WHITE);
    ssdisplay.display();
}


void setup()
{
    Wire.end();
    Wire.setSCL(1);
    Wire.setSDA(0);
    Wire.begin();

    pinMode(button_pin1, INPUT_PULLUP);
    pinMode(button_pin2, INPUT_PULLUP);
    pinMode(button_pin3, INPUT_PULLUP);
    pinMode(buzzer_pin, OUTPUT);

    ssdisplay.begin(SSD1306_SWITCHCAPVCC, SSD1306_DEVICE_ID);
    ssdisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    showWelcome();
}

void loop()
{
    if (!digitalRead(button_pin1)) {
        tone(buzzer_pin, 4200);
        while (!digitalRead(button_pin1)) { delay(10); }
        noTone(buzzer_pin);
    }
    else if (!digitalRead(button_pin2)) {
        tone(buzzer_pin, 4000);
        while (!digitalRead(button_pin2)) { delay(10); }
        noTone(buzzer_pin);
    }
    else if (!digitalRead(button_pin3)) {
        tone(buzzer_pin, 4500);
        while (!digitalRead(button_pin3)) { delay(10); }
        noTone(buzzer_pin);
    }
}
