#include "util.h"
#include <ncurses.h>

int sfifofd, cfifofd;

void terminaCliente(int i){
	char cpid[10];
	Tmsg mensagem;
	printf("\nCliente a terminar.\n\n");
	sfifofd = open("sfifo", O_WRONLY);
	mensagem.tipo = 2;
	sprintf(mensagem.msg.texto, "%d", getpid());
	write(sfifofd, &mensagem, sizeof(Tmsg));
	close(sfifofd);
	close(cfifofd);
	sprintf(cpid, "%d", getpid());
	unlink(cpid);
	endwin();
	exit(EXIT_SUCCESS);
}

int main(){
	int i, j, res, header, lidos, escritos;
	
	char fifopid[10];
	Tlogin login;
	Tjogada jogada;
	MENSAGEM resp;

	int nfd;
	fd_set read_fds;
	struct timeval tempo;

	if(signal(SIGINT, terminaCliente) == SIG_ERR){
		perror("Nao foi possivel configurar o sinal SIGINT\n");
		exit(EXIT_FAILURE);
	}
	printf("Sinal SIGINT configurado com sucesso\n");

	sprintf(fifopid, "%d", getpid());
	res = mkfifo(fifopid, 0777);

	if(res == -1){
		perror("mkfifo do FIFO do cliente deu erro\n");
		close(cfifofd);
		unlink(fifopid);
		exit(EXIT_FAILURE);
	}
	printf("FIFO do servidor criado com sucesso.\n");

	cfifofd = open(fifopid, O_RDWR);

	if(cfifofd == -1){
		perror("Erro a abrir o FIFO do cliente.\n");
		close(cfifofd);
		unlink(fifopid);
		exit(EXIT_FAILURE);
	}
	printf("FIFO o cliente aberto em READ (+WIRTE).\n");

	sfifofd = open("sfifo", O_WRONLY);

	if(sfifofd == -1){
		perror("O servidor nao esta  correr.\n");
		close(cfifofd);
		unlink(fifopid);
		exit(EXIT_FAILURE);
	}
	printf("FIFO do do servidor aberto em WRITE.\n");

	do{
		printf("Username: ");
		scanf("%s", login.log.username);
		printf("Password: ");
		scanf("%s", login.log.password);

		login.tipo = 0;
		login.log.pid = getpid();

		write(sfifofd, &login, sizeof(Tlogin));
		read(cfifofd, &header, sizeof(int));
		if(header == 1){
			read(cfifofd, &resp, sizeof(MENSAGEM));
			if(strcmp(resp.texto, "passerr") == 0){
				printf("Password errada!\n");
			}else if(strcmp(resp.texto, "logado") == 0){
				printf("A conta ja se encontra logada.\n");
			}
		}
	}while(strcmp(resp.texto, "sucesso") != 0 && strcmp(resp.texto, "registo") != 0);
		
	if(strcmp(resp.texto, "sucesso") == 0)
		printf("Login efetuado com sucesso.\n");
	else if(strcmp(resp.texto, "registo") == 0)
		printf("Registo efetuado com sucesso.\n");

	char c;

	initscr();
	clear();
	noecho();
	cbreak();
	curs_set(0);
	refresh();
	
	while(1){
		tempo.tv_sec = 1;
		tempo.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
		FD_SET(cfifofd, &read_fds);

		nfd = select(cfifofd+1, &read_fds, NULL, NULL, &tempo);

		if(nfd == 0){
			refresh();
			continue;
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
			c = getch();
			switch(c){
				case 'w':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'w';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 's':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 's';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 'd':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'd';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 'a':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'a';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 'b':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'b';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 'm':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'm';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 'i':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'i';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 'k':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'k';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 'j':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'j';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
				case 'l':
					jogada.tipo = 1;
					jogada.jog.pid = getpid();
					jogada.jog.jogd = 'l';
					write(sfifofd, &jogada, sizeof(Tjogada));
					break;
			}
		}

		if(FD_ISSET(cfifofd, &read_fds)){
			//header = 0 -> mapa
			//header = 1 -> mensagens
			//ler do fifo de cliente
			lidos = read(cfifofd, &header, sizeof(int));
			if(header == 0){
				//limpar ecra e fazer print interface
				clear();
				lidos = read(cfifofd, &resp, sizeof(MENSAGEM));
				printw("%s", resp.texto);
				refresh();
			}else if(header == 1){
				lidos = read(cfifofd, &resp, sizeof(MENSAGEM));
				if(strcmp(resp.texto, "shutdown") == 0 || strcmp(resp.texto, "kick") == 0){
					if(strcmp(resp.texto, "shutdown") == 0)
						printw("O servidor terminou, o seu cliente vai ser fechado.\n");
					else if(strcmp(resp.texto, "kick") == 0)
						printw("Foste kickado pelo servidor.\n");
					terminaCliente(1);
				}
			}
		}
	}
}
