#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <vector>
#include "my_setjmp.h"

using namespace std;

#define BUF_SIZE 32768
char func1_stack[BUF_SIZE+64];
char func2_stack[BUF_SIZE+64];
char scheduler_stack[BUF_SIZE];

#ifdef X86_32

#define my_setjmp my_x32_setjmp
#define my_longjmp my_x32_longjmp
#endif

#ifdef X86_64
#define my_setjmp my_x64_setjmp
#define my_longjmp my_x64_longjmp
#endif



union ABC
{
  int i_;
};

namespace DS
{
union ABC
{
  int i_;
};

  union pthread_attr_t
  {
    int i_;
  };

  struct pthread_t
  {
    #ifdef X86_32
    my_x32_jmp_buf jmp_buf_;
    #endif
    #ifdef X86_64
    my_x64_jmp_buf jmp_buf_;
    #endif
  };

  pthread_t pthread_self(void)
  {
    //return (pthread_t)0;
  }

  vector<pthread_t*> thread_vec;
  auto cur_thread = thread_vec.begin();
  int current_index = 0;
  int pthread_exit_ret = 1;

  auto get_next_thread()
  {
    unsigned int cnt=0;
    while(1)
    {
      current_index = (current_index + 1) % thread_vec.size();
      if (thread_vec[current_index] !=0)
        break;
      else
        ++cnt;
      if (cnt == thread_vec.size())
      {
        printf("all thread end\n");
        pause();
      }
    }
    printf("current_index: %d\n", current_index);
    return current_index;
  }

#if 0
 static __inline__ void atomic_add(int i, atomic_t *v)
 {
__asm__ __volatile__(
LOCK "addl %1,%0"
:"=m" (v->counter)
:"ir" (i), "m" (v->counter));
 }
#endif
  void push_ret_val()
  {
#ifdef X86_32
      asm volatile
      (
        "mov %eax, 8(%esp)"
      );
#endif
      //printf("xx\n"); // 假如有呼叫這行, 上面的組合語言要改成 "mov %eax, 12(%ebp)"
  }
  void pthread_exit(void *retval)
  {
    //while(1)
    {
#if 0
      asm volatile
      (
        "mov %%eax, %0"
        : "=m" (retval)
        :
      );
#endif


      thread_vec[current_index] = 0;
      //printf("thread exit: retval: %d\n", *((int*)retval));
      //printf("thread exit: %p, current_index: %d\n", retval, current_index);
      printf("thread exit: %p, retval: %d, current_index: %d\n", retval, *((int*)retval), current_index);
      //pause();

#if 1
// 不使用這個方式是因為, DS::get_next_thread 在 signal handler 也會被呼叫, 會有 reentry function 的問題。但目前想不到別的方法。
// 也許可以直接跳到一個 idle function
      // select next thread to longjmp
      int next = DS::get_next_thread();
      my_longjmp(DS::thread_vec[next]->jmp_buf_, 1);
#endif
    }
  }

  int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize)
  {
    return 0;
  }

  int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, size_t *stacksize)
  {
    return 0;
  }

  pthread_attr_t g_attr;
  void *thread_malloc_stack(int size)
  {
    return malloc(size);
  }


  int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
  {
    thread->jmp_buf_[0].eip = (intptr_t)start_routine;
    //thread->jmp_buf_[0].esp = (unsigned long)(func1_stack + BUF_SIZE);

    auto stack_addr = thread_malloc_stack(BUF_SIZE);
    if (stack_addr == 0)
      return -1;

    printf("xx sizeof(intptr_t): %u\n", sizeof(intptr_t));

    thread->jmp_buf_[0].esp = ((intptr_t)stack_addr + BUF_SIZE - sizeof(intptr_t));

    printf("stack_addr + BUF_SIZE: %p\n", (char *)stack_addr+BUF_SIZE);
    printf("thread->jmp_buf_[0].esp: %#x\n", thread->jmp_buf_[0].esp);
    printf("&pthread_exit_ret: %p\n", &pthread_exit_ret);


#if 1
    *(intptr_t*)thread->jmp_buf_[0].esp = 0; // push ebp
    thread->jmp_buf_[0].esp -= sizeof(intptr_t);
#endif

    *(intptr_t*)thread->jmp_buf_[0].esp = (intptr_t)pthread_exit;
    thread->jmp_buf_[0].esp -= sizeof(intptr_t);

#if 1
    *(intptr_t*)thread->jmp_buf_[0].esp = (intptr_t)&push_ret_val;
    //*(intptr_t*)thread->jmp_buf_[0].esp = (intptr_t)&pthread_exit_ret; // pthread_exit argument
    thread->jmp_buf_[0].esp -= sizeof(intptr_t);
#endif

    thread_vec.push_back(thread);
    cur_thread = thread_vec.end() - 1;
    current_index = thread_vec.size() - 1;
    return 0;
  }


}

