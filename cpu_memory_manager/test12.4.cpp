#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;
using std::endl;

// Test
// Test advanced ver. (non-empty arena)
int main(){

    vm_map(nullptr, 0);

    char *filename_0 = (char *) vm_map(nullptr, 0);
    strcpy(filename_0, "data1.bin");
    char *fp_0 = (char *) vm_map (filename_0, 0);
    
    char *filename_1 = (char *) vm_map(nullptr, 0);
    strcpy(filename_1, "data2.bin");
    char *fp_1 = (char *) vm_map (filename_1, 0);

    for (unsigned int i=0; i<10; i++) {
	    cout << fp_0[i];
    }

    for (unsigned int i=0; i<10; i++) {
	    fp_1[i] = 'Z';
    }

    // [?] denotes execution order
    // Parent
    if (fork()) {
        cout << "Parent starts" << endl;
        // [1]
        char *sp_0 = (char *) vm_map(nullptr, 0);
        char *filename_2 = (char *) vm_map(nullptr, 0);
        strcpy(sp_0, "swap-backed page 0");
        strcpy(filename_2, "data3.bin");
        char *fp_2 = (char *) vm_map(filename_2, 0);
        vm_yield();
        
        // [3]
        char *filename_3 = (char *) vm_map(nullptr, 0);
        strcpy(filename_3, "data3.bin");
        char *fp_3 = (char *) vm_map(filename_3, 0);
        vm_yield();

        // [5]
        for (unsigned int i=0; i<10; i++) {
	        cout << fp_3[i];
        }
        cout << endl;
        vm_yield();

        // [7]
        for (unsigned int i=0; i<10; i++) {
	        fp_3[i] = 'Z';
        }
        char *fp_5 = (char *) vm_map(filename_3, 1);
        
        cout << "Parent ends:" << endl;
    }
    // Child
    else {
        cout << "Child starts" << endl;
        // [2]
        char *filename_2 = (char *) vm_map(nullptr, 0);
        strcpy(filename_2, "data3.bin");
        char *fp_2 = (char *) vm_map(filename_2, 0);
        vm_yield();

        // [4]
        char *filename_3 = (char *) vm_map(nullptr, 0);
        strcpy(filename_3, "data4.bin");
        char *fp_4 = (char *) vm_map(filename_3, 0);
        vm_yield();

        // [6]
        for (unsigned int i=0; i<10; i++) {
	        fp_4[i] = 'Z';
        }
        vm_yield();

        // [8]
        for (unsigned int i=0; i<10; i++) {
	        cout << fp_4[i];
        }
        cout << endl;

        cout << "Child ends:" << endl;
    }

    cout << "END of Test" << endl;
    return 0;
}