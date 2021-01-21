// Wire for megaavr - Version: Latest 
#include <Wire.h>

#include "ssd1306.h"
#include "keypad.h"
#include "rom.h"
#include "font.h"


// constants for the chip8
byte memory[4096];
byte V[16]; // registers
word pc;
word I; // "index" / address register
byte timer_sound;
byte timer_delay;
word stack[16];
byte stack_pointer;
byte graphics[256];
byte graphics_tmp[256];

#define BUZZERPIN 10

void setup() {
  pinMode(BUZZERPIN, OUTPUT);
  digitalWrite(BUZZERPIN, LOW);
  ssd1306_init(0x3C, 0xFF); // TODO: erase ssd1306 ram before starting
  init_keypad();
  init_chip8();
}

void loop() {
  unsigned long curtime = millis();
  for(byte i = 0; i<2; i++){
    // handle and decrement timers
    if(timer_sound > 0){ // play sound if required
      digitalWrite(BUZZERPIN, HIGH); 
      timer_sound--;
    }
    else digitalWrite(BUZZERPIN, LOW);
    if(timer_delay > 0) timer_delay--;
    for(byte i = 0; i<9; i++) decode_execute(); // decode and execute some number of intructions (18 per frame, ~540 Hz)
  }
  ssd1306_writebuffer(graphics, 0x3C); // display graphics to screen
  long delaytime = curtime + 33 - millis();
  if(delaytime > 0) delay(delaytime);
}

void init_chip8(){
  memset(memory,0,sizeof(memory));
  memcpy_P(memory+0x200, rom_data, sizeof(rom_data));
  memcpy_P(memory, font_data, sizeof(font_data));
  memset(V,0,sizeof(V));
  memset(graphics,0,sizeof(graphics));
  I = timer_sound = timer_delay = 0;
  pc = 0x200;
  stack_pointer = -1; // should start at -1 so first call's addr gets stored at 0?
}

