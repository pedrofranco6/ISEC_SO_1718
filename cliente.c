#include "util.h"

int sfifofd, cfifofd;

void terminaCliente(int i){
	char cpid[10];
	printf("Servidor a terminar (interrompido por teclado).\n\n");
	//broadcast para todos users online que o server vai terminar
	close(cfifofd);
	sprintf(cpid, "%d", getpid());
	unlink(cpid);
	exit(EXIT_SUCCESS);
}

int main(){
	int i, j, read_res;
	
	char fifopid[10];
	LOGIN log;
	MENSAGEM resp;

	int nfd;
	fd_set read_fds;
	struct timeval tempo;

	if(signal(SIGINT, terminaCliente) == SIG_ERR){
		perror("Nao foi possivel configurar o sinal SIGINT\n");
		exit(EXIT_FAILURE);
	}
	printf("Sinal SIGINT configurado com sucesso\n");

	log.pid = getpid();
	sprintf(fifopid, "%d", log.pid);
	if(mkfifo(fifopid, 0777) == -1){
		perror("FIFO do cliente deu erro.\n");
		exit(EXIT_FAILURE);
	}
	printf("FIFO do cliente criado.\n");

	sfifofd = open("sfifo", O_WRONLY);
	if(sfifofd == -1){
		perror("O servidor nao esta  correr.\n");
		unlink(fifopid);
		exit(EXIT_FAILURE);
	}
	printf("FIFO do do servidor aberto em WRITE.\n");

	cfifofd = open(fifopid, O_RDWR);
	if(cfifofd == -1){
		perror("Erro a abrir o FIFO do cliente.\n");
		close(sfifofd);
		unlink(fifopid);
		exit(EXIT_FAILURE);
	}
	printf("FIFO o cliente aberto em READ (+WIRTE).\n");

	do{
		printf("Username: ");
		scanf("%s", log.username);
		printf("Password: ");
		scanf("%s", log.password);

		write(sfifofd, &log, sizeof(LOGIN));
		read(cfifofd, &resp, sizeof(MENSAGEM));
	}while(strcmp(resp.msg, "sucesso") == 0);

	while(1){
		tempo.tv_sec = 10;
		tempo.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
		FD_SET(sfifofd, &read_fds);

		nfd = select(sfifofd+1, &read_fds, NULL, NULL, &tempo);

		if(nfd == 0){
			fflush(stdout);
			continue; //deve ter mesmo continue? ou ficar bloqueado?
		}

		if(nfd == -1){
			perror("Erro no select.\n");
			close(cfifofd);
			unlink(fifopid);
			exit(EXIT_FAILURE);
		}

		if(FD_ISSET(0, &read_fds)){
			//ler do teclado
			//jogadas, movimentos, etc.
		}

		if(FD_ISSET(cfifofd, &read_fds)){
			//ler do fifo de cliente
		}
	}
}
