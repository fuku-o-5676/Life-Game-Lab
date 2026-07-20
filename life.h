#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "picoPwmAudio.h"

class Life_game{
  private:
  static const int CELL_W = 2;
  static const int CELL_H = 2;
  static const int GRID_ROWS = 32; // 64 /2
  static const int GRID_COLS = 64; // 128/2
  // static const int CELL_W = 4;
  // static const int CELL_H = 4;
  // static const int GRID_ROWS = 16; // 64 /4
  // static const int GRID_COLS = 32; // 128/4

  bool grid[GRID_ROWS][GRID_COLS];
  bool nextGrid[GRID_ROWS][GRID_COLS];
  uint32_t generation = 0;
  void initRandomGrid();
  int countNeighbors(int x, int y);
  void addRandomCells(int num);
  public:
  void setLife(byte buzzer_pin, byte meteo_pin, byte reset_pin, byte reset2_pin, Adafruit_SSD1306* display, PicoPwmAudio* audio);
};