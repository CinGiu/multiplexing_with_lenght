#define NUM_HOST 5
#define MEX_NUMB 5
#define MEX_SIZE 200000

#pragma pack(push, 1)
typedef struct mex{
	unsigned char 	recv;
	unsigned char 	sender;
	int 	size;
	char 	body[2000000];
} Mex;
#pragma pack(pop)

typedef struct param{
	int sender;
	int socket;
	struct sockaddr_in Serv;
}param;

/* Lista Host in char*/
char host_list[] = "0123456789";
/* Dimensione Header (6B)*/
int size_header = sizeof(unsigned char)+sizeof(unsigned char)+sizeof(int);
