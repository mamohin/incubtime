
#include <Adafruit_GPS.h>
#include "SoftwareSerial.h"
#include "GyverTM1637.h"
#include "LCD5110_SSVS.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
int resolution = 12;
float temperature;

GyverTM1637 disp(7, 12);          // CLK, DIO
SoftwareSerial mySerial(4, 3);  // Rx Tx
// NEO-6m
Adafruit_GPS GPS(&mySerial);
LCD5110 LCD(8, 9, 10, 11); //CLK, DIN, DC, RST
extern uint8_t RusFont[];
extern uint8_t SmallFont[];
uint32_t targetTime;
uint8_t LocalDay, LocalYear, LocalMonth, hh, mm, ss, dd, hh0 = 24, mm0 = 60, ss0 = 60, dd0 = 7;
const byte zzz = 2; // пищалка
const byte blight = 14; // подсветка
const byte tu = 15; // тумблер
const byte st0 = 0; // строка день недели
const byte st1 = 8; // строка тип дня
const byte st2 = 24; // строка отзвон
const byte st3 = 32; // строка обход
const byte st4 = 40; // строка отладки
char txtDay[11];
boolean point; // состояние двоеточия
int timeo = 0, timeo0 = 0, timez = 0, timez0 = 0;
byte ttumb = 0; // предыдущее значение тумблера
int ti = 0; // время в неразрывном формате



// звонки выходные
const int zv[13] =
{
  0,
  200,
  400,
  600,
  740,
  800,
  1000,
  1200,
  1400,
  1600,
  1800,
  2000,
  2200
};

// звонки рабочие
const int zr[9] =
{
  0,
  200,
  400,
  600,
  740,
  800,
  1800,
  2000,
  2200
};

// обходы рабочие
const int br[6] =
{
  130,
  400,
  630,
  1800,
  2030,
  2300,
};

// обходы выходные
const int bv[10] =
{
  200,
  430,
  700,
  830,
  1100,
  1330,
  1600,
  1830,
  2100,
  2330,
};


static String DayOfWeek[] =
{
  "djcrhtctymt",    // 0
  "gjytltkmybr",
  "  dnjhybr  ",
  "   CHTLF   ",
  "  xtndthu  ",
  "  gznybwf  ",
  "  ce,,jnf  " };

void setup() {
    pinMode(5, INPUT);
    sensors.begin();
    sensors.getAddress(tempDeviceAddress, 0);
    sensors.setResolution(tempDeviceAddress, resolution);
    delay(1000);
    sensors.requestTemperatures();
    // Serial.begin(9600);
    GPS.begin(9600);
    targetTime = millis() + 16000;
    LCD.InitLCD(120);          //запуск LCD контраст
    LCD.setFont(RusFont);
    LCD.clrScr();
    LCD.print("Byre,xfcs", CENTER, st0);
    LCD.setFont(SmallFont);
    LCD.print("by", CENTER, st1);
    LCD.print("Andrew Mamohin", CENTER, st2);
    LCD.print("2021 v.1.1", CENTER, st3);
    LCD.setFont(RusFont);
    analogWrite(blight, 600);
    delay(5000);
    disp.clear();
    disp.brightness(7);
    disp.displayByte(0x40, 0x40, 0x40, 0x40);
    pinMode(zzz, OUTPUT);
    pinMode(blight, OUTPUT);
    pinMode(tu, INPUT);
    analogWrite(blight, 0);
    digitalWrite(zzz, HIGH);
    delay(100);
    digitalWrite(zzz, LOW);
    LCD.clrScr();
}

