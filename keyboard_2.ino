#include <Adafruit_NeoPixel.h>
#include <Keyboard.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PCF8575.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// НАСТРОЙКА РАСШИРИТЕЛЯ PCF8575
Adafruit_PCF8575 pcf;

#define ROWS 4
#define COLS 11

#define LED1_PIN 3
#define LED2_PIN 4

byte ledPins[] = {LED1_PIN, LED2_PIN};
const int LED_COUNT = 2;

// Строки (выходы на PCF8575)
byte rowPins[ROWS] = {3, 2, 1, 0};

// Столбцы (входы на PCF8575)
byte colPins[COLS] = {14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4};

// ============= ОПРЕДЕЛЕНИЯ КЛАВИШ =============
#define KEY_ESC          0x29      // Escape
#define KEY_TAB          0x2B      // Tab
#define KEY_BACKSPACE    0x2A      // Backspace
#define KEY_ENTER        0x28      // Enter (Return)
#define KEY_LEFT_SHIFT   0xE1      // Left Shift
#define KEY_LEFT_CTRL    0xE0      // Left Control
#define KEY_LEFT_ALT    0x40// 0xE2      // Left Alt
#define KEY_PRINT_SCREEN 0x46      // Print Screen
#define KEY_SPACE        0x2C      // Space

// ВАШИ СПЕЦИАЛЬНЫЕ КЛАВИШИ
#define KEY_NONE         0x00
#define KEY_FN           0x90  // Слой 2 (зажать)
#define KEY_LAY          0x91  // Слой 1 (зажать)

// Виртуальные коды для переключения состояний
#define KEY_NUMLOCK      0x93  // Код для переключения Num Lock
#define KEY_CAPSLOCK     0x94  // Код для переключения Caps Lock  
#define KEY_SCROLLLOCK   0x95  // Код для переключения Scroll Lock

// Коды для отправки
//define KEY_RETURN       0x28

// Кириллические коды
#define KEY_CYR_N  0xD0
#define KEY_CYR_G  0xD1
#define KEY_CYR_SH 0xD2
#define KEY_CYR_SCH 0xD3
#define KEY_CYR_HARD 0xD4
#define KEY_CYR_R  0xD5
#define KEY_CYR_O  0xD6
#define KEY_CYR_L  0xD7
#define KEY_CYR_D  0xD8
#define KEY_CYR_ZH 0xD9
#define KEY_CYR_E  0xDA
#define KEY_CYR_T  0xDB
#define KEY_CYR_SOFT 0xDC
#define KEY_CYR_B  0xDD
#define KEY_CYR_YU 0xDE

#define KEY_F1      0xC2
#define KEY_F2      0xC3
#define KEY_F3      0xC4
#define KEY_F4      0xC5
#define KEY_F5      0xC6
#define KEY_F6      0xC7
#define KEY_F7      0xC8
#define KEY_F8      0xC9
#define KEY_F9      0xCA
#define KEY_F10     0xCB
#define KEY_F11     0xCC
#define KEY_F12     0xCD

#define KEY_UP      0xDA
#define KEY_DOWN    0xD9
#define KEY_LEFT    0xD8
#define KEY_RIGHT   0xD7
#define KEY_HOME    0x4A  // Home
#define KEY_END     0x4D  // End
#define KEY_PAGE_UP 0x4B  // Page Up
#define KEY_PAGE_DOWN 0x4E  // Page Down
#define KEY_INSERT  0x49  // Insert
#define KEY_DELETE  0x4C  // Delete


// ============= СОСТОЯНИЯ ВИРТУАЛЬНЫХ КНОПОК =============
bool numLockState = false;    // Num Lock (превращает qwertyuiop в 1234567890)
bool capsLockState = false;   // Caps Lock (большие буквы без Shift)
bool scrollLockState = false; // Scroll Lock (прокрутка)

