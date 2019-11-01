/* yikes
Author - who even cares anymore*/

#include "syscalls.h"
#include "lib.h"
#include "files.h"
#include "keyboard.h"
#include "x86_desc.h"
#include "terminal.h"
#include "paging.h"


operations_table_t STDIN;
operations_table_t STDOUT;
operations_table_t file_table;
operations_table_t rtc_table;
operations_table_t directory_table;

void PCB_start()
{
  int i;
  for(i = 0; i < MAXPROCESSES; i++)
  {
    PCB_six[i] = (PCB_t*)(EIGHTMB - ((i + 1) * EIGHTKB));
    PCB_six[i]->process_on = 0;
  }
}



int32_t halt(uint8_t status)
{
  return 0;
}

int32_t execute(const uint8_t* command)
{
  int32_t process_num;
  uint8_t first_word[WORD_SIZE];    //128?
  uint8_t rest_of_word[WORD_SIZE];
  int32_t len;
  int len_word1, i, check;
  dentry_t word1;
  uint8_t header[FIRST40];

  cli();

  if(command == 0)  //check if command is NULL
  {
    uint8_t error_buf1[] = "Invalid cmd";
    term_write(1, error_buf1, ERROR_SIZE);
    return -1;
  }

  check = -1;

  for(i = 0; i < MAXPROCESSES; i++)
  {
    if(PCB_six[i]->process_on == 0)
    {
      PCB_six[i]->process_on = 1;
      process_num = i;
      check = 0;
      break;
    }
  }

  if(check == -1)   //a max amount of 6 proccesses can be running at any time
  {
    uint8_t error_buf2[] = "Invalid cmd";
    term_write(1, error_buf2, ERROR_SIZE);
    return -1;
  }



/*+++++++++++++++++++++++++++++ PART 1: Parse Command +++++++++++++++++++++++++++++++++++++++++++++++*/

  len = strlen((int8_t*) command);

  for(i = 0; i < len; i++)  //find where first word ends
  {
    if(command[i] == ' ')   //space separated command
    {
      len_word1 = i;
      break;
    }
  }

  if(len_word1 == 0)    //there is only one word
  {
    len_word1 = len;
  }

  for(i = 0; i < len_word1; i++)  //place into first word buffer
  {
    first_word[i] = command[i];
  }
  first_word[len_word1] = '\0'; //add end indentifier

  if(len_word1 < len)   //presumambly not needed but put rest of word into another buffer
  {
    for(i = len_word1 + 1; i < len; i++)
    {
      rest_of_word[i - len_word1 - 1] = command[i];
    }
    rest_of_word[i - len_word1] = '\0';
  }
/*+++++++++++++++++++++++++++++ PART 2: Executable check +++++++++++++++++++++++++++++++++++++++++++++++*/

  //check if file name exists
  check = read_dentry_by_name(first_word, &word1);

  if(check == -1)
  {
    uint8_t error_buf3[] = "Couldn't find file";
    term_write(1, error_buf3, ERROR_SIZE);
    return -1;
  }

  //load first 40 bytes into header buffer
  check = read_data(word1.inode_num, 0, header, FIRST40);

  if(check == -1)
  {
    uint8_t error_buf4[] = "Read_data failed";
    term_write(1, error_buf4, ERROR_SIZE);
    return -1;
  }

  // compare bytes 0, 1, 2, and 3 with magic elf numbers
  if((header[0] != ELF_MAGIC0) || (header[1] != ELF_MAGIC1) ||
     (header[2] != ELF_MAGIC2) || (header[3] != ELF_MAGIC3))
     {
       uint8_t error_buf5[] = "Not an executable";
       term_write(1, error_buf5, ERROR_SIZE);
       return -1;
     }

/*+++++++++++++++++++++++++++++ PART 3: Paging +++++++++++++++++++++++++++++++++++++++++++++++*/

   //set up page directory to map to physical memory and enable S, U , R, and P bits
  page_directory[PAGE128] = (EIGHTMB + (process_num * FOURMB)) | SURP;
  //flush tlb everytime a new process is made
  flush_tlb();

/*+++++++++++++++++++++++++++++ PART 4: Program Loader +++++++++++++++++++++++++++++++++++++++++++++++*/

  //get length in bytes of our inode
  inode_t* curr_inode = (inode_t*)(word1.inode_num + inode_addr);
  int32_t length_bytes = curr_inode->length_in_B;

  //load our program into the program image at 0x08048000
  check = read_data(word1.inode_num, 0, (uint8_t*)VIRTUAL_ADDR, length_bytes);

/*+++++++++++++++++++++++++++++ PART 5: Create PCB +++++++++++++++++++++++++++++++++++++++++++++++*/

//need terminal working for this part.



/*+++++++++++++++++++++++++++++ PART 6: Context Switch +++++++++++++++++++++++++++++++++++++++++++++++*/
  return 0; //compilation's sake
}

int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
  return 0;
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
  return 0;
}

int32_t open(const uint8_t* filename)
{
  return 0;
}

int32_t close(int32_t fd)
{
  return 0;
}

int32_t getargs(uint8_t* buf, int32_t nbytes)
{
  return 0;
}

int32_t vidmap(uint8_t** screen_start)
{
  return 0;
}

int32_t set_handler(int32_t signum, void* handler_address)
{
  return 0;
}

int32_t sigreturn(void)
{
  return 0;
}
