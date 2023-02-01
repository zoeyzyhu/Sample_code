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

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.";
    char readdata[512];

    fs_clientinit(server, server_port);

    // Invalid: delete root
    status = fs_delete("user1", "/");
    cout << "1. Status:" << status << std::endl;
    assert(status == -1);

    // Invalid: Delete root
    status = fs_delete("", "/");
    cout << "2. Status:" << status << std::endl;
    assert(status == -1);

     // Invalid: Delete root
    status = fs_delete("\0", "/");
    cout << "3. Status:" << status << std::endl;
    assert(status == -1);

    // Correct Set up
    status = fs_create("user1", "/dir1", 'd');
    assert(status == 0);
    status = fs_create("user1", "/dir1/file1",'f');
    assert(status == 0);
    status = fs_create("user1", "/dir1/file2",'f');
    assert(status == 0);
    for (unsigned int i = 0; i < 20; i++) {
        status = fs_writeblock("user1", "/dir1/file2", i, writedata);
        assert(status == 0);
    }
    status = fs_delete("user1", "/dir1/file2");
    assert(status == 0);
    status = fs_create("user1", "/dir1/file3", 'f');
    assert(status == 0);
    status = fs_writeblock("user1", "/dir1/file3", 0, writedata);
    assert(status == 0);

    status = fs_create("user1", "/dir2", 'd');
    assert(status == 0);
    status = fs_create("user1", "/dir2/file1",'f');
    assert(status == 0);
    status = fs_create("user1", "/dir2/file2",'f');
    assert(status == 0);


    // Invalid: wrong user
    status = fs_delete("user2", "/dir2/file2");
    cout << "4. Status:" << status << std::endl;
    assert(status == -1);

    // Invalid: non-empty dir
    status = fs_delete("user1", "/dir2");
    cout << "5. Status:" << status << std::endl;
    assert(status == -1);

    // Invalid: invalid name 1
    status = fs_delete("user1", "/dir2/file2/");
    cout << "6. Status:" << status << std::endl;
    assert(status == -1);

    // Invalid: invalid name 2
    status = fs_delete("user1", "dir2/file2");
    cout << "7. Status:" << status << std::endl;
    assert(status == -1);

    // Invalid: invalid name 1
    status = fs_delete("user1", "//dir2/file2");
    cout << "8. Status:" << status << std::endl;
    assert(status == -1);

}