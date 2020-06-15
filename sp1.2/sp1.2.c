#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include<errno.h>
#include<string.h>
int main(char* string)
{
    char message[] = "message";
    fd_set rfdsin;
    fd_set rfdsout;
    fd_set rfdserr;
     int retval;
    FD_ZERO(&rfdsin);
    FD_SET(0, &rfdsin);
    FD_ZERO(&rfdsout);
    FD_SET(1, &rfdsout);
    FD_ZERO(&rfdserr);
    FD_SET(1, &rfdserr);
char buff[1024];

     struct timeval tv;
     tv.tv_sec = 5;
     
     
    retval = select(1, &rfdsin, NULL,NULL, &tv);
    if(retval == -1){
        perror("select()");
        retval = select(2, NULL, &rfdserr,NULL, &tv);
        if(retval == -1){
        printf("%s", strerror(errno));
            perror("select()");
        }
        else{
            if(write(STDERR_FILENO, message, 10) != -1){
                printf("Повідомлення відправлено");
            }
            else{
            printf("%s", strerror(errno));
                printf("error");
            }
        }
    }
    else{
        retval = select(2, NULL, &rfdserr,NULL, &tv);
        if(retval == -1){
        printf("%s", strerror(errno));
            perror("select()");
        }
        else{
            if(write(STDERR_FILENO, message, 10) != -1){
                printf("Повідомлення відправлено");
            }
            else{
            printf("%s", strerror(errno));
                printf("error");
            }
        }
        
        
        
        if(read(STDIN_FILENO, buff, 1024) != -1){
            printf("gy");  
        }
        else{
            printf("error");
        }
    }
    
    
    
    retval = select(2, NULL, &rfdsout, NULL, &tv);
    if(retval == -1){
        perror("select()");
        printf("%s", strerror(errno));
    }
    else{
        if(write(STDOUT_FILENO, buff, 1024) != -1){
            printf("gy");
            //break;
        }
        else{
        printf("%s", strerror(errno));
            printf("error");
        }
    }
     
}
