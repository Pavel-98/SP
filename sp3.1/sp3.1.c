#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include<errno.h>
#include<string.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<signal.h>
static FILE* d;
static void sig_usr(int signo, siginfo_t *si, void * u_context){
    fprintf(d, "signal number = %i, signal exit code = %i, sending to process with id = %i, associated with errno = %i", signo, si->si_status, si->si_pid, si->si_errno);
    fflush(d);
} 

int main(char* string)
{
    d = fopen("log", "a");
    if(d < 0){
       printf("Виникли проблеми %s", strerror(errno));
    }
    fprintf(d, "program is started\n");
    fprintf(d, "pid = %i\n", getpid());
    fflush(d);
    struct sigaction newHandler;
    newHandler.sa_handler = NULL;
    newHandler.sa_sigaction = sig_usr;
    newHandler.sa_flags = SA_RESETHAND & SA_SIGINFO;
    struct sigaction oldHandler;
    sigaction(SIGHUP, &newHandler, &oldHandler);
    
    
    
    
    
    while(1){
    pause();
    }
    
}

   
