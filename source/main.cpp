#include <cstdint>
#include <windows.h>
#include <iostream>
#include <winnt.h>
#include <random>

typedef struct{
    uint64_t capacity;//capacity is in bytes, and the size of the pointer
    uint8_t* buffer;
  //uint64_t currentPtr;
    uint8_t* currentPtr; 
  //so the reason we don't use uint64_t for our top even though the pointer is going to be 64 bits is because of the fact that that will make us do typecasting everytime we use memcpy
} stack;

typedef struct{
    float mHealth;
    int mGunID;
    int mWealth;
}playerCharacter;

typedef struct{
    float mHealth;
    uint32_t mId;
}monsterInfo;

// 2 things we can do right here: 
// No 1: Make monsters have their own small stack, and make stacks inside stacks? I think that is the better thing to do because of the fact that monsters themselves will probably change in numbers?
// No 2: Make monsters live in the same entity stack
typedef struct{
    monsterInfo* aliveMonsters;
    monsterInfo* deadMonsters;
    int16_t aliveMonsterNum;
    int16_t deadMonsterNum;
}monsters;

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

playerCharacter* initCharacter(stack* stackID, int gun_ID, float health, int wealth){
    //I have to remember because I keep forgetting that the reason I can juust create a temp character and still retain the ifnormation is because push stack does a memcpy, and I know the typethat is the player struct
    playerCharacter temp;
    temp.mHealth = health;
    temp.mWealth = wealth;
    temp.mGunID = gun_ID;
    playerCharacter* bufferPos = (playerCharacter*) pushStack(stackID, sizeof(playerCharacter), &temp);
    //std::cout<<"The position of the player struct in the memory is: "<<bufferPos;
    //std::cout<<"\nThe position of thes stack after the playerstruct in the memory is: "<< (playerCharacter*)stackID->currentPtr << "\n";
    return bufferPos;
}

void takeDamage(playerCharacter* character){
    character->mHealth -= 20;
}

monsterInfo* initMonster(stack* stackID, float health, uint32_t id){
    monsterInfo tempMonst;
    tempMonst.mHealth = health;
    tempMonst.mId = id;
    monsterInfo* bufferPos = (monsterInfo*) pushStack(stackID, sizeof(monsterInfo), &tempMonst);
    return bufferPos;
}

monsters spawnMonsters(stack* stackID, float health, int32_t number){
    monsters m = {};
    m.aliveMonsters = (monsterInfo*) stackID->currentPtr;
    //std::cout<<"\n The position of the monster struct in the memory is: "<<m.aliveMonsters<<"\n";
    m.aliveMonsterNum = number;
    for(int32_t i = 0; i < number; i++){
        monsterInfo* garbageValue = initMonster(stackID, health, i);
    }
    return m;
}

void killMonster(monsters* m, int32_t id){
    int j = -1;
    monsterInfo* tempMons;
    for(uint32_t i = 0; i < m->aliveMonsterNum; i++){
        tempMons = m->aliveMonsters + i;
        j = i;
        if(tempMons->mId == id)
            break;
    }
    if(j == -1){
        std::cout<<"The monster is either already dead or the ID doesn't exist \n";
        return;
    }
    m->deadMonsterNum++;
    monsterInfo temp = {};
    temp.mId = tempMons->mId;
    memcpy(m->aliveMonsters + j, m->aliveMonsters + m->aliveMonsterNum, sizeof(monsterInfo));
    m->deadMonsters = m->aliveMonsters + m->aliveMonsterNum;
    m->deadMonsters->mId = temp.mId;
    m->aliveMonsterNum--;
}

void doMonsterDamage(monsters* m, uint32_t id){
    if(id > m->aliveMonsterNum)
        return;
    monsterInfo* currMonster = m->aliveMonsters + id; //pointer will automatically multiply ID by the size of monsterInfo
    currMonster->mHealth -= 20;
}

void waveDamage(monsters* m, uint32_t number){
    for(uint32_t i = 0; i < number; i++){
        doMonsterDamage(m, i);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 1);
        int result = dist(gen);
        if(result == 0)
            killMonster(m, i);
    }
}

void printDeadMonsters(monsters* m){
    for (uint32_t i = 0; i < m->deadMonsterNum; i++){
        std::cout<<"The id of the dead monster is: "<<(m->deadMonsters + i)->mId<<"\n";
    }
}

int main(){
  /*
  arena a1 = initArena(16000);
  std::cout<<"Arena size is: "<<a1.arenaSize;
  std::cout<<"Arena address is: "<<a1.arenaAddress;
  */
    stack entityStack = initStack(1024);
    playerCharacter* char1 = initCharacter(&entityStack, 100, 100, 100);
    std::cout<<"Character health before damage: "<<char1->mHealth;
    takeDamage(char1);
    std::cout<<"\n Character health aftee damage: "<<char1->mHealth;
    monsters initialWave = spawnMonsters(&entityStack, 250, 100);
    monsterInfo* testMonster = initialWave.aliveMonsters + 37;
    std::cout<<"\n Monster health before damage: "<<testMonster->mHealth;
    //doMonsterDamage(&initialWave, 37);
    waveDamage(&initialWave, 50);
    std::cout<<"\n Monster health after damage: "<<testMonster->mHealth;
    printDeadMonsters(&initialWave);
    return 0;
}
