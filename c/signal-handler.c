#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
 
void exitHandler (int signal, siginfo_t *info, void *sigcontext) {
  /* get registers contents */
  //ucontext_t *context = (ucontext_t*)sigcontext;
  //mcontext_t mcontext = context->uc_mcontext;
  //size_t cr2 = context->uc_mcontext.gregs[REG_CR2];
  //size_t rip = context->uc_mcontext.gregs[REG_RIP];

  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;

  //if (signal == SIGSEGV)
  //  printf("Got signal %d, faulty address is %p, "
  //         "from %p\n", signal, cr2, rip);
  //else
  printf("Got signal %d\n", signal);

  /* get the backtrace */
  trace_size = backtrace(trace, 16);
  /* overwrite sigaction with caller's address */
  //trace[1] = (void *)rip;
  messages = backtrace_symbols(trace, trace_size);
  /* skip first stack frame (points here) */
  printf("[bt] Execution path:\n");
  for (i=1; i<trace_size; ++i)
  {
    /* print backtrace "i"-deep */
    printf("[bt] #%d %s\n", i-1, messages[i]);

    char syscom[1024];
    char prgm[512];

    /* extract program name from backtrace */
    int j = 0;
    for (j = 0; j < 511; j++) {
      if (messages[i][j] == '(' || messages[i][j] == 0) break;
      prgm[j] = messages[i][j];
    }
    prgm[j] = 0;

    /* generate the correspondance with source file */
    sprintf(syscom,"addr2line %p -e %s", trace[i], prgm); //last parameter is the name of this app
    system(syscom);
  }

  exit(1);
}

void register_signal_handler() {
  struct sigaction sa; 
  sa.sa_sigaction = exitHandler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART|SA_SIGINFO;

  sigaction (SIGINT, &sa, NULL);
  sigaction (SIGTERM, &sa, NULL);
  sigaction (SIGILL, &sa, NULL);
  sigaction (SIGSEGV, &sa, NULL);
  sigaction (SIGABRT, &sa, NULL);
}


static int *invalid = (int*)0xdeadbeef;
int fault() {
  return *invalid;
}

int main() {
  register_signal_handler();
  return fault();
}
