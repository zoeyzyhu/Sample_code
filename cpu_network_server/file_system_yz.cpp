#include "file_system.h"

mutex disk_lock; // Protect filesystem operations (RAII?)
vector<bool> disk_free_lists; // Store status (used or not used) on each disk block

Server_Lock_RAII::Server_Lock_RAII() {
    disk_lock.lock();
}

Server_Lock_RAII::~Server_Lock_RAII() {
    disk_lock.unlock();
}

void filesystem_read(Request &request) {
    // Parse the pathname to seperate filename for traversal
    vector<string> filenames = parse_pathname_to_filenames(request.pathname);
    fs_inode current_inode;
    // !!!!!!!!!! HOH Locking Here !!!!!!!!!!
    unsigned int prev_inode_block = 0;
    //locks[prev_inode_block].lock();
    std::unique_lock<std::mutex> first(locks[prev_inode_block]);
    disk_readblock(0, &current_inode);

    // Begin filesystem traversal
    for (size_t i = 0; i < filenames.size(); i++) {
        uint32_t inode_block_find = find_inode_block_by_filename(current_inode, filenames[i]);
        // Not found (return value = FS_DISKSIZE)
        if (inode_block_find == FS_DISKSIZE) {
            //locks[prev_inode_block].unlock();
            throw std::runtime_error("Error in fs_read(): Directory/Filename " + filenames[i] + "does not exist.\n");
        }
        // Found, traverse
        else {
            // HOH Operations:
            std::unique_lock<std::mutex> second(locks[inode_block_find]);
            first.swap(second);

            //locks[inode_block_find].lock();
            //locks[prev_inode_block].unlock();
            // Advance the root_node
            disk_readblock(inode_block_find, &current_inode);
            prev_inode_block = inode_block_find;
            // Check owner
            if (strcmp(current_inode.owner, request.username) != 0) {
                char buf[256];
                sprintf(buf, "Error in fs_read(): User %s does not own this directory or file.\n", request.username);
                //locks[prev_inode_block].unlock();
                throw std::runtime_error(buf);
            }
        }
    }
    
    // Now, we find the target(either dir or file)'s inode_block in "current_inode", and a unique target_lock should holds the lock to it.
    // We can only read a file
    if (current_inode.type != 'f') {
        //locks[prev_inode_block].unlock();
        throw std::runtime_error("Error in fs_read(): Attempting to read a directory.\n");
    }
    else {
        // User attempting to read outside block of this file.
        if (request.block >= current_inode.size) {
            //locks[prev_inode_block].unlock();
            throw std::runtime_error("Error in fs_read(): Attempting to read outside file block.\n");
        }
        else {
            // Read the file and store in request's variable "data"
            disk_readblock(current_inode.blocks[request.block], request.data);
            //locks[prev_inode_block].unlock();
        }
    }
}



