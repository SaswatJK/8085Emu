#include <cstdint>
#include <windows.h>
#include <iostream>
#include <winnt.h>


//Here's what I would want to do, I would want to make a Table or something.... 
//The table stores all the page adresses... Then what I would like to do is for there to be random allocations made???

typedef struct{
  void* memAddress;
  void* currentPtr;
  uint16_t memSize;
}memory;

typedef struct{
  void* arenaAddress;
  uint64_t arenaSize;
  void* currentPtr;
}arena;


//Would it be crazy if I made a bitfield or some shit where I just write wiht '1' and the bit field is the same size as the arena itself
arena initArena(uint64_t size){
  void* pages = VirtualAlloc(0, size, MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
  arena temp;
  temp.arenaAddress = pages;
  temp.arenaSize = size;
  return temp;
}

memory allocateMem(arena* arena, void* address ,uint16_t size){
  memory temp;

  temp.memSize = size;

  return temp;
}

typedef struct{
  uint8_t* buffer;
  //uint64_t currentPtr;
  uint8_t* currentPtr;
  uint64_t capacity;
  //so the reason we don't use uint64_t for our top even though the pointer is going to be 64 bits is because of the fact that that will make us do typecasting everytime we use memcpy
} stack;

stack initStack(uint64_t size){
  uint8_t* temp = (uint8_t*) VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
  stack tempStack;
  tempStack.buffer = temp;
  tempStack.currentPtr = temp;
  tempStack.capacity = size;
  return tempStack;
}

uint8_t* pushStack(stack* currentStack, uint64_t pushSize, void* data){
  if(currentStack->currentPtr + pushSize > currentStack->buffer + currentStack->capacity){
    //I should reinforce a mechanism of either throwing out a proper error or doign memory managemtn
    std::cout<<"THE SIZE TOO BIG FOR DIS STACK!!!"<<std::endl;
    return currentStack->currentPtr;
  }
  memcpy(currentStack->currentPtr, data, pushSize);
  uint8_t* location = currentStack->currentPtr;
  currentStack->currentPtr += pushSize;
  return location;
}

typedef struct{
  int mGunID;
  float mHealth;
  int mWealth;
}playerCharacter;


playerCharacter* initCharacter(stack* stackID, int gun_ID, float health, int wealth){
  playerCharacter temp;
  temp.mHealth = health;
  temp.mWealth = wealth;
  temp.mGunID = gun_ID;
  playerCharacter* bufferPos = (playerCharacter*) pushStack(stackID , sizeof(playerCharacter), &temp);
  return bufferPos; 
}

int main(){
  /*
  arena a1 = initArena(16000);
  std::cout<<"Arena size is: "<<a1.arenaSize;
  std::cout<<"Arena address is: "<<a1.arenaAddress;
  */
  stack entityStack = initStack(1024);
  playerCharacter* char1 = initCharacter(&entityStack, 100, 100, 100);
  std::cout<<"information about char1: "<<char1->mGunID;

  return 0;  
}
