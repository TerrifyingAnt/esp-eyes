#include <TFT_eSPI.h>  // Hardware-specific library
// #include <TFT_eFEX.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

int eyeWidth = 240;
int eyeHeight = 280;

int pupilRadius = 40;

int pupilX = eyeWidth / 2;  // Начальная позиция зрачка по X
int pupilY = eyeHeight / 2;  // Позиция зрачка по Y (центр глаза)
int direction = 1;          // Направление движения зрачка (1 - вправо, -1 - влево)

void drawCircle(int x, int y) {
  img.setColorDepth(8);
  img.createSprite(240, 280);
  img.fillSprite(TFT_WHITE);
  img.fillCircle(pupilX, pupilY, pupilRadius, TFT_WHITE);
  img.fillCircle(pupilX, pupilY, pupilRadius, TFT_BLACK);
  img.pushSprite(x, y, TFT_TRANSPARENT);
  img.deleteSprite();
}

int y, x = 8;
void setup() {
  tft.init();
  tft.setRotation(0);

  tft.fillScreen(TFT_WHITE);  // let say that's the background
}

void loop() {
  drawCircle(x, y);  // this is forground
  // Обновляем позицию зрачка
  pupilX += direction;

  // Если зрачок достиг края глаза, меняем направление
  if (pupilX >= eyeWidth - pupilRadius || pupilX <= pupilRadius) {
    direction *= -1;
  }
  delay(10);
} 