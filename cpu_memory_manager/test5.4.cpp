#include <cstdio>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cassert>
#include "vm_app.h"

// Test for file name is not entirely in valid arena

int main() { 
        char *filename = (char *) vm_map(nullptr, 0);

        /* Write the name of the file that will be mapped */
        filename[VM_PAGESIZE - 2] = 'l';
        filename[VM_PAGESIZE - 1] = 'a';


        char *p_7 = (char *) vm_map (filename + VM_PAGESIZE - 2, 0);
        std::cout << p_7[0] << std::endl;
}

