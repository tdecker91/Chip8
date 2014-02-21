//
//  main.cpp
//  Chip8
//
//  Created by Tyson Decker on 2/12/14.
//  Copyright (c) 2014 Solving Rubik. All rights reserved.
//

#include <iostream>
#include "Chip8.h"
#include "Screen.h"
#include "unistd.h"

Chip8 chip8;
Screen screen;

int main(int argc, const char * argv[])
{
    std::cout << "Chip8 Emulator\n";
    std::cout << "Made By: Tyson Decker\n";
    
    // Init the Chip8
    chip8.loadRom(argv[1]);
    chip8.printMemory();
    
    // Emulation Loop
    
    while(chip8.running)
    {
        // Emulate One Cycle
        chip8.emulateCycle();
        
        // Check Draw Flag
        if(chip8.drawFlag)
            chip8.drawScreen();
        
        // Store Keypress state
//for(int i = 0; i < 800000; i++){}
    }

    return 0;
}

