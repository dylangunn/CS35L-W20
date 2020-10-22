#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

int (*frobcmp(bool paramF))(const void* a, const void* b);
void* doubleMemory(void* ptr, size_t* size);
int frobcmpOriginal(const void* s1, const void* s2);
int frobcmpF(const void* s1, const void* s2);

int main(int argc, char* argv[]) {
    //Check for -f
    bool paramF = false;
    if (argc == 2) {
        if (strcmp(argv[1], "-f") == 0)
            paramF = true;
        else {
            fprintf(stderr, "%s: Invalid argument. \"%s\"", argv[0], argv[1]);
            exit(1);
        }
    }
    else if (argc > 2) {
        fprintf(stderr, "%s: Too many arguments.", argv[0]);
        exit(1);
    }

    struct stat fStat;
    if (fstat(STDIN_FILENO, &fStat) == -1) {
        fprintf(stderr, "%s: Error reading stdin.\n%s", argv[0], strerror(errno));
        exit(1);
    }

    //Set size, giving room for potential space
    size_t mSize = fStat.st_size;
    char* fBuffer = (char*) malloc(mSize+1);
    if (fBuffer == NULL) {
        fprintf(stderr, "%s: Error allocating memory.\n%s", argv[0], strerror(errno));
        exit(1);
    }
    int readBytes = read(STDIN_FILENO, fBuffer, mSize);
    if (readBytes == -1) {
        fprintf(stderr, "%s: Error reading from standard input.\n%s", argv[0], strerror(errno));
        exit(1);
    }

    //Allocate pointers to chars
    bool didReadBytes = readBytes > 0;
    size_t list1Size = 0;
    size_t list1MemSize = didReadBytes ? ((mSize / sizeof(char)) * sizeof(char*)) : (sizeof(char*) * 8);
    char** wordList1 = (char**) malloc(list1MemSize);
    if (didReadBytes) {
        wordList1[0] = &fBuffer[0];
        list1Size++;
    }

    //Don't read past buffer, don't access past readBytes
    size_t i = 0;
    while (i < readBytes) {
        char c = fBuffer[i];
        while (c != ' ' && i < readBytes) {
            i++;
            c = fBuffer[i];
        }
    if (i >= readBytes)
        break;

    //Assuming last was space
    i++;
        wordList1[list1Size] = &fBuffer[i];
        list1Size++;
    }

    if (didReadBytes && fBuffer[readBytes-1] != ' ') {
        fBuffer[readBytes] = ' ';
    }

    lseek(STDIN_FILENO, 0, SEEK_CUR);

    //Start with default of 8 words taken, grow
    size_t list2Size = 0;
    size_t list2MemSize = sizeof(char*) * 8;
    char** wordList2 = (char**) malloc(list2MemSize);
    if (wordList2 == NULL) {
        fprintf(stderr, "%s: Error allocating memory.\n%s", argv[0], strerror(errno));
        exit(1);
    }

    char c;
    int charRead = read(0, &c, sizeof(char));
    if (charRead == -1) {
        fprintf(stderr, "%s: Error reading from stdin.\n%s", argv[0], strerror(errno));
        exit(1);
    }
    while (charRead != 0) {
        //Add memory
        if (((list2Size + 1) * sizeof(char*)) >= list2MemSize) {
            char** temp = (char**) doubleMemory(wordList2, &list2MemSize);
            if (temp == NULL) {
                fprintf(stderr, "%s: Error allocating memory.\n%s", argv[0], strerror(errno));
                exit(1);
            }
            wordList2 = temp;
        }

        //Mem size at 8, use doubleMemory func if needed
        size_t sizeW = 0;
        size_t wordMemSize = sizeof(char) * 8;

        char* word = (char*) malloc(wordMemSize);
        if (word == NULL) {
            free(wordList2);
            fprintf(stderr, "%s: Error allocating memory.\n%s", argv[0], strerror(errno));
            exit(1);
        }

        while (c != ' ' && charRead != 0) {
            //Leave room of space and newline
            if (((sizeW + 2) * sizeof(char)) >= wordMemSize) {
                char* temp = (char*) doubleMemory(word, &wordMemSize);
                if (temp == NULL){
                    free(wordList2);
                    fprintf(stderr, "%s: Error allocating memory.\n%s", argv[0], strerror(errno));
                    exit(1);
                }
                word = temp;
            }
            word[sizeW] = c;
            sizeW++;
            charRead = read(0, &c, sizeof(char));
            if (charRead == -1) {
                fprintf(stderr, "%s: Error reading from stdin.\n%s", argv[0], strerror(errno));
                exit(1);
            }
        }
        word[sizeW] = ' ';
        word[sizeW+1] = '\0';

        wordList2[list2Size] = word;
        list2Size++;
        if (charRead != 0)
            charRead = read(0, &c, sizeof(char));
    }

    //Merge arrays, free all extending after
    size_t compositeSize = list1Size + list2Size;
    char** wordList = (char**) malloc(compositeSize * sizeof(char*));
    int listIndex = 0;
    for (size_t j = 0; j < list1Size; j++) {
        wordList[listIndex] = wordList1[j];
        listIndex++;
    }
    for (size_t j = 0; j < list2Size; j++) {
        wordList[listIndex] = wordList2[j];
        listIndex++;
    }

    //Sort composite, output
    qsort(wordList, compositeSize, sizeof(char*), frobcmp(paramF));

    //Run through each word, putchar every char until space
    for (size_t j = 0; j < compositeSize; j++) {
        for (int k = 0; wordList[j][k] != ' '; k++) {
            if (write(STDOUT_FILENO, &wordList[j][k], sizeof(char)) == -1) {
                fprintf(stderr, "%s: Error printing to standard output.\n", argv[0]);
                exit(1);
            }
        }
        if (putchar(' ') == EOF) {
            fprintf(stderr, "%s: Error printing to standard output.\n", argv[0]);
            exit(1);
        }
    }

    free(fBuffer);
    free(wordList1);
    for (size_t j = 0; j < list2Size; j++) {
        free(wordList2[j]);
    }
    free(wordList2);
    free(wordList);

    return 0;
}

int frobcmpOriginal(const void *s1, const void *s2) {
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

//Same as Original, but makes uppercase
int frobcmpF(const void* s1, const void* s2) {
    char const* a = *(char const**) s1;
    char const* b = *(char const**) s2;
    int i = 0;
    //Walk through, checking each character for inequality
    while (a[i] != ' ' && b[i] != ' ') {
        //Translate to upper
        char aCap = (char) (a[i] ^ 42);
        char bCap = (char) (b[i] ^ 42);

        aCap = (char) toupper((unsigned char) aCap);
        bCap = (char) toupper((unsigned char) bCap);

        if (aCap > bCap)
            return 1;
        if (bCap > aCap)
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

int (*frobcmp(bool paramF))(const void*, const void*) {
    return paramF ? &frobcmpF : &frobcmpOriginal;
}

void* doubleMemory(void* ptr, size_t* memorySize) {
    *memorySize *= 2;
    return realloc(ptr, *memorySize);
}
