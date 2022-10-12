#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <stdlib.h>
#include <netpacket/packet.h>

// https://man7.org/linux/man-pages/man7/packet.7.html

void printNetworkInterfaces();

int main(int argc, char* argv[]) {

    /** Helper code for finding network interfaces **/
//    if (argc != 2) {
//        printf("Usage: %s [interface name]\n", argv[0]);
//        return 1;
//    }
//
//    uint if_index;
//    if_index = if_nametoindex(argv[1]);
//    if (if_index == 0) {
//        printf("Interface %s : No such device \b", argv[1]);
//        return 1;
//    }

//    printf("Interface %s : %d\n", argv[1], if_index);

//    printNetworkInterfaces();

    /** END of helper code **/


    int s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s < 0) {
        printf("[UPDATE]: There was an error creating the socket\n");
        return 1;
    }

    // use helper code to find interface names
    const char* if_name = "lo";
    int if_index = (int)if_nametoindex(if_name);

    struct sockaddr_ll llAddr;
    llAddr.sll_family = AF_PACKET;
    llAddr.sll_protocol = htons(ETH_P_ALL);
    llAddr.sll_ifindex = if_index;

    // Might have issues with casting sockaddr_ll to a sockaddr
    int val = bind(s, (struct sockaddr*)&llAddr, sizeof(llAddr));
    if (val != 0) {
        printf("[ERROR]: Unable to bind to socket\n");
    }

    char buffer[1024];
    while (1) {
        socklen_t len = sizeof(llAddr);
        struct sockaddr_ll inAddr;
        ssize_t iBytesSize = recvfrom(s, &inAddr, sizeof(inAddr), 0, (struct sockaddr*)&llAddr, &len);

    }

    return 0;
}

// https://programmer.ink/think/ifname-of-network-interface-and-ifndex-query-of-interface-index-under-linux.html
void printNetworkInterfaces() {
    struct if_nameindex *head, *ifni;
    ifni = if_nameindex();
    head = ifni;

    if (head == NULL) {
        perror("if_nameindex()");
        exit(EXIT_FAILURE);
    }

    while (ifni->if_index != 0) {
        printf("Interfece %d : %s\n", ifni->if_index, ifni->if_name);
        ifni++;
    }

    if_freenameindex(head);
    head = NULL;
    ifni = NULL;
}
