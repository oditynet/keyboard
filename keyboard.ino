#include <Adafruit_NeoPixel.h>
#include <Keyboard.h>


#define LED_PIN    10   // Пин для светодиода
//Adafruit_NeoPixel strip(1, LED_PIN, NEO_GRB + NEO_KHZ800);

#define ROWS 5  // 5 строк!
#define COLS 6  // 6 столбцов!

// Строки (выходы, активируются LOW)
byte rowPins[ROWS] = {8, 9, 15, 14, 16};

// Столбцы (входы с подтяжкой к +5V)
byte colPins[COLS] = {2, 3, 4, 5, 6, 7};

// Раскладка 5x6 (ВАША РАБОЧАЯ МАТРИЦА)
char keyLayers[3][ROWS][COLS] = {
  // СЛОЙ 0: Латинская раскладка
  {
    // Колонки: 0    1    2    3    4    5
    {'6', '7', '8', '9', '0', '['},      // Строка 0
    {'y', 'u', 'i', 'o', 'p', ']'},      // Строка 1
    {'h', 'j', 'k', 'l', ';', '\''},     // Строка 2
    {'n', 'm', ',', '.', 0x84, 0x08},    // Строка 3 (FN, Backspace)
    {0x0D, 0x81, 0x82, ' ', ' ', ' '}    // Строка 4 (Enter, Ctrl, Alt)
  },
  
  // СЛОЙ 1: Кириллица
  {
    {'^', '&', '*', '(', ')', ' '},
    {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, ' '}, // н,г,ш,щ,ъ
    {0xD5, 0xD6, 0xD7, 0xD8, 0xD9, ' '}, // р,о,л,д,ж
    {0xDA, 0xDB, 0xDC, 0xDD, 0x84, 0x08}, // э,т,ь,б,FN,Backspace
    {0x0D, 0x81, 0x82, ' ', ' ', ' '}    // Enter, Ctrl, Alt
  },
  
  // СЛОЙ 2: Пустой
  {
    {' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', 0x84, 0x08},    // FN, Backspace
    {0x0D, 0x81, 0x82, ' ', ' ', ' '}    // Enter, Ctrl, Alt
  }
};

// ================================
// ПЕРЕМЕННЫЕ СОСТОЯНИЯ
// ================================
byte currentLayer = 0;
bool lastKeyState[ROWS][COLS] = {0};
unsigned long lastFnPressTime = 0;
byte fnPressCount = 0;
#define FN_TIMEOUT 300

// Коды клавиш
#define KEY_BACKSPACE 0x08
#define KEY_ENTER     0x0D
#define KEY_FN        0x84
#define KEY_CTRL      0x81
#define KEY_ALT       0x82

// Кириллические коды
#define KEY_CYR_N  0xD0  // н
#define KEY_CYR_G  0xD1  // г
#define KEY_CYR_SH 0xD2  // ш
#define KEY_CYR_SCH 0xD3 // щ
#define KEY_CYR_HARD 0xD4 // ъ
#define KEY_CYR_R  0xD5  // р
#define KEY_CYR_O  0xD6  // о
#define KEY_CYR_L  0xD7  // л
#define KEY_CYR_D  0xD8  // д
#define KEY_CYR_ZH 0xD9  // ж
#define KEY_CYR_E  0xDA  // э
#define KEY_CYR_T  0xDB  // т
#define KEY_CYR_SOFT 0xDC // ь
#define KEY_CYR_B  0xDD  // б
#define KEY_CYR_YU 0xDE  // ю

