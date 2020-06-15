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
    int d = open("log", O_CREAT | O_APPEND, 0622);
    if(d < 0){
        printf("problems %s", strerror(errno));
    }
    write(d, "start\n", 6);
    switch(fork()){
        case -1: printf("problems %s", strerror(errno)); break;
        case 0: setsid();
            struct sockaddr_in in;
            struct sockaddr_in client;
            int d_server;
             int d_client;
            in.sin_family = PF_INET;  
            in.sin_addr.s_addr =  htonl(INADDR_ANY);
            in.sin_port = htons(3216); 
            d_server = socket(PF_INET, SOCK_STREAM, 0);; 
            if(bind(d_server, (struct sockaddr*)&in, sizeof(in)) < 0){
        printf("problems %s", strerror(errno));
    }
            
            write(d, ("begin listen\n"), sizeof("begin listen"));
            listen(d_server, 5);
            while(1){
                int size = sizeof(client);
                if((d_client = accept(d_server, (struct sockaddr*)&client, &size)) < 0){
                    write(d, "problems with accept", sizeof("problems with accept"));
                }
                switch(fork()){
                case -1:
                printf("problems %s", strerror(errno));
                close(d_client);
                break;
                case 0: 
                    setsid();
                    int get;
                    int sendS;
                    char getBuff[1024];
                    char sendBuff[2048];
                    time_t timr;
                    struct tm now;
                    do {
                    get = recv(d_client, getBuff, sizeof(getBuff) - 1, 0);
                    if(get <= 0){
        printf("problems %s", strerror(errno)); continue;
    }
                    getBuff[get] = '\0';
                    time(&timr);
                    now = *(localtime(&timr));
                    sendS = sprintf(sendBuff, "process id == %i, message got %s, time %s", getpid(), getBuff, asctime(&now));
                    send(d_client, sendBuff, sendS, 0);
                   } while(strcmp(getBuff, "close") != 0);
                   write(d, "process close", 13);
                   close(d_client); break;
                default: close(d_client);
                    write(d, "work is client over\n", 19); close(d); break;
                }
            default: close(d_client);
                    write(d, "client is created, parent will exit\n", 19); close(d); break;         
            }   
    }
}
