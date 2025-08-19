#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>
#include <string.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef unsigned char byte;
typedef char16_t word;
#define MAX_MEMORY 0xFFFF

typedef struct DataStruct{
    int x;
}ds;

char* ExampleString = "Apple is a delicious fruit, it    is also healthy and it should be recommended for consumption over other stuff like fastfood.";

void printEachWord(char* sentence, u32 currentWordIndex, u16 currentWordSize){
    if(currentWordSize == 0){ //Without this we were printing empty strings, after which there were new lines. Especially when there was a whitespace right after a special character.
        return;
    }
    printf("\n");
    for(u16 i = 0; i < currentWordSize; i++)
        printf("%c", sentence[currentWordIndex + i]);
}

void skipWhiteSpace(char* sentence, u32* currentByteIndex, u32* currentWordIndex){
    u16 currentWordSize = *currentByteIndex - *currentWordIndex;
    printEachWord(sentence, *currentWordIndex, currentWordSize);
    char currentLetter = sentence[*currentByteIndex];
    while(currentLetter == ' '){
        (*currentByteIndex)++;
        currentLetter = sentence[*currentByteIndex];
    }
    *currentWordIndex = *currentByteIndex;
}

void skipSpecialCharacter(char* sentence, u32* currentByteIndex, u32* currentWordIndex){
    u16 currentWordSize = *currentByteIndex - *currentWordIndex;
    printEachWord(sentence, *currentWordIndex, currentWordSize);
    (*currentByteIndex)++;
    *currentWordIndex = *currentByteIndex;
    skipWhiteSpace(sentence, currentByteIndex, currentWordIndex);
}

void iterateEachByte(char* sentence){
    u32* currentByteIndex = (u32*)malloc(32);
    *currentByteIndex = 0;
    u32* currentWordIndex = (u32*)malloc(32);
    *currentWordIndex = 0;
    char currentLetter = sentence[*currentByteIndex];
    while(currentLetter != '\t' && currentLetter != '\n' && currentLetter != '\0'){
        if(currentLetter == ' ')
            skipWhiteSpace(sentence, currentByteIndex, currentWordIndex);
        if(currentLetter == ',' || currentLetter == '.' || currentLetter == '&' || currentLetter == '/' || currentLetter == '(' || currentLetter == ')')
            skipSpecialCharacter(sentence, currentByteIndex, currentWordIndex);
        (*currentByteIndex)++;
        currentLetter = sentence[*currentByteIndex];
    }
    printEachWord(sentence, *currentWordIndex, (*currentByteIndex - *currentWordIndex));
}

//Making an 8085 microprocessor.
typedef struct Architecture{
    struct{
        byte Data[MAX_MEMORY];
    }Mem;
    byte IR; //Instruction register.
    byte Flags; //The 8 flags. 0th bit is CY(Carry). 2nd bit is P. 4th bit is AC. 6th bit is Z. 7th bit is S.
    byte A; //Accumulator.
    byte T; //Temporary register.
    struct{
        word SP; //16 bit address.
        word PC; //Stores 16 bit address.
        union{
            struct{
                byte AddrBuffer; //A8-A15 | MSB of memory address + 8 bits of IO address.
                byte DataAddrBuffer; //A0-A7 | Lower bits of memory address. Appears on the bus during the first clock cycle (T state) of a machine cycle. Then becomes the data bus during the second and third clock cycles.
            };
            word AddressBuffer;
        }Buffer;
        union{
            struct{
                byte B;
                byte C;
            };
            word BC; //General purpose.
        }BC;
        union{
            struct{
                byte D;
                byte E;
            };
            word DE;
        }DE;
        union{
            struct{
                byte H;
                byte L;
            };
            word HL; //Data pointer.
        }HL;
        word IDAL; //Incrementer / Decrementer / Address Latch.
    }RegisterArray;
}CPU;

/*
    About Instructions and OPCODEs:
    Instructions and | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
    So some instructions that have generic registers. Like MOV R1, R2.
    The OPCODE is generic and contains registers itself.
    For example. For The afforementioned instruciton. The OPCODE would look something like this:
    01 DDD SSS
    DDD = Destination register.
    SSS = Source register.
    The 2 byte prefixes will distinguish the "type" of instruction that it is.
    00 => Mostly Immediate instructions.
    01 => Data movement instrucitons.
    10 => ALU.
    11 => Control/Branch.
    Register|Code|Full Opcode|
    A        111  87h
    B        000  80h
    C        001  81h
    BC (M)
    D        010  82h
    E        011  83h
    DE (M)
    H        100  84h
    L        101  85h
    HL (M)   110  86h
    Remember that only movement instructions have 2 registers, so it's mostly just 5 bits for OPCODE and 3 bits for source register.
*/

    /* Hexadecimal memory to array index calculation. Remember that data is only one byte long.
       First 2 bits = Which type of OPCODE.
       Since MOV is the only one with 2 registers, we can ignore some destination register unlike this one...
    */
    /*
       Right now, what I want to do is to basically have addresses in the memory and then take a byte out of that address through memory.
       0000 = [0]
       Turns out I don't need to do anything.
     */

byte fetchOPCODE(CPU* cpu, word address){
    byte OPCODE = cpu->Mem.Data[address];
    return OPCODE;
};

CPU* initCPU(){
    CPU* temp = (CPU*)malloc(sizeof(CPU));
    /* S Z 0 AC 0 P 0 CY */
    temp->Flags;
    temp->RegisterArray.PC = 0x0000; //Reset low on the pin, then set the PC to 0000H.
    memset(temp->Mem.Data, 0, sizeof(temp->Mem.Data));
    return temp;
}

//Causes the 8085 to execute the first instruction from address 0000H.

int main(){
    struct DataStruct example;
    ds example2;
    //iterateEachByte(ExampleString);
    CPU x85;
    x85.Flags = 0x00;
    x85.Flags |= (1 << 4); //set a bit.
    printf("Decimal: %u \n", x85.Flags);
    x85.Flags &= (0 << 4); //clear a bit.
    printf("Decimal: %u \n", x85.Flags);
    x85.Flags ^= (1 << 4); //Flip a bit.
    printf("Decimal: %u \n", x85.Flags);
    x85.Flags ^= (1 << 4); //Flip a bit.
    printf("Decimal: %u \n", x85.Flags);
    printf("Decimal of the madx memory: %u", MAX_MEMORY);
    return 0;
}
