#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <time.h>

#include <unistd.h>
/* fork(), getpid(), setsid(), open(), close(), write() */
#include <fcntl.h>
/* open() */
#include <sys/types.h>
/* pid_t, size_t */

#include <netinet/in.h>
/* struct sockaddr_in, struct in_addr */
#include <sys/socket.h>
/* accept(), bind(), listen(), recv(), send() */
#include <arpa/inet.h>
/* htons, htonl */

void parentProcess(int fd);
void childProcess(int fd);
void processClient(int client_sockfd, int fd);


int main(void) {
    int fd;
    pid_t fork_id;

    fd = open("test.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
      printf("Error while opening test.log for writing (initial)\n");
      return -1;
    }
    write(fd, "PROGRAM STARTED...\n", strlen("PROGRAM STARTED...\n"));

    // daemoning
    fork_id = fork();
    if (fork_id < 0) {
        write(fd, "Error occured while trying to fork()\n",
        strlen("Error occured while trying to fork()\n"));
        close(fd);
    }
    else if (fork_id == 0) {
        childProcess(fd);
    }
    else {
        // fork_id > 0 - it's child's ID
        parentProcess(fd);
    }

    // End of program
    close(fd);
    printf("\n\nMAIN SERVER: I'm done\n");
    return 0;
}

void parentProcess(int fd) {
    write(fd, "The child has been created, exiting now...\n",
    strlen("The child has been created, exiting now...\n"));
    close(fd);
    exit(EXIT_SUCCESS);
}

void childProcess(int fd) {
    int newSid;
    char temp[100];
    struct sockaddr_in sa_in;
    int sockfd;
    int client_sockfd;
    struct sockaddr_in client_sa_in;
    int client_sa_in_size;
    int fork_id;

    // setsid()
    newSid = setsid();
    if (newSid == -1) {
        printf("Error while calling setsid()\n");
    }

    sa_in.sin_family = PF_INET;
    sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
    sa_in.sin_port = htons(3216);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        write(fd, "MAIN SERVER: ERROR when opening socket...\n",
        strlen("MAIN SERVER: ERROR when opening socket...\n"));
        close(fd);
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd, (struct sockaddr *)&sa_in, sizeof(sa_in)) == -1) {
        write(fd, "MAIN SERVER: ERROR when binding...\n",
        strlen("MAIN SERVER: ERROR when binding...\n"));
        close(fd);
        exit(EXIT_FAILURE);
    }

    sprintf(temp, "[%d] MAIN SERVER: Starting listening...\n", getpid());
    write(fd, temp, strlen(temp));

    listen(sockfd, 5);
    client_sa_in_size = sizeof(client_sa_in);

    while(1) {
        client_sockfd = accept(sockfd, (struct sockaddr *)&client_sa_in, &client_sa_in_size);
        if (client_sockfd < 0) {
            write(fd, "MAIN SERVER: ERROR on acception connection from the client...\n",
            strlen("MAIN SERVER: ERROR on acception connection from the client...\n"));
        }
        else {
            // new process for this client
            fork_id = fork();
            if (fork_id < 0) {
                write(fd, "MAIN SERVER: Error occured while trying to fork() for client\n",
                strlen("MAIN SERVER: Error occured while trying to fork() for client\n"));
                close(client_sockfd);
            }
            else if (fork_id == 0) {
                close(sockfd);
                processClient(client_sockfd, fd);
                close(fd);
                exit(EXIT_SUCCESS);
            }
            else {
                write(fd, "MAIN SERVER: fork()'ed for client successfully\n",
                strlen("MAIN SERVER: fork()'ed for client successfully\n"));
                close(client_sockfd);
            } 
        }
    }

    close(fd);
}

void processClient(int client_sockfd, int fd) {
    int newSid;
    char recv_buff[1024];
    int recv_bytes;
    char send_buff[1100];
    int send_bytes;
    /* tmp variables for current time */
    time_t rawtime;
    struct tm cur_time;

    // setsid()
    newSid = setsid();
    if (newSid == -1) {
        printf("Error while calling setsid()\n");
    }

    do {
        recv_bytes = recv(client_sockfd, recv_buff, sizeof(recv_buff) - 1, 0);
        recv_buff[recv_bytes] = '\0';
        if (recv_bytes <= 0) {
            write(fd, "PROCESSING SERVER: empty request received, skipping it\n",
            strlen("PROCESSING SERVER: empty request received, skipping it\n"));
            continue;
        }

        time(&rawtime);
        cur_time = (* localtime(&rawtime));
        send_bytes = sprintf(send_buff, "PID: [%d] on %sYour message:%s", getpid(), asctime(&cur_time), recv_buff);
        if (send(client_sockfd, send_buff, send_bytes, 0) == -1) {
            write(fd, "PROCESSING SERVER: failed to perform send() to client\n",
            strlen("PROCESSING SERVER: failed to perform send() to client\n"));
        }
    } while (strcmp(recv_buff, "close") != 0);

    write(fd, "PROCESSING SERVER: <close> command received, finishing communication with him\n",
    strlen("PROCESSING SERVER: <close> command received, finishing communication with him\n"));
    close(client_sockfd);
}
