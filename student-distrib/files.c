/* parsing read only file system including opening and reading from
files, opening and reading the directory
Author - Filip Cakulev
https://wiki.osdev.org/File_Systems
https://littleosbook.github.io/#inodes-and-writable-file-systems
diagram in appendix A in mp3 documentation is hella clutch as well*/

#include "files.h"

boot_block_t* boot_block;
inode_t* inode_addr;

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
  for(i = 0; i < FILESMAX; i++)   //loop through file names to see if it can be found
  {
    found = strncmp((const uint8_t)fname, (const uint8_t*)boot_block->dir_entries[i].file_name, RESERVED32B);
    if(found == 0)
    {
      memcpy(dentry, (const void*)boot_lock->dir_entries[i], SIXTYFOURBYTES);  //copy over to dentry
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
  memcpy(dentry, (const void*)boot_lock->dir_entries[index], SIXTYFOURBYTES);  //copy over by index to dentry
  return SUCCESS;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
  int32_t cpy_length, inode_len, inode_num;

  inode_len = inode_addr[inode].length_in_B;
  inode_num =  boot_lock->num_inodes;

  if((inode < 0) || (inode >= inode_num))  // check if inode is in bounds
  {
    return FAILURE;
  }
  if(inode_len <= 0)   //check if file is empty
  {
    return FAILURE;
  }

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

  }
}
