#ifndef CONTROL_H
#define CONTROL_H

#include "Arduino.h"

#define CONFIG_CE_GPIO 4
#define CONFIG_CSN_GPIO 5
#define CONFIG_MISO_GPIO 19
#define CONFIG_MOSI_GPIO 23
#define CONFIG_SCK_GPIO 18
#define LED1 33
#define LED2 25
#define LED3 0
#define SW1 26
#define SW2 27
#define SW3 14

class Control
{
private:
    /* data */
public:
    static void init();
    static void led1(bool _flag);
    static bool led1();
    static void led2(bool _flag);
    static bool led2();
    static void led3(bool _flag);
    static bool led3();
};

#endif // CONTROL_H