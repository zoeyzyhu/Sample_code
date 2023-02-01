#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include "thread.h"
#include "disk.h"

enum {
    non_deterministic = 0,
    deterministic = 1
};

struct DiskServer {
    int max_disk_queue;
    int nthread;
    std::vector<std::string> request_files;
};

struct ThreadData {
    int my_id;
    int max_disk_queue;
    std::vector<int> my_track;
};

struct RequestData {
    int tracker_id;
    int requester_id;
};


// Shared global variables
mutex disk_mutex;
cv requester_queue, server_queue;
std::vector<RequestData> data_queue;
int current_track;
int iend;
size_t dependency_clear;
int nthread_alive;


// bit operations
//
void set_bit(size_t& p, int i) {
    p |= 1 << i;
}

void clear_bit(size_t& p, int i) {
    p &= ~(1 << i);
}

void toggle_bit(size_t& p, int i) {
    p ^= 1 << i;
}

bool check_bit(size_t const& p, int i) {
    return p & (1 << i);
}

int sum_bit(size_t p, int max_bits) {
    int n = 0;
    for (int i = 0; i < max_bits; ++i) {
        n += p & 1;
        p >>= 1;
  }
    return n;
}

void serve_disk(void *a) {
    int max_disk_queue = *((int *) a);

    while (true) {
        disk_mutex.lock();
        // 1. queue reach max;
        //    max = min(max_disk_queue, num_remaining_threads)
        while (
            iend != std::min(max_disk_queue, nthread_alive) 
        ) {
            server_queue.wait(disk_mutex);
        }
        //do stuff
        int dist = 999, j = -1;
        for (int i = 0; i < iend; ++i) {
            int d = std::abs(current_track - data_queue[i].tracker_id);
            if (d < dist) {
                dist = d;
                j = i;
            }
        }

        if (j >= 0) {
            print_service(data_queue[j].requester_id, 
                      data_queue[j].tracker_id);
            // update dependency
            toggle_bit(dependency_clear, data_queue[j].requester_id);
            // update current track number
            current_track = data_queue[j].tracker_id;
            // dequeue
            data_queue[j] = data_queue[iend-1];
            --iend;
        }

        if (nthread_alive == 0) break;
        requester_queue.broadcast();
        disk_mutex.unlock();
    }
}

void load_disk(void *arg) {
    ThreadData *pdata = (ThreadData *)arg;
    
    for (int i = 0; i < pdata->my_track.size(); ++i) {
        disk_mutex.lock();
        // 1. dependency cleared;
        // 2. queue is not full.
        while (
            !(check_bit(dependency_clear, pdata->my_id) &&
              iend < pdata->max_disk_queue)
        ) {
            requester_queue.wait(disk_mutex);
        }
        
        RequestData rd;
        rd.requester_id = pdata->my_id;
        rd.tracker_id = pdata->my_track[i];
        if (iend < data_queue.size()) {
            data_queue[iend] = rd;
        } else {
            data_queue.push_back(rd);
        }
        ++iend;
        print_request(pdata->my_id, pdata->my_track[i]);
        // update dependency
        toggle_bit(dependency_clear, pdata->my_id);

        server_queue.signal();
        disk_mutex.unlock();
    }

    disk_mutex.lock();
    // 1. servicer completed my job
    while ( 
        !check_bit(dependency_clear, pdata->my_id)
    ) {
        requester_queue.wait(disk_mutex);
    }
    // update num of threads alive
    --nthread_alive;
    server_queue.signal();
    disk_mutex.unlock();
}

void virtual_cpu(void *arg) {
    DiskServer *pdisk = (DiskServer *)arg;

    // Read request files and disk tracks
    std::vector<thread*> pthreads(pdisk->nthread);
    std::vector<ThreadData> data(pdisk->nthread);
    for (int i = 0; i < pdisk->nthread; ++i) {
        data[i].my_id = i;
        data[i].max_disk_queue = pdisk->max_disk_queue;

        std::ifstream file(pdisk->request_files[i], std::ios::in);
        int track_id;
        while (file >> track_id) {
            data[i].my_track.push_back(track_id);
        }
    }
    
    // Spawn the servicer (serve_disk) thread
    thread *servicer = new thread((thread_startfunc_t)serve_disk, (void *) &pdisk->max_disk_queue);


    // Spawn requester (load_disk) threads
    for (int i = 0; i < pdisk->nthread; ++i) {
        pthreads[i] = new thread((thread_startfunc_t)load_disk, (void *) &data[i]);
    }

    for (int i = 0; i < pdisk->nthread; ++i)
        pthreads[i]->join();

    for (int i = 0; i < pdisk->nthread; ++i)
        delete pthreads[i];

    delete servicer;
}

int main(int argc, char** argv) {
    // Read input parameters and check corner case
    if (argc == 1) {  // no input argument
        return 1;
    }
    /* Now we have at least one argument
       Can we assume that the second argument is always an integer? */
  
    // Initialize disk server
    DiskServer disk;

    // Verify input 
    disk.max_disk_queue = std::stoi(argv[1]);
    disk.nthread = argc - 2;

    // Initialize variables
    current_track = 0;
    iend = 0;
    //dependency_clear = ~0;
    nthread_alive = disk.nthread;
    
    // Enqueue request files
    for (int i = 0; i < disk.nthread; ++i) {
        disk.request_files.push_back(argv[2+i]);
        set_bit(dependency_clear, i);
    }
    
    cpu::boot((thread_startfunc_t) virtual_cpu, (void *) &disk, non_deterministic);
}   
