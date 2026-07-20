#include "charKatakana.h"
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void charKatakana::printKatakana(Adafruit_SSD1306* display, int16_t x, int16_t y, const char* string, uint8_t size, uint16_t color) {
  int16_t cur_x = x;
  int16_t cur_y = y;

  while (*string) {
    uint8_t c = (uint8_t)*string;

    // 半角カタカナの判定
    // JIS X 0201 の 0xA1 ~ 0xDFに相当する場合はビットマップデータから取得
    if (c >= 0xA1 && c <= 0xDF) {
      uint8_t index = c - 0xA1;
      // ビットマップを画面に描画
      display->drawBitmap(cur_x, cur_y, font_hankaku_katakana[index], 8, 8, color);
      cur_x += 8 * size; // 次の文字位置へシフト
    } else if (c >= 0x20 && c <= 0x7E) {
      // 通常文字
      display->setCursor(cur_x, cur_y);
      display->setTextColor(color);
      display->setTextSize(size);
      display->write(c);
      // 標準フォントは6px
      cur_x += 6 * size;
    } else if (c == '\n') {
      // 改行
      cur_x = x;
      cur_y += 8 * size;
    }
    string++;
  }
}