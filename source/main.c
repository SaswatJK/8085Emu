#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef char byte;

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
    byte currentLetter = sentence[*currentByteIndex];
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
    byte currentLetter = sentence[*currentByteIndex];
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

int main(){
    struct DataStruct example;
    ds example2;
    iterateEachByte(ExampleString);
    return 0;
}
