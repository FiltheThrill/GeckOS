#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"

#define VIRTUAL_ADDR 0x08048000
#define FIRSTPCB 0x07FE000  //first pcb will be at bottom of kernel memory? 8mb - 8kb
#define EIGHTMB (1 << 23)
#define FOURMB  (1 << 22)
#define EIGHTKB (1 << 13)
#define SURP    0x09F
#define MAXPROCESSES  6
#define MAXFILES    8
#define MAXARGS  128 //? arbitrary max length
#define ELF_MAGIC0 0x7F
#define ELF_MAGIC1  0x45
#define ELF_MAGIC2 0x4C
#define ELF_MAGIC3 0x46
#define FIRST40   40
#define WORD_SIZE 128
#define ERROR_SIZE 200
#define PAGE128   32
#define RTC 0
#define DIRECTORY 1
#define FILE 2
#define IRETESP 0x83FFFFC //128 mb + 4mb - 4
#define MINFD   2
#define MAXFD   7
#define STDINFD 0
#define STDOUTFD 1
#define PAGESIZE 4
#define VIDMEM_CPY 0x8800000  //address for user vid mem
#define VIDMEM_START 0x8000000 //address for vidmem start
#define VIDMEM_END 0x8400000
#define ATTR_HALT  0x87
#define SIG_CNT    5
//
//https://barrgroup.com/Embedded-Systems/How-To/C-Function-Pointers
typedef struct { //disgusting
  int32_t (*read)(int32_t, void*, int32_t); //int32_t fd, uint8_t * buf, int32_t nbytes
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
  uint32_t prev_esp0;
  uint16_t prev_ss0;
  uint32_t esp;
  uint32_t ebp;
  int32_t index;
  int32_t p_index;
  //ADD STUFF AS YOU SEE FIT I DONT REALLY KNOW WHAT ELSE IS SUPPOSED GO IN HERE
  uint8_t args[MAXARGS];
  int32_t argsize;
  //add for sigs
  void* sig_arr[SIG_CNT];
} PCB_t;

PCB_t* PCB_six[6];

int32_t p_process_num;
int32_t c_process_num;

extern void PCB_start();
extern int32_t halt(uint8_t status);

extern int32_t execute(const uint8_t* command);

extern int32_t read(int32_t fd, void* buf, int32_t nbytes);

extern int32_t open(const uint8_t* filename);

extern int32_t close(int32_t fd);

extern int32_t getargs(uint8_t* buf, int32_t nbytes);

extern int32_t vidmap(uint8_t** screen_start);

extern int32_t set_handler(int32_t signum, void* handler_address);

extern int32_t sigreturn(void);

extern void kill();

extern int32_t ignore();

#endif
