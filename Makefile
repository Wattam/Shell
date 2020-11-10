all: shell.c
	gcc -o main shell.c
	./main
	
clean:
	$(RM) main