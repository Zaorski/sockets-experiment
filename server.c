/* 
Thomas Zaorski
RIN: 660747712
EMAIL: zaorst@rpi.edu
*/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

//Array of words and the current size
char strs[1000][20];
int size = 0;

//Sets up the mutex lock
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Struct used as argument given to threads
struct arg_struct
{
	int sockfd;
};

void * handle_input(void *arguments);

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, thread;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

    //Not enough arguments supplied
     if (argc < 2)
     {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

    if (!(atoi(argv[1]) >= 8000 && atoi(argv[1]) <= 9000))
    {
	fprintf(stderr,"Not a valid port\n");
         exit(1);
    }

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        perror("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

    //Binds
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
          perror("ERROR on binding");

     //Listens for a max of 5 client connections
     listen(sockfd,5);
     printf("Started cache-server\n");
     printf("Listening on port %d\n", atoi(argv[1]));
     clilen = sizeof(cli_addr);

    //Infinite loop, server always listening for connections
     while (1) 
     {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             perror("ERROR on accept");
         
	//My server handles mutliple client connections by using threads, -lpthread required during compilation
	pthread_t child;
	struct arg_struct *args;

	//Allocates space for the thread argument
	args = (struct arg_struct *)malloc(sizeof(struct arg_struct));
	args->sockfd = newsockfd;

	//Creates thread
	thread = pthread_create(&child, NULL, handle_input, (void *)args);
	if ( thread != 0 )
    	{
      		perror( "MAIN: Could not create child thread" );
    	}

     } 
     close(sockfd); //Closes socket, though we'll never get here
     return 0; 
}


void * handle_input(void *arguments)
{
   struct arg_struct *args = arguments;
   int sock = args -> sockfd;
   int n;
   char buffer[256];
   char temp[100];
   char temp_word[100];
  
   //Locks for critical section
   pthread_mutex_lock( &mutex );
   bzero(buffer,256);

   n = read(sock,buffer,255);
   if (n < 0) perror("ERROR reading from socket");

   //Buffer must be at least 3 letters long
   if (strlen(buffer) > 2)
   {
   strncpy(temp_word, buffer, 3);
   
   //Handles ADD case
   if (strcmp(temp_word, "ADD") == 0)
   {
   //Checks if there is actually a word listed after the ADD command
   if (strlen(buffer) > 4)
   {
   strncpy(buffer, buffer+4, strlen(buffer));
   strcpy(temp, "Adding ");
   strcat(temp, buffer);
   strcat(temp, "\n");

   //Returns Adding <word> to the client
   n = write(sock, temp, strlen(temp));

   strcpy(strs[size], buffer);
   //Increases the size of the array size counter
   size++;
   
   if (n < 0) perror("ERROR writing to socket");
   }
   else
   {
      //Returns No Word Given if there was no word provided
      n = write(sock, "No word given\n", strlen("No word given\n"));
    }
   }
   else if (strcmp(buffer, "POP") == 0)  //Handles POP case
   {
	if (size == 0)
	   n= write(sock, "EMPTY", strlen("EMPTY"));  //Returns empty if size == 0
        else
        {
	  size = size - 1;  //Reducs size
	  bzero(buffer,256);
	  strcpy(buffer, "Removed ");
          strcat(buffer, strs[size]);
	  strcat(buffer, "\n");
	  n = write(sock, buffer, strlen(buffer));
	  strcpy(strs[size], "");
        }
	
   }
   else if (strcmp(buffer, "SIZE") == 0) //Handles size case
   {
	char str[1000];
	
	sprintf(str, "%d", size);
        strcpy(temp, "Cache-server has ");
        strcat(temp, str);
	strcat(temp, " words\n");
   	n = write(sock, temp, strlen(temp));  //Returns size of the cache
   }
   else if (strlen(buffer) > 5)  //If length is at least 6
   {
     strncpy(temp_word, buffer, 6);
     if (strcmp(temp_word, "REMOVE") == 0)  //Handles Remove case
     {
	char *sub;
	strncpy(buffer, buffer+7, strlen(buffer));
	int index;
	int size_array = size;
        int spot = 0;
	for (index = 0; index < size_array; index++) //Loops through the whole strs[][] array
        {
	   sub = strstr(strs[index], buffer); //sub will be NULL if substring is not found
	   if (sub == NULL)
           {
 	     strcpy(strs[spot], strs[index]);
	     spot++;
	   }
	   else
          {
	      size = size - 1;
	      strcpy(strs[index], "");
          }
        }
	
     }
     else
	{
		
		n = write(sock, "Invalid command\n", strlen("Invalid command\n"));
         }        
   }
   else 
   {
	n = write(sock, "Invalid command\n", strlen("Invalid command\n"));
   }
   }
   else
	n = write(sock, "Invalid command\n", strlen("Invalid command\n"));
   //Unlocks
   pthread_mutex_unlock( &mutex );
   return NULL;
	
}

