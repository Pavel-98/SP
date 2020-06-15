#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include<string.h>
#include<errno.h>
#include<sys/wait.h>
#include<stdlib.h>

int main(char* string)
{
    int pid;
    printf("ІД процесу:  %d\n", getpid());
     printf("ПІД процесу:  %d\n", getppid());
     printf("ГІД процесу:  %d\n", getgid());
     printf("СІД процесу:  %d\n", getsid(0));
     
     switch((pid = fork())){
        case -1: printf("Виникли проблеми %s", strerror(errno));
        break;
     
        case 0:
          printf("ІД дитячого процесу:  %d та завершення\n", getpid());
          exit(0);
          break;
          default: wait(&pid);
          printf("Завершення батьківського");
     }
     
     
}
