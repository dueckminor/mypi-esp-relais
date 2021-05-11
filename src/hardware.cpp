#include <Arduino.h>
#include "hardware.h"
#include "settings.h"

#ifdef ARDUINO_ESP8266_WEMOS_D1MINI
#define LED_ON_BOARD D4

uint8_t PIN[8]={
    D0,D1,D2,D3,D5,D6,D7,D8
};
#elif ARDUINO_ESP8266_ESP01
#define LED_ON_BOARD 1
uint8_t PIN[]={
};
#endif

void HardwareInitialize()
{
    // prepare LED
#ifdef LED_ON_BOARD
    pinMode(LED_ON_BOARD, OUTPUT);
    digitalWrite(LED_ON_BOARD, LOW);
#endif

    // for (int i=0; i<8; i++) {
    //     pinMode(PIN[i], OUTPUT);
    //     digitalWrite(PIN[i], HIGH);
    // }
}

void HardwareLoop()
{
}

void HardwareWrite(bool bOn)
{
    // digitalWrite(D0, bOn ? HIGH : LOW);
    // digitalWrite(D1, bOn ? HIGH : LOW);
    // digitalWrite(D2, bOn ? HIGH : LOW);
    // digitalWrite(D3, bOn ? HIGH : LOW);

    // digitalWrite(D5, bOn ? HIGH : LOW);
    // digitalWrite(D6, bOn ? HIGH : LOW);
    // digitalWrite(D7, bOn ? HIGH : LOW);
    // digitalWrite(D8, bOn ? HIGH : LOW);
}

static bool bReadWPS=false;
static bool bWPS=false;

static bool bSerial = false;

extern void HardwareSerialOn() {
#ifdef ARDUINO_ESP8266_ESP01
    if (!bSerial)
    {
        pinMode(LED_ON_BOARD, SPECIAL);
        bSerial = true;
    }
#endif
}

void HardwareLED(bool bOn)
{
#ifdef ARDUINO_ESP8266_ESP01
    if (bSerial)
    {
        pinMode(LED_ON_BOARD, OUTPUT);
        bSerial = false;
    }
    digitalWrite(LED_ON_BOARD, bOn ? LOW : HIGH);
#elif defined(LED_ON_BOARD)
    if (bReadWPS)
    {
        if (!bOn)
        {
            pinMode(LED_ON_BOARD, INPUT_PULLUP);
            bWPS = digitalRead(LED_ON_BOARD)==LOW;
            return;
        }
        pinMode(LED_ON_BOARD, OUTPUT);
    }
    digitalWrite(LED_ON_BOARD, bOn ? LOW : HIGH);
#endif
}

void HardwareDimLED(int value)
{
#ifdef LED_ON_BOARD
    analogWrite(LED_ON_BOARD, value);
#endif
}

bool HardwareReadWPS()
{
#ifdef ARDUINO_ESP8266_ESP01
    pinMode(2, INPUT_PULLUP);
    return 0==digitalRead(2);
#else
    bReadWPS = true;
    return bWPS;
#endif
}

void HardwareReadWPSDone()
{
#ifdef LED_ON_BOARD
    if (bReadWPS)
    {
        pinMode(LED_ON_BOARD, OUTPUT);
    }
    bReadWPS = false;
#endif
}