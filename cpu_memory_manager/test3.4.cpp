#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

// Test the state sync for file-backed page for all possible state in one process

int main()
{

    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "lampson83.txt");

    char *p = (char *) vm_map (filename, 0);
    char *p_2 = (char *) vm_map (filename, 0);

    std::cout << "check 1" << std::endl;
    
    std::cout << p[0] << std::endl;

    std::cout << "check 1.5" << std::endl;

    char *p_3 = (char *) vm_map (filename, 0);

    p_3[0] = '1';

    char *p_4 = (char *) vm_map (filename, 0);

    char *spaceHolder = (char *) vm_map(nullptr, 0);
    strcpy(spaceHolder, "hold pPage");

    char *spaceHolder_2 = (char *) vm_map(nullptr, 0);
    strcpy(spaceHolder_2, "hold pPage");

    char *p_5 = (char *) vm_map (filename, 0);

    std::cout << "check 2" << std::endl;

    char *spaceHolder_3 = (char *) vm_map(nullptr, 0);
    strcpy(spaceHolder_3, "hold pPage");

    char *p_6 = (char *) vm_map (filename, 0);

    std::cout << "check 3" << std::endl;

    std::cout << p_5[0] << std::endl;

    std::cout << "check 4" << std::endl;

    p[0] = '1';





}