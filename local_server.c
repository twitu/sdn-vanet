#include <stdio.h>
#include <stdlib.h>
#include<string.h>

// TODO: change gets to more secure method of input processing
void* bulk_input() {
    int packet_index = 0;
    sleep(2); // allow all nodes to be setup
    char dest[5];
    // read destination and message from input file
    while (1) {
        scanf("%[^\n]",dest); //gets(dest);
        int dest_id = atoi(dest) - 1;
        if (dest_id < 0) return;
        char message[512];
        scanf("%[^\n]",&message[sizeof(int)*3]);
        if (dest_id == id) continue; // don't send message to self
        ((int*) message)[0] = id;
        ((int*) message)[1] = dest_id;
        ((int*) message)[2] = packet_index++;
        send_next_hop(dest_id, message);
    }
}
int main()
{
    printf("Hello world!\n");
    return 0;
}
