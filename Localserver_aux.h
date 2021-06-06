#ifndef _local_server_aux_
#define _local_server_aux_

//Aux functions
int createAndBind_UNIX_stream_Socket(char * sock_addr);
int accept_connection_timeout(int * socket_af_stream);
int read_timeout(int * socket_af_stream, void * to_read, int size_to_read);
int createAuthServerSock(char * port_str,char * authaddr_str, int * Authserver_sock, struct sockaddr_in * Authserver_sock_addr);
int AuthServerCom(int request, char * group, char * secret, int Authserver_sock, struct sockaddr_in Authserver_sock_addr);



#endif