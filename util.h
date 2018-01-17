#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>

#define NLINHAS 21
#define NCOLUNAS 41
#define NMAXPLAY 20
#define NOBJECT 7
#define NENEMY 3

typedef struct{
	int pid;
	char username[20];
	char password[20];
}LOGIN;

typedef struct{
	int tipo;
	LOGIN log;
}Tlogin;

typedef struct{
	char texto[1024];
}MENSAGEM;

typedef struct{
	int tipo;
	MENSAGEM msg;
}Tmsg;

typedef struct{
	int pid;
	char jogd;
}JOGADA;

typedef struct{
	int tipo;
	JOGADA jog;
}Tjogada;

typedef struct{
	int pid;
	char username[20];
	int posx, posy;
	int atirar;
	int bombinhas;
	int megaBombas;
	int pontos;
	int vidaExtra;
}JOGADOR;

typedef struct{
	pthread_t tid;	
	int posx, posy;
	int tipo;
	int ativa;
	void *retval;
}BOMBA;

typedef struct{
	pthread_t tid;
	char letra;
	int posx, posy;
	int continua;
	void *retval;
}INIMIGO;

typedef struct{
	char letra;
	int objeto;
	char obj;
}CELULAS;

typedef struct{
	int tipo;
	CELULAS mapa[NLINHAS][NCOLUNAS];
}MAPA;
