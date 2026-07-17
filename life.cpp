#include "life.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void Life_game::initRandomGrid() {
  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      grid[y][x] = (random(2) == 1);  // 0 or 1 をランダムで
    }
  }
  generation = 0;  // ★初期化時にカウンタをリセット
}

int Life_game::countNeighbors(int x, int y) {
  int count = 0;
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) continue;  // 自分自身はカウントしない
      int nx = x + dx;
      int ny = y + dy;
      // 端は外側はすべて死んでいるとみなす（ラップなし）
      if (nx < 0 || nx >= GRID_COLS || ny < 0 || ny >= GRID_ROWS) continue;
      if (grid[ny][nx]) count++;
    }
  }
  return count;
}
void Life_game::addRandomCells(int num) {
  int added = 0;
  int attempts = 0;
  const int maxAttempts = num * 20;  // 無限ループ防止用

  while (added < num && attempts < maxAttempts) {
    int x = random(GRID_COLS);
    int y = random(GRID_ROWS);
    if (!grid[y][x]) {          // 死んでいるセルなら誕生させる
      grid[y][x] = true;
      added++;
    }
    attempts++;
  }
}

void Life_game::setLife(byte buzzer_pin, byte meteo_pin, byte reset_pin, byte reset2_pin, Adafruit_SSD1306* display)
{
  // Wire.end();
  // //I2C0の場合
  // Wire.setSDA(0);
  // Wire.setSCL(1);
  // Wire.begin();
  //I2C1の場合
  // Wire1.setSDA(PIN1_SDA);
  // Wire1.setSCL(PIN1_SCL);
  display->clearDisplay();
  display->display();
  randomSeed(analogRead(0));  // 乱数初期化
  initRandomGrid();
  for(;;){
  // 1. 描画
    display->clearDisplay();
    for (int y = 0; y < GRID_ROWS; y++) {
      for (int x = 0; x < GRID_COLS; x++) {
        if (grid[y][x]) {
          int px = x * CELL_W;
          int py = y * CELL_H;
          display->fillRect(px, py, CELL_W, CELL_H, SSD1306_WHITE);
        }
      }
    }
    // ★ 右下に世代数を表示
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(85, 56);  // (128 - 約45, 64 - 8) → 右下に収まりやすい
    //display.print("Gen:");
    display->print(generation);

    display->display();

    // 2. 次世代の計算
    for (int y = 0; y < GRID_ROWS; y++) {
      for (int x = 0; x < GRID_COLS; x++) {
        int n = countNeighbors(x, y);
        if (grid[y][x]) {
          // 今生きているセル
          // 2 or 3 個の隣接セルが生きていれば次世代も生存
          if (n == 2 || n == 3) {
            nextGrid[y][x] = true;
          } else {
            nextGrid[y][x] = false;
          }
        } else {
          // 死んでいるセル
          // ちょうど 3 個の隣接セルが生きていれば誕生
          if (n == 3) {
            nextGrid[y][x] = true;
          } else {
            nextGrid[y][x] = false;
          }
        }
      }
    }

    // 3. 次世代を現在世代にコピー
    for (int y = 0; y < GRID_ROWS; y++) {
      for (int x = 0; x < GRID_COLS; x++) {
        grid[y][x] = nextGrid[y][x];
      }
    }

    // if (generation % 250 == 0) {
    //   addRandomCells(10);
    // }
    if (!digitalRead(meteo_pin) == 1){
      display->clearDisplay();
      display->setCursor(0, 20);
      display->setTextSize(3);
      display->print("METEOR!");
      display->display();
      for (int i=0;i<3;i++){
        for (int j=0;j<=3;j++){
          tone(buzzer_pin, 2000);
          delay(40);
          noTone(buzzer_pin);
          delay(80);
        }
        noTone(buzzer_pin);
        delay(100);
      }
      delay(500);
      for (int k=2000;k>=40;k = k - 20){
        tone(buzzer_pin, k);
        delay(20);
      }
      delay(50);
      noTone(buzzer_pin);
      delay(100);
      addRandomCells(random(50, 121));
    }
    if (!digitalRead(reset_pin) && !digitalRead(reset2_pin)){
      display->clearDisplay();
      display->setCursor(0, 20);
      display->setTextSize(3);
      display->print("RESET!");
      display->display();
      for (int k=2000;k>=40;k = k - 20){
        tone(buzzer_pin, k);
        delay(20);
      }
      noTone(buzzer_pin);
      initRandomGrid();
      generation = 0;
    }
    generation++;  // ★ 世代カウント進行
    delay(100);  // 更新速度（見やすさに応じて調整）
  }
}