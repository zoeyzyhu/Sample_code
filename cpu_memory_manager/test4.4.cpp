#include <cstdio>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

int main() 
{
    std::cout << "Createing child process 1" << std::endl;
    fork();

    std::cout << "Createing child process 2" << std::endl;
    fork();

    std::cout << "Createing child process 3" << std::endl;
    fork();

    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "lampson83.txt");

    /* Map a page from the specified file */
    char *p1 = (char *) vm_map (filename, 0);

    /* Print the first part of the paper */
    for (unsigned int i=0; i<3; i++) {
	    std::cout << p1[i] << std::endl;
    }
}


