#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include<errno.h>
#include<string.h>
#include<sys/wait.h>
#include<stdlib.h>
int main(char* string)
{
    int d = open("log", O_WRONLY | O_CREAT, 0644);
    if(d < 0){
printf("Виникли проблеми %s", strerror(errno));
}
    int pid;
    write(d, "Start", 5); 
    switch((pid = fork())){
        case -1: printf("Виникли проблеми %s", strerror(errno)); break;
        case  0: setsid(); chdir("/"); close(d);
                  FILE* f = fopen("log", "a");
                  if(d < 0){
                  printf("Виникли проблеми %s", strerror(errno)); break;
                  }
                  int p = getpid();
                  int ppid = getppid();
                  int gid = getgid();
                  
                  
                  fprintf(f, "pid == %i", p);
                  
                  fprintf(f, "ppiid == %i", ppid);
                  
                  fprintf(f, "gid == %i", gid);
                  
                  
                  
                  
                  fd_set rfdsout; FD_ZERO(&rfdsout);
                  FD_SET(1, &rfdsout); FD_SET(0, &rfdsout); FD_SET(2, &rfdsout); 
                  open("/dev/null", O_WRONLY);
                  while(1){
            
                  }
                   break;
//select(3, NULL, &rfdsout, NULL, NULL);
        default: write(d, "fork", 4);  exit(0); 
                  break;
    } 
}