//Thread th1;


int func1_ret = 11;

int test_ret=888;

void *func1(void *arg)
{
  //while(1)
  {
    printf("1");
    printf("2");
    printf("3");
    printf("4");
    printf("5");
    #if 0
  for (int i=0 ; i < 1000 ; ++i)
    for (int i=0 ; i < 1000 ; ++i)
      for (int i=0 ; i < 500 ; ++i)
        ;
        #endif
    printf("6");
    printf("7");
    printf("8");
    printf("9");
    printf("a");
    printf("\n");
  }
  //return &func1_ret;
  printf("  call DS::pthread_exit, test_ret: %d\n", test_ret);
  DS::pthread_exit(&test_ret);
}

int func2_ret = 22;

void *func2(void *arg)
{
  //while(1)
  {
    printf("21 ");
    printf("22 ");
    printf("23 ");
    printf("24 ");
    printf("25 ");
    printf("\n");
  }
  printf("  directly return func2_ret: %d\n", func2_ret);
  return &func2_ret;
}

int func3_ret = 33;

void *func3(void *arg)
{
  {
    printf("331 ");
    printf("332 ");
    printf("333 ");
    printf("334 ");
    printf("335 ");
    printf("\n");
  }
  printf("  directly return func3_ret: %d\n", func3_ret);
  return &func3_ret;
}

void sigalrm_fn(int sig)
{
  static int once;

  sigset_t sigs;
  /* Unblock the SIGUSR1 signal that got us here */
#if 1
  sigemptyset (&sigs);
  sigaddset (&sigs, SIGUSR1);
  sigprocmask (SIG_UNBLOCK, &sigs, NULL);
#endif
  printf("got USR1!\n");
  if (once == 0)
  {
    once = 1;
    my_longjmp(DS::thread_vec[DS::current_index]->jmp_buf_, 1);
  }

  int cur = DS::current_index;
  #if 0
  if (DS::thread_vec[cur] == 0) // DS::thread_vec[cur] is ended
  {
    cur = DS::get_next_thread();
  }
  #endif
  int next = DS::get_next_thread();

  printf("cur: %d, next: %d\n", cur, next);

  //if (my_setjmp(*cur_th) == 0)
  //if (my_setjmp(th2) == 0)
  if (my_setjmp(DS::thread_vec[cur]->jmp_buf_) == 0)
  {
    my_longjmp(DS::thread_vec[next]->jmp_buf_, 1);
  }
  else
  {
    return;
  }

  return;
}

using DS::pthread_t;
//using DS::pthread_attr_t;
DS::ABC abc;

DS::pthread_t t1;
DS::pthread_attr_t th_attr1;

DS::pthread_t t2;
DS::pthread_attr_t th_attr2;

DS::pthread_t t3;
DS::pthread_attr_t th_attr3;

int main(int argc, char *argv[])
{
  setbuf(stdout, 0);
  signal(SIGUSR1, sigalrm_fn);
  int ret = pthread_create(&t1, &th_attr1, func1, 0);
  ret = pthread_create(&t2, &th_attr2, func2, 0);
  ret = pthread_create(&t3, &th_attr3, func3, 0);


#if 0
  my_setjmp(th1);
#ifdef X86_32
  th1[0].eip = (unsigned long)func1;
  th1[0].esp = (unsigned long)(func1_stack + BUF_SIZE);
#endif
#ifdef X86_64
  th1[0].rip = (unsigned long)func1;
  th1[0].rsp = (unsigned long)(func1_stack + BUF_SIZE);
#endif

  if (my_setjmp(th2) == 0)
  {
#ifdef X86_32
    th2[0].eip = (unsigned long)func2;
    th2[0].esp = (unsigned long)(func2_stack + BUF_SIZE);
#endif
#ifdef X86_64
    th2[0].rip = (unsigned long)func2;
    th2[0].rsp = (unsigned long)(func2_stack + BUF_SIZE);
#endif
    cur_th = &th2;
    my_longjmp(th2, 1);
  }
#endif

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
  return 0;
}
