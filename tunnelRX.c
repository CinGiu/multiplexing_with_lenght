/********************************************************************************
 * 
 * tunnelRX.c 
 * gcc -o tunnelRX tunnelRX.c && ./tunnelRX <port tunnelTX> <port ric>
 * 
 * *****************************************************************************/

#include <stdio.h>
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "header.h" 
#include "struct.h"

int main( int argc, char *argv[] ){
	Mex m;
	Recv r[SENDER_NUMB];
	struct sockaddr_in rcv_addr, TX_addr;    
    int sock_TX, sock_rcv_main, newsockfd_rcv,newsockfd_TX;
    int port_TX, port_rcv;
    int i,q,write_result, tot_mex;
    char temp_recv;
    int opt = 1;
	socklen_t clilen; 
	clilen = sizeof(TX_addr);
	
	port_TX = atoi(argv[1]);
	port_rcv = atoi(argv[2]);
	
    /* ----------------Connessione a Tunnel_TX ------------------*/
    sock_TX = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_TX < 0){
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initializzo strutture socket */
    setsockopt(sock_TX,SOL_SOCKET,SO_REUSEADDR,(const char *)&opt,sizeof(int));
    TX_addr.sin_family = AF_INET;
    TX_addr.sin_addr.s_addr = INADDR_ANY;
    TX_addr.sin_port = htons(port_TX);
    
    /* Bind per TX.*/
    if (bind(sock_TX, (struct sockaddr *) &TX_addr, sizeof(TX_addr)) < 0){
         perror("ERROR on binding");
         exit(1);
    }
    fflush(stdout);
    
    /* Ascolto per la connessione da TX */
    listen(sock_TX,10); 
    
    
    /*-----------------Connessione ai RICEVITORI-------------*/
    sock_rcv_main = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_rcv_main < 0){
        perror("ERROR opening socket");
        exit(1);
    }
    
    /* Initializzo strutture socket */
    setsockopt(sock_rcv_main,SOL_SOCKET,SO_REUSEADDR,(const char *)&opt,sizeof(int));
    rcv_addr.sin_family = AF_INET;
    rcv_addr.sin_addr.s_addr = INADDR_ANY;
    rcv_addr.sin_port = htons(port_rcv);
    
    /* Bind per i ricevitori.*/
    if (bind(sock_rcv_main, (struct sockaddr *) &rcv_addr, sizeof(rcv_addr)) < 0){
         perror("ERROR on binding");
         exit(1);
    }
    fflush(stdout);
    
    /* Ascolto per i Ricevitori */
	listen(sock_rcv_main,10);
	
	/*-----------------Connetto ogni ricevitore e salvo il socket----------------------*/
	printf("Aspetto i ricevitori\n");
    for(q = 0; q<SENDER_NUMB; q++){
		newsockfd_rcv = accept(sock_rcv_main,(struct sockaddr *) &rcv_addr, &clilen);
		read(newsockfd_rcv, &temp_recv, sizeof(temp_recv));
		
		r[q].numb =  temp_recv;	
		r[q].socket =  newsockfd_rcv;	
		printf("numero ric = %c ---> numero socket = %d\n", r[q].numb,r[q].socket);
	}
	
	/* Accept per TX */
	newsockfd_TX = accept(sock_TX,(struct sockaddr *) &TX_addr, &clilen);
	if (newsockfd_TX < 0){
            perror("ERROR on accept");
            exit(1);
    }
	tot_mex = 0;
    /* Inizio ascolto da TX e passo ai ricevitori */
    while( 1 ) {
		/*	Leggo l'Header	*/
		q = readn(newsockfd_TX,&m,size_header);
		if(q == 0){
			/*Fine, non ci sono piu pacchetti da leggere*/
			printf("totale messaggi: %d\n",tot_mex); 
			exit(0);
		}
		
		if (q < 0){
			perror("ERROR1 reading from socket");
			exit(0);
		}
		/*	Leggo il Body	*/
		q = readn(newsockfd_TX,&m.body,m.size);
		if (q < 0){
			perror("ERROR2 reading from socket");
			exit(1);
		}
		
		
		for (i=0;i<SENDER_NUMB;i++){
			if(m.recv == r[i].numb){
				/* mando al ricevitore giusto*/
				printf("da = %c - a = %c %c %d - size = %d \n", m.sender,m.recv, r[i].numb, r[i].socket,m.size);
				write_result = sendn(r[i].socket,&m, size_header + m.size);
				if(write_result < 0){
					perror("errore in scrittura");
				}
			}
		}
		tot_mex++;		 
		/*close(sock_rcv[m.recv]);*/
	}
    return 0;
}
