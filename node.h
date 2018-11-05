#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

// size is 28 bytes
typedef struct node {
    int x, y;
    struct timeval timestamp;
} node;
typedef node* Node;

// compare nodes based on timestamp
int compare(Node a, Node b);

// print node data
void* print_node(Node data, int total);