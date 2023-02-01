/*
 * fs_server.h
 *
 * Header file for the file server.
 */

#ifndef _FS_SERVER_H_
#define _FS_SERVER_H_

#include <iostream>                         // makes it safe to call cout from 
                                            // static initializers
#include <sys/types.h>
#include <cstdint>

#include "fs_param.h"

#ifdef __APPLE__

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

#endif // __APPLE__

/*
 * Size of the disk (in blocks)
 */
static const unsigned int FS_DISKSIZE = 4096;

/*
 * Definitions for on-disk data structures.
 */
struct fs_direntry {
    char name[FS_MAXFILENAME + 1];         // name of this file or directory
    uint32_t inode_block;                  // disk block that stores the inode
                                           // for this file or directory (0 if
                                           // this direntry is unused)
};

struct fs_inode {
    char type;                             // file ('f') or directory ('d')
    char owner[FS_MAXUSERNAME + 1];        // owner of this file or directory
    uint32_t size;                         // size of this file or directory
                                           // in blocks
    uint32_t blocks[FS_MAXFILEBLOCKS];     // array of data blocks for this
                                           // file or directory
};

/*
 * Number of direntries that can fit in one block
 */
static const unsigned int FS_DIRENTRIES = (FS_BLOCKSIZE / sizeof(fs_direntry));

/*
 * Interface to the disk.
 *
 * Disk blocks are numbered from 0 to (FS_DISKSIZE-1).
 * disk_readblock and disk_writeblock are both thread safe, i.e., multiple
 * threads can safely make simultaneous calls to these functions.
 */

/*
 * disk_readblock
 *
 * Copies disk block "block" into buf.  Asserts on failure.
 */
extern void disk_readblock(unsigned int block, void *buf);

/*
 * disk_writeblock
 *
 * Copies buf to disk block "block".  Asserts on failure.
 */
extern void disk_writeblock(unsigned int block, const void *buf);

#include <thread>
#include <mutex>
extern std::mutex cout_lock;

#endif /* _FS_SERVER_H_ */
