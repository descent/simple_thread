#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <vector>
#include <utility>
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

  typedef unsigned long long pthread_t;

  struct ThreadData
  {
    #ifdef X86_32
    my_x32_jmp_buf jmp_buf_;
    #endif
    #ifdef X86_64
    my_x64_jmp_buf jmp_buf_;
    #endif
  };
  typedef std::pair<pthread_t, ThreadData> ThreadPair;

  struct JoinData
  {
    pthread_t tid_;
    void *ret_;
  };

  //vector<pthread_t*> thread_vec;
  vector<std::pair<pthread_t, ThreadData> > thread_vec;
  auto cur_thread = thread_vec.begin();
  int current_index = 0;
  int pthread_exit_ret = 1;

  pthread_t pthread_self(void)
  {
    return thread_vec[current_index].first;
    //return (pthread_t)0;
  }

  int pthread_join(pthread_t thread, void **retval)
  {
    sigset_t sigs;
    siginfo_t si;
    int sig;

    while(1)
    {
      sigemptyset(&sigs);
      sigaddset(&sigs, SIGRTMIN);
      sigprocmask(SIG_SETMASK, &sigs, 0);
      sig = sigwaitinfo(&sigs, &si);
      if (sig == -1)
      {
        perror("sigwaitinfo fail\n");
        continue;
      }
      //printf("si.si_value.sival_int: %d, thread: %llu\n", si.si_value.sival_int, thread);
      JoinData *jd = (JoinData*)si.si_ptr;
      printf("jd->tid_: %llu, thread: %llu\n", jd->tid_, thread);
      //if (sig == SIGRTMIN && si.si_value.sival_int == thread)
      if (sig == SIGRTMIN && jd->tid_ == thread)
      {
        printf("xx break\n");
        *retval = jd->ret_;
        delete jd;
        break;
      }
      else
      {
        printf("sig: %d\n", sig);
      }
    }
  }

  pthread_t gen_tid()
  {
    static pthread_t tid = 1; // 1 for main thread
    return ++tid;
  }

  auto get_next_thread()
  {
    unsigned int cnt=0;
    while(1)
    {
      current_index = (current_index + 1) % thread_vec.size();
      if (thread_vec[current_index].first != 0)
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


      //printf("thread exit: retval: %d\n", *((int*)retval));
      //printf("thread exit: %p, current_index: %d\n", retval, current_index);
      //pause();

#if 1
// 不使用這個方式是因為, DS::get_next_thread 在 signal handler 也會被呼叫, 會有 reentry function 的問題。但目前想不到別的方法。
// 也許可以直接跳到一個 idle function
      //sigqueue(getpid(), SIGRTMIN, DS::thread_vec[current_index].first);
      sigval value;
      //value.sival_int = pthread_self();
      JoinData *jdp = new JoinData{};
      jdp->tid_ = pthread_self();
      //join_data.tid_ = 2;
      jdp->ret_ = retval;
      value.sival_ptr = jdp;
      //printf("in pthread_exit value.sival_int: %d\n", value.sival_int);
      printf("in pthread_exit jdp->tid_: %llu\n", jdp->tid_);
      thread_vec[current_index].first = 0;
      printf("sigqueue, thread exit: %p, retval: %d, current_index: %d\n", retval, *((int*)retval), current_index);
      sigqueue(getpid(), SIGRTMIN, value);
      // select next thread to longjmp
      int next = DS::get_next_thread();
      my_longjmp(DS::thread_vec[next].second.jmp_buf_, 1);
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

  void wrap_routine(void *(*start_routine) (void *), void *arg)
  {
    void *ptr;
    if (start_routine)
    {
      printf("arg: %p\n", arg);
      ptr = start_routine(arg);
    }
    pthread_exit(ptr);
  }

  int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
  {
    static int init_main_thread = 0;
    static DS::ThreadPair main_thread_pair;

#if 1
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGRTMIN);
    sigprocmask(SIG_SETMASK, &sigs, 0);
#endif

    if (0 == init_main_thread)
    {
      main_thread_pair.first = 1; // fixed to 1
      thread_vec.push_back(main_thread_pair);
      init_main_thread = 1;
    }

    ThreadPair thread_pair;
    *thread = gen_tid();
    thread_pair.first = *thread;

    //thread->jmp_buf_[0].eip = (intptr_t)start_routine;
    //thread->jmp_buf_[0].eip = (intptr_t)wrap_routine;
    thread_pair.second.jmp_buf_[0].eip = (intptr_t)wrap_routine;
    //thread->jmp_buf_[0].esp = (unsigned long)(func1_stack + BUF_SIZE);

    auto stack_addr = thread_malloc_stack(BUF_SIZE);
    if (stack_addr == 0)
      return -1;

    printf("xx sizeof(intptr_t): %u\n", sizeof(intptr_t));

    thread_pair.second.jmp_buf_[0].esp = ((intptr_t)stack_addr + BUF_SIZE - sizeof(intptr_t)); // current stack - 4 or 8

    printf("stack_addr + BUF_SIZE: %p\n", (char *)stack_addr+BUF_SIZE);
    //printf("thread->jmp_buf_[0].esp: %#x\n", thread->jmp_buf_[0].esp);
    //printf("&pthread_exit_ret: %p\n", &pthread_exit_ret);

    *(intptr_t*)thread_pair.second.jmp_buf_[0].esp = (intptr_t)arg;
    thread_pair.second.jmp_buf_[0].esp -= sizeof(intptr_t);

    *(intptr_t*)thread_pair.second.jmp_buf_[0].esp = (intptr_t)start_routine;
    thread_pair.second.jmp_buf_[0].esp -= sizeof(intptr_t);

    *(intptr_t*)thread_pair.second.jmp_buf_[0].esp = (intptr_t)0; // simulate push return address
    thread_pair.second.jmp_buf_[0].esp -= sizeof(intptr_t);

#if 0
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
#endif

    thread_vec.push_back(thread_pair);
    cur_thread = thread_vec.end() - 1;
    current_index = thread_vec.size() - 1;

    auto &m_thread_pair = thread_vec[0];
    if (my_setjmp(m_thread_pair.second.jmp_buf_) == 0)
    {
      my_longjmp(DS::thread_vec[DS::current_index].second.jmp_buf_, 1);
    }
    else
    {
      printf("m th\n");
    }
    return 0;
  }

}

