#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;
using std::endl;

// Test 1
// Test swap-backed page usage
int main()
{       
    char *p_0 = (char *)vm_map(nullptr, 0);
    char *p_1 = (char *)vm_map(nullptr, 0);
    char *p_2 = (char *)vm_map(nullptr, 0);
    char *p_3 = (char *)vm_map(nullptr, 0);
    char *p_4 = (char *)vm_map(nullptr, 0);

    strcpy(p_0, "Swap-backed page 0");
    for (unsigned int i=0; i<20; i++) {
	    cout << p_0[i];
    }
    cout << endl;

    strcpy(p_1, "Swap-backed page 1");
    strcpy(p_2, "Swap-backed page 2");

    // Note now we have used all ppages (default is 4)
    strcpy(p_3, "Swap-backed page 3"); // Eviction fired (clock and evict page 0)

    // Reference page 1 (page 1 goes to end of clock queue)
    cout << "Read access on page 1" << endl;
    for (unsigned int i=0; i<20; i++) {
	    cout << p_1[i];
    }
    cout << endl;

    strcpy(p_4, "Swap-backed page 4"); // Eviction fired (clock and evict page 2, because page 1 is used recently)

    // Reference page 0 (which is evicted), should evict page 1 or 3? (Here we have page 1 evicted)
    cout << "Read access on page 0" << endl;
    for (unsigned int i=0; i<20; i++) {
	    cout << p_0[i];
    }
    cout << endl;
    cout << "END of Test 7" << endl;
}
