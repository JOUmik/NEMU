#include "common.h"
#include "memory/cache.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "burst.h"

void public_ddr3_read(hwaddr_t addr, void* data);
void public_ddr3_write(hwaddr_t addr, void* data, uint8_t *mask);

void dram_write(hwaddr_t addr, size_t len, uint32_t data);

void init_cache(){
    //initialize cache L1
    int i;
    for (i=0; i < Cache_L1_Size / Cache_L1_Block_Size; i++){
	cache1[i].valid = 0;
    }

    test_time = 0;
}

int read_cache1(hwaddr_t address){

    uint32_t group_id = (address >> Cache_L1_Block_Bit) & (Cache_L1_Group_Size - 1);
    uint32_t tag_id = (address >> (Cache_L1_Block_Bit + Cache_L1_Group_Bit));
    uint32_t block_start = (address >> Cache_L1_Block_Bit) << Cache_L1_Block_Bit;

    int i, group_position;
    group_position = group_id * Cache_L1_Way_Size;
    
    
    for(i = group_position; i < (group_position + Cache_L1_Way_Size); i++){
        if(cache1[i].valid == 1 && cache1[i].tag == tag_id){ //HIT Cache_1

#ifndef TEST
            test_time += 2;
 #endif                  

            return i;
        }
    }

    //Fail to hit cache1,replace with random algorithm

    #ifndef TEST
    test_time += 200;
#endif

    srand((unsigned int)time(NULL));
    i = (rand() % Cache_L1_Way_Size) + group_position;

    /*read from memory*/
    int k;
    for(k = 0; k < (Cache_L1_Block_Size / BURST_LEN); k++){
        public_ddr3_read(block_start + (k * BURST_LEN), cache1[i].data+ (k * BURST_LEN));
    }

    cache1[i].valid = 1;
    cache1[i].tag = tag_id;

    return i;
}

void write_cache1(hwaddr_t addr, size_t len, uint32_t data){
    uint32_t group_idx = (addr >> Cache_L1_Block_Bit) & (Cache_L1_Group_Size - 1);
    uint32_t tag = (addr >> (Cache_L1_Group_Bit + Cache_L1_Block_Bit));
    uint32_t offset = addr & (Cache_L1_Block_Size - 1);

    int i,group = group_idx * Cache_L1_Way_Size;
    for (i = group + 0;i < group + Cache_L1_Way_Size;i ++){
        if (cache1[i].valid == 1 && cache1[i].tag == tag){// WRITE HIT
            /* write through */
            if (offset + len > Cache_L1_Block_Size){
                dram_write(addr,Cache_L1_Block_Size - offset,data);
                memcpy(cache1[i].data + offset, &data, Cache_L1_Block_Size - offset);
            
                write_cache1(addr + Cache_L1_Block_Size - offset,len - (Cache_L1_Block_Size - offset),data >> (Cache_L1_Block_Size - offset));
            }
	    else {
                dram_write(addr,len,data);
                memcpy(cache1[i].data + offset, &data, len);
	    }
	    return;
	}
    }

    dram_write(addr,len,data);
}
