#include "Control.h"

void Control::init()
{
    pinMode(LED1, OUTPUT);
    digitalWrite(LED1, HIGH);
    pinMode(LED2, OUTPUT);
    digitalWrite(LED2, HIGH);
    pinMode(LED3, OUTPUT);
    digitalWrite(LED3, HIGH);

    pinMode(SW1, INPUT_PULLUP);
    pinMode(SW2, INPUT_PULLUP);
    pinMode(SW3, INPUT_PULLUP);
    Serial.println("INIT");
}

void Control::led1(bool on)
{
    digitalWrite(LED1, on ? HIGH : LOW);
}

bool Control::led1()
{
    return digitalRead(LED1) == HIGH;
}

void Control::led2(bool on)
{
    digitalWrite(LED2, on ? HIGH : LOW);
}

bool Control::led2()
{
    return digitalRead(LED2) == HIGH;
}

void Control::led3(bool on)
{
    digitalWrite(LED3, on ? HIGH : LOW);
}

bool Control::led3()
{
    return digitalRead(LED3) == HIGH;
}