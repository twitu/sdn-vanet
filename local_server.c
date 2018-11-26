#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <math.h>
#include <string.h>

#include "node.h"

#define PORT 8888
#define APPLICATION 8887
#define SLEEP 1
#define LIMIT 3
#define RANGE_SQUARE 400
#define PACKET_SIZE 512

// global variables
Node table;
int id;
sem_t* mutex;
struct sockaddr_in* app_addr;
int app_sock;

void* broadcast_data(void* args);
void* recv_data(void* args);
void* print_node(Node data, int total);
void send_next_hop(int dest, char* buffer);
double calculate_square_distance(int src, int dest);
void* application_receiver();
void* application_user_input();

int main(int argc, char* argv[]) {

    // processing arguments and creating data table
    if (argc!=2) perror("invalid arguments");
    id = atoi(argv[1]) - 1;
    table = (Node) calloc(LIMIT, sizeof(node));

    // create semaphore to ensure sequential access to shared memory
    mutex = (sem_t*) malloc(sizeof(sem_t));
    sem_init(mutex, 0, 1);

    // initialize socket for sending application messages
    app_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // initialize socket address structure for application
    app_addr = calloc(1, sizeof(struct sockaddr_in));
    app_addr->sin_family = AF_INET;
    app_addr->sin_port = htons(APPLICATION);

    // create threads for sending and receiving data
    pthread_t thread1, thread2, thread3, thread4;
    int iret1 = pthread_create(&thread1, NULL, broadcast_data, NULL);
    int iret2 = pthread_create(&thread2, NULL, recv_data, NULL);
    int iret3 = pthread_create(&thread3, NULL, application_receiver, NULL);
    int iret4 = pthread_create(&thread4, NULL, application_user_input, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL); 
    pthread_join(thread3, NULL); 
    pthread_join(thread4, NULL); 
    exit(0);
}

// calculate square of distance between this node (id) and destination node
double calculate_square_distance(int src, int dest) {
    sem_wait(mutex);
    double delta_x = (table[src].x - table[dest].x)*(table[src].x - table[dest].x);
    double delta_y = (table[src].y - table[dest].y)*(table[src].y - table[dest].y);
    sem_post(mutex);
    return delta_x + delta_y;
}

void send_next_hop(int dest, char* buffer) {

    int i, hop_id = -1;
    double dist, max_dist_in_range = calculate_square_distance(id, dest);
    if (max_dist_in_range < RANGE_SQUARE) {
        hop_id = dest; // if dest is single hop away send
    } else {
        for (i = 0; i < LIMIT; i++) {
            // skip self, skip destination as it has been checked
            // also skip node if out of range
            if (i==id || i == dest || calculate_square_distance(i, id) > RANGE_SQUARE) continue;
            // consider only nodes within range of relay node
            dist = calculate_square_distance(i, dest);
            if (dist < max_dist_in_range) {
                // find node min distance from destination node
                max_dist_in_range = dist;
                hop_id = i;
            }
        }
    }

    // drop message if next hop not available
    if (hop_id == -1) {
        #ifdef DEBUG
        printf("dropped packet destined for %d containing message:\n%s\n", dest, &buffer[sizeof(int)]);
        #endif
        return;
    }

    switch (hop_id) {
            case 0: app_addr->sin_addr.s_addr = inet_addr("10.0.0.1"); break;
            case 1: app_addr->sin_addr.s_addr = inet_addr("10.0.0.2"); break;
            case 2: app_addr->sin_addr.s_addr = inet_addr("10.0.0.3"); break;
    }

    sendto(app_sock, buffer, PACKET_SIZE, 0, (struct sockaddr*) app_addr, sizeof(struct sockaddr));
}

