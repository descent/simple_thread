/*
 * test signal behavior
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define _GNU_SOURCE
#include <signal.h>

// modify from tlpi-book lib/signal_functions.c
/* Print list of signals within a signal set */
void print_sigset(const sigset_t *sigset)
{
  int sig, cnt;

  cnt = 0;
  for (sig = 1; sig < NSIG; sig++) 
  {
    if (sigismember(sigset, sig)) 
    {
      cnt++;
      printf("sigpending sig: %d\n", sig);
    }
  }

  if (cnt == 0)
    printf("<empty signal set>\n");
}

void sigalrm_fn(int sig)
{
  static int cnt;
  sigset_t set;
  printf("got sig: %d, cnt: %d\n", sig, cnt);
  for (int i=0 ; i < 10000 ; ++i)
    for (int i=0 ; i < 1000 ; ++i)
      for (int i=0 ; i < 500 ; ++i)
        ;
  sigpending(&set);
  print_sigset(&set);
  printf("  end sig: %d, cnt: %d\n", sig, cnt);
  ++cnt;
}

int main(int argc, char *argv[])
{
  printf("SIGUSR1: %d\n", SIGUSR1);
  printf("SIGRTMIN: %d\n", SIGRTMIN);
  printf("SIGRTMAX: %d\n", SIGRTMAX);

#if 0
  struct sigaction s1, old_s1;
  struct sigaction s2, old_s2;

  sigemptyset(&s1.sa_mask);
  sigemptyset(&s2.sa_mask);

  s1.sa_flags = 0;
  //s1.sa_flags = SA_NODEFER;
  s2.sa_flags = 0;

  s1.sa_handler = sigalrm_fn;
  s2.sa_handler = sigalrm_fn;

  sigaction(SIGUSR1, &s1, &old_s1);
  //sigaction(SIGRTMIN, &s2, &old_s2);

#else
  __sysv_signal(SIGUSR1, sigalrm_fn);
  //signal(SIGUSR1, sigalrm_fn);
  //signal(SIGRTMIN, sigalrm_fn);
#endif

  while (1) 
    pause();

  return 0;
}

