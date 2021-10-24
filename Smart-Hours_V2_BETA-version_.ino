/*
****************************************************************************************************************************************************

                                                                   Smart-Hours_V2(2.0)BETA-version


                                                                 by : vla-vi21 (inst)

  Author of libraries :  ("EncButton.h") Alex Gyver ;
  Chart - article : https://alexgyver.ru/lcd-plots-and-bars/ ;
****************************************************************************************************************************************************
*/
#include <EncButton.h> //библиотека для работы с кнокпой и при желании с энкодером(будет отдельная прошивка)
EncButton<EB_TICK, 3> enc; // просто кнопка <KEY>


#include <Wire.h> //подключаем библиотеку для подсветки дисплея
#include <LiquidCrystal_I2C.h> //подключаем библиотеку для управления дисплеем
LiquidCrystal_I2C lcd(0x27, 16, 2); //указываем параметры дисплея


#include <iarduino_RTC.h> //подключаем библиотеку для модуля реального времени
iarduino_RTC time(RTC_DS1302, 8, 6, 7); // для модуля DS1302 - RST, CLK, DAT


#include <AHT10.h>  //подключаем библиотеку для датчика влажности и температуры
AHT10 myAHT20(AHT10_ADDRESS_0X38, AHT20_SENSOR); //настраиваем датчик


#include <SPI.h> //подключение барометра
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

  byte customChar10[] = { //солнце
    B00100,
    B10101,
    B01110,
    B11111,
    B01110,
    B10101,
    B00100,
    B00000
  };

  byte customChar11[] = { //облака
    B00000,
    B00000,
    B01110,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000
  };

  byte customChar12[] = { //дождь
    B01110,
    B11111,
    B11111,
    B00000,
    B01000,
    B00010,
    B00000,
    B01000
  };

int plot_array[20];         // массив данных для графика
void setup() {

  delay(300); //задержка для часов
  Serial.begin(9600);


  lcd.init();  //инициализация подсветки дисплея (wire)
  lcd.backlight(); //инициализация дисплея
  lcd.clear();

  enc.setHoldTimeout(400);  // установка таймаута удержания кнопки

  time.begin(); //инициализация модуля реального времени
  //time.settime(0, 34, 7, 21, 10, 21, 4); // 0  сек, 34 мин, 7 часов, 21, октябрь, 2021, четверг

  bme.begin();  //инициализация барометра

  bme.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);


  initPlot(); // нужно вызывать эту функцию перед отрисовкой графика/полосы!
}

void loop() {
  static byte mode ; //переменная для счётчика нажатий
  enc.tick(); //функция отработки для кнопки(постоянно опрашивается)

  if (enc.isHolded())   lcd.backlight(); //при долгом нажатии на кнопку включаем подсветку
  if (enc.hasClicks(3))  lcd.noBacklight();//при третьем  нажатии на кнопку выключаем подсветку

  if (enc.isClick()) {        //нажатие кнопки
    if (++mode >= 10) mode = 0; //счётчик нажатий
  }
  switch (mode) { //переключение режимов
    case 0 : clearLCD(); break;
    case 1 : task0(); break;
    case 2 : clearLCD(); break;
    case 3 : task1(); break;
    case 4 : clearLCD(); break;
    case 5 : task2(); break;
    case 6 : clearLCD(); break;
    case 7 : task3(); break;
    case 8 : clearLCD(); break;
    case 9 : taskPaChart(); break;
    case 10 : clearLCD(); break;
      //case 11 : task4(); break; //*вывод высоты
      //case 12 : clearLCD(); break;
  }
}
void taskPaChart() { //функция для вывода графика изменений атмосферного давления

  float p = bme.readPressure(); //переменная для хранения данных
  p = p / 100 ;  //преобразуем Паскали в  hPa (для удобства)

  static uint32_t tmr;
  if (millis() - tmr >= 60000) { //таймер (мс)
    tmr = millis();
    // ПРИМЕРЫ С МС -В- ЧАС/МИН \\
    // 60000 МС - 1 МИНУТА
    // 1800000 МС - 30 МИНУТ
    // 3600000 МС - 1 ЧАС
    // 5760000 МС - 1,6 ЧАСА
    // 7200000 МС - 2 ЧАСА
    // 10800000 МС - 3 ЧАСА


    int valPa = map(p, 930, 1060, 10, 80);
    // Преобразуем показания для построения графика
    //(переменная датчика(функции датчика), мин. значение(датчика), макс. значение(датчика), мин. значение для графика, макс. значение для графика)


    drawPlotPa(0, 1, 16, 2, 10, 80, valPa);
    // drawPlotPa принимает аргументы (столбец, строка, ширина, высота, мин. значение, макс. значение, величина)
  }
}

