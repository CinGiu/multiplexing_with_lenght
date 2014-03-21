/********************************************************************************
 * 
 * Mittenti 
 * ./mit <address tunnelTX> <port tunnelTX>
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
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "header.h"
#include "struct.h"

void *mex_creator(void *p){
	Mex m;
	FILE *f;
	int q, write_result;
	struct sockaddr_in Local, Serv;
	char remote_address[100];
	int port;
	int mex_numb,mex_numb_total, mex_size, sender, socketfd;
	int connect_result;	
	srand(time(NULL));
	
	/* estrazione dei parametri del thread */
	strcpy(remote_address, ((param *)p)->addr);	/* Indirizzo di TunnelTX*/
	port = ((param *)p)->port;					/* Porta di TunnelTX*/
	sender = ((param *)p)->sender;					/* Numero host (per variazione messaggi)*/
	free(p);
	
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	/*Collegamento all'indirizzo e porta locali*/
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family		=	AF_INET;
	Local.sin_addr.s_addr	=	htonl(INADDR_ANY);         /* INADDR_ANY = indirizzo locale */
	Local.sin_port	=	htons(0);
	
	bind(socketfd, (struct sockaddr*) &Local, sizeof(Local));

	/*Collegamento all'indirizzo remoto*/
	memset ( &Serv, 0, sizeof(Serv) );
	Serv.sin_family	 =	AF_INET;
	Serv.sin_addr.s_addr =	inet_addr(remote_address);
	Serv.sin_port		 =	htons(port);
	
	mex_numb = rand() % MEX_NUMB; 		
	mex_numb = mex_numb + 1;
	mex_numb_total = mex_numb;
	
	mex_size = rand() % MEX_SIZE;
	fflush(stdout);
	q = rand() % 10;
	
	/*Connect*/
	connect_result = connect(socketfd, (struct sockaddr*) &Serv, sizeof(Serv));
	if (connect_result < 0 ){
		printf("Sender %d errore connessione",sender);
		perror("");
		pthread_exit(NULL);
	}
	printf("Connesso\n");
	
	while(mex_numb != 0){						
		m.sender = host_list[sender];
		m.recv = host_list[q];
		m.size = mex_size; 
		
		/*Creo il body*/
		f=fopen("lorem", "r");  
		fread(m.body, 1, m.size-1, f);
		fclose(f);
		/*
		m.body[m.size-1]='\0';	
		*/
		/*Send*/
		write_result = sendn(socketfd,&m, size_header + m.size);
		if(write_result < 0)
			perror("errore in scrittura");
			
		printf("%d/%d - da = %c - a = %c - size= %d\n",mex_numb,mex_numb_total, m.sender,m.recv,m.size);
		mex_numb--;
		mex_size = rand() % MEX_SIZE;
		q = rand() % 10;
	}	
	pthread_exit(NULL);
}






/*
 * Main Mittenti
 * */ 
int main(int argc, char *argv[]){
	pthread_t threadID[SENDER_NUMB];
	int i,t;
	char remote_address[100];
	int port = atoi(argv[2]);
	int tmp_pid;
	strncpy(remote_address, argv[1], 99);
	
	printf("ip %s\nportaTX %d\n\n",remote_address, port);

	for(i = 0; i < SENDER_NUMB; i++){	
		
		param *p;
		p = malloc(sizeof(struct param));
		if(p == NULL) {
			
			printf("Chiamata a malloc() fallita.\n Errore: %d \"%s\"\n", errno,strerror(errno));
			fflush(stdout);
			exit(1);
		
		}else{
			strcpy(p->addr,remote_address);	/* Indirizzo di TunnelTX*/
			p->port = port;
			p->sender = i;
						
			tmp_pid = pthread_create(&threadID[i],NULL,mex_creator,(void*) p);		
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
