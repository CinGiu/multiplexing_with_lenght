/********************************************************************************
 * 
 * Riceventi 
 * gcc -o ric ric.c && ./ric <address tunnelRX> <port tunnelRX>
 * 
 * *****************************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>

#include "header.h"
#include "struct.h"

/*
 * Funzione chiamata dai thread dei ricevitori
 * */
void *ric_creator(void *p){
	Mex m;
	char remote_address[100];
	int socketfd,port_RX,recv;
	int q, mex_tot;
	struct sockaddr_in recv_addr, RX_addr;
	
	/* estrazione dei parametri del thread */
	strcpy(remote_address, ((param *)p)->addr);	/* Indirizzo di TunnelTX*/
	port_RX = ((param *)p)->port;					/* Porta di TunnelTX*/
	recv = ((param *)p)->host;					/* Numero host (per variazione messaggi)*/
	free(p);
	
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	/*Collegamento all'indirizzo e porta locali*/
	memset ( &recv_addr, 0, sizeof(recv_addr) );
	recv_addr.sin_family		=	AF_INET;
	recv_addr.sin_addr.s_addr	=	htonl(INADDR_ANY);         /* INADDR_ANY = indirizzo locale */
	recv_addr.sin_port	=	htons(0);

	bind(socketfd, (struct sockaddr*) &recv_addr, sizeof(recv_addr));

	/*Collegamento all'indirizzo remoto*/
	memset ( &RX_addr, 0, sizeof(RX_addr) );
	RX_addr.sin_family	 =	AF_INET;
	RX_addr.sin_addr.s_addr =	inet_addr(remote_address);	/*	indirizzo TunnelRX	*/
	RX_addr.sin_port		 =	htons(port_RX);				/*	porta TunnelRX		*/

	/*Connessione a TunnelRx*/
	connect(socketfd, (struct sockaddr*) &RX_addr, sizeof(RX_addr));
	fflush(stdout);

	/* Mando il mio numero di ricevitore*/
	printf("numero %c\n", host_list[recv]);
	send(socketfd,&host_list[recv], 1,0);
	mex_tot = 0;
	 while( 1 ) {
		/*	Leggo l'Header	*/
		q = readn(socketfd,&m,size_header);
		if(q == 0){
			/*Fine, non ci sono piu pacchetti da leggere*/
			close(socketfd);
			printf("HOST %d - messaggi totali: %d\n", recv, mex_tot);
			pthread_exit(NULL);		
		}
		
		if (q < 0){
			perror("ERROR1 reading from socket");
			exit(0);
		}
		/*	Leggo il Body	*/
		q = readn(socketfd,&m.body,m.size);
		if (q < 0){
			perror("ERROR2 reading from socket");
			exit(1);
		}
		printf("HOST %d - da = %c - a = %c - size = %d\n", recv, m.sender,m.recv,m.size);
		mex_tot++;
		sleep(1);
		fflush(stdout);		
	}
}

/* 
 * Main Ricevitori
 * */
int main(int argc, char *argv[]){
	pthread_t threadID[SENDER_NUMB];
	int tmp_pid;
	char addr_RX[100];
	int i,t;
	int port_RX;
	strncpy(addr_RX, argv[1], 99);		/*indirizzo tunnelRX*/
	port_RX = atoi(argv[2]);			/*porta tunnelRX*/
	
	for(i = 0; i < SENDER_NUMB; i++){	
		
		param *p;
		p = malloc(sizeof(struct param));
		if(p == NULL) {
			
			printf("Chiamata a malloc() fallita.\n Errore: %d \"%s\"\n", errno,strerror(errno));
			fflush(stdout);
			exit(1);
		
		}else{
			strcpy(p->addr,addr_RX);	/* Indirizzo di TunnelRX*/
			p->port = port_RX;
			p->host = i;
						
			tmp_pid = pthread_create(&threadID[i],NULL,ric_creator,(void*) p);		
			if(tmp_pid != 0) {
				printf("Chiamata a pthread_create() fallita.\n Errore: %d \"%s\"\n", errno,strerror(errno));
				fflush(stdout);
	 			exit(1);
			}	
			p = NULL;
		}
	}
	printf("Generati %d mittenti\n\n", SENDER_NUMB);
	for(t = 0; t < SENDER_NUMB; t++) {
		int error;

		/* attendo la terminazione del thread t-esimo  */
		error=pthread_join( threadID[t] , NULL );
		if(error!=0){
			printf("pthread_join() failed: error=%d\n", error ); 
			exit(-1);
		}
		else {
			/*
			printf("Mittente %d-esimo ID=%d finisce\n", t, (int) threadID[t]);
			*/
		}
	}

	return 0;
}



