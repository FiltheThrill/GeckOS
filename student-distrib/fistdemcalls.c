/* yikes
Author - who even cares anymore*/

#include "fistdemcalls.h"


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
