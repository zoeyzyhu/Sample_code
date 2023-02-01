#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    
    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);
    int status;

    fs_clientinit(server, server_port);
    
    
    // Invalid long pathname
    status = fs_create("user1", "/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",'d');
    cout << "1. Status:" << status << std::endl;

    // Invalid space 1
    status = fs_create("user1", " /no1",'d');
    cout << "2. Status:" << status << std::endl;

    // Invalid space 2
    status = fs_create("user1 ", "/no2",'d');
    cout << "3. Status:" << status << std::endl;

    // Invalid func calls 1
    status = fs_create("user1 /no3", "",'d');
    cout << "4. Status:" << status << std::endl;
 
    // Invalid func calls 2
    status = fs_create("", "user1 /no4",'d');
    cout << "5. Status:" << status << std::endl;

    // Invalid type
    status = fs_create("user1", "/no5", 'g');
    cout << "6. Status:" << status << std::endl;

    // Invalid pathname 1
    status = fs_create("user1", "//no6",'d');
    cout << "7. Status:" << status << std::endl;

    // Invalid pathname 2
    status = fs_create("user1", "no7",'d');
    cout << "8. Status:" << status << std::endl;

    // Invalid pathname 3
    status = fs_create("user1", "/no8/",'d');
    cout << "9. Status:" << status << std::endl;

    // Invalid pathname 4
    status = fs_create("user1", "/",'d');
    cout << "10. Status:" << status << std::endl;

    // Invalid pathname 5
    status = fs_create("user1", "/ ",'d');
    cout << "11. Status:" << status << std::endl;

    // Invalid filename 
    status = fs_create("user1", "/no12/",'f');
    cout << "12. Status:" << status << std::endl;

    // Invalid username 1
    status = fs_create("", "/no13",'d');
    cout << "13. Status:" << status << std::endl;

    // Invalid username 2
    status = fs_create(" ", "/no14",'d');
    cout << "14. Status:" << status << std::endl;

    // Invalid username 3
    status = fs_create("userrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr", "/no15",'d');
    cout << "15. Status:" << status << std::endl;
    
    // ! Valid Directory
    status = fs_create("user1", "/yes16",'d');
    cout << "16. (Valid) Status:" << status << std::endl;

    // Invalid create same dir
    status = fs_create("user1", "/yes16",'d');
    cout << "17. Status:" << status << std::endl;

    // ! Valid File
    status = fs_create("user1", "/yes16/yes18",'f');
    cout << "18. (Valid) Status:" << status << std::endl;

    // Invalid create same file
    status = fs_create("user1", "/yes16/yes18",'f');
    cout << "19. Status:" << status << std::endl;

    // Invalid user 1
    status = fs_create("user2", "/yes16/no20",'d');
    cout << "20. Status:" << status << std::endl;

    // Invalid user 2
    status = fs_create("user2", "/yes16/no21",'f');
    cout << "21. Status:" << status << std::endl;
}  