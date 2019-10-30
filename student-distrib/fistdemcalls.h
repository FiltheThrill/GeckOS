#ifndef FISTEDEEM_H
#define FISTEDEEM_H

#include "types.h"

#define FIRSTPCB 0x07FE000  //first pcb will be at bottom of kernel memory? 8mb - 8kb


typedef struct { //disgusting
  int32_t (*read)(int32_t, void*, int32_t);
  int32_t (*write)(int32_t, const void*, int32_t);
  int32_t (*open)(const uint8_t*);
  int32_t (*close)(int32_t);
} operations_table_t;

typedef struct {
  operations_table_t* f_op_tbl_ptr;
  int32_t inode;
  int32_t f_pos;
  int32_t flags;
} file_descriptor_t;

typedef PCB_t{
  file_descriptor_t file_array[8];
  //was planning to maybe just put them in a pcb array ?
  int32_t process; //number to identify which process this pcb is
  int32_t parent_process; // number to identify parent process(this ould be a pointer as well if you ant to change it)
  int32_t ebp;
  int32_t esp;
  //ADD STUFF AS YOU SEE FIT I DONT REALLY KNOW WHAT ELSE IS SUPPOSED GO IN HERE
} PCB_t;

#endif
