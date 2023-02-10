#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef NARGS
#define NARGS 4
#endif

#define PARAMETROS 2
#define MAX_ARGS (NARGS + 2)
#define MAX_LINEA 40

static int
crear_fork()
{
	int i = fork();
	if (i < 0) {
		fprintf(stderr, "Error en el fork");
		exit(-1);
	}
	return i;
}

static void
liberar_argumentos(char *args[], int n)
{
	for (int i = 1; i < n; i++) {
		free(args[i]);
	}
}


static void
ejecutar_comando(char comando[], char *args[])
{
	int i = crear_fork();

	if (i == 0) {
		if (execvp(comando, args) < 0) {
			perror("Error al ejecutar comando");
			exit(-1);
		}
	} else
		wait(NULL);
}


static void
separar_argumentos(FILE *stream, char comando[])
{
	size_t tam = 0;
	char *linea = NULL;

	char *args[MAX_ARGS];
	args[0] = comando;
	int n = 1;


	while (getline(&linea, &tam, stream) != -1) {
		if ((linea[strlen(linea) - 1] = '\n'))
			linea[strlen(linea) - 1] = '\0';

		char *linea_dup;
		if (!(linea_dup = strdup(linea))) {
			perror("Error en dup");
			exit(-1);
		}

		if (n < NARGS + 1) {
			args[n] = linea_dup;

		} else {
			args[n] = NULL;
			ejecutar_comando(comando, args);
			liberar_argumentos(args, n);

			n = 1;
			args[n] = linea_dup;
		}
		n++;
	}
	free(linea);

	// ultima ejecucion
	if (n <= NARGS + 1) {
		args[n] = NULL;
		ejecutar_comando(comando, args);
		liberar_argumentos(args, n);
	}
}

int
main(int argc, char *argv[])
{
	if (argc != PARAMETROS) {
		fprintf(stderr, "cantidad erronea de parametros\n");
		exit(-1);
	}

	char comando[MAX_LINEA];
	strcpy(comando, argv[1]);

	separar_argumentos(stdin, comando);
	return 0;
}