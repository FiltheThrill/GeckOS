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
#include "IDT.h"


operations_table_t stdin;
operations_table_t stdout;
operations_table_t file_table;
operations_table_t rtc_table;
operations_table_t directory_table;
static void* sig_map[] = {kill,kill,kill,ignore,ignore};
/*
 * PCB_start
 *   DESCRIPTION: initializes PCB array at bottom of the 4mb kernel stack, each
                  process's will be 8kb above the last PCB
 *   INPUTS: fname: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes PCB array
 */
void PCB_start()
{
  int i;
  for(i = 0; i < MAXPROCESSES; i++)
  {
    PCB_six[i] = (PCB_t*)(EIGHTMB - ((i + 1) * EIGHTKB));
    PCB_six[i]->process_on = 0;
  }

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

  //setup function jump table for rtc
  rtc_table.read = &rtc_read;
  rtc_table.write = &rtc_write;
  rtc_table.open = &rtc_open;
  rtc_table.close = &rtc_close;

  //setup function jump table for directory
  directory_table.read = &dread;
  directory_table.write = &dwrite;
  directory_table.open = &dopen;
  directory_table.close = &dclose;

  //setup function jupm table for file
  file_table.read = &fread;
  file_table.write = &fwrite;
  file_table.open = &fopen;
  file_table.close = &fclose;

  return;
}
/*
 * halt
 *   DESCRIPTION: system call halt resets the current process's pcb and returns
                  to its parent process's pcb
 *   INPUTS: status
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success -1 for failure
 *   SIDE EFFECTS: returns to parent process, changes ebp and esp and tss
 */
int32_t halt(int8_t status)
{
  uint32_t esp_ret, ebp_ret;
  int i, root_check = 0;

  //set up parent proccess essentials before wiping
  PCB_t* parent = PCB_six[c_process_num]->parent_process;
  esp_ret = PCB_six[c_process_num]->esp;
  ebp_ret = PCB_six[c_process_num]->ebp;
  tss.esp0 = PCB_six[c_process_num]->prev_esp0;

  // wipe information in PCB
  for(i = 0; i < MAXFILES; i++)
  {
    PCB_six[c_process_num]->file_array[i].f_op_tbl_ptr = 0;
    PCB_six[c_process_num]->file_array[i].inode = -1;
    PCB_six[c_process_num]->file_array[i].f_pos = 0;
    PCB_six[c_process_num]->file_array[i].flags = 0;
  }

  for(i = 0; i<MAXARGS; i++){
    PCB_six[c_process_num]->args[i] = 0;
  }

  //change back any of the editied handlers
  for(i = 0; i<SIG_CNT; i++){
    PCB_six[c_process_num]->sig_arr[i] = sig_map[i];
  }
  if(c_process_num == 0)
  {
    root_check = 1;
  }

  //if not the  root reset everything and reset the current process number
  if(root_check == 0)
  {
    PCB_six[c_process_num]->argsize = 0;
    PCB_six[c_process_num]->process_on = 0;
    PCB_six[c_process_num]->ebp = 0;
    PCB_six[c_process_num]->esp = 0;
    PCB_six[c_process_num]->prev_ss0 = 0;
    PCB_six[c_process_num]->prev_esp0 = 0;
    PCB_six[c_process_num]->index = -1;
    PCB_six[c_process_num]->p_index = -1;
    PCB_six[c_process_num]->parent_process = 0;
    c_process_num = parent->index;
    p_process_num = parent->p_index;
  }

  //update page directory entry
  page_directory[PAGE128] = ((EIGHTMB + (c_process_num * FOURMB)) | SURP);

  //flush tlb
  asm volatile(
    "movl %%cr3, %%eax\n"
    "movl %%eax, %%cr3\n"
    :
    :
    :"eax"
  );


  int32_t temp = (int32_t)status;
  if(exception_flag == 1)
  {
    temp = 256;
    exception_flag = 0;
  }
  // update return value
  asm volatile(
      "movl %0, %%eax\n"
      :
      :"r"(temp)
    );

  //update stack pointer
  asm volatile(
      "movl %%ebx, %%esp\n"
      :
      :"b"(esp_ret)
    );

  //update base pointer
  asm volatile(
      "mov %%ebx, %%ebp\n"
      :
      :"b"(ebp_ret)
    );

//leave and return "from"" execute
  asm volatile(
    "leave\n"
    "ret"
    :
    :
  );

  return temp;  //compilation's sake; never gets here
}
/*
 * execute
 *   DESCRIPTION: system call halt searches for space to execute a process, setting up
                  a page directory entry that maps to a physical space where the
                  process will run
 *   INPUTS: command: name of executable that needs to be run plus any argument following
                      it
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success -1 for failure
 *   SIDE EFFECTS: sets up a page directory entry, flushes tlb, fakes an iret, initializes
                    stdin and stdout
 */
