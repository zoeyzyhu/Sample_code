#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"

using std::cout;
using std::string;

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

    // 124
    for (unsigned int i = 0; i < 150; i++) {
        string name = "/" + std::to_string(i);
        status = fs_create("user1", (const char*) name.c_str(), 'f');
        cout << std::to_string(i) << ". Status: " << status << std::endl;
    }

    status = fs_delete("user1", "/100");
    cout << "Status: " << status << std::endl;

    status = fs_delete("user1", "/10");
    cout << "Status: " << status << std::endl;

    status = fs_create("user1", "/newfile", 'f');
    cout << "Status: " << status << std::endl;

    status = fs_writeblock("user1", "/newfile", 0, writedata);
    cout << "Status: " << status << std::endl;

    status = fs_writeblock("user1", "/124", 0, writedata);
    cout << "Status: " << status << std::endl;
}