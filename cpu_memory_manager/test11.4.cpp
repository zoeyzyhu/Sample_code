#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;
using std::endl;

// Test
// Test advanced ver. (non-empty arena)
int main(){

    char *filename_0 = (char *) vm_map(nullptr, 0);
    strcpy(filename_0, "data1.bin");
    char *fp_0 = (char *) vm_map (filename_0, 0);
    char *sp_0 = (char *) vm_map(nullptr, 0);

    // Parent
    if (fork()) {
        cout << "Parent starts" << endl;
        
        char *filename_1 = (char *) vm_map(nullptr, 0);
        char *sp_1 = (char *) vm_map(nullptr, 0);

        strcpy(filename_1, "data2.bin");
        strcpy(sp_1, "swap-backed page 1");
        
        char *fp_1 = (char *) vm_map (filename_1, 0);

        cout << "Parent yield to child" << endl;
        vm_yield();

        char *fp_2 = (char *) vm_map (filename_1, 1);

        cout << "Parent ends:" << endl;
    }
    // Child
    else {
        cout << "Child starts" << endl;

        char *filename_1 = (char *) vm_map(nullptr, 0);

        strcpy(filename_1, "data2.bin");

        char *fp_0 = (char *) vm_map (filename_1, 0);

        cout << "Child ends:" << endl;
    }

    cout << "END of Test" << endl;
    return 0;
}