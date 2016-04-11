#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MDNS_MULTICAST_ADDRESS  "224.0.0.251"
#define MDNS_PORT               5353 

const unsigned char MDNS_HEADER[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

const unsigned char MDNS_TAIL[] = {
    0x00, 0x01, 0x00, 0x01
};

/*
const signed char GOOGLECAST[] = {
    0x0b, 0x5f, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65,
    0x63, 0x61, 0x73, 0x74, 0x04, 0x5f, 0x74, 0x63,
    0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00
};
*/

const char GOOGLECAST[] = "_googlecast._tcp";

int main(int argc, char **argv) {
    const char *name = GOOGLECAST;

    int query_socket = socket(PF_INET, SOCK_DGRAM, 0),
        response_socket = socket(PF_INET, SOCK_DGRAM, 0); 


    struct sockaddr_in addr_send, addr_receive;
    int addr_len = sizeof(addr_send);
    addr_send.sin_family = addr_receive.sin_family = AF_INET;
    addr_send.sin_port = addr_receive.sin_port = htons(MDNS_PORT);
    addr_send.sin_addr.s_addr = inet_addr(MDNS_MULTICAST_ADDRESS);
    addr_receive.sin_addr.s_addr = htonl(INADDR_ANY);
    int name_length = strlen(name);
    int message_length = sizeof(MDNS_HEADER) + strlen(name) + sizeof(".local") + sizeof(MDNS_PORT); 

    unsigned char *message = malloc(message_length);
    memcpy(message, MDNS_HEADER, sizeof(MDNS_HEADER)); 
    sprintf((char *)(message + sizeof(MDNS_HEADER)), "%s.local", name);
    memcpy(message + sizeof(MDNS_HEADER) + strlen(name) + sizeof(".local"), MDNS_TAIL, sizeof(MDNS_TAIL));

    //printf("%s", message);
    //write(1, message, message_length); 
    sendto(query_socket, message, message_length, 0, (struct sockaddr *)&addr_send, addr_len);


    if (bind(response_socket, (struct sockaddr *) &addr_receive, sizeof(addr_receive)) < 0) {        
        perror("bind");
        exit(1);
    }   

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MDNS_MULTICAST_ADDRESS);         
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
    if (setsockopt(response_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt mreq");
        exit(1);
    }
    char response[512];
    int count;
    do {
        count = recvfrom(response_socket, response, sizeof(response), 0, (struct sockaddr *)&addr_receive, &addr_len);
        write(1, response, count);
    } while (count > 0);
}
