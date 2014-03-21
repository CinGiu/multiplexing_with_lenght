/********************************************************************************
 * 
 * tunnelTX.c 
 * gcc -o tunnelTX tunnelTX.c && ./tunnelTX <address tunnelRX>  <port mitt> <port tunnelRX>
 * 
 * *****************************************************************************/
 
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "header.h"
#include "struct.h"

void usage(void){printf ("usage: ./tunnelTX RX_IP PORT_TO_RX PORT_FROM_MITT\n"); exit(1);}

int main( int argc, char *argv[] ){
	Mex m;
	int q,i, mex_counter = 0;
	struct sockaddr_in addr_Tx, addr_mit; 	
	struct sockaddr_in serv_addr, RX_addr;
    int sock_RX, sock_mit, newsockfd[NUM_HOST], port_RX, port_mit;
    socklen_t clilen;
    char address_RX[100];
    int OptVal = 1;    
    
    fd_set fset_mitt;	/* Set file descriptor mittenti per select*/
    fd_set fset_RX;	/* Set file descriptor TunnelRX per select*/
    FD_ZERO(&fset_mitt);
    FD_ZERO(&fset_RX);
    
    if(argc < 4){usage();}

    strncpy(address_RX, argv[1], 99);		/*indirizzo tunnelRX*/
    port_mit = atoi(argv[2]);				/*porta mittenti*/
	port_RX = atoi(argv[3]);				/*porta tunnelRX*/
	printf("*****************************\n* INDIRIZZO %s \n* PORTA ASCOLTO MITTENTI %d\n* PORTA INVIO A RX %d\n*****************************\n", address_RX, port_mit, port_RX);
	

	/* ---------------Connessione a Tunnel_RX--------------------*/
    sock_RX = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_RX < 0){
        perror("ERROR opening socket");
        exit(1);
    }
    /* Collegamento locale */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(0);
    	
	/*Collegamento all'indirizzo remoto*/
	setsockopt (sock_RX, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal) );
	memset ( &RX_addr, 0, sizeof(RX_addr) );
	RX_addr.sin_family	 =	AF_INET;
	RX_addr.sin_addr.s_addr =	inet_addr(address_RX);
	RX_addr.sin_port		 =	htons(port_RX);
	
    /* Now bind the host address using bind() call.*/
    if (bind(sock_RX, (struct sockaddr *) &serv_addr,
                          sizeof(serv_addr)) < 0){
         perror("ERROR on binding");
         exit(1);
    }
	connect(sock_RX, (struct sockaddr*) &RX_addr, sizeof(RX_addr));
	
	FD_SET(sock_RX, &fset_RX); 	/* Setto il set per il TunnelRX*/
	fflush(stdout);
	
	/* --------------------Connessione ai MITTENTI--------------------------*/
	
	sock_mit = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_mit < 0){
        perror("ERROR opening socket");
        exit(1);
    }
    
    setsockopt (sock_mit, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal) );
    /* Inizializzo strutture dati socket*/
    addr_Tx.sin_family = AF_INET;
    addr_Tx.sin_addr.s_addr = INADDR_ANY;
    addr_Tx.sin_port = htons(port_mit);
    
	/* Bind per i mittenti*/
    if (bind(sock_mit, (struct sockaddr *) &addr_Tx, sizeof(addr_Tx)) < 0){
         perror("ERROR on binding");
         exit(1);
    }
    
    fflush(stdout);
    listen(sock_mit,100);	
    clilen = sizeof(addr_mit);
    
    /* Accetto le connessioni per i mittenti e creo la list per la selct*/
    for (i = 0; i < NUM_HOST; i++){
		newsockfd[i] = accept(sock_mit, (struct sockaddr *) &addr_mit, &clilen);
		if( newsockfd[i] < 0 ){
			perror("errore nella accept");
			exit(1);
		}else{
			
			FD_SET(newsockfd[i], &fset_mitt); 	/* Nuovo Socket in fset per la select*/
		
		}
	}	
    printf("Tutti gli host accettati\n");	
	
	while ( 1 ){
		
		
		
		
		q = readn(newsockfd,&m,size_header);
		if(q == 0){
			printf("Messaggi letti: %d\n", mex_counter);
			exit(0);
		}
		if (q < 0){
			perror("ERROR1 reading from socket");
			exit(0);
		}
		printf("da = %c - a = %c - size = %d \n", m.sender,m.recv,m.size);
		 
		q = readn(newsockfd,&m.body,m.size);
		if( q != m.size)
			printf("errore nella read: letti %d/%d",q,m.size);
		if (q < 0){
			perror("ERROR2 reading from socket");
			exit(1);
		}
		mex_counter ++;
		fflush(stdout);
	}
	close(newsockfd);

	return 0;
}
