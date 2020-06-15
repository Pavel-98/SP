
#include<time.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include<errno.h>
#include<string.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/mman.h>

struct datum{
    char data[512];
    struct tm time;
    int pid;
    };
   
int main(char* string){
    int d;
    char data[512];
    struct datum * dat = NULL;
    if((d = shm_open("log", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0){
        printf(" problems1 %s", strerror(errno));
        return -1;
        }
    int res =  ftruncate(d, sizeof(struct datum));
    if(res != 0){
        printf(" problems2 %s", strerror(errno));
        return -1;
        }
     dat = mmap(NULL, sizeof(struct datum), PROT_READ | PROT_WRITE, MAP_SHARED, d, 0); 
     while(1){
         fgets(data, 512, stdin);
         msync(dat, sizeof(struct datum), MS_SYNC);
         printf("data %s, pid %i, time %s", dat->data, dat->pid, asctime(&(dat->time)));
         
         time_t traw;
         time(&traw);
         dat->time = *(localtime(&traw));
         dat->pid = getpid();
         strcpy(dat->data, data);
         }
    }
