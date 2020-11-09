#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Declaração das funções builtin (póprios do shell)
int comandoCd(char **args);
int comandoHelp(char **args);
int comandoQuit(char **args);

// Comandos builtin e suas respectivas funções
char *builtinStrings[] = {
	"cd",
	"help",
	"quit"
};

int (*builtinFuncao[])(char **) = {
	&comandoCd,
	&comandoHelp,
	&comandoQuit
};

int numBuiltins(){
	return sizeof(builtinStrings) / sizeof(char *);
}

// Código das funções builtin
int comandoCd(char **args){
	if(args[1] == NULL){
		fprintf(stderr, "Era esperado um argumento para \"cd\"\n");
	}
	else{
		if(chdir(args[1]) != 0){
			perror("Erro");
		}
	}
	
	return 1;
}

int comandoHelp(char **args){
	int i;
	printf("Digite todos os comandos separados por virgula e aperte Enter.\n");
	printf("Os comandos builtin são:\n");

	for (i = 0; i < numBuiltins(); i++) {
		printf("	%s\n", builtinStrings[i]);
	}

	return 1;
}

int comandoQuit(char **args){
	return 0;
}

// Função roda, resposável por realizar o fork e gerar o novo processo
int roda(char **args){
	pid_t processID, waitProcessID;
	int status;

	processID = fork();
	if(processID == 0){
		// Processo filho
		if (execvp(args[0], args) == -1) {
			perror("Erro execvp");
		}
		exit(EXIT_FAILURE);
	}
	else if(processID < 0){
		perror("Erro no fork");
	}
	else{
		// Processo pai
		do{
			waitProcessID = waitpid(processID, &status, WUNTRACED);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

// Função lerLinha, nela será feita a leitura do que foi digitado pelo usuário
char *lerLinha(void){
	char *linha = NULL;
	ssize_t bufferSize = 0;

	if (getline(&linha, &bufferSize, stdin) == -1){
		if (feof(stdin)){
			exit(EXIT_SUCCESS);  // Chegou ao fim do arquivo
		}
		else{
			perror("lerLinha");
			exit(EXIT_FAILURE);
		}
	}

	return linha;
}

// Função separarLinha, nela a linha lida a partir do usuário será dividida em argumentos
char **separarLinha(char *linha){
	int bufferSize = tokenBufferSize, posicao = 0;
	char **tokens = malloc(bufferSize * sizeof(char*));
	char *token;

	if(!tokens){
		fprintf(stderr, "Erro de alocacao\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(linha, tokenDelimiter);
	while (token != NULL){
		tokens[posicao] = token;
		posicao++;

		if(posicao >= bufferSize){
			bufferSize += tokenBufferSize;
			tokens = realloc(tokens, bufferSize * sizeof(char*));
			if(!tokens){
				fprintf(stderr, "Erro de alocacao\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, tokenDelimiter);
	}
	tokens[posicao] = NULL;
	return tokens;
}

// Função loop, nela estarão as funções de ler a linha digitada pelo usuário, separá-la em argumentos e depois executá-los
void loop(void){
	char *linha;
	char **args;
	int status;

	do{
		printf("> ");
		linha = lerLinha();
		args = separarLinha(linha);
		status = executar(args);

		free(linha);
		free(args);
	}while(status);
}

int main(int argc, char **argv){
	// Inicia o loop que será a base de todo o shell
	loop();

	return EXIT_SUCCESS;
}