#include "pthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
//#include <setjmp.h>
#include "my_setjmp.h"


//Thread th1;

#define BUF_SIZE 32768
char func1_stack[BUF_SIZE+64];
char func2_stack[BUF_SIZE+64];
char scheduler_stack[BUF_SIZE];

my_jmp_buf th1;
my_jmp_buf th2;
my_jmp_buf running_state;
my_jmp_buf scheduler_state;

my_jmp_buf *cur_th;
my_jmp_buf *next_th;


void func1()
{
  while(1)
  {
    printf("1");
    printf("2");
    printf("3");
    printf("4");
    printf("5");
    printf("6");
    printf("7");
    printf("8");
    printf("9");
    printf("a");
    printf("\n");
  }
}
void func2()
{
  while(1)
  {
    printf("21 ");
    printf("22 ");
    printf("23 ");
    printf("24 ");
    printf("25 ");
    printf("\n");
  }
}


void sigalrm_fn(int sig)
{
  sigset_t sigs;
  /* Unblock the SIGUSR1 signal that got us here */
#if 1
  sigemptyset (&sigs);
  sigaddset (&sigs, SIGUSR1);
  sigprocmask (SIG_UNBLOCK, &sigs, NULL);
#endif
  printf("got USR1!\n");
  #if 0
  if (my_setjmp(th1) == 0)
    my_longjmp(scheduler_state, 1);
    #endif
#if 1
    if (cur_th == &th1)
    {
      printf("2\n");
      next_th = &th2;
    }
    else
    {
      printf("1\n");
      next_th = &th1;
    }
#endif

  //if (my_setjmp(*cur_th) == 0)
  //if (my_setjmp(th2) == 0)
  if (my_setjmp(*cur_th) == 0)
  {
    cur_th = next_th;
    //printf("cur_th: %p\n", cur_th);
    //my_longjmp(scheduler_state, 1);
    my_longjmp(*next_th, 1);
  }
  else
  {
    return;
  }

  //context_switch (my_jmp_buf *new_state, my_jmp_buf *old_state); // go to new_state
  #if 0
  if (my_setjmp(th1))
    return;
  //alarm(2);
  #endif
  return;
}

void reschedule() 
{
  while(1)
  {
    return;
  }
}

void context_switch (my_jmp_buf *new_state, my_jmp_buf *old_state)
{
    if (my_setjmp(*old_state) == 0)
        my_longjmp(*new_state, 1);
}


void *scheduler (void *dummy) 
{
  if (my_setjmp(scheduler_state) == 0)
  {
    printf("first\n");
    pause();
  }
  else
  {
    printf("in sch select th1\n");
    cur_th = &th1;
    my_longjmp(th1, 1);
  }
#if 0
    //sched_thread->state = thread_sched;

    while (1) 
    {

        reschedule();

#if 0
        while (debug_target != NULL) {
            sched_debug_hook();
            reschedule();
        }
        current = best_thread();

        gettimeofday (&current->last_ran, NULL);
#endif
        context_switch (&scheduler_state, &th1);
#if 0
        if (*current->beforeguard != 0xdeadbeef
            || *current->afterguard != 0xdeadbeef)
        {
            assert ("Stack overflow" == 0);
        }
#endif
    }
#endif
  return 0;
}

int main(int argc, char *argv[])
{
  signal(SIGUSR1, sigalrm_fn);
  my_setjmp(th1);
  th1[0].eip = (unsigned long)func1;
  th1[0].esp = (unsigned long)(func1_stack + BUF_SIZE);

  if (my_setjmp(th2) == 0)
  {
    th2[0].eip = (unsigned long)func2;
    th2[0].esp = (unsigned long)(func2_stack + BUF_SIZE);
    cur_th = &th2;
    my_longjmp(th2, 1);
  }
  //func2();

#if 0
  my_setjmp(scheduler_state);
  scheduler_state[0].eip = (unsigned long)scheduler;
  scheduler_state[0].esp = (unsigned long)scheduler_stack;
  func1();
  //scheduler(0);
#endif


  while (1) 
    pause();

#if 0
  struct sigaction sa;

  sa.sa_handler = sigalrm_fn;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_ONSTACK;
  if (sigaction (SIGUSR1, &sa, NULL) != 0) 
  {
    perror ("sigaction");
    abort();
  }
#endif
#if 0
  running_state = scheduler_state;

  if (my_setjmp (running_state) == 0) 
  {
    kill (getpid(), SIGUSR1);
    /* Wait for signal to arrive.  If you find yourself spinning
     * here for any length of time, it's probably because SIGUSR1
     * is still blocked from the last thread creation.
     */
    while (1) 
      continue;
   }
#endif
#if 0
  //signal(SIGALRM, sigalrm_fn);
  signal(SIGVTALRM, sigalrm_fn);

  struct itimerval value, ovalue, value2;

  value2.it_value.tv_sec = 0;
  value2.it_value.tv_usec = 500000;
  value2.it_interval.tv_sec = 0;
  value2.it_interval.tv_usec = 500000;

  setitimer(ITIMER_VIRTUAL, &value2, &ovalue);
#endif
  //func1();
  //alarm(1);
#if 0
  my_longjmp(th1, 1);
#endif
  while(1);
    pause();
  return 0;
}
