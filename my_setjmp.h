/*
 * Copyright (C) 1991 DJ Delorie
 * All rights reserved.
 *
 * Redistribution, modification, and use in source and binary forms is permitted
 * provided that the above copyright notice and following paragraph are
 * duplicated in all such forms.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Modified to use SETJMP_DJ_H rather than SETJMP_H to avoid
   conflicting with setjmp.h.  Ian Taylor, Cygnus support, April,
   1993.  */

#ifndef MY_SETJMP_H
#define MY_SETJMP_H

//#define X86_32
//#define X86_64

#ifdef __cplusplus
extern "C" {
#endif

#ifdef X86_32

typedef struct {
  unsigned int eax; // 32bit size
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
  unsigned int esi;
  unsigned int edi;
  unsigned int ebp;
  unsigned int esp;
  unsigned int eip;
} my_x32_jmp_buf[1];

int my_x32_setjmp(my_x32_jmp_buf);
void my_x32_longjmp(my_x32_jmp_buf, int);
#endif

#ifdef X86_64
typedef struct {
  unsigned long long rbx; // 64bit size
  unsigned long long rbp;
  unsigned long long r12;
  unsigned long long r13;
  unsigned long long r14;
  unsigned long long r15;
  unsigned long long rsp;
  unsigned long long rip;
  unsigned long long res;
} my_x64_jmp_buf[1];

int my_x64_setjmp(my_x64_jmp_buf);
void my_x64_longjmp(my_x64_jmp_buf, int);

#endif




#ifdef __cplusplus
}
#endif

#endif
