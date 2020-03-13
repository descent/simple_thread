#include "my_setjmp.h"

#include <stdio.h>

#define BUF_SIZE 32768
#ifdef X86_32
my_x32_jmp_buf th1;
char func1_stack[BUF_SIZE+64];
#define my_setjmp my_x32_setjmp
#define my_longjmp my_x32_longjmp
#endif

void func1(int a)
{
  printf("%d", a);
  printf("1");
  printf("2");
  printf("3");
  printf("\n");

  my_longjmp(th1, 1);
}

int main(int argc, char *argv[])
{
  if (my_setjmp(th1) == 0)
  {
    func1(23);
  }
  else
  {
    printf("longjmp\n");
  }
  
  return 0;
}