int32_t execute(const uint8_t* command)
{
  int32_t process_num;
  uint8_t first_word[WORD_SIZE];    //128?
  uint8_t rest_of_word[WORD_SIZE];
  int32_t len;
  int len_word1 = 0, i, check, flag = 0, error_flag = 0;
  dentry_t word1;
  uint32_t base, stack;
  uint8_t header[FIRST40];

  if(command == 0)  //check if command is NULL
  {
    error_flag = 1;
  }

  check = FAILURE;

  for(i = 0; i < MAXPROCESSES; i++)
  {
    if(PCB_six[i]->process_on == 0)
    {
      PCB_six[i]->process_on = 1;
      PCB_six[i]->index = i;
      process_num = i;
      c_process_num = i;
      check = SUCCESS;
      break;
    }
  }

  if(check == FAILURE)   //a max amount of 6 processes can be running at any time
  {
    error_flag = 1;
  }

/*+++++++++++++++++++++++++++++ PART 1: Parse Command +++++++++++++++++++++++++++++++++++++++++++++++*/

  len = strlen((int8_t*) command);

  for(i = 0; i < len; i++)  //find where first word ends
  {
    if(command[i] ==  ' ')   //space separated command
    {
      len_word1 = i;
      flag = 1;
      break;
    }
  }

  if(flag == 0)    //there is only one word
  {
    len_word1 = len;
  }

  for(i = 0; i < len_word1; i++)  //place into first word buffer
  {
    first_word[i] = command[i];
  }
  first_word[len_word1] = '\0'; //add end indentifier

  if(flag == 1)   //put rest of word into another buffer
  {
    for(i = len_word1 + 1; i < len; i++)
    {
      rest_of_word[i - len_word1 - 1] = command[i];
    }
    rest_of_word[i - len_word1 - 1] = '\0';
    PCB_six[c_process_num]->argsize = strlen((int8_t*)rest_of_word);

    for(i = 0; i < PCB_six[c_process_num]->argsize; i++)
    {
      PCB_six[c_process_num]->args[i] = rest_of_word[i];
    }
  }

  //save the entered commandand set argsize
  i=len_word1;
  while(i<len && i<MAXARGS){
    PCB_six[c_process_num]->args[i] = rest_of_word[i];
    i++;
  }
  i++;
  PCB_six[c_process_num]->argsize = i-len_word1;

/*+++++++++++++++++++++++++++++ PART 2: Executable check +++++++++++++++++++++++++++++++++++++++++++++++*/

  check = read_dentry_by_name(first_word, &word1);

  if(check == FAILURE)
  {
    error_flag = 1;
  }

  //load first 40 bytes into header buffer
  if(error_flag == 0 && exception_flag == 0)
  {
    check = read_data(word1.inode_num, 0, header, FIRST40);
  }

  if(check == FAILURE)
  {
    error_flag = 1;
  }


  // compare bytes 0, 1, 2, and 3 with magic elf numbers
  if((header[0] != ELF_MAGIC0) || (header[1] != ELF_MAGIC1) ||
     (header[2] != ELF_MAGIC2) || (header[3] != ELF_MAGIC3))
     {
       error_flag = 1;
     }

/*+++++++++++++++++++++++++++++ PART 3: Paging +++++++++++++++++++++++++++++++++++++++++++++++*/

   //set up page directory to map to physical memory and enable S, U , R, and P bits
  page_directory[PAGE128] = ((EIGHTMB + (process_num * FOURMB)) | SURP);

  //flush tlb everytime a new process is made
  asm volatile(
    "movl %%cr3, %%eax\n"
    "movl %%eax, %%cr3\n"
    :
    :
    :"eax"
  );

/*+++++++++++++++++++++++++++++ PART 4: Program Loader +++++++++++++++++++++++++++++++++++++++++++++++*/

  //get length in bytes of our inode
  if(error_flag == 0 && exception_flag == 0)
  {
    inode_t* curr_inode = (inode_t*)(word1.inode_num + inode_addr);
    int32_t length_bytes = curr_inode->length_in_B;


    //load our program into the program image at 0x08048000
    check = read_data(word1.inode_num, 0, (uint8_t*)VIRTUAL_ADDR, length_bytes);
  }

  if(check == FAILURE)
  {
    error_flag = 1;
  }
/*+++++++++++++++++++++++++++++ PART 5: Create PCB +++++++++++++++++++++++++++++++++++++++++++++++*/

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
for(i = MINFD; i < MAXFILES; i++)
{
  PCB_six[process_num]->file_array[i].f_op_tbl_ptr = 0;
  PCB_six[process_num]->file_array[i].inode = -1;
  PCB_six[process_num]->file_array[i].f_pos = 0;
  PCB_six[process_num]->file_array[i].flags = 0;
}

//init default signals
for(i = 0; i<SIG_CNT; i++){
  PCB_six[c_process_num]->sig_arr[i] = sig_map[i];
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

  tss.esp0 = (uint32_t)(EIGHTMB - ((process_num + 1) * EIGHTKB) - PAGESIZE);
  tss.ss0 = KERNEL_DS;

  //get eip address from bytes 27-24 in the header array
  uint32_t eip_addr = header[27] << 24;
  eip_addr = eip_addr | (header[26] << 16);
  eip_addr = eip_addr | (header[25] << 8);
  eip_addr = eip_addr | header[24];

  //store ebp and esp for eventual return to execute from halt
  asm volatile(
    "movl %%esp, %0\n"
    "movl %%ebp, %1\n"
    :"=r"(stack), "=r"(base)
  );
  PCB_six[process_num]->ebp = base;
  PCB_six[process_num]->esp = stack;

  if(error_flag == 1 || exception_flag == 1) //halt if an error was encountered
  {
    halt(FAILURE);
  }

  //push iret context
  asm volatile(
    "cli\n"
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

  return SUCCESS; //compilation's sake; never gets here
}

/*
 * read
 *   DESCRIPTION: system call read uses appropriate read based on type of file found in open
 *   INPUTS: fd - the index in the file array we are trying to read
             buf - the buffer we are trying to read from
             nbytes - the amount of bytes we want to read
 *   OUTPUTS: none
 *   RETURN VALUE: whatever the appropriate read returns
 *   SIDE EFFECTS: none
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
  //check if its trying to read using stdwrite, an fd out of bounds or a file channel thats not active
   if((fd == STDOUTFD) || (fd > MAXFD) || (fd < 0) || (PCB_six[c_process_num]->file_array[fd].flags == 0))
   {
     return FAILURE;
   }
   //call appropriate read
  return PCB_six[c_process_num]->file_array[fd].f_op_tbl_ptr->read(fd, buf, nbytes);
}

/*
 * write
 *   DESCRIPTION: system call write uses appropriate read based on type of file found in open
 *   INPUTS:  fd - the index in the file array we are trying to write
             buf - the buffer we are trying to write to
             nbytes - the amount of bytes we want to write
 *   OUTPUTS: none
 *   RETURN VALUE: whatever the appropriate write returns
 *   SIDE EFFECTS: none
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
  //check if its tring to write using stdin or an fd out of bounds or an fd that isnt active
  if((fd <= STDINFD) || (fd > MAXFD) || (PCB_six[c_process_num]->file_array[fd].flags == 0))
  {
    return FAILURE;
  }
  //call appropriate write
  return PCB_six[c_process_num]->file_array[fd].f_op_tbl_ptr->write(fd, buf, nbytes);
}

/*
 * open
 *   DESCRIPTION: system call open sets up the file array based on the type of file
 *   INPUTS:  filename - the name of the file we are trying to open
 *   OUTPUTS: none
 *   RETURN VALUE: whatever the appropriate open returns or -1 for failure
 *   SIDE EFFECTS: changes file array of current process
 */
int32_t open(const uint8_t* filename)
{
  dentry_t open_dentry;
  int32_t type, check;
  int i, idx;

  //find file by read_dentry_by_name
  check = read_dentry_by_name(filename, &open_dentry);

  if(check == FAILURE)
  {
    return FAILURE;
  }

  check = FAILURE;
  type = open_dentry.file_type; //find if its a file, directory or rtc

  //find an open file array index that is not in use and set up inode and file position
 for(i = MINFD; i < MAXFILES; i++)
 {
   if(PCB_six[c_process_num]->file_array[i].flags == 0)
    {
      idx = i;
      check = SUCCESS;
      PCB_six[c_process_num]->file_array[idx].inode = open_dentry.inode_num;
      PCB_six[c_process_num]->file_array[idx].f_pos = 0;
      PCB_six[c_process_num]->file_array[i].flags = 1;
      break;
    }
 }

if(check == FAILURE)
{
  return FAILURE;
}

switch(type)
  {
    case RTC:

      //assign table
      PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr = &rtc_table;

      //use rtc open
       check = PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr->open(filename);
       return idx;
    case DIRECTORY:

      //assign table
      PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr = &directory_table;

      //use directory open
      check = PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr->open(filename);
      return idx;

    case FILE:

      //assign tables
      PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr = &file_table;

      //use file open
      check = PCB_six[c_process_num]->file_array[idx].f_op_tbl_ptr->open(filename);
      return idx;
    default:

      //somehow type wasn't any of the above 3
      return FAILURE;
  }
}

/*
 * close
 *   DESCRIPTION: system call close sets flag to 0 to indicate its not in use
 *   INPUTS:  fd - the file array index we want to close
 *   OUTPUTS: none
 *   RETURN VALUE: whatever the appropriate close returns or -1 for failure
 *   SIDE EFFECTS: changes flags
 */
int32_t close(int32_t fd)
{
  //check if it is trying to close stdin or stdout, an FD larger than 8 or if the file channel is active
  if((fd < MINFD) || (fd > MAXFD) || (PCB_six[c_process_num]->file_array[fd].flags == 0))
  {
    return FAILURE;
  }

  //set to non active channel and call appropriate close function
  PCB_six[c_process_num]->file_array[fd].flags = 0;
  return PCB_six[c_process_num]->file_array[fd].f_op_tbl_ptr->close(fd);
}
/*
 * getargs
 *   DESCRIPTION: system call for taking cmd line args into user level buffer
 *   INPUTS:  buf - the array in memory where the args are stored
 *            nbytes - the number of bytes avaliable in the buf
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on successful write or -1 on failure
 *   SIDE EFFECTS: writes to the args array
 */
int32_t getargs(uint8_t* buf, int32_t nbytes)
{
  int32_t i;
  int32_t len;
  //check for invalis cases (no args, NULL)
  if(buf == NULL || nbytes == 0){
    return FAILURE;
  }

  len = PCB_six[c_process_num]->argsize;

  //copy over to buf
  for(i = 0; i < len; i++)
  {
    buf[i] = PCB_six[c_process_num]->args[i];
  }

  buf[len] = '\0';  //add end identifier

  return SUCCESS;
}
/*
 * vidmap
 *   DESCRIPTION: system call to copy the current vid mem to an address in user space
 *                This uses a static virtual address defined above
 *   INPUTS:  screen_start pointer to array of user vid mem
 *   OUTPUTS: none
 *   RETURN VALUE: returns the address written to, or -1 if invalid write
 *   SIDE EFFECTS: writes to the user space vid mem
 */
int32_t vidmap(uint8_t** screen_start)
{
  int check;
  uint32_t Vaddr,Paddr;

  //check for invalid adresses
  if((uint32_t)screen_start < VIDMEM_START || (uint32_t)screen_start > VIDMEM_END)
  {
    return FAILURE;
  }

  //utilize static adresses
  Vaddr = (uint32_t)VIDMEM_CPY;
  Paddr = (uint32_t)VIDMEM_ADDR;

  //remap the memory from virtual loc to physical vidmems
  check = page_to_phys(Vaddr,Paddr);

  if(check == FAILURE)
  {
    return FAILURE;
  }

  //paging success! set new screen start and return
  *screen_start = (uint8_t*)VIDMEM_CPY;
  return VIDMEM_CPY;
}
/*
 * set_handler
 *   DESCRIPTION: changes the default action taken when a signal is recieved
 *   INPUTS:  signum - number of the signal to handle
 *            handler_address - address of the function to use
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0 if set, or  if set failed
 *   SIDE EFFECTS: changes default prog operation
 */
int32_t set_handler(int32_t signum, void* handler_address)
{
  if(signum < 0 || signum > SIG_CNT){
    return FAILURE;
  }

  if(handler_address == NULL){
    PCB_six[c_process_num]->sig_arr[signum] = sig_map[signum];
  }
  else{
    PCB_six[c_process_num]->sig_arr[signum] = handler_address;
  }

  return SUCCESS;
}
/*
 * sigreturn
 *   DESCRIPTION: returns from the signal interrupt
 *   INPUTS:  none
 *   OUTPUTS: none
 *   RETURN VALUE: returns
 *   SIDE EFFECTS: changes default prog operation
 */
int32_t sigreturn(void)
{
  return SUCCESS;
}
void kill()
{
  halt(255);
  return;
}
int32_t ignore()
{
  return SUCCESS;
}
