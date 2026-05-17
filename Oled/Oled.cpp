#include "Oled.h"
#include <cstdint>
#include <sys/_types.h>

Oled::Oled() : display(WIDTH, HEIGHT, &Wire, OLED_RESET) {}

bool Oled::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        return false;
    }
    display.clearDisplay();
    display.display();
    return true;
}

void Oled::print(const String &text, uint8_t x, uint8_t y, uint8_t size,
                 uint8_t clear) {
    if (clear)
        display.clearDisplay();
    display.setTextSize(size);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(x, y);
    display.println(text);
    display.display();
}

void Oled::clear() {
    display.clearDisplay();
}

void Oled::displayNow() {
    display.display();
}

void Oled::drawText(const String &text, uint8_t x, uint8_t y, uint8_t size) {
    display.setTextSize(size);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(x, y);
    display.print(text);
}

void Oled::drawVerticalLine(uint8_t x) {
    display.drawFastVLine(x, 0, HEIGHT, SSD1306_WHITE);
}

void Oled::drawbitmap(const unsigned char *logo, uint8_t logoWidth,
                      uint8_t logoHeight) {
    display.clearDisplay();

    display.drawBitmap((WIDTH - logoWidth) / 2, (HEIGHT - logoHeight) / 2, logo,
                       logoWidth, logoHeight, 1);
    display.display();
    delay(1000);
}
