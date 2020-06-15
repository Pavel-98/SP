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
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>


int main(){
            struct sockaddr_in in;
            struct sockaddr_in client;
            int d_server;
             int d_client;
             
                    int get;
                    int sendS;
                    char getBuff[1024];
                    char sendBuff[1024];
                    time_t timr;
                    struct tm now;
                    
                    
                    
            in.sin_family = PF_INET;  
            in.sin_addr.s_addr =  htonl(INADDR_ANY);
            in.sin_port = htons(3216); 
            d_server = socket(PF_INET, SOCK_STREAM, 0);;
            connect(d_server, (struct sockaddr*)&in, sizeof(in)) ;
            do{
            printf("ready for message ");
            fgets(sendBuff, get, stdin);
            if((sendS = strlen(sendBuff)) <= 0){
                continue;
            }
            
            sendBuff[sendS] = '\n';
            sendBuff[sendS + 1] = '\0';
            send(d_server, sendBuff, sendS, 0);
            get = recv(d_client, getBuff, sizeof(getBuff) - 1, 0);
            getBuff[get] = '\0';
            write(open("log", O_CREAT | O_APPEND, 0622), getBuff, sizeof(getBuff));;
            } while(strcmp(sendBuff, "close") != 0);;
}
