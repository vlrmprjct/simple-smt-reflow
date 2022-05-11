/*
 * ATTINY84 SMT REFLOW STATION
 * Version v1.0
 *
 * Input 240 V~
 *
 * ATTINY84 Pinout
 *             +---  ---+
 *         VCC | 1   14 | GND
 *      10 PB0 | 2   13 | PA0 0
 *       9 PB1 | 3   12 | PA1 1
 * (RST)   PB3 | 4   11 | PA2 2
 *       8 PB2 | 5   10 | PA3 3
 *       7 PA7 | 6    9 | PA4 4  (SCK)
 *(MOSI) 6 PA6 | 7    8 | PA5 5  (MISO)
 *             +--------+
 */

#include <Waveshare_LCD1602_RGB.h>
#include <Wire.h>
#include <ezButton.h>

#define MAX_TEMP 140
#define DELAY_TEMP 15

#define PT1000 0
#define RELAY  1
#define BUZZER 2
#define BUTTON 3

#define LOOP_STATE_STOPPED 0
#define LOOP_STATE_STARTED 1
#define TEMP_REACHED 1

Waveshare_LCD1602_RGB lcd(16, 2);

ezButton button(BUTTON);

byte heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
};

byte block[8] = {
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

byte celsius[8] = {
    0b00111,
    0b00101,
    0b00111,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

byte flash[8] = {
    0b01010,
    0b10101,
    0b01010,
    0b10101,
    0b01010,
    0b10101,
    0b00000,
    0b11111
};

int raw = 0;
int Vin = 5;
float Vout = 0;
float R1 = 10000;
float R2 = 0;
float buffer = 0;
float offset = 0;
float temp = 0;
char buff[10];
int loopState = LOOP_STATE_STOPPED;
int tempReach = 0;

void setup() {
    pinMode(BUZZER, OUTPUT);
    pinMode(RELAY, OUTPUT);
    pinMode(BUTTON, INPUT);

    digitalWrite(RELAY, LOW);
    loopState = LOOP_STATE_STOPPED;

    lcd.init();
    lcd.customSymbol(0, heart);
    lcd.customSymbol(1, block);
    lcd.customSymbol(2, celsius);
    lcd.customSymbol(3, flash);
    lcd.setRGB(0, 255, 50);
    lcd.setCursor(6, 0);
    lcd.send_string("INIT");
    lcd.setCursor(0, 1);
    lcd.send_string("");

    delay(2000);
    lcd.clear();

    tone(BUZZER, 1760, 100);
    delay(120);
    tone(BUZZER, 1760, 100);
    delay(120);
    tone(BUZZER, 1865, 100);
}

void loop() {

    button.loop();

    raw = smoothValue(analogRead(PT1000));

    if (button.isPressed()) {
        tone(BUZZER, 1760, 100);
        if (loopState == LOOP_STATE_STOPPED) {
            loopState = LOOP_STATE_STARTED;
        }
        else {
            loopState = LOOP_STATE_STOPPED;
        }
    }

    if(raw) {
        buffer = raw * Vin;
        Vout = (buffer) / 1024.0;
        buffer = (Vin / Vout) - 1;
        R2 = R1 * buffer;

        temp = getPlatinumRTD(R2, 1000) - offset;

        lcd.setCursor(0, 0);
        lcd.send_string("TEMP: ");
        lcd.send_string(dtostrf(temp, 3, 0, buff));
        lcd.write_char((unsigned char)2);
        lcd.send_string("C  ");
    }


    // PROCESS STARTED
    if (loopState == LOOP_STATE_STARTED) {
        lcd.setCursor(0, 1);
        if (temp > MAX_TEMP && (temp < (MAX_TEMP + DELAY_TEMP))) {
            digitalWrite(RELAY, HIGH);
            lcd.send_string("RELAY ON STEADY");
            lcd.write_char((unsigned char)3);
        } else if (temp < MAX_TEMP) {
            digitalWrite(RELAY, HIGH);
            lcd.send_string("RELAY ON       ");
            lcd.write_char((unsigned char)3);
        } else {
            digitalWrite(RELAY, LOW);
            lcd.send_string("RELAY OFF       ");
        }
    }

    // PROCESS STOPPED
    if (loopState == LOOP_STATE_STOPPED) {
        digitalWrite(RELAY, LOW);
        loopState = LOOP_STATE_STOPPED;
        lcd.setCursor(0, 1);
        lcd.send_string("OFF  >>>   START");
    }
}
