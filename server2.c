//Rawal Khirodkar, 130050014
//Socket Programming, server2.c

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h> 									//strlen
#include <stdbool.h>									//booleans
#include <arpa/inet.h> 									//IP address conversion
#include <sys/time.h> 									//FD_SET, FD_ISSET, FD_ZERO macros

#define MAX_INPUT_SIZE 256								//Max size of messages
#define BACKLOG 3										// how many pending connections queue will hold
#define MAX_CLIENTS 30									//max multiple client limit of server

//------------------------------------------------------------------------------------------------------------
/*function to check if argument is numeric*/
bool is_numeric(char const* string)
{
	while('0' <= *string && *string <= '9')
	{
		string++;
	}
	return (*string == '\0');
	
}															// function to check if argument is numeric

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]){

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
	/* variables needed */
	int i, j;																// for loop variables
	int client_fds[MAX_CLIENTS];											// arrray containing connected client fds
	int maxfd;																// will store the maximum fds in the Set "readfds" (defined below)
	int master_sockfd, new_fd; 												// listen on master_sockfd, new connection on new_fd
	int portnum = atoi(argv[1]);											// portnum of server
	int n, bytes_sent; 														// n is integer used as return value by functions,bytes_sent is used as return value of write
	int sin_size = sizeof(struct sockaddr_in);

	char msg_client[MAX_INPUT_SIZE]; 										// msg read from client
	char msg_server[MAX_INPUT_SIZE]; 										// msg sent to client by server
	fd_set readfds;															// Set of file-descriptors

//------------------------------------------------------------------------------------------------------------
	/* structure definitions */
	struct sockaddr_in server_addr; 										// server address info
	bzero((char *) &server_addr, sizeof(server_addr));						// initialize
	struct sockaddr_in clien_addr; 											// client address info

	memset(client_fds,0,MAX_CLIENTS*sizeof(int));							// intialize client array to zero

//------------------------------------------------------------------------------------------------------------
	/* server socket part */
	master_sockfd = socket(AF_INET, SOCK_STREAM, 0);						// TCP connection

    if (master_sockfd < 0)
    {
		fprintf(stderr, "ERROR opening socket\n");
		exit(1);
    }

	server_addr.sin_family = AF_INET; 										// in host byte order
	server_addr.sin_port = htons(portnum); 									// short,network byte order

	if(!inet_aton("127.0.0.1", &server_addr.sin_addr)) 						// 127.0.0.1 is used as server address
	{
		fprintf(stderr, "ERROR invalid server IP address\n");
		exit(1);
    }

	memset(&(server_addr.sin_zero),'\0',8);									// zero to rest of struct

//------------------------------------------------------------------------------------------------------------
	/* binding and listening procedure */
	if( bind(master_sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0 )
	{
		fprintf(stderr, "ERROR binding to given port number\n");
		exit(2);
	}																		// bind on master-sockfd

	if( listen(master_sockfd, BACKLOG) < 0)
	{
		fprintf(stderr, "ERROR listening on given port number\n");
		exit(2);
	}																		// listen on master-sockfd

//------------------------------------------------------------------------------------------------------------
	/* continuous loop */	
	while(true)
	{	
		FD_ZERO(&readfds);													// clear socket set
		FD_SET(master_sockfd, &readfds);									// add master socket fds to set

		maxfd = master_sockfd; 												// only one present intially,hence the max

		//--------------------------------------------------------------------
		/* adding clients to set */
		for(i = 0; i < MAX_CLIENTS ; i++)
		{
			if(client_fds[i] > 0)
			{
				
				FD_SET(client_fds[i],&readfds);								// adding all legitimate client socket fd from array to set
			}

			
			if(maxfd < client_fds[i])
			{
				maxfd = client_fds[i];										// updating max fds accordingly
			}
			
		}

		//--------------------------------------------------------------------
		/* using select the SET of fds is modified */
		n = select( maxfd + 1, &readfds, NULL, NULL, NULL);					// write and except and timeout fields are set NULL

		if(n < 0)
		{
			fprintf(stderr, "ERROR in select\n");
			exit(2);
		}

		//--------------------------------------------------------------------
		/* for new incoming connection on the master socket */
		if (FD_ISSET(master_sockfd, &readfds))
		{	
																			// new file descriptor assigned by server to client after accepting
			new_fd = accept(master_sockfd, (struct sockaddr*) &clien_addr, &sin_size);

			if( new_fd < 0)
			{
				fprintf(stderr, "ERROR on accepting connection\n");
				exit(2);
			}

			for(i = 0; i < MAX_CLIENTS ;i++)
			{
				
				if(client_fds[i] == 0)
				{
					client_fds[i] = new_fd;									//adding new connection to the array of fds
					break;	
				}
			}
		}

		//--------------------------------------------------------------------
		/* else processing for input-output between clients and server */
		for(i = 0; i < MAX_CLIENTS ;i++)
		{
			if(FD_ISSET( client_fds[i], &readfds ))
			{	
				//----------------------------------------------
				/*extracting client info*/
																			// stores the info of who is at the other end in "clien_addr"	
				getpeername( client_fds[i], (struct sockaddr*)&clien_addr ,&sin_size);
				char* client_IP = inet_ntoa(clien_addr.sin_addr);
				int client_portnum = ntohs(clien_addr.sin_port);

				bzero(msg_server,MAX_INPUT_SIZE);							//intialize
				bzero(msg_client,MAX_INPUT_SIZE);							//intialize

				//----------------------------------------------
				/* reading from socket */
				n = read(client_fds[i], msg_client, MAX_INPUT_SIZE);		// reading from socket in a loop.

				if(n < 0)
				{
					fprintf(stderr, "ERROR reading from socket\n");
					exit(2);
				}

				//----------------------------------------------
				/* processing msg from client */
				bool said_bye = false;										//becomes true when client said "Bye"

				//-----------------------
				/*** a check for "Bye" ***/
				if((strlen(msg_client) == 4) && (msg_client[0]=='B' && msg_client[1]=='y' && msg_client[2]=='e' && msg_client[3]=='\n'))
				{	
																			// updating msg_server
					sprintf(msg_server,"Goodbye %s:%d",client_IP,client_portnum);
					said_bye = true;
				}															

				//-----------------------
				/*** a check for "List" ***/
				else if	((strlen(msg_client) == 5) && (msg_client[0]=='L' && msg_client[1]=='i' && msg_client[2]=='s' && msg_client[3]=='t' && msg_client[4]=='\n'))
				{	
																			// updating msg_server
					sprintf(msg_server,"OK %s:%d",client_IP,client_portnum);

					for(j = 0; j< MAX_CLIENTS ;j++)							// Printing list of clients
					{	
						if(client_fds[j] > 0 && j!=i)
						{													// stores the info of who is at the other end in "clien_addr"	
							getpeername( client_fds[j], (struct sockaddr*)&clien_addr ,&sin_size);
							client_IP = inet_ntoa(clien_addr.sin_addr);
							client_portnum = ntohs(clien_addr.sin_port);
																			// updating msg_server
							sprintf(msg_server,"%s, %s:%d",msg_server,client_IP,client_portnum);
						}
					}
				}															

				//-----------------------
				/*** normal conversation ***/
				else
				{
					sprintf(msg_server,"OK %s:%d",client_IP,client_portnum);// updating msg_server

				}

				//----------------------------------------------
				/* writing to socket */
																			// sending reply to client
				bytes_sent = write(client_fds[i], msg_server, strlen(msg_server));

				if(bytes_sent < 0)
				{
					fprintf(stderr, "ERROR writing to socket\n");
					exit(2);
				}

				//----------------------------------------------
				/* closing connection on "Bye" */
				if(said_bye)
				{
					close(client_fds[i]);
					client_fds[i] = 0;
				}
			}																// if(FD_ISSET( client_fds[i], &readfds ) ends														
		}																	// for over client array ends		
	}																		// while (true) ends
//------------------------------------------------------------------------------------------------------------
}																			// end of main