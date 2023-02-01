#include "file_system.h"

std::mutex locks[FS_DISKSIZE];

int traverse_file_tree(uint32_t root, char const *pathname,
    FileOperation action, void *arg)
{
    // process current node
    int status = action(root, pathname, arg);
    if (status != 0) {
        //locks[root].unlock();
        return status;
    }

    fs_inode node;
    disk_readblock(root, &node);

    // check owner
    Request *request = (Request*)arg;
    if ((strlen(request->username) != 0) &&
        (strlen(node.owner) != 0) && 
        (strcmp(node.owner, request->username) != 0)) {
        return -1;
    }

    if (node.type == 'f') {
        // Looping through each blocks
        for (uint32_t i = 0; i < node.size; i++) {
            // process each file block
            locks[node.blocks[i]].lock();
            status = action(node.blocks[i], pathname, arg);
            locks[node.blocks[i]].unlock();
            if (status == 1) {
                //locks[root].unlock();
                return status;
            }
        }
    } else {    // directory node
        // Set up all used blocks
        for (uint32_t i = 0; i < node.size; i++) {
            // process each directorhy block
            locks[node.blocks[i]].lock();
            status = action(node.blocks[i], pathname, arg);
            locks[node.blocks[i]].unlock();
            if (status == 1) {
                //locks[root].unlock();
                return status;
            }

            // Read the fs_direntry block
            fs_direntry dirblock[FS_DIRENTRIES];
            disk_readblock(node.blocks[i], dirblock);

            // Looping through all direntries
            for (uint32_t j = 0; j < FS_DIRENTRIES; ++j) {
                uint32_t block = dirblock[j].inode_block;
                // If this is a used direntry (0 means unused)
                if (block != 0) {
                    // process subdirectory or file
                    char buf[FS_MAXPATHNAME];
                    sprintf(buf, "%s%s/", pathname, dirblock[j].name);

                    locks[block].lock();
                    locks[root].unlock();
                    status = traverse_file_tree(block, buf, action, arg);
                    locks[root].lock();
                    locks[block].unlock();

                    if (status == 1) {
                        //locks[root].unlock();
                        return status;
                    }
                }
            }
        }
    }

    //locks[root].unlock();
    return 0;
}

int init_file(uint32_t block_id, char const*, void *)
{
    disk_lock.lock();
    disk_free_lists[block_id] = false;
    disk_lock.unlock();
    return 0;
}

int fs_init() {
    disk_lock.lock();
    disk_free_lists.resize(FS_DISKSIZE, true);
    disk_lock.unlock();
    Request request;
    request.username[0] = '\0';

    locks[0].lock();
    int status = traverse_file_tree(0, "/", init_file, &request);
    locks[0].unlock();
    return status;
}
