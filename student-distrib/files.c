/* parsing read only file system including opening and reading from
files, opening and reading the directory
Author - Filip Cakulev
https://wiki.osdev.org/File_Systems
https://littleosbook.github.io/#inodes-and-writable-file-systems
diagram in appendix A in mp3 documentation is hella clutch as well*/

#include "files.h"
#include "lib.h"
#include "x86_desc.h"
#include "syscalls.h"
#include "terminal.h"
#include "pit.h"

boot_block_t* boot_block;
dentry_t global_dentry;
int d_index = 0;
/*
 * files_init
 *   DESCRIPTION: assignes start address for the boot block and start address for inodes
 *   INPUTS: int32_t start:   module_t* mod = (module_t*)mbi->mods_addr;
                              boot = (int32_t)mod->mod_start; where start equals boot
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void files_init(int32_t start)
{
  //first 4kb block is accopied by boot block. Inode addresses start thereafter
  boot_block = (boot_block_t*)start;
  inode_addr = (inode_t*)(start + FOURKB);
}

/*
 * read_dentry_by_name
 *   DESCRIPTION: given a file name will search through dir_entries to find file
 *   INPUTS: fname: name of file
            dentry: dentry that we are copying the file information to.
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
  int i;
  int found;
  int8_t* blank = "";

  if(fname == 0){   //if pointer to file is null return -1
    return FAILURE;
  }

  //check if empty string
  found = strncmp((const int8_t*)fname, (const int8_t*)blank, RESERVED32B);
  if(found == 0)
  {
    return FAILURE;
  }

  //loop through file names to see if it can be found
  for(i = 0; i < FILESMAX; i++)
  {
    found = strncmp((const int8_t*)fname, (const int8_t*)boot_block->dir_entries[i].file_name, RESERVED32B);

    if(found == SUCCESS)
    {
      //copy over to dentry
      memcpy(dentry, (const void*)&boot_block->dir_entries[i], SIXTYFOURBYTES);
      return SUCCESS;
    }
  }
  return FAILURE;   //file wasnt found
}

/*
 * read_dentry_by_index
 *   DESCRIPTION: given a file index will search through dir_entries to find file
 *   INPUTS: index: index of file
            dentry: dentry that we are copying the file information to.
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success -1 for failure
 *   SIDE EFFECTS: none
 */
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

/*
 * read_data
 *   DESCRIPTION: places information from datablocks assosciated with a particular inode into a buf
 *   INPUTS: inode: the inode # of the file we are dealing with
            offset: how many bytes into the file we want to start reading from
            buf: the buffer we are reading into
            length: how many bytes we want to read
 *   OUTPUTS: none
 *   RETURN VALUE:  the number of bytes read
 *   SIDE EFFECTS: none
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
  int32_t cpy_length, inode_len, inode_num, offset_blocks, offset_bytes, data_idx;
  int bytes_read = 0;
  //int32_t offset_length;
  uint8_t* buf_addr;
  uint8_t* data_blocks;
  int i= 0;

  inode_num =  boot_block->num_inodes;


  if((inode < 0) || (inode >= inode_num))  // check if inode is in bounds
  {
  //  printf("inode not in bounds\n");
    return FAILURE;
  }

  inode_len = inode_addr[inode].length_in_B;

  if(inode_len <= 0)   //check if file is empty
  {
//    printf("no information in this inode\n");
    return FAILURE;
  }

  if(offset >= inode_len){    //return 0 bytes read if offset it too large
    return bytes_read;
  }

  //check to see if offset + length is more bytes than the file has
  if(offset + length <= inode_len)
  {
    cpy_length = length;
  }
  else  // needed or else it will page fault
  {
    cpy_length = inode_len - offset;
  }


  //values for indexing
  data_blocks = (uint8_t*)(inode_addr + inode_num);  //first data block address (uint8_t*?)
  offset_blocks = offset / FOURKB;  //number of complete blocks
  offset_bytes = offset % FOURKB;    //number of bytes into the last block

  while(1)
  {
    //get info on which data block we are reading from
    data_idx = inode_addr[inode].data_block_idx[offset_blocks];

    for(offset_bytes = offset_bytes; offset_bytes < FOURKB; offset_bytes++)
    {
      //4kb * #blocks + offset bytes + starting address for data blocks
      buf_addr = (uint8_t*)(data_blocks + offset_bytes + data_idx * FOURKB);
      buf[i] = *buf_addr;

      //increment and decrement values accordingly
      i++;
      bytes_read++;
      cpy_length--;

      //needed or else it will page fault
      if(cpy_length <= 0)
      {
        return bytes_read;
      }
    }
    //move to next data block to be read and reset what byte we are at since its a new block
    offset_blocks++;
    offset_bytes = 0;
  }

  return bytes_read;  //shouldnt get here but just in case
}

/*
 * fopen
 *   DESCRIPTION: given a file name checks if it can be found
 *   INPUTS: fname: name of file
 *   OUTPUTS: none
 *   RETURN VALUE:  0 for success -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t fopen(const uint8_t* fname)
{
  return SUCCESS;
}

/*
 * fclose
 *   DESCRIPTION: closes the file?
 *   INPUTS: fname: name of file
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t fclose(int32_t fd)
{
  return SUCCESS;
}

/*
 * fwrite
 *   DESCRIPTION: this is a read only file system
 *   INPUTS: fname: name of file
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t fwrite(int32_t fd, const void* buf, int32_t nbytes)
{
  return FAILURE;
}

/*
 * fread
 *   DESCRIPTION: places information from datablocks assosciated with a particular inode into a buf
 *   INPUTS: offset: how many bytes into the file we want to start reading from
             buf: the buffer we are reading into
             length: how many bytes we want to read
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes read
 *   SIDE EFFECTS: none
 */
