#ifndef _local_server_aux_
#define _local_server_aux_

//Aux functions
//Deals with UNIX Stream socket creation and bidding
int createAndBind_UNIX_stream_Socket(char * sock_addr);

//Accepts connections if timeout is not reached
int accept_connection_timeout(int * socket_af_stream);

//Timeout for read functions
int read_timeout(int * socket_af_stream, void * to_read, int size_to_read);

//Timeout for recv functions
int recv_timeout(int * socket_af_stream, void * to_read, int size_to_read);

//Deals with the creation of the Auth server sock. It also assigns the address (IP and port) of the Auth Server to the Authserver_sock_addr
int createAuthServerSock(char * port_str,char * authaddr_str, int * Authserver_sock, struct sockaddr_in * Authserver_sock_addr);

//Deals with the communication between the local server and the Auth server
int AuthServerCom(int request, char * group, char * secret, int Authserver_sock, struct sockaddr_in Authserver_sock_addr);



#endif