//
//  Chip8.cpp
//  Chip8
//
//  Created by Tyson Decker on 2/12/14.
//  Copyright (c) 2014 Tyson Decker. All rights reserved.
//

#include "Chip8.h"
#include "time.h"
#include "unistd.h"
#include <iostream>
#include <fstream>
#include <iomanip>

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8()
{
    // Empty
}

Chip8::~Chip8()
{
    // Empty
}

void Chip8::init()
{
    // Initialize registers and memory once
    pc		= 0x200;		// Program counter starts at 0x200 (Start adress program)
	opcode	= 0;			// Reset current opcode
	I		= 0;			// Reset index register
	sp		= 0;			// Reset stack pointer
    running = true;
    
    // Clear memory
	for(int i = 0; i < 4096; ++i)
		memory[i] = 0;
    
    // Load fontset
	for(int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];
    
    srand (time(NULL));
}

void Chip8::loadRom(const char* file)
{
    init();
    std::cout << "Loading " << file << std::endl;
    
    FILE * romFile;
    romFile = fopen (file,"rb");
    
    if (romFile == NULL)
    {
        fputs ("File error",stderr);
        exit (1);
    }
    
    // Check file size
	fseek(romFile , 0 , SEEK_END);
	long lSize = ftell(romFile);
	rewind(romFile);
    std::cout << "Filesize: " << (int)lSize << std::endl;
    
    // Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs ("Memory error", stderr);
        exit (1);
	}
    
    // Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, romFile);
	if (result != lSize)
	{
		fputs("Reading error",stderr);
		exit (1);
	}
    
    // Print File
    for(int i = 0; i < lSize; ++i)
    {
        memory[i + 0x200] = buffer[i];
    }
    
    fclose (romFile);
}

void Chip8::printMemory()
{
    for(int i = 0; i < 4096; i +=2)
    {
        unsigned short opcode = ((memory[i] & 0x00FF) << 8 ) | (memory[i+1] & 0x00FF);
        std::cout << std::setw(4) << std::hex << std::setfill('0') << opcode << " ";
    }
    std::cout << std::endl;
}

void Chip8::drawScreen()
{
    //std::cout << std::string( 100, '\n' );
    system("clear");
    std::string tempString = "";
    for(int i = 0; i < 32; i++)
    {
        for(int j = 0; j < 64; j++)
        {
            if(gfx[i * 64 + j] == 1)
                tempString += "X";
            else
                tempString +=  " ";
        }
        tempString += "\n";
    }
    
    std::cout << tempString;
    drawFlag = false;
}

