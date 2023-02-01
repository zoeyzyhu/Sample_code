#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0);
    strcpy(filename, "data1.bin");
    char * p = (char *) vm_map(filename, 0);
    if (fork()) {
    strcpy(p, "another content");

    vm_yield();
    } else {
        for (int i = 0; i < 5; ++i){
            std::cout << p[i] << std::endl;
        }
    }
}