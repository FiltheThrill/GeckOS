/* yikes
Author - who even cares anymore*/

#include "fistdemcalls.h"
#include "lib.h"
#include "files.h"
#include "keyboard.h"

int process_amount = 0;
PCB_t* PCB_start()
{
  PCB_T* result;
  asm volatile//no idea if this works
  (
    "andl %%esp, %%ecx"
    :"=r"(result)
    :"r"(FIRSTPCB)
    :"%ecx"
  );
  return result;
}
int32_t halt(uint8_t status)
{
  //what...
}


int32_t execute(const uint8_t* command)
{
  int32_t process_num;
  uint8_t first_word[128];    //128?
  uint8_t rest_of_word[128];
  int32_t len;
  int len_word1, i, check;
  dentry_t word1;
  uint8_t header[40];
  uint8_t error_buf[200];

  cli();

  if(command == 0)  //check if command is NULL
  {
    *error_buf = "Invalid cmd";
    term_write(1, error_buf, 200);
    return -1;
  }

   process_num = process_amount + 1;  //set up process num
   process_amount++;

   // Parse command to get words
  len = strlen((int8_t*) command);

  for(i = 0; i < len; i++)  //find where first word ends
  {
    if(command[i] == ' ')
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
  first_word[len_word1] = '\0';

  if(len_word1 < len)
  {
    for(i = len_word1 + 1; i < len; i++)
    {
      rest_of_word[i - len_word1 - 1] = command[i];
    }
    rest_of_word[i - len_word1] = '\0';
  }

  check = read_dentry_by_name(first_word, &word1);
  if(check == -1)
  {
    *error_buf = "Couldn't find file";
    term_write(1, error_buf, 200);
    return -1;
  }

  check = read_data(word1.inode_num, 0, header, 40);
  if(check == -1)
  {
    *error_buf = "Read_data failed";
    term_write(1, error_buf, 200);
    return -1;
  }

  if((header[0] != 0x7F) || (header[1] != 0x45) ||
     (header[2] != 0x4C) || (header[3] != 0x46))
     {
       *error_buf = "Not an executable";
       term_write(1, error_buf, 200);
       return -1;
     }


}