void* application_receiver() {
    int app_recv_sock, enable = 1;
    if ((app_recv_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) perror("socket()");
    if ((setsockopt(app_recv_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)) perror("setsockopt()");

    struct sockaddr_in* app_recv_addr;
    app_recv_addr = calloc(1, sizeof(struct sockaddr_in));
    app_recv_addr->sin_family = AF_INET;
    app_recv_addr->sin_port = htons(APPLICATION);
    app_recv_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(app_recv_sock, (struct sockaddr*) app_recv_addr, sizeof(struct sockaddr)) == -1) perror("bind()");
    
    int dest_id;
    char buffer[PACKET_SIZE];
    while (1) {
        if (recvfrom(app_recv_sock, buffer, PACKET_SIZE, 0, NULL, 0) == -1) perror("recvfrom()");

        dest_id = ((int*) buffer)[0];
        if (dest_id == id) printf("%s\n", &buffer[sizeof(int)]);
        else send_next_hop(dest_id, buffer);
    }
}


void* application_user_input() {
    while (1) {
        char buffer[PACKET_SIZE];
        int dest_id;
        printf("enter message in %d character:\n", 508); // (PACKET_SIZE) 512 - (int) 4  = 508
        scanf("%s", &buffer[sizeof(int)]);
        printf("enter destination id:\n");
        scanf("%d", &dest_id);
        dest_id = dest_id - 1;  // to ensure zero indexing in position table
        ((int*) buffer)[0] = dest_id;

        #ifdef DEBUG
        printf("packet:\n%d %s", ((int*) buffer)[0], &buffer[sizeof(int)]);
        #endif

        send_next_hop(dest_id, buffer);
    }
}


void* broadcast_data(void* args) {
    struct arg* data = (struct arg*) args;

    // initializing variables for UDP communication
    struct sockaddr_in *sock_dest;
    int sock_len = sizeof(struct sockaddr_in);
    sock_dest = (struct sockaddr_in*) calloc(1, sizeof(struct sockaddr_in));

    // sending socket
    int conn, broadcast_opt = 1;
    if ((conn = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) perror("socket()");
    // allow socket to perform broadcast
    if (setsockopt(conn, SOL_SOCKET, SO_BROADCAST, &broadcast_opt, sizeof(broadcast_opt)) == -1) perror("setsockopt()");

    // destination address
    sock_dest->sin_family = AF_INET;
    sock_dest->sin_port = htons(PORT);
    sock_dest->sin_addr.s_addr = inet_addr("10.0.0.255");

    // intialzed shared memory and position variables
    int seg_id = shmget(6789, 4096, IPC_CREAT);
    char* shared_mem = (char*) shmat(seg_id, 0, 0);
    char* offset_mem = &shared_mem[(id-1)*10];
    char x[5], y[5];
    
    // send data after fixed time intervals
    while (1) {
        sscanf(shared_mem, "%s %s", x, y);
        // sync across thread
        sem_wait(mutex);
        // generate random position
        table[id].x = atoi(x);
        table[id].y = atoi(y);
        gettimeofday(&table[id].timestamp, NULL);
        if (sendto(conn, table, sizeof(node)*LIMIT, 0, (struct sockaddr*) sock_dest, sock_len) == -1) perror("sendto()");
        sem_post(mutex);
        sleep(SLEEP);
    }
}

void* recv_data(void* args) {

    // initializing variables for UDP communication
    struct sockaddr_in *sock_dest, *sock_recv;
    int sock_len = sizeof(struct sockaddr_in);
    sock_dest = (struct sockaddr_in*) calloc(1, sizeof(struct sockaddr_in));
    sock_recv = (struct sockaddr_in*) calloc(1, sizeof(struct sockaddr_in));

    // receiving socket
    int conn_recv, enable = 1;
    if ((conn_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) perror("socket()");
    if ((setsockopt(conn_recv, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)) perror("setsockopt()");

    // receiving address
    sock_recv->sin_family = AF_INET;
    sock_recv->sin_port = htons(PORT);
    sock_recv->sin_addr.s_addr = htonl(INADDR_ANY);

    // receive and process data
    Node new_data;
    int i;
    void* buffer = malloc(sizeof(node)*LIMIT);
    if (bind(conn_recv, (struct sockaddr*) sock_recv, sock_len) == -1) perror("bind()");
    while (1) {
        if (recvfrom(conn_recv, buffer, sizeof(node)*LIMIT, 0, NULL, 0) == -1) perror("recvfrom()");
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
        #ifdef INFO
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