void setup() {
  // Настройка строк (выходы)
  for (int r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  
  // Настройка столбцов (входы с подтяжкой)
  for (int c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  // Инициализация светодиода
//strip.begin();
  //strip.setBrightness(255);     // МАКСИМАЛЬНАЯ ЯРКОСТЬ
  //strip.setPixelColor(0, 255, 255, 255); // белый (макс)
  //strip.show();
  pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, HIGH); 
  
  
  Keyboard.begin();
  delay(500);
  
  Serial.begin(115200);

  Serial.println("Press any key to test...");
}

void loop() {
  scanMatrix();
  delay(5);
}

void scanMatrix() {
  for (int row = 0; row < ROWS; row++) {
    digitalWrite(rowPins[row], LOW);
    delayMicroseconds(50);
    
    for (int col = 0; col < COLS; col++) {
      bool isPressed = (digitalRead(colPins[col]) == LOW);
      
      if (isPressed != lastKeyState[row][col]) {
        lastKeyState[row][col] = isPressed;
        
        if (isPressed) {
          handleKeyPress(row, col);

          unsigned long pressStart = millis();
          while (digitalRead(colPins[col]) == LOW) {
            if (millis() - pressStart > 300) {  // Если держат больше 300 мс
              handleLongPress(row, col);
              break;
            }
          }

        } else {
          handleKeyRelease(row, col);
        }
      }
    }
    
    digitalWrite(rowPins[row], HIGH);
  }
}
void handleLongPress(int row, int col) {
  byte keyCode = keyLayers[currentLayer][row][col];
  if (keyCode != KEY_FN && keyCode != 0 && keyCode != ' ') {
    sendToPC(keyCode, true);  // Зажимаем
    
    // Автоповтор
    unsigned long lastRepeat = millis();
    while (digitalRead(colPins[col]) == LOW) {  // Пока клавиша нажата
      if (millis() - lastRepeat > 40) {         // Каждые 40 мс
        sendToPC(keyCode, true);                  // Отправляем повтор
        lastRepeat = millis();
      }
    }
  }
}

void handleKeyPress(int row, int col) {
  byte keyCode = keyLayers[currentLayer][row][col];
  
  // Включаем светодиод
  //strip.setPixelColor(0, strip.Color(255, 255, 255));
  //strip.show();
  
  // Отладочный вывод
  Serial.print("PRESS: Layer=");
  Serial.print(currentLayer);
  Serial.print(", R");
  Serial.print(row);
  Serial.print(" C");
  Serial.print(col);
  Serial.print(", Code=0x");
  Serial.print(keyCode, HEX);
  
  // Расшифровка
  if (keyCode == KEY_FN) {
    Serial.print(" (FN)");
  } else if (keyCode == KEY_BACKSPACE) {
    Serial.print(" (BACKSPACE)");
  } else if (keyCode == KEY_ENTER) {
    Serial.print(" (ENTER)");
  } else if (keyCode == KEY_CTRL) {
    Serial.print(" (CTRL)");
  } else if (keyCode == KEY_ALT) {
    Serial.print(" (ALT)");
  } else if (keyCode >= 0xD0 && keyCode <= 0xDE) {
    Serial.print(" (CYRILLIC)");
  } else if (keyCode >= 32 && keyCode < 127) {
    Serial.print(" ('");
    Serial.print((char)keyCode);
    Serial.print("')");
  }
  Serial.println();
  
  // Обработка FN (ИСПРАВЛЕНО!)
  if (keyCode == KEY_FN) {
    //unsigned long now = millis();
    fnPressCount++;
  /*if (now - lastFnPressTime > FN_TIMEOUT) {
    fnPressCount = 0;  // Сброс в 0 после паузы
  } else {
    fnPressCount++;    // Увеличиваем только быстрые нажатия
  }*/
  //lastFnPressTime = now;

  // Циклическое переключение: 0 → 1 → 2 → 0
  if (fnPressCount == 1) {
    currentLayer = 1;
    Serial.println(">>> SWITCHED TO LAYER 1 (CYRILLIC)");
   // strip.setPixelColor(0, strip.Color(0, 255, 0));
  } else if (fnPressCount == 2) {
    currentLayer = 2;
    Serial.println(">>> SWITCHED TO LAYER 2 (CUSTOM)");
   // strip.setPixelColor(0, strip.Color(0, 0, 255));
  } else if (fnPressCount == 3) {
    currentLayer = 0;
    fnPressCount = 0;  // Сбрасываем счётчик
    Serial.println(">>> SWITCHED TO LAYER 0 (LATIN)");
   // strip.setPixelColor(0, strip.Color(255, 0, 0));
  }
   // strip.show();
    delay(50);
   // strip.setPixelColor(0, strip.Color(0, 0, 0));
    //strip.show();
    
    return;
  }
  
  // Отправка в компьютер
  if (keyCode != 0 && keyCode != ' ') {
    sendToPC(keyCode, true);
  }
}

void handleKeyRelease(int row, int col) {
  byte keyCode = keyLayers[currentLayer][row][col];
  
  // Выключаем светодиод
  //strip.setPixelColor(0, strip.Color(0, 0, 0));
 // strip.show();
  
  if (keyCode == KEY_FN) return;
  
  Serial.print("RELEASE: R");
  Serial.print(row);
  Serial.print(" C");
  Serial.println(col);
  
  if (keyCode != 0 && keyCode != ' ') {
    sendToPC(keyCode, false);
  }
}

void sendToPC(byte keyCode, bool isPress) {
  //Serial.write(0xFF);
 // Serial.write(isPress ? 1 : 0);
  //Serial.write(keyCode);
  
  if (isPress) {
    switch(keyCode) {
      case KEY_ENTER:
        Keyboard.write(KEY_RETURN);
        break;
      case KEY_BACKSPACE:
        Keyboard.write(KEY_BACKSPACE);
        break;
      case KEY_CTRL:
        Keyboard.press(KEY_RIGHT_CTRL);
        break;
      case KEY_ALT:
        Keyboard.press(KEY_RIGHT_ALT);
        break;
      default:
        if (keyCode >= 32 && keyCode < 127) {
          Keyboard.press(keyCode);
        }
        else if (keyCode >= 0xD0 && keyCode <= 0xDE) {
          Keyboard.press(keyCode);
        }
        break;
    }
  } else {
    switch(keyCode) {
      case KEY_CTRL:
      case KEY_ALT:
        Keyboard.releaseAll();
        break;
      default:
        // Отпускаем клавишу
        Keyboard.release(keyCode);
        break;
    }
  }
}