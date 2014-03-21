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


/* FUNZIONE PER LEGGERE DA UN SOCKET nbytes*/
int readn (int fd, char *ptr, int nbytes) {

	int nleft,nread,totread=0;

	nleft=nbytes;

	while(nleft > 0) {
			do {
				nread=read(fd,ptr,nleft);
			} while ( (nread<0) && (errno==EINTR));

			if ((nread < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
				char msg[2000];
				sprintf(msg,"readn: errore in lettura [result %d] :",nread);
				perror(msg);
				return(-1);
			}

			else if ((nread < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
				
				nread = 0;
			}

			else if(nread == 0)
					return(totread); /* assumo che quando il server mi ha mandato tutto chiude, e read torna 0 */

			totread += nread;
			nleft -= nread;
			ptr   += nread;
		}

	return(totread);
}

/* FUNZIONE PER SCRIVERE E CONTROLLARE GLI ERRORI EVENTUALI*/
int sendn (int fd, const void *buf, size_t n) {
   
	size_t	nleft;     
	ssize_t  nwritten;  
	char	*ptr;

	ptr = (void *)buf;
	nleft = n;
	while (nleft > 0) {

		if ( (nwritten = send(fd, ptr, nleft, MSG_NOSIGNAL)) < 0) {

			if (errno == EINTR)	nwritten = 0;   /* send() again */

			else if (((errno == EAGAIN) || (errno == EWOULDBLOCK)) ) {
				nwritten = 0;
			}

			else
				return(-1); /* error */
		}

		/* Caso ok o EINTR/EAGAIN/EWOULDBLOCK, avanzo */
		nleft -= nwritten;
		ptr   += nwritten;

	}

	return(n);
}
