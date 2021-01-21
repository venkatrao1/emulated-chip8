#include <Wire.h>
#include <Arduino.h>

#define SSD1306_POWER A3

void ssd1306_init(byte port, byte brightness){
  pinMode(SSD1306_POWER, OUTPUT);
  digitalWrite(SSD1306_POWER, LOW);
  delay(50);
  digitalWrite(SSD1306_POWER, HIGH);
  delay(50);
  Wire.begin();
  Wire.setClock(400000);
  Wire.beginTransmission(port);
  Wire.write(0x00);
  Wire.write(0xAE); // turn off to reset settings
  Wire.write(0xA1); // change horizontal scan direction
  Wire.write(0xC8); // change vertical scan direction
  Wire.write(0x81); // Contrast control:
  Wire.write(brightness); // 00-FF brightness
  Wire.write(0x8D); // Change charge pump setting:
  Wire.write(0x14); // turn charge pump on
  Wire.write(0xA4); // display from RAM
  Wire.write(0x20); // change addressing mode:
  Wire.write(0x01); // vertical
  //Wire.write(0x21); // Set start and end columns:
  //Wire.write(0);    // cols 0-63
  //Wire.write(63);
  //Wire.write(0x22); // set start and end pages:
  //Wire.write(0);    // 0-3 (rows 0-32)
  //Wire.write(3);
  Wire.write(0xAF); // display on
  Wire.endTransmission();
}

void ssd1306_pixelpacket(int numbytes, byte pixels[], byte port){
  Wire.beginTransmission(port);
  Wire.write(0x40);
  int sizecounter = 1;
  for(int i = 0; i<numbytes; i++){
    if(sizecounter == 32){
      Wire.endTransmission();
      Wire.beginTransmission(port);
      Wire.write(0x40);
      sizecounter = 1;
    }
    Wire.write(pixels[i]);
    sizecounter++;
  }
  Wire.endTransmission();
}

const byte nybtobyte[] = {
  0x00, 0x03, 0x0C, 0x0F, 
  0x30, 0x33, 0x3C, 0x3F, 
  0xC0, 0xC3, 0xCC, 0xCF,
  0xF0, 0xF3, 0xFC, 0xFF
};

void ssd1306_writebuffer(byte buffer[], byte port){
  for(int c = 0; c < 256; c+=4){
    Wire.beginTransmission(port);
    Wire.write(0x40);
    for(int i = 0; i<8; i++){
      byte curbyte = buffer[c+(i&3)];
      Wire.write(nybtobyte[curbyte & 0xF]);
      Wire.write(nybtobyte[curbyte >> 4]);
    }
    Wire.endTransmission();
  }
}