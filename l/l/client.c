#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <unistd.h>
/* close() */

#include <netinet/in.h>
/* struct sockaddr_in, struct in_addr */
#include <sys/socket.h>
/* connect(), recv(), send() */
#include <arpa/inet.h>
/* htons, htonl */

int main(void) {
    struct sockaddr_in sa_in;
    int sockfd;
    char send_buff[1024];
    int send_bytes;
    char recv_buff[1024];
    int recv_bytes;


    sa_in.sin_family = PF_INET;
    sa_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sa_in.sin_port = htons(3216);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("CLIENT: ERROR when opening socket...\n");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&sa_in, sizeof(sa_in)) == -1) {
        printf("CLIENT: ERROR when calling connect()...\n");
        return -1;
    }

    do {
        printf("> ");
        fgets(send_buff, 1024, stdin);
        // replace newline symbol added by fgets() at the very end with \0 end-of-string character
        send_bytes = strlen(send_buff);
        if ((send_bytes > 0) && (send_buff[send_bytes - 1] == '\n')) {
            send_buff[--send_bytes] = '\0';
        }

        if (send(sockfd, send_buff, send_bytes, 0) == -1) {
            printf("CLIENT: failed to perform send() to server\n");
        }

        recv_bytes = recv(sockfd, recv_buff, sizeof(recv_buff) - 1, 0);
        recv_buff[recv_bytes] = '\0';
        printf("CLIENT: Response from server:\n%s\n", recv_buff);
    } while(strcmp(send_buff, "close") != 0);

    // End of program
    close(sockfd);
    printf("\n\nCLIENT: I'm done\n");
    return 0;
}
