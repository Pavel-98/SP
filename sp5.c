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
int size = 0;
void print(char* buffer){
    int d = open("temp", O_CREAT | O_RDONLY);
    write(d, buffer, 2048 + (int)2048 / 60 * 40 + 1);
    close(d);
}
void transmit3(char* buffer){
    char temp[2048 + (int)2048 / 60 * 40 + 1];
    for(int i = 0; i < 2048 + (int)2048 / 60 * 40 + 1; i++){
          temp[i] = buffer[i];
    }
    
}



void transmit2(char* buffer){
    
    
    int parts = 2048 / 40;
    if(parts > 60){
        parts = 60;
    }
    char temp[2048];
    int i = 0;
    for(int i = 0; i < 2048; i++){
        temp[i] = buffer[i];
    }
    for(int i = 0; i < 2048 ; i += 1){
        
         buffer[i] = 0; 
         } 
      
      int pos;
      for(int i = 0; i < 2048 + (int)2048 / 60 * 40 + 1; ){
        for(int j = 0; j < 40; j++){
         buffer[i + j] = 0; 
         }
         i += 40;
         for(int j = 0; j < 60; j++, pos++){
         buffer[i + j] = temp[pos];
         }
         i += 60;
         }  
         print(buffer);  
         transmit3(buffer); 
}


void transmit1(char *path){
   int file = open(path, O_RDONLY);
   printf("Gyyyyy%s", strerror(errno));
   if(file == -1){
   printf("%s", strerror(errno));
   }
   size = 2048 / 60 + 1;
   size = 60 * size;
   size += 2048;
   
   char buffer[size];
   char text[2048];
   read(file, text, 2048);
   for(int j = 0; j < 2048; j++){
       
       
       buffer[j] = text[j];
   }
   print(buffer);
   transmit2(text);
   printf("Haaaaa%s", strerror(errno));
}






int main(){
    printf("Gyyyyy%s", strerror(errno));
    transmit1("1");
    return 0;
}
