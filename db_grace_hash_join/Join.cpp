#include "Join.hpp"
#include <functional>
#include <iostream>

using namespace std;
const int B = MEM_SIZE_IN_PAGE;

/*
 * Partition Function ========================================================
 *
 * Input: Disk, 
 *        Memory, 
 *        Disk page ids for left relation, 
 *        Disk page ids for right relation
 * Output: Vector of Buckets of size (MEM_SIZE_IN_PAGE - 1) after partition
 *============================================================================
*/

vector<Bucket> partition(
    Disk* disk, 
    Mem* mem, 
    pair<unsigned int, unsigned int> left_rel, 
    pair<unsigned int, unsigned int> right_rel) {

    // initialize output vector
    Bucket empty_bucket(disk);
    vector<Bucket> partitions((B - 1), empty_bucket);

    // hash two relations one by one 
    vector< pair<unsigned int, unsigned int> > relations;
    relations.push_back(left_rel);
    relations.push_back(right_rel);

    for (unsigned int rel = 0; rel < 2; ++rel) {

        mem->reset();

        pair<unsigned int, unsigned int> relation = relations[rel];

        // Step 1: put record in mem hash bucket -----------------------------
        for (unsigned int i = relation.first; i < relation.second; ++i) {

            // set input buffer: the last page in mem
            mem->loadFromDisk(disk, i, (B - 1)); 
            Page* input_buffer = mem->mem_page((B - 1));

            unsigned int num_records = input_buffer->size();
            for (unsigned int r = 0; r < num_records; ++r) {

                // get hash value for a record
                Record record = input_buffer->get_record(r); 
                unsigned int hash_val = (record.partition_hash()) % (B - 1);

                // write to disk if mem hash bucket full
                if ((mem->mem_page(hash_val))->full()) {

                    unsigned int flushed_disk_page = mem->flushToDisk(disk, hash_val);

                    if (rel == 0) {
                        partitions[hash_val].add_left_rel_page(flushed_disk_page);
                    } else {
                        partitions[hash_val].add_right_rel_page(flushed_disk_page);
                    }
                }

                // load the record to mem hash bucket
                (mem->mem_page(hash_val))->loadRecord(record);

            }

        }

        // Step 2: flush B-1 mem hash buckets (output buffers) to disk -------
        for (unsigned int i = 0; i < (B - 1); ++i) {

            // -- if the mem page has records in it
            if (((mem->mem_page(i))->size()) > 0) {

                // -- get the hash bucket number from any record
                Record record = (mem->mem_page(i))->get_record(0);

                unsigned int hash_val = (record.partition_hash()) % (B - 1);
                
                unsigned int flushed_disk_page = mem->flushToDisk(disk, i);
                if (rel == 0) {
                    partitions[hash_val].add_left_rel_page(flushed_disk_page);
                } else {
                    partitions[hash_val].add_right_rel_page(flushed_disk_page);
                }
                
            }

        }

    }

    return partitions;
  
}



/*
 * Probe Function ============================================================
 *
 * Input: Disk, Memory, Vector of Buckets after partition
 * Output: Vector of disk page ids for join result
 * ===========================================================================
*/

vector<unsigned int> probe(Disk* disk, Mem* mem, vector<Bucket>& partitions) {

    mem->reset();
    
    // initiate output buffer and result vector
    Page* output_buffer = mem->mem_page((B - 1));
    vector<unsigned int> result;

    // decide the outer/inner relations
    unsigned int num_right_rel = 0;
    unsigned int num_left_rel = 0;

    for (unsigned int i = 0; i < partitions.size(); ++i) {
        num_right_rel += partitions[i].num_right_rel_record;
        num_left_rel += partitions[i].num_left_rel_record;
    }

    string outer_flag;
    if (num_right_rel >= num_left_rel) {
        outer_flag = "left";
    }
    else {
        outer_flag = "right";
    }
    vector<unsigned int> outer_rel;
    vector<unsigned int> inner_rel;


    for (unsigned int i = 0; i < partitions.size(); ++i) {
        
        // Step 1: get the outer_rel, re-hash and build hash table -----------
        if (outer_flag == "left") {
            outer_rel = partitions[i].get_left_rel();
        } else {
            outer_rel = partitions[i].get_right_rel();
        }

        for (unsigned int j = 0; j < outer_rel.size(); j++) {

            // load outer_rel page to input buffer: last but one mem page
            unsigned int disk_page = outer_rel[j]; 
            mem->loadFromDisk(disk, disk_page, (B - 2));
            Page* input_buffer = mem->mem_page((B - 2)); 

            // loop records in the outer_rel page and re-hash
            unsigned int num_records = input_buffer->size();

            for (unsigned int r = 0; r < num_records; ++r) {
                Record record = input_buffer->get_record(r);
                unsigned int hash_val = (record.probe_hash()) % (B - 2);
                (mem->mem_page(hash_val))->loadRecord(record);
            }

        } 
        

        // Step 2: stream the inner_rel to hash-match outer_rel --------------
        if (outer_flag == "left") {
            inner_rel = partitions[i].get_right_rel();
        } else {
            inner_rel = partitions[i].get_left_rel();
        }

        for (unsigned int j = 0; j < inner_rel.size(); j++) {

            // load inner_rel into input buffer: last but one mem page
            unsigned int disk_page = inner_rel[j];
            mem->loadFromDisk(disk, disk_page, (B - 2));
            Page* input_buffer = mem->mem_page((B - 2));
            
            // loop records in the inner_rel page, re-hash and match
            unsigned int num_records = input_buffer->size();

            for (unsigned int r = 0; r < num_records; ++r) {

                // get inner record hash_val
                Record innerRecord = input_buffer->get_record(r);
                unsigned int hash_val = (innerRecord.probe_hash()) % (B - 2);

                // find matched record -> output buffer
                Page* matched_page = mem->mem_page(hash_val);

                for (unsigned int s = 0; s < matched_page->size(); s++) {

                    Record outerRecord = matched_page->get_record(s);
                    if (outerRecord == innerRecord) { 
                        // if full, flush to disk
                        if (output_buffer->full()) {
                            unsigned int flushed_disk_page = mem->flushToDisk(disk, (B - 1));
                            result.push_back(flushed_disk_page);
                        }
                        // load to output buffer
                        (mem->mem_page((B - 1)))->loadPair(outerRecord, innerRecord);
                    }

                }
                
            }

        }

        // clear B-2 buffers
        for(unsigned int p = 0; p < B - 2; p++){
            (mem->mem_page(p))->reset();
        }

    }


    // Step 3: flush output buffer to disk -----------------------------------
    if((output_buffer->size() > 0)){
        unsigned int flushed_disk_page = mem->flushToDisk(disk, (B - 1));
        result.push_back(flushed_disk_page);
    }

    return result;
}