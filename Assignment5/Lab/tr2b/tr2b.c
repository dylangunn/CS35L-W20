#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    char *from, *to;
    if (argc != 3) {
        fprintf(stderr, "%s: Incorrect number of arguments.", argv[0]);
        exit(1);
    }

    from = argv[1];
    to = argv[2];
    
    // Check same size, check duplicates
    int size = 0;
    while(from[size] != '\0' && to[size] != '\0') {
        for (int i = 0; i < size; i++) {
            if (from[i] == from[size]) {
                fprintf(stderr, "%s: Invalid argument; from has duplicate bytes.", argv[0]);
                exit(1);
            }
        }
        size++;
    }

    if ((from[size] == '\0' && to[size] != '\0') || (from[size] != '\0' && to[size] == '\0')){
        fprintf(stderr, "%s: Arguments must be of the same length.", argv[0]);
        exit(1);
    }

    //Translate and Output
    int src = getchar();
    while (src != EOF) {
        char c = (char) src;
        for (int i = 0; i < size; i++) {
            if (from[i] == c) {
                c = to[i];
                break;
            }
        }
        putchar(c);
        src = getchar();
    }
    return 0;
}    