void filesystem_write(Request &request) {
    // Parse the pathname to seperate filename for traversal
    vector<string> filenames = parse_pathname_to_filenames(request.pathname);
    fs_inode current_inode;
    // !!!!!!!!!! HOH Locking Here !!!!!!!!!!
    unsigned int prev_inode_block = 0;
    //locks[prev_inode_block].lock();
    std::unique_lock<std::mutex> first(locks[prev_inode_block]);
    disk_readblock(0, &current_inode);

    // Begin filesystem traversal
    for (size_t i = 0; i < filenames.size(); i++) {
        uint32_t inode_block_find = find_inode_block_by_filename(current_inode, filenames[i]);
        // Not found (return value = FS_DISKSIZE)
        if (inode_block_find == FS_DISKSIZE) {
            //locks[prev_inode_block].unlock();
            throw std::runtime_error("Error in fs_write(): Directory/Filename " + filenames[i] + "does not exist.\n");
        }
        // Found, traverse
        else {
            // HOH Operations:
            std::unique_lock<std::mutex> second(locks[inode_block_find]);
            first.swap(second);

            //locks[inode_block_find].lock();
            //locks[prev_inode_block].unlock();
            // Advance the root_node
            disk_readblock(inode_block_find, &current_inode);
            prev_inode_block = inode_block_find;
            // Check owner
            if (strcmp(current_inode.owner, request.username) != 0) {
                char buf[256];
                sprintf(buf, "Error in fs_write(): User %s does not own this directory or file.\n", request.username);
                //locks[prev_inode_block].unlock();
                throw std::runtime_error(buf);
            }
        }
    }

    // Now, we find the target(either dir or file)'s inode_block in "current_inode", and a unique target_lock should holds the lock to it.
    if (current_inode.type != 'f') {
        //locks[prev_inode_block].unlock();
        throw std::runtime_error("Error in fs_write(): Attempting to write to a directory.\n");
    }
    else {
        // Write to file blocks!
        // Spec 3.2
        // <block> specifies which block of the file to write.
        // <block> may refer to an existing block in the file, or it may refer to the block immediately after the current end of the file.
        // Condition 1. User attempting to write outside block of the file.
        if (request.block > current_inode.size) {
            //locks[prev_inode_block].unlock();
            throw std::runtime_error("Error in fs_write(): Attempting to write outside file block.\n");
        }
        // Condition 2. User attempting to write at the end of the file.
        // Need to allocate free disk blocks, if valid, update inode size and blocks
        else if (request.block == current_inode.size) {
            // Find a free space.
            uint32_t free_disk_block = allocate_free_disk_block();
            if (free_disk_block == FS_DISKSIZE) {
                //locks[prev_inode_block].unlock();
                throw std::runtime_error("Error in fs_write(): Attempting to write to a full disk.\n");
            }
            else { // Atomicity here??? Is this crash consistent???
                current_inode.blocks[current_inode.size] = free_disk_block;
                current_inode.size += 1;
                disk_writeblock(free_disk_block, request.data);
                // Write the updated "current_inode" back (similar to commit log, if crashes, nothing will be changed)
                disk_writeblock(prev_inode_block, &current_inode);
            }
        }
        // Condition 3. User attempting to write somewhere in the file.
        else {
            disk_writeblock(current_inode.blocks[request.block], request.data);
        }
        //locks[prev_inode_block].unlock();
    }
}

vector<string> parse_pathname_to_filenames(string pathname) {
    vector<string> filenames;
    std::stringstream ss(pathname);
    string filename;
    while (!ss.eof()) {
        std::getline(ss, filename, '/');
        filenames.push_back(filename);
    }
    // Remove first empty item
    filenames.erase(filenames.begin());
    // Check invalid filenames
    for (size_t i = 0; i < filenames.size(); i ++) {
        if (filenames[i].size() > FS_MAXFILENAME) {
            throw std::runtime_error("File/directory name is too long.\n");
        }
    }
    return filenames;
}



uint32_t find_inode_block_by_filename(fs_inode &current_inode, string filename) {
    // Looping through each blocks
    for (uint32_t i = 0; i < current_inode.size; i ++) {
        // Read the fs_direntry block
        fs_direntry current_dirblock[FS_DIRENTRIES];
        disk_readblock(current_inode.blocks[i], &current_dirblock);
        // Looping through all direntries
        for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
            uint32_t inode_block = current_dirblock[j].inode_block;
            // If this is a used direntry (0 means unused)
            if (inode_block != 0) {
                // If the filename is what we looking for
                const char* filename_cstr = filename.c_str();
                if (strcmp(filename_cstr, current_dirblock[j].name) == 0) {
                    return inode_block;
                }
            }
        }
    }
    // Fail to find filename
    return FS_DISKSIZE;
}



uint32_t allocate_free_disk_block() {
    Server_Lock_RAII server_protector;
    for (u_int32_t i = 0; i < FS_DISKSIZE; i++) {
        if (disk_free_lists[i]) {
            disk_free_lists[i] = false;
            return i;
        }
    }
    return FS_DISKSIZE;
}

void release_free_disk_block(unsigned i) {
    Server_Lock_RAII server_protector;
    disk_free_lists[i] = true;
}
