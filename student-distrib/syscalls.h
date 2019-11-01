#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"

#define VIRTUAL_ADDR 0x08048000
#define FIRSTPCB 0x07FE000  //first pcb will be at bottom of kernel memory? 8mb - 8kb
#define EIGHTMB 0x0800000
#define FOURMB  0x0400000
#define EIGHTKB 0x02000
#define SURP    0x087
#define MAXPROCESSES  6
#define MAXFILES    8
#define ELF_MAGIC0 0x7F
#define ELF_MAGIC1  0x45
#define ELF_MAGIC2 0x4C
#define ELF_MAGIC3 0x46
#define FIRST40   40
#define WORD_SIZE 128
#define ERROR_SIZE 200
#define PAGE128   32

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

typedef struct PCB_t{
  file_descriptor_t file_array[MAXFILES];
  //was planning to maybe just put them in a pcb array ?
  //int32_t process; //number to identify which process this pcb is
  struct PCB_t* parent_process; // number to identify parent process(this ould be a pointer as well if you ant to change it)
  int32_t process_on;
  //ADD STUFF AS YOU SEE FIT I DONT REALLY KNOW WHAT ELSE IS SUPPOSED GO IN HERE
} PCB_t;

PCB_t* PCB_six[6];


#endif
