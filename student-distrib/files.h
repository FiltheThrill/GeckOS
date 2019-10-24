

#ifndef _FILES_H
#define _FILES_H

#define FAILURE       -1
#define SUCCESS        0
#define RESERVED32B    32
#define RESERVED24B    24
#define RESERVED52B    52
#define FILESMAX       63
#define FILL4KB        1023
#define SIXTYFOURBYTES 64
#define FOURKB          4096




typedef struct{
  uint32_t num_dir_entries;     //file system statistics
  uint32_t num_inodes;
  uint32_t num_data_blocks;
  uint8_t reserved[RESERVED52B];     // 52 bytes reserved in boot block
  dentry_t dir_entries[FILESMAX];      //both statistics and each directory occupy 64B
                                     //so the system can hold 63 files
} boot_block_t;

typedef struct{ // 1 * 32 + 4 + 4 + 24 = 64 bytes in size
  char file_name[RESERVED32B];     //Each directory entry gives a name (up to 32 characters, zero-padded)
  uint32_t file_type;              //0 for RTC, 1 for directory, 2 for file
  uint32_t num_inodes;             // only relevant for regular files
  uint8_t reserved[RESERVED24B];   //24 bytes reserved
} dentry_t;

typedef struct{
  uint32_t length_in_B;
  uint32_t data_block_idx[FILL4KB];
} inode_t;
#endif
