
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>


#define DEBUG 1
#define SOCKET_BACKLOG 5

#define log_error_if(cond, msg) \
if (cond) { \
    if (log_file) fprintf(log_file, "%s", msg); \
    exit(EXIT_FAILURE); \
}

#ifdef DEBUG
#  define log_debug(msg) if (log_file) fprintf(log_file, "%s", msg);
#else
#  undef log_debug(msg) 
#endif


typedef void* (*thread_func_t)(void*);

typedef struct {
    char method[8];
    char* host;
    char* path;
    char* file;
    struct timeval time;
    struct timezone timezone;
} http_request;

typedef struct con {
    int socket;
    struct sockaddr_in* client_addr;
    http_request* request;
    struct con* next;
} connection;

typedef struct {
    connection* begin;
    connection* end;
    pthread_mutex_t mutex;
    pthread_cond_t semaphore;
} connection_queue;

typedef struct {
    pthread_t** threads;
    _Bool* thread_is_working;
    size_t thread_pool_size;
    pthread_mutex_t mutex;
    thread_func_t worker_func;
    connection_queue* queue;
} thread_pool;


thread_pool* pool;
connection_queue* queue;

void* worker(void*);
void* pooler_f(void*);
pthread_t pooler;

char* server_root = "/";
FILE* log_file = NULL;

void thread_pool_init(size_t thread_count) {
    pool = (thread_pool*)malloc(sizeof(thread_pool));
    log_error_if(!pool, "Cannot init thread pool!\n");

    pool->threads = (pthread_t**)calloc(sizeof(pthread_t*), thread_count);
    log_error_if(!pool->threads, "Cannot init thread pool!\n");

    pool->thread_is_working = (_Bool*)calloc(sizeof(_Bool), thread_count);
    log_error_if(!pool->thread_is_working, "Cannot init thread pool\n");

    for (int i = 0; i < thread_count; i++) {
        pool->threads[i] = (pthread_t*)malloc(sizeof(pthread_t));
        log_error_if(!pool->threads[i], "Cannot create thread\n");
    }

    pool->thread_pool_size = thread_count;
    pool->worker_func = worker;
    pool->queue = queue;

    int err = pthread_create(&pooler, NULL, pooler_f, (void*)queue);
    log_error_if(err, "Cannot create pooler thread");
}

void thread_pool_destroy() {
    if (!pool) return;

    pthread_cancel(pooler);
    for (int i = 0; i < pool->thread_pool_size; i++) {
        if (pool->threads[i]) {
            pthread_cancel(*pool->threads[i]);
        }
    }

    pthread_join(pooler, NULL);
    for (int i = 0; i < pool->thread_pool_size; i++) {
        if (pool->threads[i]) {
            pthread_join(*pool->threads[i], NULL);
            free(pool->threads[i]);
        }
    }

    free(pool->threads);
    free(pool->thread_is_working);
    free(pool);
}

pthread_t* thread_pool_get() {
    pthread_t* res = NULL;
    pthread_mutex_lock(&pool->mutex);

    for (int i = 0; i < pool->thread_pool_size; i++) {
        if (!pool->thread_is_working[i]) {
            res = pool->threads[i];
            pool->thread_is_working[i] = 1;
            break;
        }
    }

    pthread_mutex_unlock(&pool->mutex);
    return res;
}

void thread_pool_return(pthread_t thr) {
    pthread_mutex_lock(&pool->mutex);
    for (int i = 0; i < pool->thread_pool_size; i++)
        if (pool->threads[i] && pthread_equal(thr, *pool->threads[i])) {
            pool->thread_is_working[i] = 0;
            break;
        }
    pthread_mutex_unlock(&pool->mutex);
}

void connection_queue_init() {
    queue = (connection_queue*)malloc(sizeof(connection_queue));
    log_error_if(!queue, "Error creating connection queue\n");

    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->semaphore, NULL);
}

void connection_queue_destroy() {
    if (!queue) return;

    connection* tmp;
    while (queue->begin) {
        tmp = queue->begin;
        queue->begin = tmp->next;
        free(tmp);
    }

    pthread_cond_destroy(&queue->semaphore);
    pthread_mutex_destroy(&queue->mutex);
    free(queue);
}

void connection_queue_push(connection* new) {
    pthread_mutex_lock(&queue->mutex);

    if (!queue->begin) {
        queue->begin = new;
        queue->end = new;
    }
    else {
        queue->end->next = new;
        queue->end = new;
    }

    pthread_cond_signal(&queue->semaphore);
    pthread_mutex_unlock(&queue->mutex);
}

