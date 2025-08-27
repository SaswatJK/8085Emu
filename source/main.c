#include <corecrt_wconio.h>
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
    Since MOV is the only one with 2 registers, we can ignore some destination register unlike this one... Right now, what I want to do is to basically have addresses in the memory and then take a byte out of that address through memory.
     */
typedef enum { //Remember that HL stores Memory regardless.
    //When we are giving addresses to as operands. The first byte will always be the lower order address byte and the second byte will be the higher order address byte in memory. So an instruction like LHLD 4050H in memory will be: OPCODE + 50H + 40H.
    ADDR = 0b10000000,
    ADDM = 0b10000110, //Last 3 bits = 110->HL->Memory.
    SUBR = 0b10010000,
    SUBM = 0b10010110,
    MOVRR = 0b01000000,
    MOVMR = 0b01110000,
    MOVRM = 0b01000110,
    //00RRR110
    MVIR = 0b00000110, //Move to register R, immediate data.
    MVIM = 0b00110110, //Move to memory M, stored in HL, immediate data.
    //Load transfer data to register from memory.
    //00RRR001
    LXIB = 0b00000001, //Load only register pair's BC, DH, DE or HL
    LXID = 0b00010001, //Load only register pair's BC, DH, DE or HL
    LXIH = 0b00100001, //Load only register pair's BC, DH, DE or HL
    //Store transfer data to memory from register.
    //00RRR010
    STAXB = 0b00000010, //Load into A, data from BC
    STAXD = 0b00010010, //Load into A, data from DE
    //Since we are only going to be taking memory from either BC or DE, if we get the register code for HL or A...
    STA = 0b00110010,
    LDA = 0b00111010,
    SHLD = 0b00100010, //Load into Memory, the contents from HL register pair. Starting from the L register -> MemAddr and H register -> MemAddr + 0x0001.
    LHLD = 0b00101010, //Load into the HL register pair, the contents of the memory location. Starting from MemAddr -> L register and MemAddr + 0x001 -> H register.
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
    SP_Register,
    PC_Register,
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
                byte L;
                byte H;
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

//NOTE: B, D, H hold the MSB while the respective pairs hold the LSB. 0bHHHHLLLL. We can only store to the BC and DE register pairs as pairs from the stack operations (PUSH/POP).
//NOTE: Look at the timing diagrams and think of the way to incoroprate cycles and if it's even practical to implement it.

static const structInfo registerTable[] = {
    //The array of struct Infos is stored in the array in the same order as the enum of registers. So, when we use B register, that's the 'first' member in the enum, it will take the first element of this array, so it will take informatin about the offset in the struct, of the register, it's size, and it's name (which is mostly for our own ease).
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

void writeMemory(CPU* cpu, word address, byte d){
    cpu->RAM->Data[address] = d;
    return;
}

void readMemory(CPU* cpu, word address, RegisterName r){ //Read memory to a register.
    //Read either the address itself or the contents.
    const structInfo* currentInfo = &registerTable[r];
    if(currentInfo->size == 2){
        char* writePtr = (char*) cpu;
        writePtr += currentInfo->offset;
        *(word*)writePtr = address;
        return;
    }
    else{
        byte* writePtr = (byte*) cpu;
        writePtr += currentInfo->offset;
        *writePtr = cpu->RAM->Data[address];
        return;
    }
}

void incrementPC(CPU* cpu, uint8_t bytes){
    cpu->RegisterArray.PC += bytes;
}

//Set register needs to be replaced with MOVR and MVI 0xHH
//Shoudl I also get rid of the memory functions with just normal instructions?
void setRegister(CPU* cpu, RegisterName r, byte d){
    const structInfo* currentInfo = &registerTable[r];
    char* writePtr = (char*) cpu;
    writePtr += currentInfo->offset;
    *writePtr = d;
    return;
}

byte getRegister(CPU* cpu, RegisterName r){
    const structInfo* currentInfo = &registerTable[r];
    char* readPtr = (char*) cpu;
    readPtr += currentInfo->offset;
    return *readPtr;
}

//NOTE: I should use the setRegister, getRegister, WriteMemory, and readMemory functions instead of doing things directly/implicitly in the functions.
//NOTE: Programs with OPCODE and data of the program should be stored in a special region of memory itself. This is called the ROM? Remember the Program Counter points to the next address to execute. The PC points to a series of contiguous address continually if the program doesn't branch, or there's no interrupts, and if there is, it will point to the new address where a new program will start.
//NOTE: When branching takes place, the return address is pushed to the Stack. When a return instruciton is executed, the Stack address is popped, and basically the PC's new pointer is to that popped value. So, the SP makes it easier to do recursive stuff.

void LHLD_OP(CPU* cpu, RegisterName r);

void LXI_OP(CPU* cpu, RegisterName r){
    const structInfo* destInfo = &registerTable[r];
    char* destPtr = (char*) cpu;
    destPtr = destPtr + destInfo->offset;
    word* destReg = (word*) destPtr;
    //Can also write to BC and DH?? OR can it only be written by stack? I am confused.
    *destReg = (cpu->RAM->Data[cpu->RegisterArray.PC + 2] << 8) | cpu->RAM->Data[cpu->RegisterArray.PC + 1]; //This way I can do H and L and get it in correct order.
    incrementPC(cpu, 3);
    return;
}

void MVI_OP(CPU* cpu, RegisterName r){
    const structInfo* currentInfo = &registerTable[r];
    char* registerPtr = (char*) cpu;
    registerPtr += currentInfo->offset;
    if(currentInfo->size == 1){
        *registerPtr = cpu->RAM->Data[(cpu->RegisterArray.PC + 1)];
        incrementPC(cpu, 2);
        return;
    }
    //increment by 2 bytes for this, 3 bytes for the MVI_M
    cpu->RAM->Data[*(word*)registerPtr] = cpu->RAM->Data[(cpu->RegisterArray.PC + 1)]; //Write to a memory which is stored in the HL register, the contents from the instruction operand.
    incrementPC(cpu, 2);
}

void MOV_OP(CPU* cpu, RegisterName src, RegisterName dest){
    const structInfo* srcInfo = &registerTable[src];
    const structInfo* destInfo = &registerTable[dest];
    char* srcPtr = (char*) cpu;
    srcPtr = srcPtr + srcInfo->offset;
    char* destPtr = (char*) cpu;
    destPtr = destPtr + destInfo->offset;
    byte srcData;
    srcData = (srcInfo->size == 1) ? *srcPtr : cpu->RAM->Data[*(word*)srcPtr];
    if (destInfo->size == 1){
        *destPtr = srcData;
        incrementPC(cpu, 1);
        return;
    }
    cpu->RAM->Data[*(word*)destPtr] = srcData;
    incrementPC(cpu, 1);
    return;
}

void ADD_OP(CPU* cpu, RegisterName r){
    const structInfo* currentInfo = &registerTable[r];
    char* registerPtr = (char*) cpu;
    printf("Yes the addition function has been called! \n");
    registerPtr += currentInfo->offset;
    if(currentInfo->size == 1){
        cpu->A = cpu->A + *registerPtr;
        incrementPC(cpu, 1);
        return;
    }
        cpu->A = cpu->A + cpu->RAM->Data[*(word*)registerPtr];
        incrementPC(cpu, 1);
        return;
}

byte fetchOPCODE(CPU* cpu, word address){
    byte OPCODE = cpu->RAM->Data[address];
    return OPCODE;
};

RegisterName parseRegisterBinary(byte rightShiftedByte){
    RegisterName tempName =  NONE_Register; //Default Value.
    byte cleanedByte = rightShiftedByte & 0b00000111;
    if(cleanedByte > NONE_Register)
        return tempName;
    tempName = cleanedByte;
    return tempName;
}

//After every byte, the PC changes.
//NOTE:I want to add hot-reload where the .asm file is being read to memory, and a simple keystroke will print out all register values.

//Parse OPCODE can either parse OPCODE through like passing the byte itself, or it can do it, by looking at the memory location of the PC itself.
void decodeOPCODE(CPU* cpu){
    byte OPCODE = fetchOPCODE(cpu, cpu->RegisterArray.PC);
    byte result = OPCODE & (0b11000000);
    //If any of the results from the RegisterName is HL/M then remember to load only the instructions that have the M in it.
    switch (result) {
        case 0: { //Immediate instructions.
            byte destinationByte = (OPCODE >> 3);
            RegisterName destinationReg = parseRegisterBinary(destinationByte);
            printf("The destination register is: %u \n", destinationReg);
            //Since they're all immediate insturctinos, their source register bits (the 3 LSBs) can be used to differentiate between them.
            byte immediateMask = 0b00000111;
            byte maskedByte = OPCODE & immediateMask;
            if (maskedByte == 1){ //LXI
                LXI_OP(cpu, destinationReg);
                return;
            }
            else if (maskedByte == 6){ //MVI
                MVI_OP(cpu, destinationReg);
                return;
            }
            break;
        }
        case 64:{ //Data movement.
            byte destinationByte = (OPCODE >> 3);
            RegisterName destinationReg = parseRegisterBinary(destinationByte);
            printf("The destination register is: %u \n", destinationReg);
            byte sourceByte = OPCODE;
            RegisterName sourceReg = parseRegisterBinary(sourceByte);
            printf("The source register is: %u \n", sourceReg);
            MOV_OP(cpu, sourceReg, destinationReg);
            break;
        }
        case 128:{ //ALU.
            byte sourceByte = (OPCODE); //No need to shift bits to the right because there's no destination except for the Accumulator.
            RegisterName sourceReg = parseRegisterBinary(sourceByte);
            printf("The ALU case has been confirmed! \n");
            byte addSubMask = 0b00111000;
            byte maskedByte = addSubMask & OPCODE;
            if(maskedByte == 0){ //It's addition. Remember we don't have to check for anomalies because the parser will not output bad OPCODEs.
                ADD_OP(cpu, sourceReg);
                break;
            }
            printf("The source register is: %u \n", sourceReg);
            break;
        }
        case 255: //Branch/Control
            break;
    }
    return;
}

void runProgram(CPU* cpu, word programAddress){
    cpu->RegisterArray.PC = programAddress;
    while(cpu->RegisterArray.PC != 0x0107){
        decodeOPCODE(cpu);
    }
    //Problem is I don't have a quit instruction.
    return;
}

CPU* initCPU(){
    CPU* temp = (CPU*)malloc(sizeof(CPU));
    /* S Z 0 AC 0 P 0 CY */
    temp->Flags = 0b11010101; //All flags raised?
    temp->RegisterArray.PC = 0x0000; //Reset low on the pin, then set the PC to 01000H.
    temp->RegisterArray.SP = 0x00FF; //Pointing towards the top of the stack.
    Mem* tempRam = (Mem*) malloc(sizeof(Mem));
    temp->RAM = tempRam;
    memset(temp->RAM->Data, 0, sizeof(temp->RAM->Data)); //Setting everything in the memory to be 0 to prevent weird errors.
    return temp;
}

//Causes the 8085 to execute the first instruction from address 0000H.

int main(){
    //iterateEachByte(ExampleString);
    CPU* x8085 = initCPU();
    setRegister(x8085, C_Register, 32);
    setRegister(x8085, A_Register, 1);
    printf("The value of A is: %u \n", x8085->A);
    writeMemory(x8085, 0x0100, 0b00110001); //LXI HL
    writeMemory(x8085, 0x0101, 0xF0);
    writeMemory(x8085, 0x0102, 0xF0);
    writeMemory(x8085, 0x0103, 0b00110110); //MVI M
    writeMemory(x8085, 0x0104, 32);
    writeMemory(x8085, 0x0105, 0b01001110); //MOV A, HL
    writeMemory(x8085, 0x0106, 0b10000001); //ADD A, C
    //We would want to read form a file to a program.
    runProgram(x8085, 0x0100);
    printf("The value of A is: %u \n", x8085->A);

    /*x85.Flags = 0x00;
    x85.Flags |= (1 << 4); //set a bit.
    x85.Flags &= (0 << 4); //clear a bit.
    x85.Flags ^= (1 << 4); //Flip a bit.
    x85.Flags ^= (1 << 4); //Flip a bit.*/

    return 0;
}
