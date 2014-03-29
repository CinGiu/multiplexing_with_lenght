#define SENDER_NUMB 10
#define MEX_NUMB 10
#define MEX_SIZE 2000000
#define SEGMENT_SIZE 10000

#pragma pack(push, 1)
typedef struct mex{
	unsigned char 	recv;
	unsigned char 	sender;
	int 	size;
	char 	body[2000000];
} Mex;
#pragma pack(pop)

/*	PARAMETRI PER I THREAD */
typedef struct param{
	char addr[100];
	int port;
	int host;
}param;

/*	AUSILIRE AL TUNNELRX PER IDENTIFICA RICEVITORI */
typedef struct recver{
	char numb;
	int socket;
}Recv;

/* 	AUSILIARE A TUNNELTX PER SEGMENTAZIONE MESSAGGI LUNGHI	*/
typedef struct segment{
	char 	recv;
	char 	sender;
	int 	total_size;
	int 	to_read;
	char	body[SEGMENT_SIZE];
}Segment;
/* Lista Host in char*/
char host_list[] = "0123456789";
/* Dimensione Header (6B)*/
int size_header = sizeof(unsigned char)+sizeof(unsigned char)+sizeof(int);
int size_header_s = sizeof(char)+sizeof(char)+sizeof(int)+sizeof(int);
