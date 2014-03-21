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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>

#include "header.h"
#include "struct.h"

int main(int argc, char *argv[]){
	struct sockaddr_in recv_addr, RX_addr;
	char addr_RX[100];
	int socketfd;
	Mex m;		
	int q, i, tmp_pid;
	int port_RX;
	
	
	strncpy(addr_RX, argv[1], 99);		/*indirizzo tunnelRX*/
	port_RX  = atoi(argv[2]);		/*porta tunnelRX*/
	
		for( i = 0; i < 10; i++){	/*creo tanti figli quanti num*/
		tmp_pid = fork();	/*da qui in poi ci sono num figli*/
		if(tmp_pid == -1){
			exit(-1);
		}else if(tmp_pid == 0){		/*questo é il figlio perche ha id == 0*/
			
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
			RX_addr.sin_addr.s_addr =	inet_addr(addr_RX);
			RX_addr.sin_port		 =	htons(port_RX);

			/*Connessione a TunnelRx*/
			connect(socketfd, (struct sockaddr*) &RX_addr, sizeof(RX_addr));
			fflush(stdout);

			/* Mando il mio numero di ricevitore*/
			printf("numero %d\n", host_list[i]);
			send(socketfd,&host_list[i], 1,0);

			/*
			while(readn(socketfd,&m,sizeof(m)) > 0) {
				printf("da = %c - a = %c - size= %d\n", m.sender,m.recv,m.size);
				sleep(1);
			}*/
					
			
			while ( 1 ){
				q = readn(socketfd,&m,sizeof(m));
				printf("q=%d\n", q);
				if (q == 0)
					exit(0);
					
				if(q != sizeof(m)) {
					printf("non funziona\n");
				}
				
				printf("ho letto = %d - da = %c - a = %c - size= %d body = %s\n", q,m.sender,m.recv,m.size,m.body);
				sleep(1);
			}
			exit(0);
		}

	}

	while(errno!=ECHILD)
		wait(0);
	return 0;
}
