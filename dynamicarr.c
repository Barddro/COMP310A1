#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* *arr;
    int length;
    int capacity;
} DynamicArr;

DynamicArr* init() {
    DynamicArr* myarr = malloc(sizeof *myarr);
    if (!myarr) return NULL;

    myarr->length = 0;
    myarr->capacity = 8;

    myarr->arr = malloc(sizeof *myarr->arr * myarr->capacity);
    if (!myarr->arr) {
        free(myarr);
        return NULL;
    }

    return myarr;
}