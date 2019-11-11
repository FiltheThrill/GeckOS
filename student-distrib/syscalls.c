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
  uint32_t esp_ret, ebp_ret;
  int i, root_check = 0;

  PCB_t* parent = PCB_six[c_process_num]->parent_process;
  esp_ret = parent->esp;
  ebp_ret = parent->ebp;

  if(c_process_num != 0)
  {
    root_check = 1;
  }
  if(root_check == 0)
  {
    for(i = 0; i < MAXFILES; i++)
    {
      PCB_six[c_process_num]->file_array[i].f_op_tbl_ptr = 0;
      PCB_six[c_process_num]->file_array[i].inode = -1;
      PCB_six[c_process_num]->file_array[i].f_pos = 0;
      PCB_six[c_process_num]->file_array[i].flags = 0;
    }

    PCB_six[c_process_num]->process_on = 0;
    c_process_num = parent->index;
    p_process_num = parent->p_index;
  }
  tss.esp0 = (uint32_t)(EIGHTMB - ((c_process_num + 1) * EIGHTKB) - 4);
  page_directory[PAGE128] = ((EIGHTMB + (c_process_num * FOURMB)) | SURP);
  asm volatile(
    "movl %%cr3, %%eax\n"
    "movl %%eax, %%cr3\n"
    :
    :
    :"eax"
  );
  uint32_t temp = (uint32_t)status;
  asm volatile(
    "movl %0, %%ebp\n"
    "movl %1, %%esp\n"
    "movl %2, %%eax\n"
    :
    :"r"(ebp_ret), "r"(esp_ret), "r"(temp)
  );
  asm volatile(
    "leave\n"
    "ret\n"
  );
  return status;
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
      PCB_six[i]->index = i;
      process_num = i;
      c_process_num = i;
      check = 0;
      break;
    }
  }

  if(check == -1)   //a max amount of 6 processes can be running at any time
  {
    uint8_t error_buf2[] = "No available processes";
    term_write(1, error_buf2, ERROR_SIZE);
    return -1;
  }



/*+++++++++++++++++++++++++++++ PART 1: Parse Command +++++++++++++++++++++++++++++++++++++++++++++++*/

  len = strlen((int8_t*) command);

  for(i = 0; i < len; i++)  //find where first word ends
  {
    if((command[i] == ' ') || (command[i] ==  '\0') || command[i] == '\n')   //space separated command
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
    uint8_t error_buf3[] = "Couldn't find exectuable file";
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
  page_directory[PAGE128] = ((EIGHTMB + (process_num * FOURMB)) | SURP);
  //flush tlb everytime a new process is made
  //flush_tlb();
  asm volatile(
    "movl %%cr3, %%eax\n"
    "movl %%eax, %%cr3\n"
    :
    :
    :"eax"
  );


/*+++++++++++++++++++++++++++++ PART 4: Program Loader +++++++++++++++++++++++++++++++++++++++++++++++*/

  //get length in bytes of our inode
  inode_t* curr_inode = (inode_t*)(word1.inode_num + inode_addr);
  int32_t length_bytes = curr_inode->length_in_B;

  //load our program into the program image at 0x08048000
  check = read_data(word1.inode_num, 0, (uint8_t*)VIRTUAL_ADDR, length_bytes);

  if(check == -1)
  {
    uint8_t error_buf6[] = "Failed to load program";
    term_write(1, error_buf6, ERROR_SIZE);
    return -1;
  }
/*+++++++++++++++++++++++++++++ PART 5: Create PCB +++++++++++++++++++++++++++++++++++++++++++++++*/

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

//load stdin and stdout into filearray 0 and 1 respectively
PCB_six[process_num]->file_array[0].f_op_tbl_ptr = &stdin;
PCB_six[process_num]->file_array[1].f_op_tbl_ptr = &stdout;
PCB_six[process_num]->file_array[0].inode = -1;
PCB_six[process_num]->file_array[0].f_pos = 0;
PCB_six[process_num]->file_array[0].flags = 1;
PCB_six[process_num]->file_array[1].inode = -1;
PCB_six[process_num]->file_array[1].f_pos = 0;
PCB_six[process_num]->file_array[1].flags = 1;

//initialize rest of file array
for(i = 2; i < 8; i++)
{
  PCB_six[process_num]->file_array[i].f_op_tbl_ptr = 0;
  PCB_six[process_num]->file_array[i].inode = -1;
  PCB_six[process_num]->file_array[i].f_pos = 0;
  PCB_six[process_num]->file_array[i].flags = 0;

}

// if this is is our first process make its own parent,
// otherwise use the last process as parent
if(process_num == 0)
{
  PCB_six[process_num]->parent_process = PCB_six[process_num];
  p_process_num = process_num;
  PCB_six[process_num]->p_index = p_process_num;
}
else
{
  PCB_six[process_num]->parent_process = PCB_six[p_process_num];
  p_process_num = process_num;
  PCB_six[process_num]->p_index = p_process_num;
}
/*+++++++++++++++++++++++++++++ PART 6: Context Switch +++++++++++++++++++++++++++++++++++++++++++++++*/

  // update esp and ss
  PCB_six[process_num]->prev_esp0 = tss.esp0;
  PCB_six[process_num]->prev_ss0 = tss.ss0;

  tss.esp0 = (uint32_t)(EIGHTMB - ((process_num + 1) * EIGHTKB) - 4);
  tss.ss0 = KERNEL_DS;


  uint32_t eip_addr = header[27] << 24;
  eip_addr = eip_addr | (header[26] << 16);
  eip_addr = eip_addr | (header[25] << 8);
  eip_addr = eip_addr | header[24];

  uint32_t base, stack;

  asm volatile(
    "movl %%ebp, %0\n"
    "movl %%esp, %1\n"
    :"=r"(base), "=r"(stack)
  );

  PCB_six[process_num]->ebp = base;
  PCB_six[process_num]->esp = stack;

  //push iret context
  asm volatile(
    //"cli\n"
    "pushl %0\n"
    "pushl $0x083ffffc\n" //128 + 4 mb - 4bytes for last page
    "pushf\n"
    "pushl %1\n"
    "pushl %2\n"
    :
    : "i"(USER_DS), "i"(USER_CS), "r"(eip_addr)
  );
  asm volatile (
    "iret\n"
  );

  return 0; //compilation's sake
}

