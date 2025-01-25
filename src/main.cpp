#include <TFT_eSPI.h>  // Hardware-specific library
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite eyePupil = TFT_eSprite(&tft);
int eyeWidth = 280;
int eyeHeight = 240;

int pupilRadius = 40;

int pupilX = eyeWidth / 2;  // Начальная позиция зрачка по X
int pupilY = eyeHeight / 2;  // Позиция зрачка по Y (центр глаза)

int targetX = -1;
int targetY = -1;

int directionX = 1;  // Направление движения зрачка (1 - вправо, -1 - влево)
int directionY = 1;

int blinkingState = 0;  // 0 - не моргает, 1 - закрывает, -1 - открывает
int blinkRatio = 2.2;
int blinkingRate = 40;

int counter = 0;

// Семафор для синхронизации доступа к targetX и targetY
SemaphoreHandle_t xSemaphore = NULL;

void checkForBlinking() {
  if (blinkingState == 1) {
    if (blinkRatio < eyeHeight - 80) {
      blinkRatio += 10 * counter % blinkingRate;
    } else {
      blinkingState = -1;
    }
  } else {
    if (blinkingState == -1) {
      if (blinkRatio > 2.2) {
        blinkRatio -= 10 * counter % blinkingRate;
      } else {
        blinkingState = 0;
        blinkRatio = 2.2;
      }
    }
  }
}

// Функция для парсинга данных из последовательного порта
void parseSerialPort(void *parameter) {
  while (1) {
    if (Serial.available()) {
      String coords = Serial.readString();
      if (coords != "") {
        int xPos = coords.indexOf("x: ");
        int yPos = coords.indexOf("y: ");

        // Если оба значения найдены
        if (xPos != -1 && yPos != -1) {
          // Извлекаем значение x
          String xStr = coords.substring(xPos + 3, coords.indexOf(';', xPos));
          int x = xStr.toInt();
          // Извлекаем значение y
          String yStr = coords.substring(yPos + 3);
          int y = yStr.toInt();

          // Захватываем семафор для безопасного доступа к targetX и targetY
          if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
            targetX = x;
            targetY = y;
            xSemaphoreGive(xSemaphore);
          }
        }
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Небольшая задержка для освобождения процессора
  }
}

void drawCircle() {
  eyePupil.setColorDepth(8);
  eyePupil.createSprite(280, 240);
  eyePupil.fillSprite(TFT_BLACK);

  checkForBlinking();

  eyePupil.fillEllipse(eyeWidth / 2, eyeHeight / 2, eyeWidth / 1.5, eyeHeight / blinkRatio, TFT_WHITE);
  // Рисуем зрачок
  eyePupil.fillCircle(pupilX, pupilY, pupilRadius, TFT_BLACK);

  eyePupil.pushSprite(20, 0, TFT_TRANSPARENT);
  eyePupil.deleteSprite();
}

void setup() {
  Serial.begin(9600);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_RED);  // let say that's the background

  // Создаем семафор для синхронизации
  xSemaphore = xSemaphoreCreateMutex();
  if (xSemaphore == NULL) {
    Serial.println("Ошибка создания семафора!");
    while (1);
  }

  // Создаем задачу для парсинга данных
  xTaskCreate(
    parseSerialPort,  // Функция задачи
    "ParseTask",      // Имя задачи
    10000,            // Размер стека
    NULL,             // Параметры задачи
    1,                // Приоритет
    NULL              // Дескриптор задачи
  );
}

void loop() {
  if (counter % blinkingRate == 0 && blinkingState == 0) {
    blinkingState = 1;
    counter = 0;
  }

  drawCircle();  // this is forground

  // Обновляем позицию зрачка
  if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
    Serial.println(targetX);
    Serial.println(targetY);
    if (targetX != -1 && targetY != -1) {
      if (targetX < pupilX) {
        directionX = -1;
      } else {
        directionX = 1;
      }
      if (targetY < pupilY) {
        directionY = -1;
      } else {
        directionY = 1;
      }
    }
    xSemaphoreGive(xSemaphore);
  }

  if (pupilX != targetX && targetX != -1) {
    pupilX += directionX;
  } else {
    if (pupilX >= eyeWidth - pupilRadius || pupilX <= pupilRadius) {
      directionX *= -1;
    }
    pupilX += directionX;
  }
  if (pupilY != targetY && targetY != -1) {
    pupilY += directionY;
  } else {
    if (pupilY >= eyeHeight - pupilRadius || pupilY <= pupilRadius) {
      directionX = 0;
    }
  }

  counter++;
  delay(10);
}