connection* connection_queue_pull() {
    pthread_mutex_lock(&queue->mutex);

    connection* res = NULL;
    if (queue->begin) {
        res = queue->begin;
        queue->begin = queue->begin->next;
        if (res == queue->end)
            queue->end = NULL;
        res->next = NULL;
    }

    pthread_mutex_unlock(&queue->mutex);
    return res;
}

http_request* create_request(char* buffer, size_t size) {
    if (!buffer || size <= 0) return NULL;

    http_request* request = (http_request*)malloc(sizeof(http_request));
    if (!request) return NULL;

    char* ptoken = strtok(buffer, "\n");
    if (ptoken) {
        int len = strstr(ptoken, " ") - ptoken;
        strncpy(request->method, ptoken, sizeof(request->method) - 1);

        ptoken += len + 1;
        len = strlen(ptoken);
        int file_start = strrchr(ptoken, ' ') - ptoken;

        char* tmp = (char*)malloc(file_start + 1);
        strncpy(tmp, ptoken, file_start);

        file_start = strrchr(tmp, '/') - tmp;
        len = strlen(tmp) - file_start;

        request->path = (char*)malloc(file_start + 1);
        request->file = (char*)malloc(len + 1);

        strncpy(request->path, tmp, file_start);
        strncpy(request->file, tmp + file_start + 1, len);

        ptoken = strtok(NULL, "\n");
        while (ptoken) {
            if (!strncmp(ptoken, "Host:", 5)) {
                len = strlen(ptoken) - 5;
                request->host = (char*)malloc(len + 1);
                strncpy(request->host, ptoken + 6, len);
                break;
            }
        }

        gettimeofday(&request->time, &request->timezone);
    }

    free(buffer);
    return request;
}

void free_request(http_request* request) {
    if (!request) return;
    if (request->path) free(request->path);
    if (request->host) free(request->host);
    if (request->file) free(request->file);
    free(request);
}

void free_connection(connection* con) {
    if (!con) return;
    free_request(con->request);
    free(con->client_addr);
    close(con->socket);
    free(con);
}

#define streq(ext, str) (!strncmp(ext, str, strlen(str)) && strlen(ext) == strlen(str))
#define mimestrncpy(res, str) {\
    int l = strlen(str); \
    res = (char*)malloc(l+1); \
    strncpy(res, str, l); \
}

char* get_content_type(const char* file) {
    char* res = NULL;

    char* ext = strrchr(file, '.') + 1;
    if (streq(ext, "html") || streq(ext, "htm")) {
        mimestrncpy(res, "text/html");
    }
    else if (streq(ext, "jpg") || streq(ext, "jpeg")) {
        mimestrncpy(res, "image/jpeg");
    }
    else if (streq(ext, "png")) {
        mimestrncpy(res, "image/png");
    }
    else if (streq(ext, "txt")) {
        mimestrncpy(res, "text/plain");
    }
    else {
        mimestrncpy(res, "text/plain");
    }

    return res;
}

