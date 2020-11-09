#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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