int func1_ret = 11;

int test_ret=888;

using DS::pthread_self;

void *func1(void *arg)
{
  int val = *(int *)arg;
  while(1)
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
    printf("arg: %d\n", val);
  }
  //return &func1_ret;
  printf("  call DS::pthread_exit, test_ret: %d, pthread_self(): %llu\n", test_ret, pthread_self() );
  DS::pthread_exit(&test_ret);
}

int func2_ret = 22;

void *func2(void *arg)
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
  printf("  directly return func2_ret: %d, pthread_self: %llu\n", func2_ret, pthread_self());
  return &func2_ret;
}

int func3_ret = 33;

void *func3(void *arg)
{
  while(1)
  {
    printf("331 ");
    printf("332 ");
    printf("333 ");
    printf("334 ");
    printf("335 ");
    printf("\n");
  }
  printf("  directly return func3_ret: %d, pthread_self(): %llu\n", func3_ret, pthread_self());
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
    //printf("DS::current_index: %d\n", DS::current_index);
    //my_longjmp(DS::thread_vec[DS::current_index].second.jmp_buf_, 1);
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
  if (my_setjmp(DS::thread_vec[cur].second.jmp_buf_) == 0)
  {
    my_longjmp(DS::thread_vec[next].second.jmp_buf_, 1);
  }
  else
  {
    return;
  }

  printf("signal handler end\n");
  return;
}

//using DS::pthread_t;
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
  int th1_arg = 111;
  printf("t1: %llu\n", t1);
  int ret = pthread_create(&t1, &th_attr1, func1, &th1_arg);
  printf("after pthread_create t1: %llu\n", t1);
  printf("t2: %llu\n", t2);
  ret = pthread_create(&t2, &th_attr2, func2, 0);
  printf("pthread_create t2: %llu\n", t2);
  printf("t3: %llu\n", t3);
  ret = pthread_create(&t3, &th_attr3, func3, 0);
  printf("pthread_create t3: %llu\n", t3);


  printf("main thread\n");
#if 0
  int *t1_ret;
  DS::pthread_join(t1, (void**)&t1_ret);
  printf("join t1 ret val: %d\n", *t1_ret);

  int *t2_ret;
  DS::pthread_join(t2, (void**)&t2_ret);
  printf("join t2 ret val: %d\n", *t2_ret);
#endif

#if 1
  for (int i=0 ; i < 1000 ; ++i)
    for (int i=0 ; i < 1000 ; ++i)
      for (int i=0 ; i < 5000 ; ++i)
        ;
#endif
  printf("main code\n");

#if 0
  while (1) 
    pause();
#endif

  return 0;
}
