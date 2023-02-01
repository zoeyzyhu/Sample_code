#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    int childPid = fork();

    // ---- first set 
    char *filename2 = (char *) vm_map(nullptr, 0);
    (char *) vm_map(nullptr, 0);

    // parent yield
    if (childPid!=0) vm_yield();

    strcpy(filename2 + VM_PAGESIZE - 5, "lampson83.txt");
    char *p2 = (char *) vm_map (filename2 + VM_PAGESIZE - 5, 0);
    std::cout << "mapped" << childPid << std::endl;
    for (int i = 0; i < 10; ++i)
        std::cout << p2[i] << childPid << std::endl;

    // find out which process
    std::cout << childPid << std::endl;




    // ---- second set
    char *filename1 = (char *) vm_map(nullptr, 0);

    // fork again
    int childPid2 = fork();
    (char *) vm_map(nullptr, 0);

    // find out which process
    std::cout << childPid2 << std::endl;

    strcpy(filename1 + VM_PAGESIZE - 5, "aaaabbbbbb.txt");
    char *p1 = (char *) vm_map (filename1 + VM_PAGESIZE - 5, 0);
    std::cout << "mapped" << childPid2 << std::endl;

    // child yield: first child and it's child inherit childPid=0
    if (childPid==0) vm_yield();
    for (int i = 0; i < 10; ++i)
        std::cout << p1[i] << std::endl;
}