void Chip8::emulateCycle()
{
    // Fetch Opcode
    unsigned short opcode = ((memory[pc] & 0x00FF) << 8 ) | (memory[pc+1] & 0x00FF);
    //std::cout << std::hex << opcode << "\t";
    
    //std::cout << pc << std::endl;
   
    // Decode Opcode
    switch(opcode & 0xF000)
    {
        case 0x0000:
			switch(opcode & 0x000F)
            {
                case 0x0000: // 0x00E0: Clears the screen
                    
                    for(int i = 0; i < 2048; ++i)
                        gfx[i] = 0x0;
                    
                    drawFlag = true;
                    pc += 2;
                    //printf ("Clear the screen\n");
                    break;
                
                case 0x000E: // 0x00EE: Returns from subroutine
                    --sp;			// 16 levels of stack, decrease stack pointer to prevent overwrite
                    pc = stack[sp];	// Put the stored return address from the stack back into the program counter
                    pc += 2;		// Don't forget to increase the program counter!
                    //printf ("Returned from a subroutine\n");
                    break;
                
                default:
                    printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }
            break;
            
        case 0x1000:    // 1NNN	- Jumps to address NNN.
        {
            //std::cout << std::dec << "Jumping to address " << (opcode & 0x0FFF) << std::endl;
            pc = (opcode & 0x0FFF);
        }
            break;
            
        case 0x2000:    // 2NNN	- Calls subroutine at NNN.
        {
            stack[sp] = pc;			// Store current address in stack
			++sp;					// Increment stack pointer
			pc = (opcode & 0x0FFF);	// Set the program counter to the address at NNN
        }
            break;
            
        case 0x3000:    // 3XNN - Skips the next instruction if VX equals NN.
        {
            if(registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
            {
                //std::cout << std::dec << "Skipping when pc = " << pc << std::endl;
                pc += 4;
            }
            else
            {
                //std::cout << std::dec << "Not skipping when pc = " << pc << std::endl;
                pc += 2;
            }
        }
            break;
            
        case 0x4000:    // 4XNN - Skips the next instruction if VX doesn't equal NN.
        {
            if(registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
            {
                //printf("Skipping instruction\n");
                pc += 4;
            }
            else
            {
                //printf("Not skipping instruction\n");
                pc += 2;
            }
        }
            break;
            
        case 0x6000:    // 6XNN - Sets register X to NN
        {
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            //std::cout << std::dec << "Setting register V" << ((opcode & 0x0F00) >> 8) << " to " << (opcode & 0x00FF) << std::endl;
            pc += 2;
        }
            break;
            
        case 0x7000:    // 7XNN	- Adds NN to VX.
        {
            registers[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            //std::cout << std::dec << "Adding " << (opcode & 0x00FF) << " to V" << ((opcode & 0x0F00) >> 8) << std::endl;
            pc += 2;
        }
            break;
            
        case 0x8000: // 8XY?
        {
            switch(opcode & 0x000F)
            {
                case 0x0000:   // Sets VX to the value of VY.
                {
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4];
                    //std::cout << std::dec << "Setting V" << ((opcode & 0x0F00) >> 8) << " to the value in V" << ((opcode & 0x00F0) >> 4) << std::endl;
                    pc += 2;
                }
                    break;
                    
                case 0x0001:
                {
                    
                }
                    break;
                    
                case 0x0002:    // 8XY2 - Sets VX to VX and VY.
                {
                    //printf("Setting V%X to V%X & V%X\n", (opcode & 0x0F00) >> 8,(opcode & 0x0F00) >> 8,(opcode & 0x00F0) >> 4);
                    registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4];
                    pc += 2;
                }
                    break;
                    
                case 0x0003:
                {
                    
                }
                    break;
                    
                case 0x0004:    // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
                {
                    
                    if(registers[(opcode & 0x00F0) >> 4] > (0xFF - registers[(opcode & 0x0F00) >> 8]))
						registers[0xF] = 1; //carry
					else
						registers[0xF] = 0;
                    
					registers[(opcode & 0x0F00) >> 8] += registers[(opcode & 0x00F0) >> 4];
					pc += 2;
                }
                    break;
                    
                case 0x0005:    // VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                {
                    
                    if(registers[(opcode & 0x00F0) >> 4] > registers[(opcode & 0x0F00) >> 8])
						registers[0xF] = 0; // there is a borrow
					else
						registers[0xF] = 1;
                    
					registers[(opcode & 0x0F00) >> 8] -= registers[(opcode & 0x00F0) >> 4];
					pc += 2;
                }
                    break;
                    
                case 0x0006:
                {
                    
                }
                    break;
                    
                case 0x0007:
                {
                    
                }
                    break;
                    
                case 0x000E:
                {
                    
                }
                    break;
            }
        }
            break;
        case 0xA000:    // ANNN - Sets I to NNN
        {
            //std::cout << std::dec << "Setting Index register to " << ((opcode & 0x0FFF)) << std::endl;
            I = (opcode & 0x0FFF);
            pc += 2;
        }
            break;
        case 0xC000:    // CXNN	- Sets VX to a random number and NN.
        {
            unsigned char randNumber = ((rand() % 0xFF) & (opcode & 0x00FF));
            //std::cout << std::dec << "Setting V" << ((opcode & 0x0F00) >> 8) << " to random " << (int)randNumber << std::endl;
            registers[(opcode & 0x0F00) >> 8] = randNumber;
            pc += 2;
        }
            break;
        case 0xD000:    // DXYN - Draws a sprite at coordinate (VX, VY), Height of N
        {
            //std::cout << std::dec << "Drawing pixel at (V" << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4) << ") with a height of " << ((opcode) & 0x000F) << std::endl;
            unsigned short x = registers[(opcode & 0x0F00) >> 8];
			unsigned short y = registers[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;
            
			registers[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++)
				{
					if((pixel & (0x80 >> xline)) != 0)
					{
						if(gfx[(x + xline + ((y + yline) * 64))] == 1)
						{
							registers[0xF] = 1;
						}
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}
            
			drawFlag = true;
			pc += 2;
        }
            break;
            
        case 0xE000:
        {
            switch(opcode & 0x00FF)
            {
				case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
					if(key[registers[(opcode & 0x0F00) >> 8]] != 0)
						pc += 4;
					else
						pc += 2;
                    break;
                    
				case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
					if(key[registers[(opcode & 0x0F00) >> 8]] == 0)
						pc += 4;
					else
						pc += 2;
                    break;
                    
				default:
					printf ("Unknown opcode [0xE000]: 0x%X\n", opcode);
			}
        }
            break;
        case 0xF000:
        {
            switch(opcode & 0x00FF)
            {
                case 0x0007:        // FX07 - Sets VX to the value of the delay timer.
                {
                    //printf("Setting V%X to %X\n", ((opcode & 0x0F00) >> 8), delay_timer);
                    registers[((opcode & 0x0F00) >> 8)] = delay_timer;
                    pc += 2;
                }
                    break;
                    
                case 0x000A: break;
                    
                case 0x0015:        // FX15 - Sets the delay timer to VX.
                {
                    //printf("Setting delay timer to %X\n", ((opcode & 0x0F00) >> 8));
                    delay_timer = ((opcode & 0x0F00) >> 8);
                    pc += 2;
                }
                    break;
                    
                case 0x0018:        // FX18	- Sets the sound timer to VX.
                {
                    //printf("Setting sound timer to V%X\n", (opcode & 0x0F00) >> 8);
                    sound_timer = registers[(opcode & 0x0F00) >> 8];
                    pc += 2;
                }
                    break;
                case 0x001E:        // FX1E	- Adds VX to I
                {
                    //printf("Adding V%X to I\n", (opcode & 0x0F00) >> 8);
                    if(I + registers[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
						registers[0xF] = 1;
					else
						registers[0xF] = 0;
                    
					I += registers[(opcode & 0x0F00) >> 8];
					pc += 2;
                }
                    break;
                case 0x0029:        // FX29 - Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                {
                    //printf("Setting I %X\n", ((opcode & 0x0F00) >> 8));
                    I = registers[((opcode & 0x0F00) >> 8)] * 0x05;
                    pc += 2;
                }
                    break;
                case 0x0033:
                {
                    //std::cout << std::dec << "0x0F00: " << ((opcode & 0x0F00) >> 8) << std::endl;
                    //std::cout << "Setting memroy[ " << I << " ] = " << (registers[(opcode & 0x0F00) >> 8] / 100) << std::endl;
                    //std::cout << "Setting memroy[ " << I + 1 << " ] = " << ((registers[(opcode & 0x0F00) >> 8] / 10) % 10) << std::endl;
                    //std::cout << "Setting memroy[ " << I + 2 << " ] = " << ((registers[(opcode & 0x0F00) >> 8] % 100) % 10) << std::endl;
                    memory[I]     = registers[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (registers[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (registers[(opcode & 0x0F00) >> 8] % 100) % 10;
					pc += 2;
                }
                    break;
                case 0x0055: break;
                case 0x0065:    // FX65 - Fills V0 to VX with values from memory starting at address I.
                {
                    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
                    {
                        //printf("Filling V%d with memory[%X]\n", i, I+i);
                        registers[i] = memory[I + i];
                    }
                    // On the original interpreter, when the operation is done, I = I + X + 1.
					I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                }
                    break;
            }
        }
            break;
            
        default:
        {
            //std::cout << "Unrecognized opcode [ " << std::hex << opcode << " ]" << std::endl;
            running = false;
        }
            break;
    }
    // Execute Opcode
    
    // Update timers
    // Update timers
	if(delay_timer > 0)
		--delay_timer;
    
	if(sound_timer > 0)
	{
		if(sound_timer == 1)
        {
			//printf("BEEP!\n");
        }
		--sound_timer;
	}
}