int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
  if((fd == 1) || (fd > 7) || (fd < 0) || (PCB_six[c_process_num]->file_array[fd].flags == 0))
  {
    return 0;
  }
  return PCB_six[c_process_num]->file_array[fd].f_op_tbl_ptr->read(fd, buf, nbytes);
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{


  if((fd <= 0) || (fd > 7) || (PCB_six[c_process_num]->file_array[fd].flags == 0))
  {
    return 0;
  }
  return PCB_six[c_process_num]->file_array[fd].f_op_tbl_ptr->write(fd, buf, nbytes);
}

int32_t open(const uint8_t* filename)
{
  dentry_t open_dentry;
  int32_t type, check;
  int i, idx;
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

 for(i = 2; i < MAXFILES; i++)
 {
   if(PCB_six[c_process_num]->file_array[i].flags == 0)
    {
      idx = i;
      check = 0;
      PCB_six[c_process_num]->file_array[i].flags = 1;
      break;
    }
 }

if(check == -1)
{
  term_write(1, error_buf2, ERROR_SIZE);
  return -1;
}

switch(type)
  {
    case RTC:

      rtc_table.read = &rtc_read;
      rtc_table.write = &rtc_write;
      rtc_table.open = &rtc_open;
      rtc_table.close = &rtc_close;

      PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr = &rtc_table;
      return PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr->open(filename);

    case DIRECTORY:

      directory_table.read = &dread;
      directory_table.write = &dwrite;
      directory_table.open = &dopen;
      directory_table.close = &dclose;

      PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr = &directory_table;
      return PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr->open(filename);;

    case FILE:

      file_table.read = &fread;
      file_table.write = &fwrite;
      file_table.open = &fopen;
      file_table.close = &fclose;

      PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr = &file_table;
      PCB_six[c_process_num]->file_array[idx].inode = open_dentry.inode_num;
      return PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr->open(filename);;

    default:

      term_write(1, error_buf3, ERROR_SIZE);
      return -1;
  }
}

int32_t close(int32_t fd)
{
  if((fd < 2) || (fd > 7) || (PCB_six[c_process_num]->file_array[fd].flags == 0))
  {
    return -1;
  }
  PCB_six[c_process_num]->file_array[fd].flags = 0;
  return PCB_six[c_process_num]->file_array[fd].f_op_tbl_ptr->close(fd);
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
