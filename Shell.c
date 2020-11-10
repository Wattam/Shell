#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Declaração das funções builtin (próprias do shell)
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

	// Busca e apresenta ao usuário o nome de todos os comandos builtin
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
		// Caso o fork falhe será gerada uma mensagem para o usuário e ele poderá digitar um novo comando
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

// Função executar, verifica se foi inserido um comando builtin e o executa, caso não chama a função roda()
int executar(char **args){
	int i;

	if(args[0] == NULL){
		// Caso receba um comando vazio
		return 1;
	}
	
	// Busca no vetor de argumentos se existe algum comando builtin
	for (i = 0; i < numBuiltins(); i++) {
		if(strcmp(args[0], builtinStrings[i]) == 0){
			return (*builtinFuncao[i])(args);
		}
	}

	return roda(args);
}

// Função lerLinha, nela será feita a leitura do que foi digitado pelo usuário
char *lerLinha(void){
	char *linha = NULL;
	ssize_t bufferSize = 0;

	if(getline(&linha, &bufferSize, stdin) == -1){
		if(feof(stdin)){
			exit(EXIT_SUCCESS);  // Chegou ao fim do arquivo
		}
		else{
			perror("lerLinha");
			exit(EXIT_FAILURE);
		}
	}

	return linha;
}

// Função separarVirgula(), separa os comandos concatenados por vírgula
#define tokenBufferSize 64
char **separarVirgula(char *linha){
	int bufferSize = tokenBufferSize, posicao = 0;
	char **tokensVirgula = malloc(bufferSize * sizeof(char*));
	char *tokenVirgula;

	if(!tokensVirgula){
		fprintf(stderr, "Erro de alocacao\n");
		exit(EXIT_FAILURE);
	}
	
	tokenVirgula = strtok(linha, ",");
	while(tokenVirgula != NULL){
		tokensVirgula[posicao] = tokenVirgula;
		posicao++;

		if(posicao >= bufferSize){
			bufferSize += tokenBufferSize;
			tokensVirgula = realloc(tokensVirgula, bufferSize * sizeof(char*));
			if(!tokensVirgula){
				fprintf(stderr, "Erro de alocacao\n");
				exit(EXIT_FAILURE);
			}
		}
		
		tokenVirgula = strtok(NULL, ",");
	}
	tokensVirgula[posicao] = NULL;
	
	// Retorna um vetor de argumentos com os comandos após serem separados por vírgulas
	return tokensVirgula;
}

// Função separarLinha, nela a linha lida a partir do usuário será dividida em argumentos
#define tokenBufferSize 64
#define tokenDelimiter " \t\r\n\a" // Define qualquer tipo de espaço como um delimitador para splitar os argumentos
char **separarLinha(char *linha){
	int bufferSize = tokenBufferSize, posicao = 0;
	char **tokens = malloc(bufferSize * sizeof(char*));
	char *token;

	if(!tokens){
		fprintf(stderr, "Erro de alocacao\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(linha, tokenDelimiter);
	while(token != NULL){
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
	
	// Retorna um vetor de argumentos com os comandos já splitados
	return tokens;
}

// Função loop, nela é feita a leitura da entrada inserida pelo usuário, depois é separada de acordo com as vírgulas,
// os argumentos são splitados de acordo com os espaços e finalmente são executados
void loop(void){
	char *linha, *fim;
	char **args;
	char **argsPorVirgula;
	int status = 1, posicao = 0;

	do{
		printf("> ");
		linha = lerLinha();
		argsPorVirgula = separarVirgula(linha);
		
		while(argsPorVirgula[posicao] != NULL){
			
			if(argsPorVirgula[posicao] != NULL){
				args = separarLinha(argsPorVirgula[posicao]);
				status = executar(args);
				
				free(args);
				posicao++;
				
				if(status == 0){
					break;
				}	
			}
		}
		free(argsPorVirgula);
		free(linha);
		posicao = 0;
	}while(status != 0);
}

int main(int argc, char **argv){
	// Inicia o loop que será a base de todo o shell
	loop();

	return EXIT_SUCCESS;
}