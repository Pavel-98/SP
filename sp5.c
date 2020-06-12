#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include<errno.h>
#include<string.h>
#include<sys/wait.h>
#include<stdlib.h>

void transmit3(char** buffer){


}



void transmit2(char*** buffer){
    
    
    int parts = 2048 / 40;
    if(parts > 60){
        parts = 60;
    }
    
    int i = 0;
    for(int i = 2048 - 1; i >= 0; i -= 1){
        buffer[i][1] = buffer[i][0];
    }
    for(int i = 0; i < 2048 ; i += 1){
        char header[40];
        for(int j = 0; j < 40; j++){
            header[i] = '0';
            }
         buffer[i][0] = header; 
         } 
      //print(buffer);    
}


void transmit1(char *path){
   int file = open(path, O_RDONLY);
   printf("Gyyyyy%s", strerror(errno));
   if(file == -1){
   printf("Gyyyyy%s", strerror(errno));
   }
   char** buffer[2048];
   char text[2048];
   read(file, text, 2048);
   for(int j = 0; j < 2048; j++){
       char* item[3];
       buffer[j] = item;
       buffer[j][0] = &text[j];
   }
   transmit2(buffer);
   printf("Haaaaa%s", strerror(errno));
}






int main(char* string){
    printf("Gyyyyy%s", strerror(errno));
    transmit1("sp5.c");
}
