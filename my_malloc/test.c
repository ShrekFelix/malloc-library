#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"

int* initArray(int size, char method){
    int* array = method=='f' ? ff_malloc(size * sizeof(*array)) : bf_malloc(size * sizeof(*array));
    if(array){
        for(int i=0; i<size; i++){
            array[i] = i;
        }
    }
    return array;
}

int main(int argc, char *argv[]){
    int size = (int)argv[1];
    char method = argv[2];
    int* array = initArray(size, method);
    for(int i=0; i<size; i++){
        printf("%d ",array[i]);
    }
    return 0;
}