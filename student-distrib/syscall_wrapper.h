// not sure how this file works...
//but copying format from x86_desc.h
#ifndef _SYS_CALL_WRAPPER_H
#define _SYS_CALL_WRAPPER_H
extern int32_t syscall_handler();
extern int32_t handler14();
extern int32_t handler13();
extern int32_t handler0();
extern void fake_iret();
#endif
