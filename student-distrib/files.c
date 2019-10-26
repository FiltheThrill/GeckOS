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
uint8_t d_index = 0;

void files_init(int32_t start)
{
  //printf("%d \n", start);
  //first 4kb block is accopied by boot block. Inode addresses start thereafter
  boot_block = (boot_block_t*)start;
  //printf("%d \n", (int32_t)boot_block);
  inode_addr = (inode_t*)(start + FOURKB);



//  printf("%d \n", boot_block->num_inodes);
//  printf("%d \n", boot_block->num_dir_entries);
//  printf("%d\n", inode_addr[0].data_block_idx[0]);
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
  //printf("%d\n", inode);
  int32_t cpy_length, inode_len, inode_num, offset_blocks, offset_bytes, data_idx;
  int bytes_read = 0;
  int i = 0;

//  printf("Attempting to get info on inodes\n");
  inode_num =  boot_block->num_inodes;

  if((inode < 0) || (inode >= inode_num))  // check if inode is in bounds
  {
  //  printf("inode not in bounds\n");
    return FAILURE;
  }
//  printf("Attempting to get length of inodes\n");
  inode_len = inode_addr[inode].length_in_B;


  if(inode_len <= 0)   //check if file is empty
  {
//    printf("no information in this inode\n");
    return FAILURE;
  }
  if(offset >= inode_len){    //return 0 bytes read if offset it too large
    return bytes_read;
  }
//  printf("begin logic\n");
  //values for indexing
  uint8_t* data_blocks = (uint8_t*)(inode_addr + inode_num);  //first data block address (?)
  offset_blocks = offset / FOURKB;  //number of complete blocks
  offset_bytes = offset % FOURKB;    //number of bytes into the last block
//  printf("set up indexes\n");
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
  //  printf("%d\n", offset_blocks);
  //  printf("attempt to get data block start\n");
  //  printf("%d\n", inode_addr[inode].data_block_idx[offset_blocks]);
    data_idx = inode_addr[inode].data_block_idx[offset_blocks];
    while(offset_bytes < FOURKB)
    {
      //4kb * #blocks + offset bytes + starting address for data blocks
    //  printf("attempting to place into buffer at %d\n", i);
      buf[i] = *(data_blocks + offset_bytes + data_idx * FOURKB);
      //increment and decrement values accordingly
      offset_bytes++;
      bytes_read++;
      i++;
      cpy_length--;
      if(cpy_length <= 0)
      {
        break;
      }
    }
    //move to next data block to be read and reset what byte we are at since its a new block
  //  printf("%d\n", offset_blocks);
    offset_blocks++;
    offset_bytes = 0;
  }
  //printf("bytes placed in buf\n");
  return bytes_read;
}

int32_t fopen(const uint8_t* fname)
{
   int32_t check;
   //copies file given by fname into global dentry
   //printf("trying to find file\n");
   check = read_dentry_by_name(fname, &global_dentry);
  // printf("checking if file is ok\n");
   if(check == -1)  //check if it found the file
   {
    // printf("couldnt find file\n");
     return -1;
   }

   if(global_dentry.file_type != 2) //check if file type is a file
   {
    // printf("This is not a file\n");
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

int32_t fread(void* buf, uint32_t offset, uint32_t length)
{
  int32_t result;
  result = read_data(global_dentry.inode_num, offset, buf, length);
  return result;
}

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

int32_t dread(void* buf)
{
  int i;
  int32_t check;
  uint32_t name_length;

  if(d_index > 16)
  {
    d_index = 0;
    return 0;
  }

  check = read_dentry_by_index(d_index, &global_dentry);

  if(check == -1)
  {
    return 0;
  }

  name_length = strlen((int8_t*)global_dentry.file_name);

  if(name_length > RESERVED32B)
  {
    name_length = RESERVED24B;
  }
  for(i = 0; i < name_length; i++)
  {
    ((uint8_t*)buf)[i] = global_dentry.file_name[i];
  }

  return i;
}
