/* file_system.h
 * 
 * Header file for file systems
 */

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <shared_mutex>

#include "fs_server.h"

using std::cout;
using std::endl;
using std::string;
using std::mutex;
using std::shared_mutex;
using std::vector;
using std::unordered_map;

//---------- Variables ----------

// Provided by: (needs parsing and verificaton before store and use ondisk)
// - FS_READBLOCK <username> <pathname> <block><NULL>
// - FS_WRITEBLOCK <username> <pathname> <block><NULL><data>
// - FS_CREATE <username> <pathname> <type><NULL>
// - FS_DELETE <username> <pathname><NULL>
class Request {
    public:
        //string username; // Username passed in by client (name of requester)
        //string pathname; // Pathname passed in by client (name of file being accessed/created/deleted)
        char username[FS_MAXUSERNAME+1]; // Username passed in by client (name of requester)
        char pathname[FS_MAXPATHNAME+1]; // Pathname passed in by client (name of file being accessed/created/deleted)
        unsigned int block = 0; // which block to read
        char data[FS_BLOCKSIZE] = "\0"; // Data to be used on target file
        char type = '\0'; // Pathname type (f or d)
};

class Server_Lock_RAII {
    public:
        Server_Lock_RAII();
        ~Server_Lock_RAII();
};

extern mutex disk_lock; // Protect filesystem operations (RAII?)
extern vector<bool> disk_free_lists; // Store status (used or not used) on each disk block

// hzin implementation
extern std::mutex locks[FS_DISKSIZE];

//---------- Functions ----------

// Setup required parameters
int fs_init();

// hzin implementation
int fs_init_node(uint32_t block_id);

using FileOperation = int (*)(uint32_t block_id, char const*, void*);

int traverse_file_tree(uint32_t root, char const* pathname, 
    FileOperation action, void *arg);
// hzin out
void filesystem_init();

// File system operations (May need to further determine the parameters for the following functions)
// Temporarily assume that param "request" have all the correct information.

// Read files specified by request from disk to request.data
void filesystem_read(Request &request);

// Write to files specified by request from request.data to disk
void filesystem_write(Request &request);

void filesystem_create(Request &request);

void filesystem_delete(Request &request);

// Helpers

// Traverse the existing file system (if not empty) and set up the free list.
// Recursively calls on child directories
void filesystem_init_helper(fs_inode &current_inode);

// Parse a complete pathname to filenames at each level. Helper functions for filesystem traversal.
// Example: "/Desktop/EECS482/Project4"  ===>  "Desktop", "EECS482", "Project4"
vector<string> parse_pathname_to_filenames(string pathname);

// Traverse through current inode block to find inode_block number corresponding to filename (So we can access later).
// May not be thread-safe, need to use with lock
// Return:
// 1. uint32_t inode_block if found
// 2. FS_DISKSIZE if NOT found (since all blocks are within bound of [0, FS_DISKSIZE-1])
uint32_t find_inode_block_by_filename(fs_inode &current_inode, string filename);

// Allocate a available block recorded in "disk_free_lists"
// Thread-safe (Achieved by RAII)
// Return:
// 1. uint32_t inode_block of free block
// 2. FS_DISKSIZE if the disk ran out of free blocks (full)
uint32_t allocate_free_disk_block();

void release_free_disk_block(unsigned i);

#endif
