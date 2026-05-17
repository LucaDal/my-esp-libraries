// Oled.h
#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <cstdint>
#include <sys/types.h>

class Oled {
private:
    static const int WIDTH = 128;
    static const int HEIGHT = 32;
    static const int OLED_RESET = -1;

    Adafruit_SSD1306 display;

public:
    Oled();
    bool begin();
    void print(const String& text, uint8_t x, uint8_t y, uint8_t size = 1, uint8_t clear = false);
    void clear();
    void displayNow();
    void drawText(const String& text, uint8_t x, uint8_t y, uint8_t size = 1);
    void drawVerticalLine(uint8_t x);
    void drawbitmap(const unsigned char* logo, uint8_t logoWidth, uint8_t logoHeight);
};
