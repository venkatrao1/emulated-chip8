#include <Arduino.h>

#include "keypad.h"

void init_keypad(){
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
}

byte keycode(byte rowpin, byte colpin){
  if(colpin == 9) return rowpin+10;
  if(rowpin != 5) return ((3*rowpin)+colpin)-11;
  if(colpin == 7) return 0;
  return (colpin == 6) ? 0xA : 0xB;
}

byte get_key(){
  for(byte rowpin = 2; rowpin < 6; rowpin++){
    digitalWrite(rowpin, LOW);
    for(byte colpin = 6; colpin < 10; colpin++){
      if(digitalRead(colpin)==LOW){
        digitalWrite(rowpin, HIGH);
        return keycode(rowpin, colpin);
      }
    }
    digitalWrite(rowpin, HIGH);
  }
  return NULL_KEY;
}