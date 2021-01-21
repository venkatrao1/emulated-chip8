#include <Arduino.h>

#ifndef keypadchip8_h
#define keypadchip8_h

#define NULL_KEY 0xFF

byte get_key();

void init_keypad();

#endif