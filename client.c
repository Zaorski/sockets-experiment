/* 
Thomas Zaorski
RIN: 660747712
EMAIL: zaorst@rpi.edu
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h> 

int sockfd, portno, n, i, j;
struct sockaddr_in serv_addr;
struct hostent *server;
	
//Function used to connect to server
void connecter();


int main(int argc, char *argv[])
{
    //Checks if -r is in the command line
    bool flag = false;
    if(strcmp(argv[1], "-r") == 0)
      flag = true;

    char word2[100];
    char buffer[256];

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    printf("Started cache-client\n");

    //Sets port number and host server depending on whether -r was supplied
    if (flag)
    {
    	printf("Server is %s:%d\n", argv[3], atoi(argv[4]));
        portno = atoi(argv[4]);
        server = gethostbyname(argv[3]);
    }
    else
    {
	printf("Server is %s:%d\n", argv[1], atoi(argv[2]));
        portno = atoi(argv[2]);
        server = gethostbyname(argv[1]);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    //Produces 25 random words and sends them to the server as ADD <word>
    char temp[20];
    srand(time(NULL));
    for(i=0; i<25; i++) 
    {
	int run=rand()%12 + 3;
	for(j=0; j< run ; j++)
	{
		temp[j]= rand()%26+'a';
	}
	temp[run] = '\0';
	strcat(word2, "ADD ");
	strcat(word2, temp);
	
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
       perror("ERROR connecting");
    
    bzero(buffer,256);
    
    n = write(sockfd,word2,strlen(word2));
    if (n < 0) perror("ERROR writing to socket");
    strcpy(word2, "");
    
    n = read(sockfd,buffer,255);
    if (n < 0) 
         perror("ERROR reading from socket");
    printf("%s",buffer);
    close(sockfd);
    connecter();
     }

     //Retrieves size of words array from the server
     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        perror("ERROR connecting");
	n = write(sockfd, "SIZE", strlen("SIZE"));
	bzero(buffer,256);
	n = read(sockfd, buffer, 255);
        if (n < 0) perror("ERROR writing to socket");
	   printf("%s", buffer);
    	close(sockfd);
	connecter();

     //Pops last ten words, returns EMPTY if theres nothing to pop
     for (i = 0; i<10; i++)
	{
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
          perror("ERROR connecting");
	n = write(sockfd, "POP", strlen("POP"));
	bzero(buffer,256);
	n = read(sockfd, buffer, 255);
	if (n < 0) perror("ERROR writing to socket");
	printf("%s", buffer);
    	close(sockfd);
	connecter();
	}

     //Gets new size after the pops
     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        perror("ERROR connecting");
	n = write(sockfd, "SIZE", strlen("SIZE"));
	bzero(buffer,256);
	n = read(sockfd, buffer, 255);
        if (n < 0) perror("ERROR writing to socket");
	   printf("%s", buffer);
    	close(sockfd);

	//Removes words containing provided substring
	if(flag)
	{

		printf("-r flag detected\n");
		printf("Removing all occurrences of %s\n", argv[2]);
		strcpy(word2, "REMOVE ");
		strcat(word2, argv[2]);
	        
		//Sends remove command
		connecter();
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        	perror("ERROR connecting");
		n = write(sockfd, word2, strlen(word2));
		close(sockfd);
		connecter();
		
		//Prints new size
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
       		perror("ERROR connecting");
		n = write(sockfd, "SIZE", strlen("SIZE"));
		bzero(buffer,256);
		n = read(sockfd, buffer, 255);
		if (n < 0) perror("ERROR writing to socket");
		printf("%s", buffer);
    		close(sockfd);
		
	}
    
    return 0;
}

//Helps with connection to server
void connecter()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
}

