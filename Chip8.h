//
//  Chip8.h
//  Chip8
//
//  Created by Tyson Decker on 2/12/14.
//  Copyright (c) 2014 Solving Rubik. All rights reserved.
//

//
//  Chip8 Memory Map
//
//  0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
//  0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
//  0x200-0xFFF - Program ROM and work RAM
//

#ifndef __Chip8__Chip8__
#define __Chip8__Chip8__

#include <iostream>

class Chip8
{
public:
    Chip8();
    ~Chip8();
    
    bool drawFlag;
    bool running;

    // Other Methods
    void init();
    void loadRom(const char*);
    void emulateCycle();
    void printMemory();
    void drawScreen();
    
private:
    unsigned short opcode;          // Current opcode
    unsigned char memory[4096];     // Memory (size = 4k)
    unsigned char registers[16];    // V-regs (V0 - VF)
    unsigned short I;               // Index Register
    unsigned short pc;              // Program Counter
    unsigned char  gfx[64 * 32];	// Total amount of pixels: 2048
    
    unsigned short stack[16];       // Stack (16 levels)
    unsigned short sp;              // Stack pointer
    
    unsigned char delay_timer;
    unsigned char sound_timer;
    
    unsigned char key[16];          // Input Buttons (16 different keys)
};

#endif  //defined(__Chip8__Chip8__)
