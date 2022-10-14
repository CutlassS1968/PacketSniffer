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
#include <netinet/ip.h>
#include <netinet/ether.h>

// Useful information: "ip addr show" will show you addresses for debugging
int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("[ERROR]: Usage: ./sniffer <network_interface>\n");
        return 0;
    }

    char* if_name = (char*)malloc(100*sizeof(char));
    strcpy(if_name, argv[1]);

    int s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s < 0) {
        printf("[UPDATE]: There was an error creating the socket\n");
        return 1;
    }

    // use helper code to find interface names
    // Two interfaces on CIS457 RDP: ens160 and lo
    int if_index = (int)if_nametoindex(if_name);

    free(if_name);

    struct sockaddr_ll llAddr;
    llAddr.sll_family = AF_PACKET;
    llAddr.sll_protocol = htons(ETH_P_ALL);
    llAddr.sll_ifindex = if_index;

    // Might have issues with casting sockaddr_ll to a sockaddr
    int val = bind(s, (struct sockaddr*)&llAddr, sizeof(llAddr));
    if (val != 0) {
        printf("[ERROR]: Unable to bind to socket\n");
    }

    printf("[UPDATE]: Listening for incoming traffic...\n");

    unsigned char *buffer = (unsigned char *)malloc(65536);
    struct ether_header *eh = (struct ether_header *) buffer;
    struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ether_header));

    while (1) {
        socklen_t len = sizeof(llAddr);
        struct sockaddr_ll inAddr;

        ssize_t iBytesSize = recvfrom(s, buffer, 65536, 0, (struct sockaddr*)&llAddr, &len);
        
        // get the ethernet header location in the buffer
        if (iBytesSize == -1) {
            perror("recvfrom");
            return 1;
        } else if (iBytesSize == sizeof(buffer)) {
            printf("[ERROR]: Frame is too large for buffer");
        } else {
            // Handle frame
            if (inAddr.sll_pkttype != PACKET_OUTGOING) {
            	
                memcpy(eh, (struct ether_header *) buffer, sizeof(struct ether_header));
                printf("[INCOMING]:\n");
                printf("Frame Size = %ld bytes\n", iBytesSize);
                printf("Destination MAC Address = %s\n", ether_ntoa((struct ether_addr*)eh->ether_dhost));
                printf("Source MAC Address = %s\n", ether_ntoa((struct ether_addr*)eh->ether_shost));
                printf("Type = 0x%04x\n", eh->ether_type);

                // If Ethernet packet is type IPv4 (0x0800 or ETH_P_IP), print src and dest addresses
                if (eh->ether_type == ETH_P_IP) {
                    // Get the ip header from its location in the buffer
                    memcpy(iph, (struct iphdr *) (buffer + sizeof(struct ether_header)), sizeof(struct iphdr));
                    uint32_t ip;
                    ip = iph->saddr;
                    printf("IPv4: Source Address = %s\n", inet_ntoa(*(struct in_addr*)&ip));
                    ip = iph->daddr;
                    printf("IPv4: Destination Address = %s\n", inet_ntoa(*(struct in_addr*)&ip));
                    memset(iph, 0, sizeof(struct iphdr));
                }
                memset(eh, 0, sizeof(struct ether_header));
                printf("\n\n");
            }
        }
        memset(buffer, 0, 65536);
    }
    close(s);

    return 0;
}
