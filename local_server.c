#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "node.h"

#define PORT 8888
#define SLEEP 1
#define LIMIT 3

void die(char* s);
void* broadcast_data(void* args);
void* recv_data(void* args);
void* print_node(Node data, int total);

struct arg {
    Node data;
    sem_t* mutex;
    int seq;
};

int main(int argc, char* argv[]) {

    // processing arguments and creating data table
    if (argc!=2) die("invalid arguments");
    int id = atoi(argv[1]) - 1;
    Node data = (Node) calloc(LIMIT, sizeof(node));

    // create semaphore to ensure sequential access to shared memory
    sem_t* mutex = (sem_t*) malloc(sizeof(sem_t));
    sem_init(mutex, 0, 1);

    // argument for passing to threads
    struct arg* Arg = (struct arg*) malloc(sizeof(struct arg));
    Arg->data = data;
    Arg->seq = id;
    Arg->mutex = mutex;

    // create threads for sending and receiving data
    pthread_t thread1, thread2;
    int iret1 = pthread_create(&thread1, NULL, broadcast_data, (void*) Arg);
    int iret2 = pthread_create(&thread2, NULL, recv_data, (void*) Arg);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL); 
    exit(0);
}

void* broadcast_data(void* args) {
    struct arg* data = (struct arg*) args;

    // initializing variables for UDP communication
    struct sockaddr_in *sock_dest;
    int sock_len = sizeof(struct sockaddr_in);
    sock_dest = (struct sockaddr_in*) calloc(1, sizeof(struct sockaddr_in));

    // sending socket
    int conn, broadcast_opt = 1;
    if ((conn = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) die("socket()");
    // allow socket to perform broadcast
    if (setsockopt(conn, SOL_SOCKET, SO_BROADCAST, &broadcast_opt, sizeof(broadcast_opt)) == -1) die("setsockopt()");

    // destination address
    sock_dest->sin_family = AF_INET;
    sock_dest->sin_port = htons(PORT);
    sock_dest->sin_addr.s_addr = inet_addr("10.0.0.255");

    // seed for random position
    srand(time(0));
    
    Node table = data->data;
    sem_t* mutex = data->mutex;
    int id = data->seq;
    // send data after fixed time intervals
    while (1) {
        // sync across thread
        sem_wait(mutex);
        // generate random position
        table[id].x = rand();
        table[id].y = rand();
        gettimeofday(&table[id].timestamp, NULL);
        if (sendto(conn, table, sizeof(node)*LIMIT, 0, (struct sockaddr*) sock_dest, sock_len) == -1) die("sendto()");
        sem_post(mutex);
        sleep(SLEEP);
    }
}

void* recv_data (void* args) {
    struct arg* data = (struct arg*) args;

    // interpret arguments
    Node table = data->data;
    sem_t* mutex = data->mutex;

    // initializing variables for UDP communication
    struct sockaddr_in *sock_dest, *sock_recv;
    int sock_len = sizeof(struct sockaddr_in);
    sock_dest = (struct sockaddr_in*) calloc(1, sizeof(struct sockaddr_in));
    sock_recv = (struct sockaddr_in*) calloc(1, sizeof(struct sockaddr_in));

    // receiving socket
    int conn_recv;
    if ((conn_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) die("socket()");

    // receiving address
    sock_recv->sin_family = AF_INET;
    sock_recv->sin_port = htons(PORT);
    sock_recv->sin_addr.s_addr = htonl(INADDR_ANY);

    // receive and process data
    Node new_data;
    int i;
    void* buffer = malloc(sizeof(node)*LIMIT);
    if (bind(conn_recv, (struct sockaddr*) sock_recv, sock_len) == -1) die("bind()");
    while (1) {
        if (recvfrom(conn_recv, buffer, sizeof(node)*LIMIT, 0, (struct sockaddr*) sock_recv, &sock_len) == -1) die("recvfrom()");
        new_data = (Node) buffer;

        sem_wait(mutex);
        // check and update table data
        for (i = 0; i < LIMIT; i++) {
            if (table[i].timestamp.tv_sec < new_data[i].timestamp.tv_sec) {
                table[i].x = new_data[i].x;
                table[i].y = new_data[i].y;
                table[i].timestamp.tv_sec = new_data[i].timestamp.tv_sec;
                table[i].timestamp.tv_sec = new_data[i].timestamp.tv_sec;
            }
        }
        #ifdef DEBUG
        print_node(table, LIMIT);
        printf("\n");
        fflush(stdout);
        #endif
        sem_post(mutex);
    }
}

void* print_node(Node data, int total) {
    int i = 0;
    for (i = 0; i < total; i++) {
        printf("id: %d, position: (%d, %d), time: %ld.%06ld\n", i+1, data[i].x, data[i].y, data[i].timestamp.tv_sec, data[i].timestamp.tv_usec);
    }
}

void die(char* s) {
    perror(s);
    exit(1);
}