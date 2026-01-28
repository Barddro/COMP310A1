#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compare_strings(const void *a, const void *b) {
    // Cast the void pointers to char pointers (or char array pointers)
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    // Use strcmp to compare the actual strings
    return strcmp(*ia, *ib);
}

int main() {

    char* myarr[] = {"a", "9", "A", "1"};

    qsort(myarr, 4, sizeof(const char*), compare_strings);

    printf("sorted order\n");
    for(int i = 0; i < 4; i++) {
        printf("%s\n", myarr[i]);
    }
    return 0;
}
