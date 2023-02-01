#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    std::cout << "I'm at 2 step" << std::endl;

    /* Write the name of the file that will be mapped */
    strcpy(filename, "lampson83.txt");

    std::cout << "I'm at 3 step" << std::endl;

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);

    std::cout << "I'm at 4 step" << std::endl;

    /* Print the first part of the paper */
    for (unsigned int i=0; i<1900; i++) {
        cout << p[i];
    }
    cout << std::endl;
}
