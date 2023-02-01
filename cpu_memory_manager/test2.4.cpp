#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;
using std::endl;

// Test
// Test fork, filename, file page states
int main()
{   
    // Swap-backed pages
    char *filename_1 = (char*)vm_map(nullptr, 0);
    char *filename_2 = (char*)vm_map(nullptr, 0);
    char *filename_3 = (char*)vm_map(nullptr, 0);

    strcpy(filename_1, "lampson83.txt");
    char *p_1 = (char *)vm_map(filename_1, 0);

    cout << "Read and print lampson83.txt:" << endl;
    for (unsigned int i=0; i < 20; i++) {
        cout << p_1[i];
    }
    cout << endl;

    // Parent yield to child
    if (fork()) {
        vm_yield();
        cout << "Child finished, parent begins:" << endl;
    }

    // Repetition in allocating file-backed page
    char *p_2 = (char *)vm_map(filename_1, 0);
    // Write to swap
    for (unsigned int i=0; i < 20; i++) {
        filename_2[i] = p_2[i];
    }
    
    // Access swap
    cout << "Current filename 1: " << filename_1 << endl;
    // Attempt to rename this (from "lampson83.txt" to "data1.bin")
    strcpy(filename_1, "data1.bin");
    cout << "New filename 1: " << filename_1 << endl;
    // Allocate new file-backed page with new filename
    char *p_3 = (char *)vm_map(filename_1, 0);

    for (unsigned int i=0; i < 20; i++) {
        filename_3[i] = p_3[i];
    }
    for (unsigned int i=0; i < 20; i++) {
        cout << filename_3[i];
    }
    cout << endl;
    for (unsigned int i=0; i < 20; i++) {
        cout << p_3[i];
    }
    cout << endl;
    strcpy(filename_3, "This is not working");
    for (unsigned int i=0; i < 20; i++) {
        cout << filename_3[i];
    }
    cout << endl;
    cout << "END of Test" << endl;
}
