



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
#include <pthread.h>
FILE* d;
struct sockaddr_in in;
struct sockaddr_in client;
int d_server;

struct thread_pool {
    pthread_t** threads;
    int counter;
    int exist;
    pthread_mutex_t mutex;
    void* obrobnik;
};
struct connection {
    char dataSend[1024];
    char dataGet[1024];
    struct connection* next;

};
struct connection_queue {
    struct connection* begin;
    struct connection* end;
    int semafor;
    int size;
};
struct connection_queue connection_queue_init(int size) {
    struct connection_queue queue;
    queue.begin = (struct connection*)malloc(sizeof(struct connection) * size);
    queue.size = 0;
    return queue;
};

void connection_queue_push(struct connection_queue* queue, int port) {
    struct connection con;

    if (queue->size == 0) {
        queue->begin = &con;
        queue->size++;
        return;
    }
    struct connection* temp = queue->begin;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = &con;
    queue->size++;
}

void connection_queue_destroy(struct connection_queue* queue) {
    queue->begin = queue->end = NULL;
    queue->size = 0;
    queue->semafor = 0;
}



void thread_pool_destroy(struct thread_pool* pool) {
    pool->threads = NULL;
    pool->counter = 0;
    pool->exist = 0;
    //pool->mutex = NULL;
    pool = NULL;
}

void connection_queue_pull(struct connection_queue* queue, struct connection* con) {
    struct connection* temp = queue->begin;
    while (temp->next != con) {
        temp = temp->next;
    }
    temp->next = con->next;
    queue->size--;
}


void obrobnik(struct connection* con, int d_client) {

    unsigned int size = sizeof(client);
    if ((d_client = accept(d_server, (struct sockaddr*)&(client), &size)) < 0) {
        fprintf(d, "problems with accept");
    }
    switch (fork()) {
    case -1:
        printf("problems %s", strerror(errno));
        close(d_client);
        break;
    case 0:
        setsid();
        int get;
        int sendS;
        char getBuff[1024];
        char sendBuff[2200];
        time_t timr;
        struct tm now;
        do {
            get = recv(d_client, getBuff, sizeof(getBuff) - 1, 0);
            if (get <= 0) {
                printf("problems %s", strerror(errno)); continue;
            }
            FILE* g;
            if ((g = fopen(getBuff, "r")) == NULL) {
                int n = sizeof("no file");
                sprintf(sendBuff, "no file");
                continue;
            }
            char temp[3];
            char info[2048];
            fread(info, 2048, 1, g);
            getBuff[get] = '\0';
            temp[0] = getBuff[get - 3];
            temp[1] = getBuff[get - 2];
            temp[2] = getBuff[get - 1];
            time(&timr);
            now = *(localtime(&timr));
            sendS = sprintf(sendBuff, "HTTP/1.1 200 Ok, Content-Type: %s, time %s, content length %i, buffer %s", temp, asctime(&now), (int)sizeof(info), info);
            send(d_client, sendBuff, sendS, 0);
        } while (strcmp(getBuff, "close") != 0);
        fprintf(d, "process close");
        close(d_client); break;
    default: close(d_client);
        fprintf(d, "work is client over\n"); fclose(d); break;
    }

}
struct thread_pool thread_pool_init() {
    struct thread_pool pool;
    pool.obrobnik = obrobnik;
    return pool;
};
struct connection* get_connection(struct connection_queue* queue, int pos) {
    struct connection* temp = queue->begin;
    int i = 0;
    while (i < pos) {
        temp = temp->next;
        i++;
    }
    return temp;
}
int main(int argc, char* argv[]) {
    int port;
    char* root;
    int threads;
    int queueSize;
    char* log;
    int rez;
    while ((rez = getopt(argc, argv, "p:r:t:q:l")) != -1) {
        switch (rez) {
        case 'a': port = atoi(optarg); break;
        case 'r': root = optarg; break;
        case 't': threads = atoi(optarg); break;
        case 'q': queueSize = atoi(optarg); break;
        case 'l': log = optarg; break;

        }
    }

    d = fopen("log", "w");
    if (d < 0) {
        printf("problems %s", strerror(errno));
    }
    port = 3216;
    root = "log";
    threads = 4;
    queueSize = 4;
    log = "log";
    fprintf(d, "start\n");
    //printf("%s", strerror(errno));
    switch (fork()) {
    case -1: printf("problems %s", strerror(errno)); break;
    case 0:
        in.sin_family = PF_INET;
        in.sin_addr.s_addr = htonl(INADDR_ANY);
        in.sin_port = htons(port);
        d_server = socket(PF_INET, SOCK_STREAM, 0);;
        printf("%s", strerror(errno));
        if (queueSize >= 128) {
            queueSize = 127;
        }
        struct connection_queue queue = connection_queue_init(queueSize);
        printf("problems %s", strerror(errno));
        if (threads >= 16) {
            threads = 15;
        }

        struct thread_pool pool = thread_pool_init();
        pool.threads = (pthread_t**)malloc(sizeof(pthread_t) * threads);
        printf("problems %s", strerror(errno));
        pool.counter = threads;
        pool.exist = 0;
        if (bind(d_server, (struct sockaddr*)&(in), sizeof(in)) < 0) {
            printf("problems %s", strerror(errno));
        }

        fprintf(d, "begin listen\n");
        listen(d_server, 5);
        int i;
        while (i < 5) {
            i++;
            int d_client;
            unsigned int size = sizeof(client);
            if ((d_client = accept(d_server, (struct sockaddr*)&(client), &(size))) < 0) {
                fprintf(d, "problems with accept");
                continue;
            }

            connection_queue_push(&queue, port);
            while (queue.size >= 127) {

            }
            if (pool.exist >= pool.counter) {
                printf("Too many threads");
                sleep(5);
            }
            pthread_t thread;
            pool.threads[pool.exist] = &thread;
            pool.exist++;
            int err = pthread_create(&thread, NULL, obrobnik, get_connection(&queue, queue.size - 1));
            if (err) {
                printf("Error creating thread: %d\n", err);
                return 1;
            }



        }
        connection_queue_destroy(&queue);
        printf("the end");
        printf("problems %s", strerror(errno));
    default:
        fprintf(d, "pool is created, parent will exit\n"); fclose(d); break;
    }
}
