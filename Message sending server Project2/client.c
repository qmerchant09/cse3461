#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#define MAXWAITTIME 0.5
/* This program is the client for sending a message to a server.

  Quantez Merchant
*/

int main(int argc, char *argv[]){
        int sd;
        struct sockaddr_in server_address;
        int portNumber;
        char serverIP[29];
        int rc = 0;
        char str[500];
        FILE *input;

        /* Check if user supplied the right amount of arguments */
        if (argc < 3){
                printf ("usage is client <ipaddr> <port>\n");
                exit(1);
        }

        /* Create socket, fill variables with user supplied values, and set server variables */
        sd = socket(AF_INET, SOCK_DGRAM, 0);

        portNumber = strtol(argv[2], NULL, 10);
        strcpy(serverIP, argv[1]);

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(portNumber);
        server_address.sin_addr.s_addr = inet_addr(serverIP);

        /* Get the message to send and its length */
        printf("Enter a string:  ");
        fgets(str,sizeof(str),stdin);
        int strLen = 0;
        char c = str[0];
        while(c != '\n'){
                strLen++;
                c = str[strLen];
        }
	int k;
	char str2[500]; 
	for(k = 0; k < strLen; k++){
		str2[k] = str[k];
	}

        /* Send the length of the message to the server */
        strLen = htonl(strLen);
        rc = sendto(sd,&strLen,sizeof(strLen),0,(struct sockaddr *)&server_address,sizeof(server_address));
        strLen = ntohl(strLen);
        int cpy = (strLen/2)+1;
        int i;
        int j = 0;
        int seq = 0;
        char send[2];
	char osend[1];
        time_t timeSent,currentTime;
        char bufferRead[100];
        memset(bufferRead,0,100);
        int fromLength = sizeof(server_address);
        int ackNumber;	
        for(i = 0; i < cpy; i++){
                if(strLen%2 == 1 && i == cpy-1){
                        sprintf (osend, "%11d%4d%c", seq, sizeof(osend),str2[j]);
                        rc = sendto(sd,&osend,17,0,(struct sockaddr *)&server_address,sizeof(server_address));
			timeSent = time(NULL); 
                }else{
                        sprintf (send, "%11d%4d%c%c", seq, sizeof(send),str2[j],str2[j+1]);
                        rc = sendto(sd,&send,17,0,(struct sockaddr *)&server_address,sizeof(server_address));
			timeSent = time(NULL);
		}
		sleep(1); 
		memset (bufferRead, 0, 100);
    		rc = recvfrom(sd, &bufferRead, 100, MSG_DONTWAIT,(struct sockaddr *)&server_address, &fromLength);
		if (rc > 0){
			sscanf(bufferRead, "%11d", &ackNumber);
			seq+=2;
		}else{
			currentTime = time(NULL);
			if(currentTime-timeSent > MAXWAITTIME){
				rc = sendto(sd,&send,17,0,(struct sockaddr *)&server_address,sizeof(server_address));
				timeSent = time(NULL);
				usleep(1000);

				memset (bufferRead, 0, 100);
    				rc = recvfrom(sd, &bufferRead, 100, MSG_DONTWAIT,(struct sockaddr *)&server_address, &fromLength);
				if (rc > 0){
					sscanf(bufferRead, "%11d", &ackNumber);
					seq+=2;
				}
			}
		}
		j+=2;
	}
        return 0;
}