//Fetch Opcode 
void decode_execute(){
  word opcode = memory[pc++];
  opcode <<= 8;
  opcode |= memory[pc++]; // to reach a new address we must increment the pc by 2
  
  byte x = (opcode >> 8) & 0xF;
  byte y = (opcode >> 4) & 0xF;
  word nnn = opcode & 0xFFF;
  byte nn = opcode & 0xFF;
  byte init;

  switch (opcode >> 12) {
    case 0x0:
      switch (opcode & 0xF){
        case 0x0: // Clears the display (CLS x00E0)
          memset(graphics,0,sizeof(graphics));
          break;
    		case 0xE: // Returns from subroutine (RET x00EE)
    		  pc = stack[stack_pointer];
    		  stack_pointer--;
      		// TODO: handle stack underflow?
      		break;
      }
      break;
      
    case 0x1: // Jump to location nnn (JP x1nnn)
      pc = nnn;
      break;
      
    case 0x2: // Call subroutine at nnn. (Call x2nnn)
      stack_pointer++; 
      stack[stack_pointer] = pc;
      // TODO: Handle stack overflow?
      pc = nnn;
      break;
      
    case 0x3: // Skip next instruction if Vx = kk. (SE x3xkk)
      if(V[x] == nn) pc += 2;
      break;

    case 0x4: // Skip next instruction if Vx != kk. (SNE x4xkk)
  		if(V[x] != nn) pc += 2;
      break;
      
    case 0x5: // Skip next instruction if Vx = Vy. (SE x5xy0)
      if(V[x] == V[y]) pc += 2;
      break;
         
    case 0x6: // Set Vx = kk. (LD x6xkk)
      V[x] = nn;
      break;
         
    case 0x7: // Set Vx = Vx + kk. (ADD 7xkk)
    	V[x] += nn;
      break;
         
    case 0x8: 
      switch (opcode & 0xF) {
        case 0x0: // Set Vx = Vy
          V[x] = V[y];
          break;
          
        case 0x1: // Set Vx = Vx OR Vy (OR x8xy1)
        	V[x] |= V[y];
          break;
        
        case 0x2: // Set Vx = Vx AND Vy. (AND 8xy2)
          V[x] &= V[y];
          break;
        
        case 0x3: // Set Vx = Vx XOR Vy. (XOR 8xy3)
          V[x] ^= V[y];
          break;
        
        case 0x4: // Set Vx = Vx + Vy, set VF = carry. (ADD 8xy4)
          init = V[x];
          V[x] += V[y];
          V[0xF] = (init > V[x]) ? 1 : 0; // if the new value is smaller than the initial, carry occured
          // TODO: check above line, and borrow line below
          break;
        
        case 0x5: // Set Vx = Vx - Vy, set VF = NOT borrow. (SUB 8xy5)
          init = V[x];
          V[x] -= V[y];
          V[0xF] = (V[x] > init) ? 0 : 1; // if new value is bigger, borrow occured
          break;
        
        case 0x6: // Set Vx = Vx SHR 1. (SHR x8xy6)
          V[0xF] = V[x] & 1;
          V[x] >>= 1;
          break;
        
        case 0x7: // Set Vx = Vy - Vx, set VF = NOT borrow. (SUBN x8xy7)
          init = V[y];
          V[x] = V[y] - V[x];
          V[0xF] = (V[x] > init) ? 0 : 1; // if new value is bigger, borrow occured
          break;
        
        case 0xE: // Set Vx = Vx SHL 1.(SKL x8xyE)
          V[0xF] = V[x] >> 7;
          V[x] <<= 1;
          break;
        }
      break;
         
  	case 0x9: // Skip next instruction if Vx != Vy. (SNE x9xy0)
      if(V[x] != V[y]) pc += 2;
  	  break;     
  
  	case 0xA: // Set I = nnn. (LD xAnnn)
      I = nnn;
  	  break;
  
		case 0xB: // Jump to location nnn + V0. (JP Bnnn)
		  pc = nnn + V[0];
  	  break;
  
  	case 0xC: // Set Vx = random byte AND kk.
  	  V[x] = random(255) & nn;
      break;
        
    case 0xD: // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
      {
      //Serial.println("g");
      byte rowdat, xloc, yloc, xfinal, yfinal, tmp, numrows, pix;
      memcpy(graphics_tmp, graphics, sizeof(graphics));
      xloc = V[x];
      yloc = V[y];
      numrows = opcode & 0xF;
      for(byte row = 0; row < numrows; row++){
        rowdat = memory[I+row]; // get row data
        for(byte col = 0; col < 8; col++){ // bytes are unsigned, so this counts 7-0
          yfinal = (yloc + row) & 31;
          xfinal = (xloc + col) & 63;
          if((rowdat & bit(7-col))>0){
            //Serial.println(xfinal, HEX);
            //Serial.println(yfinal, HEX);
            //Serial.println((xfinal << 2) + (yfinal >> 3), HEX);
            graphics[(xfinal << 2) + (yfinal >> 3)] ^= bit(yfinal&7);
          }
        }
      }
      V[0xF] = 0;
      for(word i = 0; i<256; i++){ // figure out V[f]
        tmp = graphics_tmp[i] & (~graphics[i]);
        if(tmp != 0){
          V[0xF] = 1;
          break;
        }
      }
      break;
      }
        
    case 0xE:
      switch(opcode & 0xFF){
          case 0x9E: // Skip next instruction if key with the value of Vx is pressed.
            if(get_key()==V[x]) pc+=2;
            break;
          case 0xA1: // Skip next instruction if key with the value of Vx is not pressed.
            if(get_key()!=V[x]) pc+=2;
            break;
        }
      break;
        
    case 0xF:
      switch(opcode & 0xFF){
          case 0x07: // Set Vx = delay timer value. (LD xFx07)
            V[x] = timer_delay;
            break;
          
          case 0x0A: // Wait for a key press, store the value of the key in Vx. (LD xFx0A)
            {
            byte tmp = get_key();
            if(tmp==NULL_KEY) pc -= 2;
            else V[x] = tmp;
            break;
            }
          
          case 0x15: // Set delay timer = Vx. (LD xFx15)
            timer_delay = V[x];
            break;
          
          case 0x18: // Set sound timer = Vx. (LD xFx18)
            timer_sound = V[x];
            break;
          
          case 0x1E: // Set I = I + Vx. (ADD xFx1E)
            I += V[x];
            break;
          
          case 0x29: // Set I = location of sprite for digit Vx. (LD xFx29)
            I = V[x]*5;
            break;
          
          case 0x33: // Store BCD representation of Vx in memory locations I, I+1, and I+2. (LD xFx33)
            memory[I+2] = V[x] % 10;
            memory[I+1] = (V[x] / 10) % 10;
            memory[I] = V[x] / 100;
            break;
        
          case 0x55: // Store registers V0 through Vx in memory starting at location I. (LD xFx55)
            for(byte i = 0; i <= x; i++) memory[I+i]=V[i];
            break;
          
          case 0x65: //Read registers V0 through Vx from memory starting at location I. (LD xFx65)
            for(byte i = 0; i <= x; i++) V[i]=memory[I+i];
            break;
          }
      break;
  }
}