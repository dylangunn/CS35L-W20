#include <stdio.h>
#include <stdlib.h>

//Compare two frobnicated strings
//Return 1 if 'a' is greater, 0 if equal, -1 if less
//void* as arguments, casted to *(char const**) so work with qsort
int frobcmp(const void *a, const void *b);

//Double memory size of memory pointed to and memory size variable
void* doubleMemory(void* ptr, int* size);

//If program encounters any error (input, output, memory allocation),
//report to stderr and exit with status 1
int failure();

#define DEFAULT_SIZE 8

int main() {
    //List sizing
    int sizeL = 0;
    int memSizeL = DEFAULT_SIZE;
    char** wordBuff = (char**) malloc(memSize);
    //malloc failure
    if (wordBuff == NULL)
        return failure();

    char c = (char) getchar();
    //Give more space if necessary
    while (c != EOF) {
        if (((size + 1) * sizeof(char*)) >= memSize) {
            char** temp = (char**) doubleMemory(wordBuff, &memSize);
            if (temp == NULL)
                return failure();
            else
                wordBuff = temp;
        }
        
        //Memory sizing
        int sizeW = 0;
        int memSizeW = DEFAULT_SIZE;

        char* word = (char*) malloc(memSizeW);
        if (word == NULL) {
            free(wordBuff);
            return failure();
        }

        while (c != ' ' && c != EOF) {
            //Give room for space and newline
            if (((sizeW + 2) * sizeof(char)) >= memSizeW) {
                char* temp = (char*) doubleMemory(word, &memSizeW);
                if (temp == NULL) {
                    free(wordBuff);
                    return failure();
                }
                else
                    word = temp;
            }
            word[sizeW] = c;
            sizeW++;
            c = (char) getchar();
        }
        //Throw in space and newline
        word[sizeW] = ' ';
        word[sizeW + 1] = '\0';
        
        wordBuff[sizeL] = word;
        sizeL++;
        if (c != EOF)
            c = (char) getchar();
    }
    
    //Sort array with frobcmp
    qsort(wordBuff, sizeL, sizeof(char*), frobcmp);
    //Print sorted
    for (int i = 0; i < sizeL; i++) {
        printf("%s", wordBuff[i]);
        free(wordBuff[i]);
    }

    return 0;
}

int frobcmp(const void *s1, const void *s2) {
    char const* a = *(char const**) s1;
    char const* b = *(char const**) s2;
    int i = 0;
    //Walk through, checking each character for inequality
    while (a[i] != ' ' && b[i] != ' ') {
        if (((char) (a[i] ^ 42)) > ((char) (b[i] ^ 42)))
            return 1;
        if (((char) (a[i] ^ 42)) < ((char) (b[i] ^ 42)))
            return -1;
        i++;
    }
    //Check equal
    if (a[i] == ' ' && b[i] == ' ')
        return 0;
    //Check a prefix of b
    if (a[i] == ' ' && b[i] != ' ')
        return 1;
    //Check b prefix of a
    if (b[i] == ' ' && a[i] != ' ')
        return -1;
}

void* doubleMemory(void* ptr, int* memorySize) {
    *memorySize *= 2;
    return realloc(ptr, *memorySize);
}

int failure() {
    fprintf(stderr, "Program has encountered an error. Exited with status 1.");
    return 1;
}
