#include "util.h"

int sfifofd, cfifofd;
JOGADOR jogdrs[NMAXPLAY];

void terminaServer(){
	int i;
	char cpid[10];
	Tmsg mensagem;
	printf("\nServidor a terminar (interrompido por teclado).\n\n");
	//broadcast para todos users online que o server vai terminar
	for(i=0; i<NMAXPLAY; i++){
		if(jogdrs[i].pid != -1){
			sprintf(cpid, "%d", jogdrs[i].pid);
			mensagem.tipo = 2;
			sprintf(mensagem.msg.texto, "shutdown");
printf("%d %s %s\n", i, cpid, mensagem.msg.texto); //terminar o erro do broadcast para o quit
			cfifofd = open(cpid, O_WRONLY);
			write(cfifofd, &mensagem, sizeof(Tmsg));
			close(cfifofd);
		}
	}
	close(sfifofd);
	unlink("sfifo");
	exit(EXIT_SUCCESS);
}

int procuraLogin(char username[], char password[]){
	char user[20], pass[20];
	FILE *fp;

	fp = fopen("logins.txt", "r+");

	while(fgetc(fp) != EOF){
		fscanf(fp, "%s %s", user, pass);
		if(strcmp(username, user) == 0){
			if(strcmp(password, pass) == 0){
				fclose(fp);
				return 1; //user e pass correta -> login sucesso
			}else{
				fclose(fp);
				return 0; //pass errada
			}
		}
	}
	fprintf(fp, "%s %s\n", username, password);
	fclose(fp);
	return 2; //feito registo
}

