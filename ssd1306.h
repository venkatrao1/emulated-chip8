// Wire for megaavr - Version: Latest 
#include <Wire.h>

#ifndef ssd1306_h
#define ssd1306_h

void ssd1306_init(byte port, byte brightness);

void ssd1306_pixelpacket(int numbytes, byte pixels[], byte port);

void ssd1306_writebuffer(byte buffer[], byte port);

#endif