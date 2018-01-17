#include "util.h"
#include <pthread.h>

int sfifofd, cfifofd;
JOGADOR jogdrs[NMAXPLAY];
INIMIGO inimigos[NENEMY];
BOMBA bombas[200];
CELULAS array[NLINHAS][NCOLUNAS];
CELULAS (*array_ptr)[NCOLUNAS] = array;

void terminaServer(){
	int i, escritos;
	char cpid[10];
	Tmsg mensagem;
	printf("\nServidor a terminar (interrompido por teclado).\n\n");
	//broadcast para todos users online que o server vai terminar
	mensagem.tipo = 1;
	sprintf(mensagem.msg.texto, "shutdown");

	for(i=0; i<NMAXPLAY; i++){
		if(jogdrs[i].pid != -1){
			sprintf(cpid, "%d", jogdrs[i].pid);
			cfifofd = open(cpid, O_WRONLY);
			escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
			close(cfifofd);
		}
	}
	close(sfifofd);
	unlink("sfifo");
	exit(EXIT_SUCCESS);
}

int procuraLogin(char username[], char password[]){
	int i;
	char user[20], pass[20], linha[256];
	FILE *fp;

	fp = fopen("logins.txt", "r+");
	fseek(fp, 0, SEEK_SET);

	for(i=0; i<NMAXPLAY; i++){
		if(jogdrs[i].pid != -1){
			if(strcmp(jogdrs[i].username, username) == 0){
				return 3;
			}
		}
	}

	while(fgets(linha, sizeof(linha), fp)){
		sscanf(linha, "%s %s", user, pass);
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

void *botsAndar(void * arg){
	srand(time(NULL));
	INIMIGO * inimigo = (INIMIGO *) arg;
	int random, tru;	

	while(1){
		tru = 0;

		//andar a sorte
		do{
		random = rand() % 4;
		switch(random){
			case 0:
				if(array_ptr[inimigo->posy - 1][inimigo->posx].letra == 32){
					array_ptr[inimigo->posy][inimigo->posx].letra = 32;
					inimigo->posy = inimigo->posy - 1;
					tru = 1;}
				break;
			case 1:
				if(array_ptr[inimigo->posy + 1][inimigo->posx].letra == 32){
					array_ptr[inimigo->posy][inimigo->posx].letra = 32;
					inimigo->posy = inimigo->posy + 1;
					tru = 1;}
				break;
			case 2:
				if(array_ptr[inimigo->posy][inimigo->posx - 1].letra == 32){
					array_ptr[inimigo->posy][inimigo->posx].letra = 32;
					inimigo->posx = inimigo->posx - 1;
					tru = 1;}
				break;
			case 3:
				if(array_ptr[inimigo->posy][inimigo->posx + 1].letra == 32){
					array_ptr[inimigo->posy][inimigo->posx].letra = 32;
					inimigo->posx = inimigo->posx + 1;
					tru = 1;}
				break;
		}
		}while(tru != 1);

		array_ptr[inimigo->posy][inimigo->posx].letra = inimigo->letra;
		sleep(1);

		if(inimigo->continua == 0)
			break;
	}
}

void *explodeBomba(void * arg){
	int i, j;	
	BOMBA * bomba = (BOMBA *) arg;
	for(i=0; i<10; i++){
		if(bomba->tipo == 1)
			array_ptr[bomba->posy][bomba->posx].letra = 'b';
		else if(bomba->tipo == 2)
			array_ptr[bomba->posy][bomba->posx].letra = 'm';
		usleep(200000);
	}
	for(j=0; j<10; j++){
		for(i=-bomba->tipo*2; i<bomba->tipo*2+1; i++){
			if(array_ptr[bomba->posy+i][bomba->posx].letra != '+' && array_ptr[bomba->posy+i][bomba->posx].letra != '|' && array_ptr[bomba->posy+i][bomba->posx].letra != '-'){
				if((array_ptr[bomba->posy+1][bomba->posx].letra != '+' && i<0) || (array_ptr[bomba->posy-1][bomba->posx].letra != '+' && i>0))
					array_ptr[bomba->posy+i][bomba->posx].letra = 'X';
			}
			if(array_ptr[bomba->posy][bomba->posx+i].letra != '+' && array_ptr[bomba->posy][bomba->posx+i].letra != '|' && array_ptr[bomba->posy][bomba->posx+i].letra != '-'){
				if((array_ptr[bomba->posy][bomba->posx+1].letra != '+' && i<0) || (array_ptr[bomba->posy][bomba->posx-1].letra != '+' && i>0))
					array_ptr[bomba->posy][bomba->posx+i].letra = 'X';
			}
		}
		if(bomba->tipo == 1)
			array_ptr[bomba->posy][bomba->posx].letra = 'b';
		else if(bomba->tipo == 2)
			array_ptr[bomba->posy][bomba->posx].letra = 'm';
		usleep(200000);
	}
	for(i=-bomba->tipo*2; i<bomba->tipo*2+1; i++){
		if(array_ptr[bomba->posy+i][bomba->posx].letra != '+' && array_ptr[bomba->posy+i][bomba->posx].letra != '|' && array_ptr[bomba->posy+i][bomba->posx].letra != '-'){
			if((array_ptr[bomba->posy+1][bomba->posx].letra != '+' && i<0) || (array_ptr[bomba->posy-1][bomba->posx].letra != '+' && i>0))
				array_ptr[bomba->posy+i][bomba->posx].letra = ' ';
			}
		if(array_ptr[bomba->posy][bomba->posx+i].letra != '+' && array_ptr[bomba->posy][bomba->posx+i].letra != '|' && array_ptr[bomba->posy][bomba->posx+i].letra != '-'){
			if((array_ptr[bomba->posy][bomba->posx+1].letra != '+' && i<0) || (array_ptr[bomba->posy][bomba->posx-1].letra != '+' && i>0))
					array_ptr[bomba->posy][bomba->posx+i].letra = ' ';
		}
	}
	array_ptr[bomba->posy][bomba->posx].letra = ' ';
	bomba->ativa = 0;
}

int posicaoLivre(int posy, int posx){
	int i;

	if(array_ptr[posy][posx].letra == '+' || array_ptr[posy][posx].letra == '-' || array_ptr[posy][posx].letra == '|' || array_ptr[posy][posx].letra == 'H')
		return 0;
	for(i=0; i<20; i++){
		if(jogdrs[i].pid != -1 && jogdrs[i].posx == posx && jogdrs[i].posy == posy)
			return 0;
	}

	return 1;
}

void kickJogador(int i){
	int escritos;
	char cpid[10];
	Tmsg mensagem;

	sprintf(cpid, "%d", jogdrs[i].pid);
	mensagem.tipo = 1;
	sprintf(mensagem.msg.texto, "kick");
	cfifofd = open(cpid, O_WRONLY);
	escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
	printf("Mensagem de kick enviada (%d bytes)\n", escritos);
	jogdrs[i].pid = -1;
	close(cfifofd);
}

int main(void){
	int lidos, escritos, res, i, j, nfd, pid, posx, posy;
	char cmd[50], cmdaux[50], username[20], password[20], usernameaux[20], passwordaux[20], primap[10] = "mapa0.txt", aux[2];
	FILE *fp_logins;
	fd_set read_fds;
	struct timeval tempo;

	int header, bombs=-1;
	LOGIN log;
	Tmsg mensagem;
	MENSAGEM msg;
	JOGADA jogd;
	char cpid[10];

	for(i=0; i<20; jogdrs[i++].pid = -1);
	for(i=0; i<200; bombas[i++].ativa = 0);

	if(signal(SIGINT, terminaServer) == SIG_ERR){
		perror("Nao foi possivel configurar o sinal SIGINT\n");
		exit(EXIT_FAILURE);
	}
	printf("Sinal SIGINT configurado com sucesso\n");

	res = mkfifo("sfifo", 0777);

	if(res == -1){
		perror("mkfifo do FIFO do servidor deu erro\n");
		close(sfifofd);
		exit(EXIT_FAILURE);
	}
	printf("FIFO do servidor criado com sucesso.\n");

	sfifofd = open("sfifo", O_RDWR);

	if(sfifofd == -1){
		perror("Erro a abrir o FIFO do servidor (RDWR/bloqueado).\n");
		close(sfifofd);
		unlink("sfifo");
		exit(EXIT_FAILURE);
	}
	printf("FIFO do servidor aberto com sucesso para READ (+WRITE) bloqueante.\n");

	//faz o primeiro mapa do jogo
	srand(time(NULL));
	FILE *fp = fopen(primap, "r");
	char linha[256];
	i = 0;
	while(fgets(linha, sizeof(linha), fp)){
		for(j=0; j<NCOLUNAS; j++)
			array_ptr[i][j].letra = linha[j];
		i++;
	}
	fclose(fp);
	i = ((NLINHAS-2) * (NCOLUNAS-2) - 171) * 0.2;
	//i = 0;
	while(i > 0){
		do{
			posy = rand() % 19 + 1;
			posx = rand() % 39 + 1;
		}while(array_ptr[posy][posx].letra != 32);
		array_ptr[posy][posx].letra = 'H';
		i--;
	}
	/*i = NOBJECT;
	while(i != 0){
		do{
			posy = rand() % 19 + 1;
			posx = rand() % 39 + 1;
			if(array_ptr[posy][posx].letra == ' ')
				array_ptr[posy][posx].letra = 'O';
		}while(array_ptr[posy][posx].letra == ' ');
		i--;
	}*/
	//fazer bots
	for(i=0; i<NENEMY; i++){
		inimigos[i].continua = 1;
		do{
			posy = rand() % 19 + 1;
			posx = rand() % 39 + 1;
		}while(array_ptr[posy][posx].letra != 32);
		inimigos[i].posx = posx;
		inimigos[i].posy = posy;
		inimigos[i].letra = 87-i;
		res = pthread_create(&inimigos[i].tid, NULL, botsAndar, (void *)&inimigos[i]);
		if(res != 0){
			perror("Erro na criacao da thread\n");
			exit(1);
		}
	}

	while(1){
		tempo.tv_sec = 1;
		tempo.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
		FD_SET(sfifofd, &read_fds);

		//mata jogadores
		for(i=0; i<20; i++){
			if(jogdrs[i].pid != -1){
				if(array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra == 'X' || array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra == 'b' || array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra == 'm')
					kickJogador(i);
			}
		}

		//mata bots
		for(i=0; i<NENEMY; i++){
			if(array_ptr[inimigos[i].posy][inimigos[i].posx].letra == 'X' || array_ptr[inimigos[i].posy][inimigos[i].posx].letra == 'b' || array_ptr[inimigos[i].posy][inimigos[i].posx].letra == 'm')
				inimigos[i].continua = 0;
		}

		//envia mapa para os clientes
		memset(mensagem.msg.texto, 0, sizeof(mensagem.msg.texto));
		for(i=0; i<20; i++){
			if(jogdrs[i].pid != -1){
				array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = i+1+'0';
			}
		}
		for(i=0; i<NLINHAS; i++){
			for(j=0; j<NCOLUNAS; j++){
				sprintf(aux, "%c", array_ptr[i][j].letra);
				strcat(mensagem.msg.texto, aux);
			}
			strcat(mensagem.msg.texto, "\n");
		}
		for(i=0; i<NMAXPLAY; i++){
			if(jogdrs[i].pid != -1){
				sprintf(cpid, "%d", jogdrs[i].pid);				
				mensagem.tipo = 0;
				cfifofd = open(cpid, O_WRONLY);
				escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
				close(cfifofd);
				printf("Enviei mapa para %d.\n", jogdrs[i].pid);
			}
		}

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
				fseek(fp_logins, 0, SEEK_SET);
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
						kickJogador(i);
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
				i = 0;
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
			//header = 2 -> quit cliente
			lidos = read(sfifofd, &header, sizeof(int));
			printf("Recebido header tipo %d (%d bytes)\n", header, lidos);
			if(header == 0){
				lidos = read(sfifofd, &log, sizeof(LOGIN));
				printf("Recebido tentativa de Login do pid: %d (%d bytes)\n", log.pid, lidos);
				printf("Tratamento do Login: Username: %s / Password: %s\n", log.username, log.password); 

				sprintf(cpid, "%d", log.pid);

				res = procuraLogin(log.username, log.password);
				if(res == 1){ //login correto
					sprintf(mensagem.msg.texto, "sucesso");
				}else if(res == 0){ //pass errada
					sprintf(mensagem.msg.texto, "passerr");
				}else if(res == 2){ //registo
					sprintf(mensagem.msg.texto, "registo");
				}else if(res == 3){ //logado
					sprintf(mensagem.msg.texto, "logado");
				}else{
					sprintf(mensagem.msg.texto, "erro");
				}

				mensagem.tipo = 1;
				cfifofd = open(cpid, O_WRONLY);
				escritos = write(cfifofd, &mensagem, sizeof(Tmsg));
				close(cfifofd);

				if(res == 1 || res == 2){
					for(i=0; i<NMAXPLAY; i++){
						if(jogdrs[i].pid == -1){
							jogdrs[i].pid = log.pid;
							sprintf(jogdrs[i].username, "%s", log.username);
							do{
								posy = rand() % 19 + 1;
								posx = rand() % 39 + 1;
							}while(array_ptr[posy][posx].letra != 32);
							jogdrs[i].posx = posx;
							jogdrs[i].posy = posy;
							jogdrs[i].pontos = 0;
							jogdrs[i].atirar = 0;
							jogdrs[i].bombinhas = 3;
							jogdrs[i].megaBombas = 2;
							array_ptr[posy][posx].letra = i+1+'0';
							break;
						}
					}
				}
			}else if(header == 1){ //jogadas, movimentos, bombas
				lidos = read(sfifofd, &jogd, sizeof(JOGADA));
				for(i=0; i<NMAXPLAY; i++){
					if(jogdrs[i].pid == jogd.pid)
						break;
				}
				switch(jogd.jogd){
					case 'w':
						if(posicaoLivre(jogdrs[i].posy - 1, jogdrs[i].posx) == 1){
							array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = ' ';
							jogdrs[i].posy = jogdrs[i].posy - 1;
							array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = i+1+'0';
						}
						break;
					case 's':
						if(posicaoLivre(jogdrs[i].posy + 1, jogdrs[i].posx) == 1){
							array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = ' ';
							jogdrs[i].posy = jogdrs[i].posy + 1;
							array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = i+1+'0';
						}
						break;
					case 'a':
						if(posicaoLivre(jogdrs[i].posy, jogdrs[i].posx - 1) == 1){
							array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = ' ';
							jogdrs[i].posx = jogdrs[i].posx - 1;
							array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = i+1+'0';
						}
						break;
					case 'd':
						if(posicaoLivre(jogdrs[i].posy, jogdrs[i].posx + 1) == 1){
							array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = ' ';
							jogdrs[i].posx = jogdrs[i].posx + 1;
							array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = i+1+'0';
						}
						break;
					case 'b':
						if(jogdrs[i].bombinhas > 0){
							jogdrs[i].bombinhas = jogdrs[i].bombinhas - 1;
							bombs++;
							bombas[bombs].posx = jogdrs[i].posx;
							bombas[bombs].posy = jogdrs[i].posy;
							bombas[bombs].tipo = 1;
							bombas[bombs].ativa = 1;
							pthread_create(&bombas[bombs].tid, NULL, explodeBomba, (void *)&bombas[bombs]);
						}
						break;
					case 'm':
						if(jogdrs[i].megaBombas > 0){
							jogdrs[i].megaBombas = jogdrs[i].megaBombas - 1;
							bombs++;
							bombas[bombs].posx = jogdrs[i].posx;
							bombas[bombs].posy = jogdrs[i].posy;
							bombas[bombs].tipo = 2;
							bombas[bombs].ativa = 1;
							pthread_create(&bombas[bombs].tid, NULL, explodeBomba, (void *)&bombas[bombs]);
						}
						break;
					case 'i':
						if(jogdrs[i].bombinhas > 0 && jogdrs[i].atirar == 1 && array_ptr[jogdrs[i].posy-2][jogdrs[i].posx].letra == 32){
							jogdrs[i].bombinhas = jogdrs[i].bombinhas - 1;
							bombs++;
							bombas[bombs].posx = jogdrs[i].posx;
							bombas[bombs].posy = jogdrs[i].posy - 2;
							bombas[bombs].tipo = 1;
							bombas[bombs].ativa = 1;
							pthread_create(&bombas[bombs].tid, NULL, explodeBomba, (void *)&bombas[bombs]);
						}
						break;
					case 'k':
						if(jogdrs[i].bombinhas > 0 && jogdrs[i].atirar == 1 && array_ptr[jogdrs[i].posy+2][jogdrs[i].posx].letra == 32){
							jogdrs[i].bombinhas = jogdrs[i].bombinhas - 1;
							bombs++;
							bombas[bombs].posx = jogdrs[i].posx;
							bombas[bombs].posy = jogdrs[i].posy + 2;
							bombas[bombs].tipo = 1;
							bombas[bombs].ativa = 1;
							pthread_create(&bombas[bombs].tid, NULL, explodeBomba, (void *)&bombas[bombs]);
						}
						break;
					case 'j':
						if(jogdrs[i].bombinhas > 0 && jogdrs[i].atirar == 1 && array_ptr[jogdrs[i].posy][jogdrs[i].posx-2].letra == 32){
							jogdrs[i].bombinhas = jogdrs[i].bombinhas - 1;
							bombs++;
							bombas[bombs].posx = jogdrs[i].posx - 2;
							bombas[bombs].posy = jogdrs[i].posy;
							bombas[bombs].tipo = 1;
							bombas[bombs].ativa = 1;
							pthread_create(&bombas[bombs].tid, NULL, explodeBomba, (void *)&bombas[bombs]);
						}
						break;
					case 'l':
						if(jogdrs[i].bombinhas > 0 && jogdrs[i].atirar == 1 && array_ptr[jogdrs[i].posy][jogdrs[i].posx+2].letra == 32){
							jogdrs[i].bombinhas = jogdrs[i].bombinhas - 1;
							bombs++;
							bombas[bombs].posx = jogdrs[i].posx + 2;
							bombas[bombs].posy = jogdrs[i].posy;
							bombas[bombs].tipo = 1;
							bombas[bombs].ativa = 1;
							pthread_create(&bombas[bombs].tid, NULL, explodeBomba, (void *)&bombas[bombs]);
						}
						break;
				}
			}else if(header == 2){ //quits dos clientes
				lidos = read(sfifofd, &msg, sizeof(MENSAGEM));
				printf("Recebido quit do cliente com pid: %s (%d bytes)\n", msg.texto, lidos);
				for(i=0; i<NMAXPLAY; i++){
					if(jogdrs[i].pid == atoi(msg.texto)){
						array_ptr[jogdrs[i].posy][jogdrs[i].posx].letra = ' ';
						jogdrs[i].pid = -1;
						break;
					}
				}
			}
		}
	}
	//termina threads
	for(i=0; i<NENEMY; i++)
		inimigos[i].continua = 0;
	for(i=0; i<NENEMY; i++)
		pthread_join(inimigos[i].tid, &inimigos[i].retval);
	for(i=0; i<bombs; i++)
		pthread_join(bombas[i].tid, &bombas[i].retval);
}
