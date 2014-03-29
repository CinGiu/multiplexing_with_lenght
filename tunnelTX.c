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
#include <sys/ioctl.h>
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
	Segment s[SENDER_NUMB+50];
	int q,i, tot_mex;
	int select_result,write_result, select_nfd;
	struct sockaddr_in addr_Tx, addr_mit;
	struct sockaddr_in serv_addr, RX_addr;
	struct timeval timeout;
    int sock_RX, sock_mit, newsockfd[SENDER_NUMB], port_RX, port_mit;    
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


     printf("Inizio accept per mittenti...\n");
    /* Accetto le connessioni per i mittenti e creo la list per la selct*/
    for (i = 0; i < SENDER_NUMB; i++){
		newsockfd[i] = accept(sock_mit, (struct sockaddr *) &addr_mit, &clilen);
		if( newsockfd[i] < 0 ){
			perror("errore nella accept");
			exit(1);
		}else{
			s[newsockfd[i]].to_read = 0; 		/*	Inizializzo a -1 la lista dei socket letti (nessuno è stato letto ora) */
			printf("socket n° %d accettato\n",i);
		}
	}
    printf("Tutti gli host accettati\n");
	/* Inizializzo la struttura timeout per la select*/
	timeout.tv_sec  = 15;	/*Time Up select -> 60 secondi*/
	timeout.tv_usec = 0;

	tot_mex = 0;
	select_nfd = (SENDER_NUMB * MEX_NUMB) + 1;
	while ( 1 ){

		for (i = 0; i < SENDER_NUMB; i++) {
			if(!(newsockfd[i] == -1)){
				FD_SET(newsockfd[i], &fset_mitt); 	/* Nuovo Socket in fset per la select*/
			}
		}
		
		select_result = select(select_nfd,&fset_mitt,NULL, NULL,&timeout);
		if (select_result == 0) {
			printf("select() timed out....messaggi letti %d\n",tot_mex);
			exit(0);
		}else if (select_result < 0 && errno != EINTR) {
			printf("Error in select(): %s - %d\n", strerror(errno),tot_mex);
			exit(0);
		}else if (select_result > 0) {

				for (i = 0; i < SENDER_NUMB; i++) {
					
					if (FD_ISSET(newsockfd[i], &fset_mitt)) {
						
						int current_fd;
						current_fd 	= newsockfd[i];		/*	socket scelto */
						
						/*
						 * 	INIZIO CODICE PER MULTILPEXER
						 * 	controllo che abbia ancora messaggio da leggere (controllo current_s.to_read):
						 * 		no-> inizio lettura header e creo il primo segment
						 * 		si-> continuo a leggere il body e creo il segment
						 */
						
						if(s[current_fd].to_read == 0){
							/*Prima lettura nuovo messaggio	*/
							q = readn(current_fd,&m,size_header);	/*	Leggo header	*/
							if (q < 0)
								perror("ERROR2 reading from socket");
							if(q == 0){								/*	Fine messaggi per newsockfd[i] 	*/
								FD_CLR(current_fd, &fset_mitt);
								close(current_fd);
								current_fd = -1;
								s[current_fd].to_read = 0;
							}else{	
								/*	inizializzo la struttura segment con i giusti paramentri */
								s[current_fd].recv = m.recv;
								s[current_fd].sender = m.sender;
								s[current_fd].total_size = m.size;
								s[current_fd].to_read = m.size;
							}
						}
						
						/*	Continuo la lettura del body di un messaggio già iniziato	*/
						if(s[current_fd].to_read <= SEGMENT_SIZE){
							/*	il segmento è l'ultimo perche la roba dal leggere rimanete ci sta tutta dentro */
							int to_write;
							
							q = readn(current_fd,&s[current_fd].body,s[current_fd].to_read);
							if (q < 0)
								perror("ERROR2 reading from socket");
							
							to_write = sizeof(s[current_fd]) - SEGMENT_SIZE + s[current_fd].to_read;
							
							write_result = sendn(sock_RX, &s[current_fd], to_write);	
							if(write_result < 0)
								perror("errore in scrittura");	
							/*	Resetto il segment perche ho finito di mandare tutto il mex	*/
							s[current_fd].recv = ' ';
							s[current_fd].sender = ' ';
							s[current_fd].total_size = 0;
							s[current_fd].to_read = 0;
							
						}else{
							/*	segmento in mezzo, scrivo tutto header + SEGMENT_SIZE*/
							write_result = sendn(sock_RX,&s[current_fd], sizeof(s[current_fd]));
							if(write_result < 0)
								perror("errore in scrittura");	
							
							/*	aggiorno il segment */
							s[current_fd].to_read = s[current_fd].to_read - SEGMENT_SIZE;
						}
					}
				}/*For*/
		}/*If*/
	}/*While*/
	return 0;
}
