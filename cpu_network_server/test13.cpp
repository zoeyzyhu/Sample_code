#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

// mainlly test fs_write

int main(int argc, char *argv[]) {
    char *server;
    int server_port;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    
    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    fs_create("user1", "/dir", 'd');

    fs_create("user1", "/dir/file", 'f');

    fs_create("user1", "/dir/file2", 'f');
    fs_create("user1", "/dir/file3", 'f');
    fs_create("user1", "/dir/file4", 'f');
    fs_create("user1", "/dir/file5", 'f');
    fs_create("user1", "/dir/file6", 'f');
    fs_create("user1", "/dir/file7", 'f');
    fs_create("user1", "/dir/file8", 'f');

    fs_create("user1", "/dir/file10", 'f');
    fs_create("user1", "/dir/file11", 'f');
    fs_create("user1", "/dir/file12", 'f');
    fs_create("user1", "/dir/file13", 'f');
    fs_create("user1", "/dir/file14", 'f');
    fs_create("user1", "/dir/file15", 'f');
    fs_create("user1", "/dir/file16", 'f');
    fs_create("user1", "/dir/file17", 'f');


    fs_delete("user1", "/dir/file2");
    fs_delete("user1", "/dir/file15");

    fs_create("user1", "/dir/file18", 'f');
    fs_create("user1", "/dir/file19", 'f');
    fs_create("user1", "/dir/file20", 'f');


}