void write_response(int code, const connection* conn, char* buffer, size_t size) {
    char* response;
    size_t response_size;

    char* code_str;
    switch (code) {
    case 200: code_str = "OK"; break;
    case 403: code_str = "Forbidden"; break;
    case 503:
    default:  code_str = "Service Unavailable"; break;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm* tm_time = localtime(&now);
    char date[30];
    strftime(date, sizeof(date), "%a, %d %b %Y %T GMT", tm_time);

    response = (char*)malloc(size + 300);
    if (!buffer || !size) {
        sprintf(response, "HTTP/1.1 %d %s\nDate: %s\n\n", code, code_str, date);
    }
    else {
        char* content_type = get_content_type(conn->request->file);
        sprintf(response, "HTTP/1.1 %d %s\nDate: %s\nContent-Type: %s\nContent-Length: %lu\n\n%s\n",
            code, code_str, date, content_type, size, buffer);
        free(content_type);
    }
    log_debug("Sending response:\n");
    log_debug(response);
    log_debug("\n");

    write(conn->socket, response, response_size);
    free(response);
    if (buffer) free(buffer);
}

#define _200(req, buf, len) write_response(200, req, buf, len)
#define _403(req)           write_response(403, req, 0, 0)
#define _503(req)           write_response(503, req, 0, 0)

void* pooler_f(void* args) {
    connection_queue* queue = (connection_queue*)args;
    if (!queue) return NULL;


    while (1) {
        pthread_mutex_lock(&queue->mutex);
        if (!queue->begin)
            pthread_cond_wait(&queue->semaphore, &queue->mutex);

        pthread_t* thread = thread_pool_get();
        if (thread) {
            int err = pthread_create(thread, NULL, pool->worker_func, NULL);
            if (err && log_file) {
                fprintf(log_file, "Error creating worker thread\n");
            }
        }

        pthread_mutex_unlock(&queue->mutex);
        sleep(1);
    }

    return NULL;
}

void* worker(void* args) {
    connection* conn = connection_queue_pull();
    http_request* request = conn->request;
    if (strncmp(request->method, "GET", 3)) {
        _503(conn);
    }
    else {
        char fname[PATH_MAX] = { 0, };
        size_t len = strlen(server_root);
        strncpy(fname, server_root, len);
        fname[len] = '/';
        strncpy(fname + len + 1, request->path, strlen(request->path));
        if (access(fname, R_OK) != -1) {
            FILE* file = fopen(fname, "r");
            if (file) {
                fseek(file, 0, SEEK_END);
                size_t len = ftell(file);
                fseek(file, 0, SEEK_SET);

                char* buf = (char*)malloc(len);
                if (buf) {
                    fread(buf, 1, len, file);
                }
                fclose(file);
                _200(conn, buf, len);
            }
        }
        else {
            _403(conn);
        }
    }
    free_connection(conn);
    thread_pool_return(pthread_self());
    return NULL;
}

void cleanup(int unused) {
    thread_pool_destroy();
    connection_queue_destroy();
    if (log_file) fclose(log_file);
}

int main(int argc, char const* argv[])
{
    int port = 80;
    int thread_count = 5;
    int queue_size = 5;
    char* log_file_name = "log";

    int opt, len;
    while ((opt = getopt(argc, (char* const*)argv, "p:d:t:q:l:")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'd':
            len = strlen(optarg);
            server_root = (char*)malloc(len + 1);
            strncpy(server_root, optarg, len);
            break;
        case 't':
            thread_count = atoi(optarg);
            break;
        case 'q':
            queue_size = atoi(optarg);
            break;
        case 'l':
            len = strlen(optarg);
            log_file_name = (char*)malloc(len + 1);
            strncpy(log_file_name, optarg, len);
            break;
        default:
            fprintf(stderr, "Not enough params\n");
            exit(EXIT_FAILURE);
        }
    }

    log_file = fopen(log_file_name, "a");
    if (!log_file) log_file = stderr;

    int pid = fork();
    if (pid < 0) {
        fprintf(log_file, "Cannot demonize\n");
        return EXIT_FAILURE;
    }
    if (pid > 0) {
        fprintf(log_file, "Daemon created!\n");
        return EXIT_SUCCESS;
    }
    if (setsid() < 0) {
        return EXIT_FAILURE;
    }
    pid = fork();
    if (pid < 0) return EXIT_FAILURE;
    if (pid > 0) return EXIT_SUCCESS;
    //unmask(0);
    //chdir(server_root);

    sigaction(SIGPIPE, &(struct sigaction){ cleanup }, NULL);
    sigaction(SIGTERM, &(struct sigaction){ cleanup }, NULL);

    connection_queue_init();
    thread_pool_init(thread_count);

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = (struct in_addr){ htonl(INADDR_ANY) }
    };

    int insock = socket(AF_INET, SOCK_STREAM, 0);
    log_error_if(insock, "Cannot create socket\n");
    int err = bind(insock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    log_error_if(err, "Cannot bind socket\n");
    err = listen(insock, SOCKET_BACKLOG);
    log_error_if(err, "Cannot listen\n");

    int connsock;
    while (1) {
        struct sockaddr_in* in_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        if (!in_addr) {
            fprintf(log_file, "No memory!\n");
            continue;
        }
        connsock = accept(insock, (struct sockaddr*)in_addr, NULL);
        if (connsock > 0) {
            size_t buf_size = 1024;
            char* buf = (char*)malloc(buf_size);

            int tmp;
            while ((tmp = read(connsock, (void*)buf, 1024))) {
                char* newbuf = (char*)realloc(buf, buf_size + tmp);
                if (!newbuf) {
                    fprintf(log_file, "No memory\n");
                    free(buf);
                    close(connsock);
                    free(in_addr);
                    continue;
                }
                buf = newbuf;
                buf_size += tmp;
            }
            log_debug("Received request:\n");
            log_debug(buf);
            log_debug("\n");

            connection* new = (connection*)malloc(sizeof(connection));
            new->socket = connsock;
            new->client_addr = in_addr;
            new->next = NULL;
            new->request = create_request(buf, buf_size);
            if (!new->request) {
                free(buf);
                close(connsock);
                free(in_addr);
                continue;
            }

            connection_queue_push(new);
        }
        else {
            free(in_addr);
        }
        sleep(2);
    }

    cleanup(0);
    return EXIT_SUCCESS;
}