int32_t fread(int32_t fd, void* buf, int32_t nbytes)
{
  int32_t bytes_read;
  uint32_t inode, offset;

  inode = PCB_arr[terminals[curterm_nodisp].process_idx]->file_array[fd].inode;
  offset = PCB_arr[terminals[curterm_nodisp].process_idx]->file_array[fd].f_pos;

  bytes_read = read_data(inode, offset, buf, nbytes);
  PCB_arr[terminals[curterm_nodisp].process_idx]->file_array[fd].f_pos += bytes_read;

  return bytes_read;
}

/*
 * dopen
 *   DESCRIPTION: given a directory name checks if it can be found
 *   INPUTS: fname: name of directory
 *   OUTPUTS: none
 *   RETURN VALUE:  0 for success -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t dopen(const uint8_t* fname)
{
 return SUCCESS;
}

/*
 * dclose
 *   DESCRIPTION: closes the directory?
 *   INPUTS: fname: name of directory
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t dclose(int32_t fd)
{
  return SUCCESS;
}
/*
 * fwrite
 *   DESCRIPTION: this is a read only file system
 *   INPUTS: fname: name of file
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t dwrite(int32_t fd, const void* buf, int32_t nbytes)
{
  return FAILURE;
}

/*
 * dread
 *   DESCRIPTION: places information from dir_entries
 *   INPUTS: buf: the buffer we are reading into
             d_index: which index to read from
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes read
 *   SIDE EFFECTS: none
 */
int32_t dread(int32_t fd, void* buf, int32_t nbytes)
{
  int i;
  int32_t check;
  uint32_t name_length;
  dentry_t read_dentry;
  int32_t bytes_read = 0;

  if(d_index > 16)//check if its in bounds of the directories
  {
    d_index =  0;
    return bytes_read;
  }

  check = read_dentry_by_index(d_index, &read_dentry);  //finds directory by index and checks if it found it

  if(check == FAILURE)
  {
    return bytes_read;
  }

  name_length = strlen((int8_t*)read_dentry.file_name);

  if(name_length > RESERVED32B) //make sure name is less than 32
  {
    name_length = RESERVED32B;
  }
  //printf("%d\n", name_length);

  for(i = 0; i < name_length; i++)  //place directories into buf
  {
    ((uint8_t*)buf)[i] = read_dentry.file_name[i];
    bytes_read++;
  }
  // if(name_length != RESERVED32B)
  // {
  //   ((uint8_t*)buf)[i+1] = ' ';
  // }
  //printf("i:%d d:%d ", i, d_index);
  d_index++;
  return bytes_read;
}
