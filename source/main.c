#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>

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
        word SP;
        word PC;
        union{
            struct{
                byte AddrBuffer;
                byte DataAddrBuffer;
            };
            word AddressBuffer;
        }Buffer;
        union{
            struct{
                byte B;
                byte C;
            };
            word BC;
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
            word HL;
        }HL;
        word IDAL; //Incrementer / Decrementer / Address Latch.
    }Registers;
}CPU;


//Reset low on the pin, then set the PC to 0000H.
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
