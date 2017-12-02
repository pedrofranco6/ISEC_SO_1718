#include "util.h"

int sfifofd, cfifofd;

void terminaServer(){
	printf("Servidor a terminar (interrompido por teclado).\n\n");
	//broadcast para todos users online que o server vai terminar
	close(sfifofd);
	unlink("sfifo");
	exit(EXIT_SUCCESS);
}

int procuraLogin(char username[], char password[]){
	char user[20], pass[20];
	FILE *fp;

	fp = fopen("logins.txt", "r");

	do{
		fscanf(fp, "%s %s", user, pass);
		if(strcmp(username, user) == 0){
			if(strcmp(password, pass) == 0)
				return 1;
			else
				return 0;
		}
	}while(fgetc(fp) != EOF);
	fprintf(fp, "%s %s\n", username, password);
	return 2;
}

int main(void){
	int lidos, escritos, res, i, j, nfd;
	int pids[NMAXPLAY];
	char c;
	char pid[10], cmd[50], cmdaux[50], username[20], password[20], usernameaux[20], passwordaux[20];
	CELULAS *m;
	FILE *fp_logins;
	fd_set read_fds;
	struct timeval tempo;

	CELULAS array[NLINHAS][NCOLUNAS];
	CELULAS (*array_ptr)[NCOLUNAS] = array;

	int header;
	LOGIN log;
	MENSAGEM msg;
	char cpid[10];

	for(i=0; i<20; pids[i++] = -1);

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

	//nao fica em bloqueante
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
					if(pids[i]!=-1){
						sprintf(pid, "%d", pids[i]);
						//Terminar a parte dos pipes para pedir usernames
						//aos users que estao online
						//s = open(pid, O_RDONLY);
						//read(fd, &p, sizeof(PEDIDO));
						//printf("Pid: %d / Username: %s", p.pid, p.username);
						//close(fd);
						//Vale a pena fazer dicionario? com os users e os pids?
						//Simplifica muito os comandos kick e users
					}
				}
			}else if(strcmp(cmdaux, "kick") == 0){
				for(i=0; i<20; i++){
					if(pids[i]!=-1){
						//sprintf(pid, "%d", pids[i]);
						//fd = open(pid, O_RDONLY);
						//read(fd, &p, sizeof(PEDIDO));
							//if(strcmp(username, p.username) == 0){
								//kick player
								//break;
							//}
						//close(fd);
					}
				}
			}else if(strcmp(cmdaux, "game") == 0){
				printf("Nenhum jogo a decorrer neste momento.");
			}else if(strcmp(cmdaux, "shutdown") == 0 || strcmp(cmdaux, "quit") == 0){
				terminaServer(1);
			}else if(strcmp(cmdaux, "map") == 0){
				char test[20];
				sscanf(cmd, "map %s", test);
//				sprintf(cmdaux, "mapa0.txt");
				printf("A carregar mapa...\n");				
//				m = lerMapa(cmdaux);
				FILE *fp = fopen(cmdaux, "r");;
				char linha[NCOLUNAS];
				while(fgets(linha, sizeof(linha), fp)){
					for(i=0; i<NLINHAS; i++){
						
					}
					printf("%s", linha);
				}				
				/*for(i=0; i<NLINHAS; i++){
				fgets(linha, sizeof(NCOLUNAS), fp);
					for(j=0; j<NCOLUNAS; j++){
						array_ptr[i][j].letra = linha[j];
					}
				}*/
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
			//
			lidos = read(sfifofd, &header, sizeof(int));
			printf("Recebido header tipo %d (%d bytes)\n", header, lidos);
			if(header == 0){
				lidos = read(sfifofd, &log, sizeof(LOGIN));
				printf("Recebido tentativa de Login do pid: %d (%d bytes)\n", log.pid, lidos);
				printf("Tratamento do Login:\nUsername: %s / Password: %s", log.username, log.password);
				sprintf(cpid, "%d", log.pid);
				cfifofd = open(cpid, O_WRONLY);
				sprintf(msg.msg, "Login feito com sucesso.\n");
				escritos = write(cfifofd, &msg, sizeof(MENSAGEM));
			}else{
				//outro tipo de mensagem
			}
/*
			if(procuraLogin(p.username, p.password) == 0){
				sprintf(p.cmd, "Falha no login, password errada.");
				write(sfifofd, &p, sizeof(PEDIDO));
			}else if(procuraLogin(p.username, p.password) == 1){
				i = 0;
				do{
					if(p.pid == pids[i]){
						i = -1;
						break;
					}
					i++;
				}while(pids[i] != -1);
				if(i == -1){
					sprintf(p.cmd, "A conta ja se encontra logada.");
					write(cfifofd, &p, sizeof(PEDIDO));
				}else{
					sprintf(p.cmd, "sucesso");
					sprintf(p.msg, "Login efetuado com sucesso!");
					write(cfifofd, &p, sizeof(PEDIDO));
				}
			}else if(procuraLogin(p.username, p.password) == 2){
				sprintf(p.cmd, "sucesso");
				sprintf(p.msg, "Registo efetuado com sucesso.");
				write(cfifofd, &p, sizeof(PEDIDO));
			}
*/
		close(cfifofd);
		}	
	}
}
