//COMP 429 SOCKET PROGRAMMING PROJECT
//CHARLES MINDERHOUT & MICHAEL MERABI

#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h> 
#include <arpa/inet.h>     
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>

//for use of easily identify flags, and inputs in code
#define STDIN  0    
#define TRUE   1  
#define FALSE  0 

     
int main(int argc, char *argv[]) {  

    if (argc !=2)
    { // checks to see if the program was initiated correctly
        printf("Arguments passed in incorrectly...please try again.");
        return(0);
    }

    int addrlen, new_socket, connection_socket[30], max_clients = 30, activity, i, valread, sd;   
    int max_sd;   
	int server_port = atoi(argv[1]); // converting command line argument to integer for port number

	
    for (i = 0; i < max_clients; i++)
    { // initializing array to all 0s for use of identifying connections later
        connection_socket[i] = 0;   
    }   
    
    char buffer[256];  //data buffer
         
    fd_set readfds; //data structure for storing sockets and their corresponding information
        
	int master_socket; //main listening socket

    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    { //creating main listening socket
        perror("socket");   
        exit(EXIT_FAILURE);   
    }   
	
    

	char hostbuffer[256]; 
	char *ip_buffer; // to store computers ip address in
	struct hostent *host_entry; 
	int host_name; 
  
	host_name = gethostname(hostbuffer, sizeof(hostbuffer)); 
	host_entry = gethostbyname(hostbuffer); 
	ip_buffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
                     
	struct sockaddr_in address; // used to hold specific information about the socket and where we are opening it on
	
	bzero(&address, sizeof(address));
	
    address.sin_family = AF_INET; // specify ipv4
    address.sin_addr.s_addr = INADDR_ANY; // tells socket to listen on all interfaces
    address.sin_port = htons(server_port); // specify port while converting to the data type needed 
	 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {   //bind socket and the corresponding ip and port together
        perror("bind");   
        exit(EXIT_FAILURE);   
    }  

    printf("Listening on port %d \n", server_port);   

	if (listen(master_socket, 3) < 0)
	{ // tell the program to listen on that master socket
		perror("listen");
		exit(EXIT_FAILURE);
	}
    
	//while loop to accept incoming communication from sockets, or keyboard
	addrlen = sizeof(address);   
	puts("Waiting for connections...");
    puts("Please input a command...For a list of console commands type, \"help\"");

    while(TRUE)
    {//run infinitely

        FD_ZERO(&readfds);   
		
		FD_SET(STDIN, &readfds); //for use in reading console input
		
        FD_SET(master_socket, &readfds); // our server socket
		
        max_sd = master_socket; 
		
		//add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++) 
        {   
            //socket descriptor  
            sd = connection_socket[i];   
                 
            //if valid socket descriptor then add
            if(sd > 0)   
                FD_SET(sd , &readfds);   
                 
            //highest file descriptor number 
            if(sd > max_sd)   
                max_sd = sd;   
        } 
		
		
		 
		//activity = select(max_sd+1 , &readfds, NULL, NULL, NULL); //checks for activity on one of the sockets,
		if ((select(max_sd+1 , &readfds, NULL, NULL, NULL) < 0) && (errno!=EINTR))
        { // makes sure there is really an error and the sytem call wesnt merely interuptted by retrying the call.
			printf("select error");
		}
		
		if (FD_ISSET(STDIN, &readfds)) 
        { // checking for communication on the command line
			int valread = read(STDIN_FILENO, buffer, sizeof(buffer));
			if (valread == 0)
            {
				printf("command error\n");
			}
			else
			{
				int index;
				int count = 0;
				
				buffer[valread] = '\0';		
			    char* token = strtok(buffer, " ");

				while(token != NULL)
				{   
					if ((strcmp(token, "help\n")) == 0)
					{
                        printf("The list of acceptable commands is as follows:\n\n");
                        printf("myip: returns the ip address of the computer\n\n"); 
                        printf("myport: returns the port the server is listening for connections on\n\n");
                        printf("connect: connects to another server for communication when using the command in the following format...\n"); 
                        printf("\tconnect 'ipaddress' 'portnumber'\n");
                        printf("\texample: connect 172.0.0.1 8888\n\n");
                        printf("list: returns a list of all current connections with their respective id, ip address, and port\n\n");
                        printf("terminate: terminates connection of the specified id number\n");
                        printf("\texample: terminate 0\n\n");
                        printf("send: sends message to the connection of the specified id number\n");
                        printf("\texample: send 0 This is a test\n\n");
                        printf("exit: gracefully closes all connections and exits the program\n\n");
                        printf("RETURNING USER TO CONSOLE COMMAND LINE\n\n");
                        printf("Please input a command...\n");
						break;
					}
					else if ((strcmp(token, "myip\n")) == 0)
					{
						printf("The servers (this computer) ip address is: %s\n\n", ip_buffer);
						break;
					}
					else if ((strcmp(token, "myport\n")) == 0)
					{
						printf("\nThe Port Number of the server is: %d\n\n", server_port);
						break;
					}
					else if ((strcmp(token, "connect")) == 0)
					{  
						// parse through command inputed and separate specific information
						int duplicate_connection = FALSE;
						char* ip_address_connect;
						char* temp_port;
						int port_number_connect;
						ip_address_connect = strtok(NULL, " ");
						temp_port = strtok(NULL, " ");
						port_number_connect = atoi(temp_port);
					
						// cycle through connection list
						for (i = 0; i < max_clients; i++)   
						{   
							//if position is not empty  
							if(connection_socket[i] > 0)   
							{  //checks to see if client is already connected to
								sd = connection_socket[i];
								getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
								char* temp_string = inet_ntoa(address.sin_addr);
								int temp_int = ntohs(address.sin_port);
								
								if (!(strcmp(ip_address_connect, temp_string)) && port_number_connect == temp_int) 
								{
									printf("The connection that you have requested is already established...please try another connection.\n\n");
									duplicate_connection = TRUE;
								}
								else if (!(strcmp(ip_address_connect, temp_string)))
								{
									printf("The IP address is a match.\n");
								}
								else if (port_number_connect == temp_int)
								{
									printf("The port number is a match.\n");
								}
							}
						}
						
						if (!(strcmp(ip_address_connect, ip_buffer)) && port_number_connect == server_port)
						{
							printf("You are not able to connect with yourself...please try another connection.\n\n");
							duplicate_connection = TRUE;
						}
			
						
						if (!(duplicate_connection))
                        {// if what you are trying to connect to is not yourself (same ip & port) and is not already an active connection, proceed with connect
							int network_socket;
							network_socket = socket(AF_INET, SOCK_STREAM, 0);
						
							int connection_success = FALSE;
							
							struct sockaddr_in server_address;
							server_address.sin_family = AF_INET;
							server_address.sin_port = htons(port_number_connect);
							server_address.sin_addr.s_addr = inet_addr(ip_address_connect);
							
							
							
							if (connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1)
                            {
								printf("There was an error making a connection to the remote socket \n\n");
							}
							else
                            {
								printf("Connection successfull\n");
								connection_success = TRUE;
							}
							
							if (connection_success)
                            {
								//add new socket to array of sockets  
								for (i = 0; i < max_clients; i++)   
								{   
									//if position is empty  
									if(connection_socket[i] == 0)   
									{   
										connection_socket[i] = network_socket;   
										printf("Adding to list of sockets as %d\n\n" , i);   	 
										break;   
									}   
								}
							}
                        }	
						break;
					}
					else if ((strcmp(token, "list\n")) == 0)
                    {    
						
						printf("List of Connected Sockets:\n\n");
						int total_connections = 0;

						// go through connection list
						for (i = 0; i < max_clients; i++)   
						{   
							//if position is empty  
							if(connection_socket[i] > 0)   
							{  
								total_connections++;
								if (total_connections == 1) printf("ID: IP address \t Port Number\n");
								sd = connection_socket[i];
								getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
								printf(" %d: %s \t %d\n", i, inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); // print out each connections information
							} 
						} 

						printf("\n");
	
						if(total_connections == 0)
                        {
							printf("You currently have no connections connections.\n\n");
						}
						
						break;
					}
					else if ((strcmp(token, "terminate")) == 0)
                    {
						
						char* temp_id;
                        int socket_to_terminate;
						int is_valid_connection = FALSE;
						
						//find connection socket to terminate using id given
                        temp_id = strtok(NULL, " ");
                        int id_to_terminate = atoi(temp_id);
						
						// cycle through connection list
						for (i = 0; i < max_clients; i++)   
						{   
							if (id_to_terminate == i)
							{
								if (connection_socket[i] > 0)
								{
									socket_to_terminate = connection_socket[id_to_terminate];
									
									
									getpeername(socket_to_terminate , (struct sockaddr*)&address , (socklen_t*)&addrlen);
									printf("Termination Successful: ID: %d\n\tIP: %s \n\tPORT: %d \n\n", id_to_terminate, inet_ntoa(address.sin_addr), ntohs(address.sin_port)); //shows user what socket they are terminating
									
									//terminate socket
									close(socket_to_terminate);
									connection_socket[id_to_terminate] = 0;
									is_valid_connection = TRUE;
								}
							}
						}
						// not a valid conenction to terminate
						if (!(is_valid_connection))
                        {
							printf("Not able to terminate connection: The connection you are trying to terminate does not exist.\n\n");
						}
						break;
					}
					else if ((strcmp(token, "send")) == 0)
                    { 
						//format input
						char* id_temp = strtok(NULL, " ");
						int id = atoi(id_temp);  // id of connection
						char* input = strtok(NULL, " ");
						char messageArray[256] = "";  // holds concat message
						int counter = 0;
						while(input != NULL)
                        { // looop through message input and concat to string
							strcat(messageArray, input);
							strcat(messageArray, " ");
							input = strtok(NULL, " ");
						}
						int isValidConn = FALSE; // if the connection exists
						for (i = 0; i < max_clients; i++)   
						{
							if (id == i)
							{
								if (connection_socket[i] > 0)
								{
									int socket_test = connection_socket[id];
									send(socket_test , messageArray, strlen(messageArray) , 0);
									memset(messageArray, 0, sizeof(messageArray));
									printf("Message sent to %d\n\n", id);
									isValidConn = TRUE;
								}
							}
						}
						//if not a valid connection, let user know
						if (!(isValidConn))
						{
							printf("Not a valid connection to send to\n\n");
						}
						break;
					}
					else if ((strcmp(token, "exit\n")) == 0)
					{
						printf("The application is closing now...\n");
						
						//closing all sockets connected
						for (i = 0; i < max_clients; i++)   
						{   
							if( connection_socket[i] > 0 )   
							{  
								sd = connection_socket[i];
								close(sd);
							}   
						}
						
						exit(0);
					}
					else
					{
						printf("incorrect input\n\n");
						break;
					}					
				}	
			}			
		}
		
		
		if (FD_ISSET(master_socket, &readfds)) // activity on servers listening socket
		{
			if ((new_socket = accept(master_socket,  (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            printf("New Connection! Socket fd is %d, IP is : %s, Port Number is : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
        
                for (i = 0; i < max_clients; i++) // adds new socket (client)
                {   
                    //if position is empty  add socket
                    if( connection_socket[i] == 0 )   
                    {   
                        connection_socket[i] = new_socket;   
                        printf("Adding to list with id: %d\n\n" , i); 
                            
                        break;   
                    }   
                }     
		}
		
		
		for (i = 0; i < max_clients; i++)   
        {
			sd = connection_socket[i];
			if (FD_ISSET( sd , &readfds) && (sd > 0))
			{
				char buffer_recv[256];
				int rval = recv(sd , buffer_recv , sizeof(buffer_recv) , 0 ); 

				if (rval > 0)  //new messafe
				{
					int b_length = strlen(buffer_recv);
			
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					
					printf("Message received from %s\n", inet_ntoa(address.sin_addr));
					printf("Sender's Port: %d\n", ntohs(address.sin_port));
					printf("Message: %s \n", buffer_recv);
					memset(buffer_recv, 0, sizeof(buffer_recv));
				}
				else if (rval == 0)  //a disconnection
				{
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);   
                    printf("Host Disconnected: ip %s , port %d \n\n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    close( sd ); // close socket
                    connection_socket[i] = 0; // allow it to be used again
					
				}
				else if (rval < 0)
				{
					printf("Receiving Error.\n\n");
				}
			}	
		}
    }      
    return(0);   
}   
