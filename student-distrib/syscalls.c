/* yikes
Author - who even cares anymore*/

#include "syscalls.h"
#include "lib.h"
#include "files.h"
#include "keyboard.h"
#include "x86_desc.h"
#include "terminal.h"
#include "paging.h"
#include "x86_desc.h"
#include "rtc.h"


operations_table_t stdin;
operations_table_t stdout;
operations_table_t file_table;
operations_table_t rtc_table;
operations_table_t directory_table;

int32_t p_process_num;
int32_t c_proccess_num;
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
   printf("checkign command arg\n");

  if(command == 0)  //check if command is NULL
  {
    uint8_t error_buf1[] = "Invalid cmd";
    term_write(1, error_buf1, ERROR_SIZE);
    return -1;
  }

  check = -1;

  printf("checking # processes\n");
  for(i = 0; i < MAXPROCESSES; i++)
  {
    if(PCB_six[i]->process_on == 0)
    {
      PCB_six[i]->process_on = 1;
      process_num = i;
      c_proccess_num = i;
      check = 0;
      break;
    }
  }

  if(check == -1)   //a max amount of 6 proccesses can be running at any time
  {
    uint8_t error_buf2[] = "No available processes";
    term_write(1, error_buf2, ERROR_SIZE);
    return -1;
  }



/*+++++++++++++++++++++++++++++ PART 1: Parse Command +++++++++++++++++++++++++++++++++++++++++++++++*/

printf("parsing\n");

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

  printf("trying to find executable\n");

  //check if file name exists
  check = read_dentry_by_name(first_word, &word1);

  if(check == -1)
  {
    uint8_t error_buf3[] = "Couldn't find exectuable file";
    term_write(1, error_buf3, ERROR_SIZE);
    return -1;
  }

  printf("loading 40 bytes\n");

  //load first 40 bytes into header buffer
  check = read_data(word1.inode_num, 0, header, FIRST40);

  if(check == -1)
  {
    uint8_t error_buf4[] = "Read_data failed";
    term_write(1, error_buf4, ERROR_SIZE);
    return -1;
  }

  printf("checking elf magic\n");

  // compare bytes 0, 1, 2, and 3 with magic elf numbers
  if((header[0] != ELF_MAGIC0) || (header[1] != ELF_MAGIC1) ||
     (header[2] != ELF_MAGIC2) || (header[3] != ELF_MAGIC3))
     {
       uint8_t error_buf5[] = "Not an executable";
       term_write(1, error_buf5, ERROR_SIZE);
       return -1;
     }

/*+++++++++++++++++++++++++++++ PART 3: Paging +++++++++++++++++++++++++++++++++++++++++++++++*/

  printf("setting up virtual map\n");

   //set up page directory to map to physical memory and enable S, U , R, and P bits
  page_directory[PAGE128] = ((EIGHTMB + (process_num * FOURMB)) | SURP);
  printf("addr: %x\n", ((EIGHTMB + (process_num * FOURMB)) | SURP));
  printf("addr wout surp: %x\n", (EIGHTMB + (process_num * FOURMB)));
  //flush tlb everytime a new process is made
  flush_tlb();

/*+++++++++++++++++++++++++++++ PART 4: Program Loader +++++++++++++++++++++++++++++++++++++++++++++++*/

  //get length in bytes of our inode
  inode_t* curr_inode = (inode_t*)(word1.inode_num + inode_addr);
  int32_t length_bytes = curr_inode->length_in_B;
  printf("%d\n", length_bytes);
  printf("loading program image\n");
  //load our program into the program image at 0x08048000
  check = read_data(word1.inode_num, 0, (uint8_t*)VIRTUAL_ADDR, length_bytes);

  if(check == -1)
  {
    uint8_t error_buf6[] = "Failed to load program";
    term_write(1, error_buf6, ERROR_SIZE);
    return -1;
  }

/*+++++++++++++++++++++++++++++ PART 5: Create PCB +++++++++++++++++++++++++++++++++++++++++++++++*/
printf("setting up ops table\n");

//setup stdin operations table
stdin.read = &term_read;
stdin.write = 0;
stdin.open = &term_open;
stdin.close = &term_close;

//setup stdout operations table
stdout.read = 0;
stdout.write = &term_write;
stdout.open = &term_open;
stdout.close = &term_close;

printf("mapping to pcb\n");

//load stdin and stdout into filearray 0 and 1 respectively
PCB_six[process_num]->file_array[0].f_op_tbl_ptr = &stdin;
PCB_six[process_num]->file_array[1].f_op_tbl_ptr = &stdout;
PCB_six[process_num]->file_array[0].inode = -1;
PCB_six[process_num]->file_array[0].f_pos = 0;
PCB_six[process_num]->file_array[0].flags = 1;
PCB_six[process_num]->file_array[1].inode = -1;
PCB_six[process_num]->file_array[1].f_pos = 0;
PCB_six[process_num]->file_array[1].flags = 1;
printf("mapping to rest of pcb\n");