int main(void){
	int lidos, escritos, res, i, j, nfd, pid;
	char cmd[50], cmdaux[50], username[20], password[20], usernameaux[20], passwordaux[20];
	CELULAS *m;
	FILE *fp_logins;
	fd_set read_fds;
	struct timeval tempo;

	CELULAS array[NLINHAS][NCOLUNAS];
	CELULAS (*array_ptr)[NCOLUNAS] = array;

	int header;
	LOGIN log;
	Tmsg mensagem;
	MENSAGEM msg;
	char cpid[10];

	for(i=0; i<20; jogdrs[i++].pid = -1);

	if(signal(SIGINT, terminaServer) == SIG_ERR){
		perror("Nao foi possivel configurar o sinal SIGINT\n");
		exit(EXIT_FAILURE);
	}
	printf("Sinal SIGINT configurado com sucesso\n");

	res = mkfifo("sfifo", 0777);

	if(res == -1){
		perror("mkfifo do FIFO do servidor deu erro\n");
		close(sfifofd);
		unlink("sfifo");
		exit(EXIT_FAILURE);
	}
	printf("FIFO do servidor criado com sucesso.\n");

	sfifofd = open("sfifo", O_RDWR);

	if(sfifofd == -1){
		perror("Erro a abrir o FIFO do ervidor (RDWR/bloqueado).\n");
		close(sfifofd);
		unlink("sfifo");
		exit(EXIT_FAILURE);
	}
	printf("FIFO do servidor aberto com sucesso para READ (+WRITE) bloqueante.\n");

	while(1){
		tempo.tv_sec = 10;
		tempo.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
		FD_SET(sfifofd, &read_fds);

		nfd = select(sfifofd+1, &read_fds, NULL, NULL, &tempo);

		if(nfd == 0){
			printf("Estou a espera...\n"); fflush(stdout);
			continue;
		}

		if(nfd == -1){
			perror("Erro no select.\n");
			close(sfifofd);
			unlink("sfifo");
			exit(EXIT_FAILURE);
		}

		//envia mapa para os clientes
		/*for(i=0; i<NMAXPLAY; i++){
			if(jogdrs[i].pid != -1){
				sprintf(cpid, "%d", jogdrs[i].pid);
				
			}
		}*/

		if(FD_ISSET(0, &read_fds)){ //alguma coisa do teclado?
			fgets(cmd, sizeof(cmd), stdin);
			sscanf(cmd, "%s", cmdaux);
			if(strcmp(cmdaux, "add") == 0){
				i=0;
				sscanf(cmd, "add %s %s", username, password);
				fp_logins = fopen("logins.txt", "r+");
				while(!feof(fp_logins)){
					fscanf(fp_logins, "%s %s", usernameaux, passwordaux);
					if(strcmp(username, usernameaux) == 0){
						printf("O utilizador ja existe.\n");
						i=1;
					}
				}
				if(i==0){
					fprintf(fp_logins, "%s %s\n", username, password);		
					printf("Utilizador criado com sucesso.\n");
				}
				fclose(fp_logins);
			}else if(strcmp(cmdaux, "users") == 0){
				printf("Lista de Utilizadores:\n");
				for(i=0; i<20; i++){
					if(jogdrs[i].pid != -1){
						printf("PID: %d / Username: %s\n", jogdrs[i].pid, jogdrs[i].username);
					}
				}
			}else if(strcmp(cmdaux, "kick") == 0){
				sscanf(cmd, "kick %s", username);
				for(i=0; i<20; i++){
					if(strcmp(jogdrs[i].username, username) == 0){
						sprintf(cpid, "%d", jogdrs[i].pid);
						mensagem.tipo = 1;
						sprintf(mensagem.msg.texto, "kick");
						cfifofd = open(cpid, O_WRONLY);
						escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
						printf("Mensagem de kick enviada (%d bytes)\n", escritos);
						jogdrs[i].pid = -1;
						close(cfifofd);
						break;
					}
				}
			}else if(strcmp(cmdaux, "game") == 0){
				printf("Nenhum jogo a decorrer neste momento.");
			}else if(strcmp(cmdaux, "shutdown") == 0 || strcmp(cmdaux, "quit") == 0){
				terminaServer(1);
			}else if(strcmp(cmdaux, "map") == 0){
				//sscanf(cmd, "map %s", cmdaux);
sprintf(cmdaux, "mapa0.txt");
				printf("A carregar mapa... -> %s\n", cmdaux);
				FILE *fp = fopen(cmdaux, "r");
				char linha[256];
				i=0;
				while(fgets(linha, sizeof(linha), fp)){
					for(j=0; j<NCOLUNAS; j++){
						array_ptr[i][j].letra = linha[j];
					}
					i++;
				}
				printf("Mapa carregado com sucesso.\n");
				fclose(fp);
			}else if(strcmp(cmdaux, "mostra") == 0){
				for(i=0; i<NLINHAS; i++){
					for(j=0; j<NCOLUNAS; j++){
						printf("%c", array_ptr[i][j].letra);
					}
					printf("\n");
				}
			}else{
				printf("Comando nao conhecido.\n");
			}
		}

		if(FD_ISSET(sfifofd, &read_fds)){ //alguma coisa no fifo do servidor?
			//falta rever o login nao se sabe o PID nao primeira mensagem do cliente para o servidor
			//header = 0 -> logins
			//header = 1 -> jogadas
			lidos = read(sfifofd, &header, sizeof(int));
			printf("Recebido header tipo %d (%d bytes)\n", header, lidos);
			if(header == 0){ //REVER OS LOGINS AINDA ESTAO COM ERROS
				lidos = read(sfifofd, &log, sizeof(LOGIN));
				printf("Recebido tentativa de Login do pid: %d (%d bytes)\n", log.pid, lidos);
				printf("Tratamento do Login: Username: %s / Password: %s\n", log.username, log.password); 

				sprintf(cpid, "%d", log.pid);

				for(i=0; i<NMAXPLAY; i++){
					if(jogdrs[i].pid != -1){
						if(strcmp(jogdrs[i].username, log.username) == 0){
							mensagem.tipo = 1;
							sprintf(mensagem.msg.texto, "logado");
							cfifofd = open(cpid, O_WRONLY);
							escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
							close(cfifofd);
						}
					}
				}

				if(strcmp(mensagem.msg.texto, "logado") != 0){
					res = procuraLogin(log.username, log.password);
					if(res == 1){ //login correto
						mensagem.tipo = 1;
						sprintf(mensagem.msg.texto, "sucesso");
						cfifofd = open(cpid, O_WRONLY);
						escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
						close(cfifofd);
					}else if(res == 0){ //pass errada
						mensagem.tipo = 1;
						sprintf(mensagem.msg.texto, "passerr");
						cfifofd = open(cpid, O_WRONLY);
						escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
						close(cfifofd);
					}else if(res == 2){ //registo
						mensagem.tipo = 1;
						sprintf(mensagem.msg.texto, "registo");
						cfifofd = open(cpid, O_WRONLY);
						escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
						close(cfifofd);
					}
				}

				if(strcmp(mensagem.msg.texto, "sucesso") == 0 || strcmp(mensagem.msg.texto, "registo") == 0){
					for(i=0; i<NMAXPLAY; i++){
						if(jogdrs[i].pid == -1){
							jogdrs[i].pid = log.pid;
							sprintf(jogdrs[i].username, "%s", log.username);
							break;
						}
					}
				}
			}else if(header == 1){ //jogadas, movimentos, bombas, etc
				//ainda nao implementado
			}else if(header == 2){ //quits dos clientes
				lidos = read(sfifofd, &msg, sizeof(MENSAGEM));
				printf("Recebido quit do cliente com pid: %s (%d bytes)\n", msg.texto, lidos);
				for(i=0; i<NMAXPLAY; i++){
					if(jogdrs[i].pid == atoi(msg.texto)){
						jogdrs[i].pid = -1;
						break;
					}
				}
			}
		}	
	}
}
