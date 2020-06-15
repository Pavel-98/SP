#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include<errno.h>
#include<string.h>
int main()
{
    int f1;
    int f2;
    char path1[] = "file";
    char path2[] = "file2";
    char buffer[512];
    int count;
    if((f1 = open(path1, O_RDONLY)) == -1)
        printf("%s", strerror(errno));
    else{
        
    }    
    if((f2 = open(path2, O_WRONLY | O_CREAT, 0644)) == -1)
        printf("%s", strerror(errno));
    else{
        
    }   
    int y = 0;
    while(y < 5){
        read(f1, buffer, 512);
        for(int i = 0; i < 512; i++){
            buffer[i] -= 32; 
        }
        count += write(f2, buffer, 512);
        y++;
    }
}
