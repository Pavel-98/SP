#include<time.h>
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
#include<sys/stat.h>

char fifo[] = "fifo";

struct buffer_item {
    int size;
    char* header;
    char* prefix;
    char data[60];
    char* sufix;
};

int controlSum(struct buffer_item* item) {
    return sizeof(item->prefix) + sizeof(item->data) + sizeof(item->header) + 1;
}

struct buffer {
    struct buffer_item items[(int)(2048 / 60)];
};

void level1(struct buffer_item* item, int size) {
    //printf("------%d\n", size);
    FILE* d = fopen(fifo, "a");
    if (d == NULL) {
        printf("fifo %s", strerror(errno));
        return;
    }
    printf("%s\n", "fifo opened");
    char info[size];
    sprintf(info, "info \n%s, \n%s, \n%s, \n%s\n", item->prefix, item->header, item->data, item->sufix);
    printf("problems %s", strerror(errno));
    fprintf(d, "%s", info);
    printf("problems %s", strerror(errno));
    fflush(d);
}

void level2(struct buffer* buf) {

    for (int i = 0; i < (int)(2048 / 60); i++) {
        char prefix[8];
        prefix[0] = 0x02;
        if (i != (int)(2048 / 60) - 1) {
            prefix[1] = '1';
            prefix[2] = '2';
            prefix[3] = '0';
        }
        else {

            char toChar[3];
            sprintf(toChar, "%i", (int)sizeof(buf->items[i].data));
            prefix[1] = toChar[0];
            prefix[2] = toChar[1];
            prefix[3] = toChar[2];
        }
        prefix[7] = '0';

        if (i != (int)(2048 / 60) - 1) {
            prefix[6] = 0x01;
        }
        else {
            prefix[6] = 0x0f;
        }
        char number[2];
        sprintf(number, "%i", (int)i);
        prefix[5] = number[0];

        prefix[7] = 0;
        buf->items[i].prefix = prefix;
        char sufix[3];
        char sum[2];
        sprintf(sum, "%i", controlSum(&(buf->items[i])));
        sufix[0] = sum[0];
        sufix[1] = sum[1];
        sufix[2] = '0';
        buf->items[i].sufix = sufix;
    }

    for (int i = 0; i < (int)(2048 / 60); i++) {
        level1(&(buf->items[i]), sizeof(buf->items[0]));
    }
}

void level3(struct buffer* buff) {

    for (int i = 0; i < (int)(2048 / 60); i++) {
        char header[40];

        for (int j = 0; j < 40; j++) {
            header[i] = '0';
        }
        buff->items[i].header = header;
    }

    fflush(stdout);
    level2(buff);

}

void level4(char* path, struct buffer* buf) {
    int d;
    if ((d = open(path, O_CREAT | O_RDWR, 0622)) < 0) {
        printf("problems %s", strerror(errno));
    }
    char text[2048];
    read(d, text, 2048);

    for (int i = 0; i < (int)(2048 / 60); i += 1) {
        for (int j = 0; j < 60; j += 1) {

            buf->items[i].data[j] = text[i * 60 + j];

        }

    }
    //printf("%s\n", buf->items[0].data);

    level3(buf);
}

void retrieve4(struct buffer_item* item, int pershiy) {
    if (pershiy) {

        FILE* d = fopen("file", "w");
        fprintf(d, "%s", item->data);
        printf("problems %s", strerror(errno));
    }
    else {

        FILE* d = fopen("file", "w");
        fprintf(d, "%s", item->data);
        printf("problems %s", strerror(errno));
    }
}

int retrieve3(struct buffer* buf) {
    for (int i = 0; i < 40; i++) {
        buf->items[i].header = NULL;
    }
    return sizeof(*buf);
}

int retrieve2(struct buffer* buf) {
    for (int i = 0; i < 40; i++) {
        buf->items[i].sufix = NULL;
        buf->items[i].prefix = NULL;
    }

    return sizeof(*buf);
}

struct buffer_item retrieve1(char* path, int pos) {
    FILE* f = fopen(path, "r");
    fseek(f, 1, pos);
    printf("problems %s", strerror(errno));
    struct buffer_item item;
    fread(item.prefix, 1, 8, f);
    fread(item.header, 1, 40, f);
    fread(item.data, 1, 60, f);
    fread(item.data, 1, 3, f);
    return item;

}

int main() {
    char args[2];

    args[0] = 'r';
    if (args[0] == 't') {

        struct  buffer buf;
        level4("sp5", &buf);
        return 0;
    }
    else if (args[0] == 'r') {
        struct buffer buf;

        int pos = 0;
        int i = 0;
        while (i < 40) {

            buf.items[i] = retrieve1(fifo, pos);
            pos += 111;
            i++;
        }
        retrieve2(&buf);
        retrieve3(&buf);
        retrieve4(&(buf.items[i]), 1);
        i = 1;
        while (i < 40) {
            retrieve4(&(buf.items[i]), 0);
            i++;
        }
        return 0;
    }
}