//initialize rest of file array
for(i = 2; i < 8; i++)
{
  PCB_six[process_num]->file_array[i].f_op_tbl_ptr = 0;
  PCB_six[process_num]->file_array[i].inode = -1;
  PCB_six[process_num]->file_array[i].f_pos = 0;
  PCB_six[process_num]->file_array[i].flags = 1;

}

printf("assigning parents\n");

// if this is is our first proccess make its own parent,
// otherwise use the last process as parent
if(process_num == 0)
{
  PCB_six[process_num]->parent_process = PCB_six[process_num];
  p_process_num = process_num;
}
else
{
  PCB_six[process_num]->parent_process = PCB_six[p_process_num];
  p_process_num = process_num;
}

/*+++++++++++++++++++++++++++++ PART 6: Context Switch +++++++++++++++++++++++++++++++++++++++++++++++*/
  //lol what
  // update esp and ss
  printf("updating tss\n");

  PCB_six[process_num]->prev_esp0 = tss.esp0;
  PCB_six[process_num]->prev_ss0 = tss.ss0;

  tss.esp0 = (uint32_t)(EIGHTMB - ((process_num + 1) * EIGHTKB) - 4);
  tss.ss0 = KERNEL_DS;
  printf("finding eip\n");

  uint32_t eip_addr = header[27] << 24;
  eip_addr = eip_addr | (header[26] << 16);
  eip_addr = eip_addr | (header[25] << 8);
  eip_addr = eip_addr | header[24];

  printf("27: %x\n", header[27]);
  printf("26: %x\n", header[26]);
  printf("25: %x\n", header[25]);
  printf("24: %x\n", header[24]);
  printf("23: %x\n", header[23]);
  printf("eip: %x\n", eip_addr);

  //push iret context
  asm volatile(
    "pushl %0\n"
    "pushl $0x083ffffc\n" //128 + 4 mb - 4bytes for last page
    "pushfl\n"
    "pushl %1\n"
    "pushl %2\n"
    :
    :"r"(USER_DS), "r"(USER_CS), "r"(eip_addr)
  );
  printf("pushed shit\n");
  asm ("iret");
  printf("?\n");
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
  dentry_t open_dentry;
  int32_t type, check;
  int i;
  uint8_t error_buf1[] = "Failed to open file by name";
  uint8_t error_buf2[] = "No descriptors are free";
  uint8_t error_buf3[] = "File type doesn't match";

  check = read_dentry_by_name(filename, &open_dentry);

  if(check == -1)
  {
    term_write(1, error_buf1, ERROR_SIZE);
    return -1;
  }

  check = -1;
  type = open_dentry.file_type;

  switch(type)
  {
    case RTC:

      rtc_table.read = &rtc_read;
      rtc_table.write = &rtc_write;
      rtc_table.open = &rtc_open;
      rtc_table.close = &rtc_close;

      for(i = 0; i < MAXPROCESSES; i++)
      {
        if(PCB_six[c_proccess_num]->file_array[i].f_op_tbl_ptr == 0);
          {
            check = 0;
            PCB_six[c_proccess_num]->file_array[i].f_op_tbl_ptr = &rtc_table;
          }
      }

      if(check != 0)
      {
        term_write(1, error_buf2, ERROR_SIZE);
        return -1;
      }

      return 0;

    case DIRECTORY:

      directory_table.read = &dread;
      directory_table.write = &dwrite;
      directory_table.open = &dopen;
      directory_table.close = &dclose;

      for(i = 0; i < MAXPROCESSES; i++)
      {
        if(PCB_six[c_proccess_num]->file_array[i].f_op_tbl_ptr == 0);
          {
            check = 0;
            PCB_six[c_proccess_num]->file_array[i].f_op_tbl_ptr = &directory_table;
          }
      }

      if(check != 0)
      {
        term_write(1, error_buf2, ERROR_SIZE);
        return -1;
      }

      return 0;

    case FILE:

      file_table.read = &fread;
      file_table.write = &fwrite;
      file_table.open = &fopen;
      file_table.close = &fclose;

      for(i = 0; i < MAXPROCESSES; i++)
      {
        if(PCB_six[c_proccess_num]->file_array[i].f_op_tbl_ptr == 0);
          {
            check = 0;
            PCB_six[c_proccess_num]->file_array[i].f_op_tbl_ptr = &file_table;
          }
      }

      if(check != 0)
      {
        term_write(1, error_buf2, ERROR_SIZE);
        return -1;
      }

      return 0;

    default:

      term_write(1, error_buf3, ERROR_SIZE);
      return -1;
  }
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