void loop() {
    if (targetTime < millis()) {
        GPS.read();
        if (GPS.newNMEAreceived()) {
            if (GPS.parse(GPS.lastNMEA())) {
                LocalMonth = GPS.month;
                LocalYear = GPS.year;
                LocalDay = GPS.day;
                hh = GPS.hour + 3;
                dd = dow(LocalYear, LocalMonth, LocalDay);
                if (hh > 23) {
                    hh = hh - 24;
                    dd++;
                    if (dd > 6) dd = 0;
                }
                mm = GPS.minute;
                ss = GPS.seconds;

                if ((LocalDay > 0) && (LocalDay < 32) && (LocalMonth > 0) && (LocalMonth < 13) && (LocalYear > 19) && (LocalYear < 31)
                    && (hh >= 0) && (hh < 24) && (mm >= 0) && (mm < 60) && (ss >= 0) && (ss < 60)) {

                    //  каждый день
                    if (dd != dd0) {
                        DayOfWeek[dd].toCharArray(txtDay, sizeof(txtDay));
                        LCD.setFont(RusFont);
                        LCD.invertText(1);
                        LCD.print(txtDay, CENTER, st0);
                        LCD.invertText(0);
                        LCD.print("JNPDJY", LEFT, st2);
                        LCD.print(" J<{JL", LEFT, st3);
                        LCD.setFont(SmallFont);
                        LCD.print(":", 62, st2);
                        LCD.print(":", 62, st3);
                        LCD.setFont(RusFont);
                        vrs();
                        dd0 = dd;
                    }

                    //  каждый час    
                    //    if (hh != hh0) {
                    //     hh0 = hh;
                    //    }

                    //  каждая минута
                    if (mm != mm0) {
                        disp.displayClock(hh, mm);
                        sensors.requestTemperatures();
                        delay(50);
                        temperature = sensors.getTempCByIndex(0);
                        LCD.setFont(SmallFont);
                        LCD.print(String(temperature) + "~", CENTER, st4);
                        LCD.setFont(RusFont);
                        timez = print_z();
                        if (timez != timez0) {
                            beep(500, 8);
                            timez0 = timez;
                        }
                        timeo = print_o();
//                        if (timeo != timeo0) {
//                            beep(100, 20);
//                            timeo0 = timeo;
//                        }
                        mm0 = mm;
                    }

                    //  каждая секунда
                    if (ss != ss0) {
                        if (ttumb != tumb()) {
                            vrs();
                            ttumb = tumb();
                        }
                        point = !point;
                        ss0 = ss;
                        disp.point(point);
                    }
                }
            }
        }
    }
}

int print_z() {
    byte ddd = 0;
    int t1 = 0;
    int t2 = 0;
    ti = hh * 100 + mm;
    if (vrb()) {
        for (byte i = 0; i < (sizeof(zr) / 2); i++) {
            if (ti < zr[i]) {
                ddd = i;
                break;
            }
        }
        t1 = zr[ddd] / 100;
        t2 = zr[ddd] % 100;
    }
    else {
        for (byte i = 0; i < (sizeof(zv) / 2); i++) {
            if (ti < zv[i]) {
                ddd = i;
                break;
            }
        }
        t1 = zv[ddd] / 100;
        t2 = zv[ddd] % 100;
    }
    LCD.printNumI(t1, 50, st2, 2);
    LCD.printNumI(t2, 68, st2, 2);
    if (t1 < 10) LCD.printNumI(0, 50, st2);
    if (t2 < 10) LCD.printNumI(0, 68, st2);
    return zv[ddd];
}

int print_o() {
    byte ddd = 0;
    int t1 = 0;
    int t2 = 0;
    ti = hh * 100 + mm;
    if (vrb()) {
        for (byte i = 0; i < (sizeof(br) / 2); i++) {
            if (ti < br[i]) {
                ddd = i;
                break;
            }
        }
        t1 = br[ddd] / 100;
        t2 = br[ddd] % 100;
    }
    else {
        for (byte i = 0; i < (sizeof(bv) / 2); i++) {
            if (ti < bv[i]) {
                ddd = i;
                break;
            }
        }
        t1 = bv[ddd] / 100;
        t2 = bv[ddd] % 100;
    }
    LCD.printNumI(t1, 50, st3, 2);
    LCD.printNumI(t2, 68, st3, 2);
    if (t1 < 10) LCD.printNumI(0, 50, st3);
    if (t2 < 10) LCD.printNumI(0, 68, st3);
    return bv[ddd];
}

boolean vrb() { // возвращает вых - раб (true)
    byte k = tumb();
    if (k == 3) return false;
    if (k == 5) return true;
    if ((dd > 5) || (dd < 1)) {  // вых
        return false;
    }
    else {
        return true;
    }
}

void vrs() {
    LCD.clrRow(1);
    LCD.invertText(tumb() != 6);  // если ручной выбор - инверсно
    if (vrb()) {
        LCD.print("hf,jxbq ltym", CENTER, st1);    // рабочий
    }
    else {
        LCD.print("ds[jlyjq ltym", CENTER, st1);   // выходной
    }
    LCD.invertText(0);
}

void beep(int b, byte i1) {
    for (byte i = 0; i < i1; i++) {
        digitalWrite(zzz, HIGH);
        analogWrite(blight, 0);
        delay(b);
        analogWrite(blight, 160);
        digitalWrite(zzz, LOW);
        delay(b);
    }
    delay(b);
}

byte tumb() {
    return analogRead(tu) / 100;
    // средн (авто) = 6, верх (вых) = 3, низ (раб) = 5
}

int dow(uint8_t y, uint8_t m, uint8_t d) {
    static uint8_t t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    y -= m < 3;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;    // 0 = Sunday, 1 = Monday, etc
}
