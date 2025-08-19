#include <stddef.h>
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

/*  Hexadecimal memory to array index calculation. Remember that data is only one byte long.
    First 2 bits = Which type of OPCODE.
    Since MOV is the only one with 2 registers, we can ignore some destination register unlike this one...
    Right now, what I want to do is to basically have addresses in the memory and then take a byte out of that address through memory.
     */

typedef enum { //Remember that HL stores Memory regardless.
    //If any of the right most 3 bits is 001 then It's memory.
    ADDR = 0b10000000,
    ADDM = 0b10100110,
    SUBR = 0b10010000,
    SUBM = 0b10010110,
    MOVRR = 0b01000000,
    MOVMR = 0b01110000,
    MOVRM = 0b01000110,
    MVIR = 0b00000000,
    MVIM = 0b00110000,
    LXIB = 0b00, //Load only register pair's BC, DH, DE or HL
    LXID = 0b11, //Load only register pair's BC, DH, DE or HL
    LXIH = 0b10, //Load only register pair's BC, DH, DE or HL
    STAXR = 0b11,
    STA = 0b1111,
    LDA = 0b110000,
    SHLD = 0b01010,
    LHLD = 0b110
}Instructions;

typedef enum {
    B_Register = 0,
    C_Register,
    D_Register,
    E_Register,
    H_Register,
    L_Register,
    HL_Register,
    A_Register,
    NONE_Register
    //I should probably add other registers here from which the enum value can be easily used to map stuff.
}RegisterName;
//Make an instruciton decoder and machine cycle encoder that uses these.
//We can basically allocate a region in memory for "Stack" that is used by stack pointer for stuff like branching.

typedef struct {
    int offset;
    int size;
    const char* name;
} structInfo;

typedef struct{
    byte Data[MAX_MEMORY];
}Mem;

//Making an 8085 microprocessor.
typedef struct Architecture{
    struct{
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
        word SP; //16 bit address.
        word PC; //Stores 16 bit address.
        word IDAL; //Incrementer / Decrementer / Address Latch.
    }RegisterArray;
    Mem* RAM;
    byte IR; //Instruction register.
    /* S Z 0 AC 0 P 0 CY */
    byte Flags; //The 8 flags. 0th bit is CY(Carry). 2nd bit is P. 4th bit is AC. 6th bit is Z. 7th bit is S.
    byte A; //Accumulator.
    byte T; //Temporary register.
    Instructions currentInstruction;
}CPU;

static const structInfo registerTable[] = {
    //We need to make an enum of all these registers that start with 0 and are placed in this lookup table at the same 'offset' or 'enumeration'.
    // BC union - individual bytes
    {offsetof(CPU, RegisterArray.BC.B), 1, "B"},
    {offsetof(CPU, RegisterArray.BC.C), 1, "C"},
    // DE union - individual bytes
    {offsetof(CPU, RegisterArray.DE.D), 1, "D"},
    {offsetof(CPU, RegisterArray.DE.E), 1, "E"},
    // HL union - individual bytes
    {offsetof(CPU, RegisterArray.HL.H), 1, "H"},
    {offsetof(CPU, RegisterArray.HL.L), 1, "L"},
    // HL union - as word
    {offsetof(CPU, RegisterArray.HL.HL), 2, "HL"},
    // Accumulator - individual byte
    {offsetof(CPU, A), 1, "A"},
    // Word registers
    {offsetof(CPU, RegisterArray.SP), 2, "SP"},
    {offsetof(CPU, RegisterArray.PC), 2, "PC"},
    // Buffer union - individual bytes
    {offsetof(CPU, RegisterArray.Buffer.AddrBuffer), 1, "AddrBuffer"},
    {offsetof(CPU, RegisterArray.Buffer.DataAddrBuffer), 1, "DataAddrBuffer"},
    // Buffer union - as word
    {offsetof(CPU, RegisterArray.Buffer.AddressBuffer), 2, "AddressBuffer"},
    // BC union - as word
    {offsetof(CPU, RegisterArray.BC.BC), 2, "BC"},
    // DE union - as word
    {offsetof(CPU, RegisterArray.DE.DE), 2, "DE"},
    // Other word register
    {offsetof(CPU, RegisterArray.IDAL), 2, "IDAL"},
    // Individual byte registers
    {offsetof(CPU, IR), 1, "IR"},
    {offsetof(CPU, Flags), 1, "Flags"},
    {offsetof(CPU, T), 1, "T"}
};

byte fetchOPCODE(CPU* cpu, word address){
    byte OPCODE = cpu->RAM->Data[address];
    return OPCODE;
};

RegisterName parseRegister(byte rightShiftedByte){
    RegisterName tempName =  NONE_Register; //Default Value.
    byte cleanedByte = rightShiftedByte & 0b00000111;
    tempName = cleanedByte;
    return tempName;
}

void parseOPCODE(byte OPCODE, Instructions* currentInstruction){
    byte result = OPCODE | (0b00000000);
    //If any of the results from the RegisterName is HL/M then remember to load only the instructions that have the M in it.
    //if (result == 0)
}

void writeMemory(CPU* cpu, word address, byte d){
    cpu->RAM->Data[address] = d;
    return;
}

void setRegister(CPU* cpu, RegisterName r, byte d){
    structInfo* currentInfo = &registerTable[r];
    char* writePtr = (char*) cpu;
    writePtr += currentInfo->offset;
    *writePtr = d;
    return;
    //I haven't thought of how to write to HL???? or DE or BC.
}

void ADD_OP(CPU* cpu, RegisterName r){
    structInfo* currentInfo = &registerTable[r];
    char* writePtr = (char*) cpu;
    writePtr += currentInfo->offset;
    if(currentInfo->size == 1){
        cpu->A = cpu->A + *writePtr;
    }
}

CPU* initCPU(){
    CPU* temp = (CPU*)malloc(sizeof(CPU));
    temp->Flags;
    temp->RegisterArray.PC = 0x0000; //Reset low on the pin, then set the PC to 0000H.
    Mem* tempRam = (Mem*) malloc(sizeof(Mem));
    temp->RAM = tempRam;
    memset(temp->RAM->Data, 0, sizeof(temp->RAM->Data));
    return temp;
}

//Causes the 8085 to execute the first instruction from address 0000H.

int main(){
    struct DataStruct example;
    ds example2;
    //iterateEachByte(ExampleString);
    CPU* x8085 = initCPU();
    writeMemory(x8085, 0x0000, 0b10010011);
    CPU x85;
    byte code = fetchOPCODE(x8085, 0x0000);
    printf("The opcode is: %u \n", code);
    setRegister(x8085, C_Register, 32);
    setRegister(x8085, A_Register, 1);
    printf("The value of A is: %u \n", x8085->A);
    ADD_OP(x8085, C_Register);
    printf("The value of A is: %u \n", x8085->A);
    x85.Flags = 0x00;
    x85.Flags |= (1 << 4); //set a bit.
    printf("Decimal: %u \n", x85.Flags);
    x85.Flags &= (0 << 4); //clear a bit.
    printf("Decimal: %u \n", x85.Flags);
    x85.Flags ^= (1 << 4); //Flip a bit.
    printf("Decimal: %u \n", x85.Flags);
    x85.Flags ^= (1 << 4); //Flip a bit.
    printf("Decimal: %u \n", x85.Flags);

    return 0;
}
