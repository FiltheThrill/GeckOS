
#include "pit.h"
#include "i8259.h"
#include "lib.h"
#include "syscalls.h"
#include "x86_desc.h"
#include "terminal.h"

int first_shell = 0;
int shell_flag = 0;

void pit_init()
{
   outb(MODE3, COMMANDREG);
   outb(RELOAD_VAL & MASK, CHANNEL0);
   outb(RELOAD_VAL >> SHIFT8, CHANNEL0);
   enable_irq(PITIRQ);
}

void pit_handler()
{
  PCB_t* curr_PCB;
  uint32_t base, stack;
  cli();

  if(first_shell == 0)
  {
    first_shell = 1;
    // sti();
    send_eoi(PITIRQ);
    curterm_nodisp = 0;
    execute((const uint8_t*) "shell");
  }
  if((terminals[curterm_nodisp].process_idx < 3) && (shell_flag == 0))
  {
    if(curterm_nodisp == 2)
    {
      shell_flag = 1;
    }

    curr_PCB = PCB_arr[terminals[curterm_nodisp].process_idx];

    asm volatile(
      "movl %%esp, %0\n"
      "movl %%ebp, %1\n"
      :"=r"(stack), "=r"(base)
    );

    curr_PCB->esp_scheduling = stack;
    curr_PCB->ebp_scheduling = base;
    curr_PCB->esp0_scheduling = tss.esp0;

    curterm_nodisp++;

    if(curterm_nodisp < 3)
    {
      send_eoi(PITIRQ);
      sti();
      execute((const uint8_t*) "shell");
    }
    else
    {
      curterm_nodisp = 0;
      curr_PCB = PCB_arr[0];
      stack = curr_PCB->esp_scheduling;
      base = curr_PCB->ebp_scheduling;
      tss.esp0 = curr_PCB->esp0_scheduling;

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

      send_eoi(PITIRQ);
      sti();
    }
  }
  else
  {
    curr_PCB = PCB_arr[terminals[curterm_nodisp].process_idx];
    asm volatile(
      "movl %%esp, %0\n"
      "movl %%ebp, %1\n"
      :"=r"(stack), "=r"(base)
    );

    curr_PCB->esp_scheduling = stack;
    curr_PCB->ebp_scheduling = base;
    curr_PCB->esp0_scheduling = tss.esp0;
    curterm_nodisp = curterm_nodisp + 1;
    curterm_nodisp = curterm_nodisp % 3;
    curr_PCB = PCB_arr[terminals[curterm_nodisp].process_idx];
    tss.esp0 = curr_PCB->esp0_scheduling;
    stack = curr_PCB->esp_scheduling;
    base = curr_PCB->ebp_scheduling;
    //send_eoi(PITIRQ);
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
 asm volatile(
   "leave\n"
   "iret\n"
 );

}