// ============= РАСКЛАДКИ =============
char keyLayers[3][ROWS][COLS] = {
  // СЛОЙ 0: Обычная работа (латиница)
  {
    {KEY_ESC, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
    {KEY_TAB, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', KEY_BACKSPACE},
    {KEY_LEFT_SHIFT, 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', KEY_ENTER},
    {KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_PRINT_SCREEN, KEY_NONE, '-', KEY_SPACE, '/', '-', '\'', KEY_LAY, KEY_FN}
  },
  
  // СЛОЙ 1: Кириллица (при зажатии KEY_LAY)
  {
    {'^', '&', '*', '(', ')', '0', '0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0', '0', 0xD0, 0xD1, 0xD2, 0xD3, 0xD4},
    {'0', '0', '0', '0', '0', '0', 0xD5, KEY_UP, 0xD7, 0xD8, 0xD9},
    {'0', '0', '0', '0','0', '0', KEY_LEFT, KEY_DOWN, KEY_RIGHT,  '0', '0'}
  },
  
  // СЛОЙ 2: Кастомный (при зажатии KEY_FN)
  {
    {KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
    {KEY_NUMLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10},
    {KEY_CAPSLOCK, '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
    {KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_PRINT_SCREEN, '0', '0', '0', '0', '0', '0',  KEY_LAY, KEY_FN}
  }
};

// ============= ПЕРЕМЕННЫЕ СОСТОЯНИЯ =============
byte currentLayer = 0;
bool lastKeyState[ROWS][COLS] = {0};

// Состояния зажатых клавиш-модификаторов
bool layer1Pressed = false;  // KEY_LAY зажата
bool layer2Pressed = false;  // KEY_FN зажата

// Для отслеживания, какую клавишу отправили
byte lastSentKey[ROWS][COLS] = {0};

// ============= ФУНКЦИИ СВЕТОДИОДОВ =============
void bothLedsOn() {
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
}

void bothLedsOff() {
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
}

// ============= ОПРЕДЕЛЕНИЕ АКТИВНОГО СЛОЯ =============
byte getActiveLayer() {
  if (layer2Pressed) return 2;      // FN зажата → слой 2
  if (layer1Pressed) return 1;      // LAY зажата → слой 1
  return 0;                          // Иначе слой 0
}

// ============= ПРИМЕНЕНИЕ NUM LOCK К СИМВОЛУ =============
char applyNumLock(char keyCode) {
  if (!numLockState) return keyCode;
  
  // Преобразуем qwertyuiop в 1234567890
  switch(keyCode) {
    case 'q': return '1';
    case 'w': return '2';
    case 'e': return '3';
    case 'r': return '4';
    case 't': return '5';
    case 'y': return '6';
    case 'u': return '7';
    case 'i': return '8';
    case 'o': return '9';
    case 'p': return '0';
    default: return keyCode;
  }
}

// ============= ПРИМЕНЕНИЕ CAPS LOCK К СИМВОЛУ =============
char applyCapsLock(char keyCode) {
  if (!capsLockState) return keyCode;
  
  // Преобразуем буквы в верхний регистр
  if (keyCode >= 'a' && keyCode <= 'z') {
    return keyCode - 32;  // 'a' -> 'A'
  }
  return keyCode;
}

// ============= ОБНОВЛЕНИЕ ДИСПЛЕЯ =============
void updateDisplayLocks() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print(F("Num : "));
  display.println(numLockState ? "On " : "Off");
  display.setCursor(0, 20);
  display.print(F("Caps: "));
  display.println(capsLockState ? "On " : "Off");
  display.setCursor(0, 35);
  display.print(F("Scrl: "));
  display.println(scrollLockState ? "On " : "Off");
  display.display();
}

// ============= SETUP =============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Сканирование I2C
  Serial.println("I2C Scanner");
  for(byte addr=1; addr<127; addr++) {
    Wire.beginTransmission(addr);
    if(Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(addr, HEX);
    }
  }
  
  // Инициализация PCF8575
  Wire.begin();
  pcf.begin(0x20);
  Serial.println("PCF8575 initialized");
  
  // Настройка строк (выходы)
  for (int r = 0; r < ROWS; r++) {
    pcf.pinMode(rowPins[r], OUTPUT);
    pcf.digitalWrite(rowPins[r], HIGH);
  }
  
  // Настройка столбцов (входы с подтяжкой)
  for (int c = 0; c < COLS; c++) {
    pcf.pinMode(colPins[c], INPUT_PULLUP);
  }
  
  // Настройка светодиодов
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  
  bothLedsOn();
  delay(500);
  bothLedsOff();
  
  // Инициализация клавиатуры
  Keyboard.begin();
  delay(500);
  
  // Инициализация OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED INIT FAILED!");
  } else {
    updateDisplayLocks();  // Показываем начальное состояние
    delay(1000);
  }
  
  Serial.println("Keyboard ready!");
}

// ============= ОСНОВНОЙ ЦИКЛ =============
void loop() {
  scanMatrix();
  delay(5);
}

// ============= СКАНИРОВАНИЕ МАТРИЦЫ =============
void scanMatrix() {
  for (int row = 0; row < ROWS; row++) {
    pcf.digitalWrite(rowPins[row], LOW);
    delayMicroseconds(100);
    
    for (int col = 0; col < COLS; col++) {
      bool isPressed = (pcf.digitalRead(colPins[col]) == LOW);
      
      if (isPressed != lastKeyState[row][col]) {
        lastKeyState[row][col] = isPressed;
        
        if (isPressed) {
          handleKeyPress(row, col);
        } else {
          handleKeyRelease(row, col);
        }
      }
    }
    
    pcf.digitalWrite(rowPins[row], HIGH);
  }
}

// ============= ОБРАБОТКА НАЖАТИЯ =============
void handleKeyPress(int row, int col) {
  byte keyCode = keyLayers[0][row][col];
  
  Serial.print("PRESS: R");
  Serial.print(row);
  Serial.print(" C");
  Serial.print(col);
  Serial.print(", Code=0x");
  Serial.print(keyCode, HEX);
  
  // Обработка клавиш-модификаторов (LAY и FN) - берем из слоя 0
  if (keyCode == KEY_LAY) {
    layer1Pressed = true;
    Serial.println(" (LAY PRESSED - Layer 1 active)");
    digitalWrite(LED1_PIN, HIGH);
    return;
  }
  
  if (keyCode == KEY_FN) {
    layer2Pressed = true;
    Serial.println(" (FN PRESSED - Layer 2 active)");
    digitalWrite(LED2_PIN, HIGH);
    return;
  }
  
  // Получаем актуальный слой с учетом зажатых модификаторов
  byte activeLayer = getActiveLayer();
  byte actualKeyCode = keyLayers[activeLayer][row][col];
  
  // ============= ОБРАБОТКА ВИРТУАЛЬНЫХ КНОПОК (ПО ACTUAL KEYCODE) =============
  if (actualKeyCode == KEY_NUMLOCK) {
    numLockState = !numLockState;
    Serial.print(" -> Num Lock: ");
    Serial.println(numLockState ? "ON" : "OFF");
    updateDisplayLocks();
    return;
  }
  
  if (actualKeyCode == KEY_CAPSLOCK) {
    capsLockState = !capsLockState;
    Serial.print(" -> Caps Lock: ");
    Serial.println(capsLockState ? "ON" : "OFF");
    updateDisplayLocks();
    return;
  }
  
  if (actualKeyCode == KEY_SCROLLLOCK) {
    scrollLockState = !scrollLockState;
    Serial.print(" -> Scroll Lock: ");
    Serial.println(scrollLockState ? "ON" : "OFF");
    updateDisplayLocks();
    return;
  }
  
  // Применяем Num Lock (только для слоя 0 и только для букв qwertyuiop)
  if (activeLayer == 0) {
    actualKeyCode = applyNumLock(actualKeyCode);
  }
  
  // Применяем Caps Lock (только для слоя 0)
  if (activeLayer == 0) {
    actualKeyCode = applyCapsLock(actualKeyCode);
  }
  
  Serial.print(" -> Active Layer=");
  Serial.print(activeLayer);
  Serial.print(", Actual Code=0x");
  Serial.println(actualKeyCode, HEX);
  
  // Отправка в компьютер (если не пустая клавиша)
  if (actualKeyCode != KEY_NONE ){//&& actualKeyCode != 0 && actualKeyCode != '0') {
    sendToPC(actualKeyCode, true);
    lastSentKey[row][col] = actualKeyCode;
  }
  
  // Включаем светодиод-вспышку
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  delay(20);
  if (!layer1Pressed && !layer2Pressed) {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
  }
}

void handleKeyRelease(int row, int col) {
  byte keyCode = keyLayers[0][row][col];
  
  Serial.print("RELEASE: R");
  Serial.print(row);
  Serial.print(" C");
  Serial.print(col);
  Serial.print(", Code=0x");
  Serial.println(keyCode, HEX);
  
  // Обработка отпускания модификаторов
  if (keyCode == KEY_LAY) {
    layer1Pressed = false;
    Serial.println(" -> LAY RELEASED - Layer 0 active");
    digitalWrite(LED1_PIN, LOW);
    return;
  }
  
  if (keyCode == KEY_FN) {
    layer2Pressed = false;
    Serial.println(" -> FN RELEASED - Layer 0 active");
    digitalWrite(LED2_PIN, LOW);
    return;
  }
  
  // Получаем актуальный код из активного слоя для проверки виртуальных кнопок
  byte activeLayer = getActiveLayer();
  byte actualKeyCode = keyLayers[activeLayer][row][col];
  
  // Виртуальные кнопки не отправляем на компьютер
  if (actualKeyCode == KEY_NUMLOCK || actualKeyCode == KEY_CAPSLOCK || actualKeyCode == KEY_SCROLLLOCK) {
    return;
  }
  
  // Отпускаем клавишу, которая была отправлена
  byte sentKeyCode = lastSentKey[row][col];
  if (sentKeyCode != KEY_NONE && sentKeyCode != 0) {
    sendToPC(sentKeyCode, false);
    lastSentKey[row][col] = 0;
  }
}

// ============= ОТПРАВКА КОМПЬЮТЕРУ =============
void sendToPC(byte keyCode, bool isPress) {
  if (isPress) {
    // Сначала проверяем специальные клавиши (включая стрелки)
    switch(keyCode) {
      case KEY_ENTER:
        Keyboard.press(KEY_RETURN);
        break;
      case KEY_BACKSPACE:
        Keyboard.press(KEY_BACKSPACE);
        break;
      case KEY_ESC:
        Keyboard.press(KEY_ESC);
        break;
      case KEY_TAB:
        Keyboard.press(KEY_TAB);
        break;
      case KEY_LEFT_SHIFT:
        Keyboard.press(KEY_LEFT_SHIFT);
        break;
      case KEY_LEFT_CTRL:
        Keyboard.press(KEY_LEFT_CTRL);
        break;
      case KEY_LEFT_ALT:
        Keyboard.press(KEY_LEFT_ALT);
        break;
      case KEY_PRINT_SCREEN:
        Keyboard.press(KEY_PRINT_SCREEN);
        break;
      case KEY_SPACE:
        Keyboard.press(' ');
        break;
      // ============= СТРЕЛКИ =============
      case KEY_LEFT:
        Keyboard.press(KEY_LEFT);
        break;
      case KEY_RIGHT:
        Keyboard.press(KEY_RIGHT);
        break;
      case KEY_UP:
        Keyboard.press(KEY_UP);
        break;
      case KEY_DOWN:
        Keyboard.press(KEY_DOWN);
        break;
      // ============= НАВИГАЦИЯ =============
      case KEY_HOME:
        Keyboard.press(KEY_HOME);
        break;
      case KEY_END:
        Keyboard.press(KEY_END);
        break;
      case KEY_PAGE_UP:
        Keyboard.press(KEY_PAGE_UP);
        break;
      case KEY_PAGE_DOWN:
        Keyboard.press(KEY_PAGE_DOWN);
        break;
      case KEY_INSERT:
        Keyboard.press(KEY_INSERT);
        break;
      case KEY_DELETE:
        Keyboard.press(KEY_DELETE);
        break;
      // ============= ФУНКЦИОНАЛЬНЫЕ =============
      case KEY_F1: case KEY_F2: case KEY_F3: case KEY_F4:
      case KEY_F5: case KEY_F6: case KEY_F7: case KEY_F8:
      case KEY_F9: case KEY_F10: case KEY_F11: case KEY_F12:
        Keyboard.press(keyCode);
        break;
      default:
        // Обычные символы (ASCII)
        if (keyCode >= 32 && keyCode < 127) {
          Keyboard.press(keyCode);
        }
        // Кириллица
        else if (keyCode >= 0xD0 && keyCode <= 0xDE) {
          Keyboard.press(keyCode);
        }
        // Кастомные клавиши - не отправляем
        else if (keyCode >= 0x90 && keyCode <= 0x9F) {
          return;
        }
        break;
    }
  } else {
    // ОТПУСКАНИЕ
    switch(keyCode) {
      case KEY_ENTER:
        Keyboard.release(KEY_ENTER);
        break;
      case KEY_BACKSPACE:
        Keyboard.release(KEY_BACKSPACE);
        break;
      case KEY_ESC:
        Keyboard.release(KEY_ESC);
        break;
      case KEY_TAB:
        Keyboard.release(KEY_TAB);
        break;
      case KEY_LEFT_SHIFT:
        Keyboard.release(KEY_LEFT_SHIFT);
        break;
      case KEY_LEFT_CTRL:
        Keyboard.release(KEY_LEFT_CTRL);
        break;
      case KEY_LEFT_ALT:
        Keyboard.release(KEY_LEFT_ALT);
        break;
      case KEY_PRINT_SCREEN:
        Keyboard.release(KEY_PRINT_SCREEN);
        break;
      case KEY_SPACE:
        Keyboard.release(' ');
        break;
      case KEY_LEFT:
        Keyboard.release(KEY_LEFT);
        break;
      case KEY_RIGHT:
        Keyboard.release(KEY_RIGHT);
        break;
      case KEY_UP:
        Keyboard.release(KEY_UP);
        break;
      case KEY_DOWN:
        Keyboard.release(KEY_DOWN);
        break;
      case KEY_HOME:
        Keyboard.release(KEY_HOME);
        break;
      case KEY_END:
        Keyboard.release(KEY_END);
        break;
      case KEY_PAGE_UP:
        Keyboard.release(KEY_PAGE_UP);
        break;
      case KEY_PAGE_DOWN:
        Keyboard.release(KEY_PAGE_DOWN);
        break;
      case KEY_INSERT:
        Keyboard.release(KEY_INSERT);
        break;
      case KEY_DELETE:
        Keyboard.release(KEY_DELETE);
        break;
      case KEY_F1: case KEY_F2: case KEY_F3: case KEY_F4:
      case KEY_F5: case KEY_F6: case KEY_F7: case KEY_F8:
      case KEY_F9: case KEY_F10: case KEY_F11: case KEY_F12:
        Keyboard.release(keyCode);
        break;
      default:
        if (keyCode >= 32 && keyCode < 127) {
          Keyboard.release(keyCode);
        }
        else if (keyCode >= 0xD0 && keyCode <= 0xDE) {
          Keyboard.release(keyCode);
        }
        else if (keyCode >= 0x90 && keyCode <= 0x9F) {
          return;
        }
        break;
    }
  }
}