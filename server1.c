//Rawal Khirodkar, 130050014
//Socket Programming, server1.c

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 256								//Max size of messages
#define BACKLOG 1 										// how many pending connections queue will hold

//------------------------------------------------------------------------------------------------------------
/*function to check if argument is numeric*/
bool is_numeric(char const* string)
{
	while('0' <= *string && *string <= '9')
	{
		string++;
	}
	return (*string == '\0');
	
}

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{

	/*accepting input*/
	if(argc == 2)
	{
		char* s = argv[1];
		if(!is_numeric(argv[1]))
		{
			fprintf(stderr,"usage %s <server-port>\n", argv[0]);
			exit(1);
		}

	}
	else
	{
		fprintf(stderr,"usage %s <server-port>\n", argv[0]);
		exit(1);
	}

//------------------------------------------------------------------------------------------------------------
	/* variables needed*/
	int sockfd, new_fd, portnum = atoi(argv[1]);			//listen on sockfd, new connection on new_fd, portnum of server
	int n,bytes_sent; 										//n is integer used as return value by functions,bytes_sent is used as return value of write
	char msg_client[MAX_INPUT_SIZE]; 						//msg read from client
	char msg_server[MAX_INPUT_SIZE]; 						//msg sent to client by server
	int sin_size = sizeof(struct sockaddr_in);

//------------------------------------------------------------------------------------------------------------
	/* structure definitions*/
	struct sockaddr_in server_addr; 						//server address info
	bzero((char *) &server_addr, sizeof(server_addr));		//initialize
	struct sockaddr_in clien_addr; 							//client address info

//------------------------------------------------------------------------------------------------------------
	/* server socket part*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);				//TCP connection

    if (sockfd < 0)
    {
		fprintf(stderr, "ERROR opening socket\n");
		exit(1);
    }

	server_addr.sin_family = AF_INET; 						// in host byte order
	server_addr.sin_port = htons(portnum); 					// short,network byte order
	
	
	if(!inet_aton("127.0.0.1", &server_addr.sin_addr)) 		// 127.0.0.1 is used as server address
	{
		fprintf(stderr, "ERROR invalid server IP address\n");
		exit(1);
    }

	memset(&(server_addr.sin_zero),'\0',8);					// zero to rest of struct

//------------------------------------------------------------------------------------------------------------
	/*binding ,listening and accepting procedure*/
	if( bind(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0 )
	{
		fprintf(stderr, "ERROR binding to given port number\n");
		exit(2);
	} 														//bind on sockfd

	if( listen(sockfd, BACKLOG) < 0 )
	{
		fprintf(stderr, "ERROR listening on given port number\n");
		exit(2);
	} 														//listen on sockfd


															// new file descriptor assigned by server to client after accepting
	new_fd = accept(sockfd, (struct sockaddr*) &clien_addr, &sin_size);

	if( new_fd < 0)
	{
		fprintf(stderr, "ERROR on accepting connection\n");
		exit(2);
	}

	close(sockfd);											// to avoid more than one client connections

//------------------------------------------------------------------------------------------------------------	
	/*extracting client info after accepting */
	char* client_IP = inet_ntoa(clien_addr.sin_addr);		//IP of client 
	int client_portnum = ntohs(clien_addr.sin_port);		//portnumber of client
	
//------------------------------------------------------------------------------------------------------------	
	/* Conversation Begins!*/
	bool said_bye = false;									//becomes true when client said "Bye"

	while(!said_bye)
	{	
		bzero(msg_server,MAX_INPUT_SIZE);					//intialize
		bzero(msg_client,MAX_INPUT_SIZE);					//intialize

		//---------------------------------------------
		/*reading from socket*/
		n = read(new_fd, msg_client, MAX_INPUT_SIZE);
		if(n < 0)
		{
			fprintf(stderr, "ERROR reading from socket\n");
			exit(2);
		}

		//---------------------------------------------
		/*processing msg from client*/
		if((strlen(msg_client) == 4) && (msg_client[0]=='B' && msg_client[1]=='y' && msg_client[2]=='e' && msg_client[3]=='\n'))
		{
			sprintf(msg_server,"Goodbye %s:%d",client_IP,client_portnum);// updating msg_server
			said_bye = true;											//client said "Bye", time to exit
		}																//a check for "Bye"

		else
		{
			sprintf(msg_server,"OK %s:%d",client_IP,client_portnum);	// updating msg_server
		}

		//---------------------------------------------
		/* writing to socket*/
		bytes_sent = write(new_fd, msg_server, strlen(msg_server));		// sending reply to client
		if(bytes_sent < 0)
		{
			fprintf(stderr, "ERROR writing to socket\n");
			exit(2);
		}
		
	}
//------------------------------------------------------------------------------------------------------------	
}																		//end of main
