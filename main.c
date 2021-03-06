#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_TAM 512
#define STDOUT_FILENO 1 

///////////////////////////////////////////////////////////////////////////////////////////////
// Nesta função foi tratado todas as possiveis ocorrencias de comando, sendo:                //
// 1 - encerramento do programa utilizando o comando 'quit';                                 //
// 2 - uma entrada nula no programa;                                                         //
// 3 - trata-se que o comando sempre o primeiro e o ultimo;                                  //
// 4 - o comando nao é o primeiro mas é o ultimo;                                            //
// 5 - esta ocorrencia tratada foi o primeiro comando caso tenha outros separados por ','.   //
//////////////////////////////////////////////////////////////////////////////////////////////

void verificarComando(char* args[], char* ProxComando, int OutroComando, int* fd){

  if(strcmp(args[0], "quit") == 0)
	  	exit(0);

	if(args[0] == NULL)
    return;

  if(OutroComando == 1 && ProxComando == NULL){ 
		pid_t pid = fork(); //processo filho é criado.
		if (pid < 0)
			perror("fork errado");
		if (pid == 0) { //processo filho executa o bloco.	
			if (execvp(args[0], args) == -1) {
				perror("erro no comando");
        exit(1);
			}
		}	
		//processo pai em espera.
		if (wait(0) == -1) {
			perror("erro no wait 1");
		}

	} else if(OutroComando == 0 && ProxComando == NULL){ 
		pid_t pid = fork(); //processo filho é criado.
		if (pid < 0)
			perror("erro no fork\n");
		if (pid == 0) { //processo filho executa o bloco.	
			dup2(fd[0], STDIN_FILENO);	
			close(fd[0]);
			close(fd[1]);

			if (execvp(args[0], args) == -1) {
				perror("erro no comando\n");
				exit(2);
			}
		}
		//pipe do programa principal é encerrado.
		close(fd[0]);
		close(fd[1]);

		if (wait(0) == -1)
			perror("erro no wait\n");

	} else if(OutroComando == 1 && ProxComando != NULL){ 
		pid_t pid = fork(); //processo filho é criado.
		if (pid < 0)
			perror("erro no fork\n");
		if(pid == 0){ //bloco executado pelo filho.		
			dup2(fd[1], STDOUT_FILENO);
			close(fd[0]);			
			close(fd[1]);

			if(execvp(args[0], args) == -1){
				perror("erro no comando\n");
				exit(3);
			}
		}

		if (wait(0) == -1)
			perror("erro no wait\n");
	}
}


void tratarEntrada(char *str){
	int pos=0; // pos do ultimo args adicionado ao vetor de strings(char **args)
	char **args;
  int OutroComando=1;
	char *tokenVir = strtok(str, ",");
  int fd[2]; //comunica os processos (files descriptors)

	if (pipe(fd) < 0) {
		perror("erro no pipe\n");
		return ;
	}
  do{
    char *tokenEsp = strtok(tokenVir, " ");     
	args = malloc(sizeof(char*)*MAX_TAM);
    do{         
			args[pos] = malloc(sizeof(char)*MAX_TAM);
			args[pos] = tokenEsp;			
			pos++;
			tokenEsp = strtok(NULL, " ");
		args[pos] = NULL; 
		tokenVir = strtok(NULL, ","); 
		verificarComando(args, tokenVir, OutroComando, fd);       
		free(args);
		pos = 0;
		if (OutroComando == 1)
			OutroComando = 0;
    }while (tokenEsp != NULL);  
  
  }while (tokenVir != NULL);
}

int main( ){

	int len = 0;
	char entrada[MAX_TAM];

	do{
		printf("\e[93m");
		printf("Meu.Shell> ");		
		printf("\e[32m");
		fgets(entrada, sizeof(entrada), stdin);
		len = strlen(entrada);
		
		if (len > 0 && entrada[len-1] == '\n') 
      		  entrada[len-1] = '\0';  
		fflush(stdin);
		tratarEntrada(entrada);
	}while(1);

	return 0;
}
