#include <string.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#define BUFFSIZE 1000

/* This program is the client for a simple file transfer to a server.
 
  Quantez Merchant 
*/

int main(int argc, char *argv[]){ 
	int sd; 
	struct sockaddr_in server_address; 
	int portNumber; 
	char serverIP[29]; 
	int rc = 0;
	char filename[50];
	FILE *input;

	/* Check if user supplied the right amount of arguments */       
	if (argc < 3){ 
		printf ("usage is client <ipaddr> <port>\n"); 
		exit(1); 
	} 

	/* Create socket, fill variables with user supplied values, and set server variables */		    
	sd = socket(AF_INET, SOCK_STREAM, 0); 
			       
	portNumber = strtol(argv[2], NULL, 10); 
	strcpy(serverIP, argv[1]); 
				    
	server_address.sin_family = AF_INET; 
	server_address.sin_port = htons(portNumber); 
	server_address.sin_addr.s_addr = inet_addr(serverIP); 
					   
	/* Check if the server can be connected to by the client */
	if(connect(sd, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0) {
		close(sd); 
		perror("error connecting stream socket"); 
		exit(1); 
	}
	
	/* Get the file to transfer and check if it can be opened */
	printf("What is the name of the file you'd like to send? ");
	scanf("%s", filename);
	input = fopen(filename, "rb");
	if (!input && strcmp(filename, "DONE") != 0){
		perror("File cannot be opened");
		exit(1);
	}
	
	while (strcmp(filename, "DONE") != 0) {
		/* Send the filename, filename size and filesize of the user file to the server */
		printf("filename to send is '%s'\n",filename);	
		int sizeOfFilename = strlen(filename);
		sizeOfFilename = htonl(sizeOfFilename);		
		rc = write (sd, &sizeOfFilename, sizeof(sizeOfFilename)); 
		printf("wrote %d bytes to send the filesize\n", rc);
		if (rc < 0){ 
			perror ("sendto");  
		}
		rc = write(sd,filename,htonl(sizeOfFilename));	
		printf("wrote %d bytes to send the filename\n",rc);
		
		/* Get the size of the file and send it to the server */
		fseek(input,0,SEEK_END);
		int size = ftell(input);
		size = htonl(size);
		rc = write(sd,&size, sizeof(size));
		fseek(input,0,SEEK_SET);
		
		/* Send the bytes of the file to the server */
		unsigned char buffer [BUFFSIZE];
		int nob;		//number of bytes
		int tbr=0,tbw=0;	//total bytes read/written
		nob = fread(buffer,1,BUFFSIZE,input); 
		while (nob > 0){
			tbr += nob;
			rc = write(sd,buffer,nob);
			tbw += rc;
			nob = fread(buffer,1,BUFFSIZE,input);
		}
		printf("wrote %d bytes out of %d bytes\n",tbw,tbr);
		
		/* Check how many files the server says it recieved */
		int br;		//Bytes recieved
		rc = recv(sd,&br,sizeOfFilename,0);
		printf("The server recieved %d bytes from the client\n",ntohl(br));
		printf("The client sent %d bytes to the server\n", tbw);

		/* Get another file to transfer and check if it can be opened */
		printf("What is the name of the file you'd like to send (Enter DONE to quit)? ");
		scanf("%s", filename);
		input = fopen(filename, "rb");
		if (!input && strcmp(filename,"DONE") != 0){
			perror("File cannot be opened");
			exit(1);
		}
	}
	return 0 ;  								    
}
