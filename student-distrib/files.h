

#ifndef _FILES_H
#define _FILES_H

#include "types.h"

#define FAILURE       -1
#define SUCCESS        0
#define RESERVED32B    32
#define RESERVED24B    24
#define RESERVED52B    52
#define FILESMAX       63
#define FILL4KB        1023
#define SIXTYFOURBYTES 64
#define FOURKB          4096



typedef struct dentry_t { // 1 * 32 + 4 + 4 + 24 = 64 bytes in size
  char file_name[RESERVED32B];     //Each directory entry gives a name (up to 32 characters, zero-padded)
  uint32_t file_type;              //0 for RTC, 1 for directory, 2 for file
  uint32_t num_inodes;             // only relevant for regular files
  uint8_t reserved[RESERVED24B];   //24 bytes reserved
} dentry_t;

typedef struct boot_block_t {
  uint32_t num_dir_entries;     //file system statistics
  uint32_t num_inodes;
  uint32_t num_data_blocks;
  uint8_t reserved[RESERVED52B];     // 52 bytes reserved in boot block
  dentry_t dir_entries[FILESMAX];      //both statistics and each directory occupy 64B
                                     //so the system can hold 63 files
} boot_block_t;
typedef struct inode_t {
  uint32_t length_in_B;
  uint32_t data_block_idx[FILL4KB];
} inode_t;


int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
extern int32_t fopen(const uint8_t* fname);
extern int32_t fclose(const uint8_t* fname);
extern int32_t fwrite(const uint8_t* fname);
extern int32_t dopen(const uint8_t* fname);
extern int32_t dclose();
extern int32_t dwrite();

#endif