void task0() { //функция для вывода время и значков примерной погоды
  lcd.createChar(10, customChar10);
  lcd.createChar(11, customChar11);
  lcd.createChar(12, customChar12);

  float p = bme.readPressure();
  p = p / 100 ;

  static uint32_t tmr;
  if (millis() - tmr >= 1000) { //таймер (мс)
    tmr = millis();

    lcd.setCursor(0, 0);
    lcd.print(time.gettime("d M Y, D"));
    lcd.setCursor(4, 1);
    lcd.print(time.gettime("H:i:s"));
    //вывод времени

    if (p < 1035 && p > 1015) { //солнечно
      lcd.setCursor(14, 1);
      //(sun);
      lcd.write(10);
    }
    else if (p < 1015 && p > 980) { //облачно
      lcd.setCursor(14, 1);
      //(sky);
      lcd.write(11);
    }
    else if (p < 980 && p > 940) { //облачно с дождём
      lcd.setCursor(14, 1);
      //(sky and rain);
      lcd.write(12);
    }
    //вывод примерной погоды (значок) БЕТА-ВЕРСИЯ

  }
}
void task1() { //функция для вывода температуры
  float t = myAHT20.readTemperature();

  static uint32_t tmr;
  if (millis() - tmr >= 5000) { //таймер (мс)
    tmr = millis();
    lcd.setCursor(2, 0);
    lcd.print("Temperature:");
    lcd.setCursor(5, 1);
    lcd.print(t);
    lcd.print(" C");
  }
}
void task2() { //функция для вывода влажности воздуха
  float H =  myAHT20.readHumidity();

  static uint32_t tmr;
  if (millis() - tmr >= 5000) { //таймер (мс)
    tmr = millis();
    lcd.setCursor(3, 0);
    lcd.print("Humidity:");
    lcd.setCursor(5, 1);
    lcd.print(H);
    lcd.print("%");
  }
}
void task3()  { //функция для вывода атмосферного давления
  float p = bme.readPressure();

  p = p / 100 ; //преобразуем Паскали в  hPa (для удобства)
  //float p = bme.readPressure();
  //p = p * 0.00750062; //преобразуем Паскали в мм.рт.ст
  static uint32_t tmr;
  if (millis() - tmr >= 5000) { //таймер (мс)
    tmr = millis();
    lcd.setCursor(3, 0);
    lcd.print("Pressure:");
    lcd.setCursor(2, 1);
    lcd.print(p);
    lcd.print(" hPa");
  }
}
/*
  void task4() {  //Функция для вывода высоты (использовать по желанию)
  float h = bme.readAltitude(1013.25);
  static uint32_t tmr;
  if (millis() - tmr >= 5000) { //таймер (мс)
    tmr = millis();
    lcd.setCursor(5, 0);
    lcd.print("Height");
    lcd.setCursor(5, 1);
    lcd.print(h);
    lcd.print(" m");
  }
  }
*/

void initPlot() {
  // необходимые символы для работы
  // создано в http://maxpromer.github.io/LCD-Character-Creator/
  byte row8[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row7[8] = {0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row6[8] = {0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row5[8] = {0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row4[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row3[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};
  byte row2[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};
  byte row1[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111};
  lcd.createChar(0, row8);
  lcd.createChar(1, row1);
  lcd.createChar(2, row2);
  lcd.createChar(3, row3);
  lcd.createChar(4, row4);
  lcd.createChar(5, row5);
  lcd.createChar(6, row6);
  lcd.createChar(7, row7);
}
void drawPlotPa(byte pos, byte row, byte width, byte height, int min_val, int max_val, int fill_val) {
  for (byte i = 0; i < width; i++) {
    plot_array[i] = plot_array[i + 1];
  }
  fill_val = constrain(fill_val, min_val, max_val);
  plot_array[width] = fill_val;
  for (byte i = 0; i < width; i++) {                  // каждый столбец параметров
    byte infill, fract;
    // найти количество целых блоков с учётом минимума и максимума для отображения на графике
    infill = floor((float)(plot_array[i] - min_val) / (max_val - min_val) * height * 10);
    fract = (infill % 10) * 8 / 10;                   // найти количество оставшихся полосок
    infill = infill / 10;
    for (byte n = 0; n < height; n++) {     // для всех строк графика
      if (n < infill && infill > 0) {       // пока мы ниже уровня
        lcd.setCursor(i, (row - n));        // заполняем полными ячейками
        lcd.write(0);
      }
      if (n >= infill) {                    // если достигли уровня
        lcd.setCursor(i, (row - n));
        if (fract > 0) lcd.write(fract);          // заполняем дробные ячейки
        else lcd.write(16);                       // если дробные == 0, заливаем пустой
        for (byte k = n + 1; k < height; k++) {   // всё что сверху заливаем пустыми
          lcd.setCursor(i, (row - k));
          lcd.write(16);
        }
        break;
      }
    }
  }
}

void clearLCD() { //функция для очистки дисплея
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}
