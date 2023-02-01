#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0);
    strcpy(filename, "some content");
    char *filename1 = (char *) vm_map(nullptr, 0);
    strcpy(filename1, "some content");
    char *filename2 = (char *) vm_map(nullptr, 0);
    strcpy(filename2, "some content");
    if (fork()) {
        std::cout << "parentRunning" << std::endl;

    } else {
        char *filename3 = (char *) vm_map(nullptr, 0);
        strcpy(filename3, "some content");
        std::cout << filename[0] << std::endl;

    }
}