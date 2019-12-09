
#include "pit.h"
#include "i8259.h"
#include "lib.h"
#include "syscalls.h"
#include "x86_desc.h"
#include "terminal.h"

//for setting up first 3 shells
int shell_flag = 0;

/*
 * pit_init
 *   DESCRIPTION: inializes channel 0 frequency to 41hz on square wave
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets reload value for 41hz, enables pit irq
 */
void pit_init()
{
   //mode 3? high and low will be the same
   outb(MODE3, COMMANDREG);
   //send reload value to channel 0 byte by byte
   outb(RELOAD_VAL & MASK, CHANNEL0);
   outb(RELOAD_VAL >> SHIFT8, CHANNEL0);
   //enable irq for pit
   curterm_nodisp = 0;
   enable_irq(PITIRQ);
}
/*
 * pit_handler
 *   DESCRIPTION: shcedules between 3 terminals at 41hz. Context switches
                  the 3 terminal's top proces's each interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes page directory, changes tss, changes esp, and ebp
 */
void pit_handler()
{
  PCB_t* curr_PCB;
  uint32_t base, stack;

  cli();

  //if 3 shells arent running set up the other 2 shells
  if((shell_flag == 0))
  {
    //starts at 0
    //set pcb to be base shell of the current serviced terminal
    curr_PCB = PCB_arr[terminals[curterm_nodisp].process_idx];

    //save esp and ebp for context switch for all 3 base shells
    asm volatile(
      "movl %%esp, %0\n"
      "movl %%ebp, %1\n"
      :"=r"(stack), "=r"(base)
    );
    curr_PCB->esp_scheduling = stack;
    curr_PCB->ebp_scheduling = base;

   //store tss stack pointer
    curr_PCB->esp0_scheduling = tss.esp0;

    //continue to next terminal
    curterm_nodisp = curterm_nodisp + 1;
    curterm_nodisp = curterm_nodisp % 3;

    //last 2 shells not launched yet; launch a shell
    if(curterm_nodisp == 2)
    {
      //we've launched 3 shells ( about to :) )
      shell_flag = 1;
    }
    sti();
    send_eoi(PITIRQ);
    execute((const uint8_t*) "shell");
  }
  else        //intterupt after first 3 shells are set up
  {
    //starts at 2
    //get current top process on the terminal we're servicing  and save its contents
    curr_PCB = PCB_arr[terminals[curterm_nodisp].process_idx];
    asm volatile(
      "movl %%esp, %0\n"
      "movl %%ebp, %1\n"
      :"=r"(stack), "=r"(base)
    );
    curr_PCB->esp_scheduling = stack;
    curr_PCB->ebp_scheduling = base;
    curr_PCB->esp0_scheduling = tss.esp0;

    //increment the terminal # we want to service looping back to 0 after 2
    curterm_nodisp = curterm_nodisp + 1;
    curterm_nodisp = curterm_nodisp % 3;

    //get its top process
    curr_PCB = PCB_arr[terminals[curterm_nodisp].process_idx];

    //set up page directory to map to physical memory and enable S, U , R, and P bits
   page_directory[PAGE128] = ((EIGHTMB + (terminals[curterm_nodisp].process_idx * FOURMB)) | SURP);

   //flush tlb
   asm volatile(
     "movl %%cr3, %%eax\n"
     "movl %%eax, %%cr3\n"
     :
     :
     :"eax"
   );

    //context switch to next terminal's top process
    tss.esp0 = curr_PCB->esp0_scheduling;
    stack = curr_PCB->esp_scheduling;
    base = curr_PCB->ebp_scheduling;

    asm volatile(
        "movl %%ebx, %%esp\n"
        :
        :"b"(stack)
      );

    asm volatile(
        "mov %%ebx, %%ebp\n"
        :
        :"b"(base)
      );

    sti();
    send_eoi(PITIRQ);
  }

  asm("leave");
  asm("iret");
}
