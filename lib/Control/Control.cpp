#include "Control.h"

void Control::init()
{
    pinMode(LED1, OUTPUT);
    digitalWrite(LED1, LOW);
    pinMode(LED2, OUTPUT);
    digitalWrite(LED2, LOW);
    pinMode(LED3, OUTPUT);
    digitalWrite(LED3, LOW);

    pinMode(SW1, INPUT_PULLUP);
    pinMode(SW2, INPUT_PULLUP);
    pinMode(SW3, INPUT_PULLUP);
    Serial.println("[CONTROL] INIT");
}

void Control::led1(bool on)
{
    digitalWrite(LED1, on ? LOW : HIGH);
}

bool Control::led1()
{
    return digitalRead(LED1) == LOW;
}

void Control::led2(bool on)
{
    digitalWrite(LED2, on ? LOW : HIGH);
}

bool Control::led2()
{
    return digitalRead(LED2) == LOW;
}

void Control::led3(bool on)
{
    digitalWrite(LED3, on ? LOW : HIGH);
}

bool Control::led3()
{
    return digitalRead(LED3) == LOW;
}