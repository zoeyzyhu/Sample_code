#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    int childPid = fork();
    if (childPid != 0) {
        char *filename = (char *) vm_map(nullptr, 0);
        (char *) vm_map(nullptr, 0);
        strcpy(filename + VM_PAGESIZE - 5, "aaaabbbbbb.txt");
        char *p = (char *) vm_map (filename + VM_PAGESIZE - 5, 0);
        std::cout << "mapped" << std::endl;
        for (int i = 0; i < 10; ++i)
            std::cout << p[i] << std::endl;
    } else {
        vm_yield();
        char *filename = (char *) vm_map(nullptr, 0);
        (char *) vm_map(nullptr, 0);
        strcpy(filename + VM_PAGESIZE - 5, "lampson83.txt");
        char *p = (char *) vm_map (filename + VM_PAGESIZE - 5, 0);
        std::cout << "mapped" << std::endl;
        for (int i = 0; i < 10; ++i)
            std::cout << p[i] << std::endl;
    }
}