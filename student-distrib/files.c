/* parsing read only file system including opening and reading from
files, opening and reading the directory
Author - Filip Cakulev
https://wiki.osdev.org/File_Systems
https://littleosbook.github.io/#inodes-and-writable-file-systems
diagram in appendix A in mp3 documentation is hella clutch as well*/

#include "files.h"
#include "lib.h"

boot_block_t* boot_block;
inode_t* inode_addr;
dentry_t global_dentry;
void files_init(boot_block_t* start)
{
  //first 4kb block is accopied by boot block. Inode addresses start thereafter
  boot_block = start;
  inode_addr = (inode_t*)(start + FOURKB);
}

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
  int i;
  int found;
  if(fname == 0){   //if pointer to file is null return -1
    return FAILURE;
  }
  //loop through file names to see if it can be found
  for(i = 0; i < FILESMAX; i++)
  {
    found = strncmp((const int8_t*)fname, (const int8_t*)boot_block->dir_entries[i].file_name, RESERVED32B);
    if(found == 0)
    {
      //copy over to dentry
      memcpy(dentry, (const void*)&boot_block->dir_entries[i], SIXTYFOURBYTES);
      return SUCCESS;
    }
  }
  return FAILURE;   //file wasnt found
}

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
  if((FILESMAX < index) || (index < 0))    //check if index is in bounds
  {
    return FAILURE;
  }
  //copy over by index to dentry
  memcpy(dentry, (const void*)&boot_block->dir_entries[index], SIXTYFOURBYTES);
  return SUCCESS;
}

//bruh wtf
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
  int32_t cpy_length, inode_len, inode_num, offset_blocks, offset_bytes, data_idx;
  int bytes_read = 0;
  int i = 0;

  inode_len = inode_addr[inode].length_in_B;
  inode_num =  boot_block->num_inodes;

  if((inode < 0) || (inode >= inode_num))  // check if inode is in bounds
  {
    return FAILURE;
  }
  if(inode_len <= 0)   //check if file is empty
  {
    return FAILURE;
  }
  if(offset >= inode_len){    //return 0 bytes read if offset it too large
    return bytes_read;
  }

  //values for indexing
  uint8_t* data_blocks = (uint8_t*)(inode_addr + inode_num);  //first data block address (?)
  offset_blocks = offset / FOURKB;  //number of complete blocks
  offset_bytes = offset % FOURKB;    //number of bytes into the last block

  //check to see if offset + length is more bytes than the file has
  if(offset + length <= inode_len)
  {
    cpy_length = length;
  }
  else
  {
    cpy_length = inode_len - offset;
  }

  while(cpy_length > 0)
  {
    //get info on which data block we are reading from
    data_idx = inode_addr[inode].data_block_idx[offset_blocks];
    while(offset_bytes < FOURKB)
    {
      //4kb * #blocks + offset bytes + starting address for data blocks
      buf[i] = *(data_blocks + offset_bytes + data_idx * FOURKB);
      //increment and decrement values accordingly
      offset_bytes++;
      bytes_read++;
      i++;
      cpy_length--;
    }
    //move to next data block to be read and reset what byte we are at since its a new block
    offset_blocks++;
    offset_bytes = 0;
  }
  return bytes_read;
}

int32_t fopen(const uint8_t* fname)
{
   int32_t check;
   //copies file given by fname into global dentry
   check = read_dentry_by_name(fname, &global_dentry);

   if(check == -1)  //check if it found the file
   {
     return -1;
   }

   if(global_dentry.file_type != 2) //check if file type is a file
   {
     return -1;
   }

  return 0;
}
int32_t fclose(const uint8_t* fname)
{
  return 0;
}
int32_t fwrite(const uint8_t* fname)
{
  return -1;
}
// int32t fread()
// {
//
// }
int32_t dopen(const uint8_t* fname)
{
  int32_t check;
  //copies file given by fname into global dentry
  check = read_dentry_by_name(fname, &global_dentry);

  if(check == -1)   //check if it found the file
  {
    return -1;
  }

  if(global_dentry.file_type != 1)  //check if file type is a directory
  {
    return -1;
  }

 return 0;
}

int32_t dclose()
{
  return 0;
}
int32_t dwrite()
{
  return -1;
}
// int32t dread()
// {
//
// }
