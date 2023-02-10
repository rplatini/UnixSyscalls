#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define PARAMETROS 2
#define CANTIDAD_MINIMA 2
#define PRIMER_PRIMO 2

static void
escribir_numero(int fds[], int buffer, int primo)
{
	if (buffer % primo != 0) {
		if (write(fds[1], &buffer, sizeof(int)) < 0) {
			perror("[padre 1] error en write");
			close(fds[1]);
			exit(-1);
		}
	}
}

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
crear_pipe(int fds[])
{
	int r = pipe(fds);
	if (r < 0) {
		perror("Error en pipe");
		exit(-1);
	}
}

static void
buscar_primes(int fds[])
{
	close(fds[1]);
	int primo;
	int bytes_leidos;
	bytes_leidos = read(fds[0], &primo, sizeof(int));

	if (bytes_leidos < 0) {
		perror("error en read");
		close(fds[0]);
		exit(-1);

	} else if (bytes_leidos == 0)
		close(fds[0]);

	else {
		printf("primo %d\n", primo);

		int fds2[2];
		crear_pipe(fds2);

		int i = crear_fork();
		if (i == 0) {
			close(fds[0]);
			close(fds2[1]);
			buscar_primes(fds2);

		} else {
			int value;
			close(fds2[0]);

			while ((bytes_leidos =
			                read(fds[0], &value, sizeof(int))) != 0) {
				if (bytes_leidos < 0) {
					perror("error en read");
					close(fds[0]);
					close(fds2[1]);
					exit(-1);
				}
				escribir_numero(fds2, value, primo);
			}
			close(fds[0]);
			close(fds2[1]);
			wait(NULL);
		}
	}
}

static int
cantidad_numeros(int argc, char *argv[])
{
	if (argc != PARAMETROS) {
		fprintf(stderr, "cantidad erronea de parametros\n");
		exit(-1);
	}
	int n = atoi(argv[1]);
	if (n < CANTIDAD_MINIMA) {
		fprintf(stderr, "numero demasiado bajo\n");
		exit(-1);
	}
	return n;
}

int
main(int argc, char *argv[])
{
	int n = cantidad_numeros(argc, argv);

	int fds[2];
	crear_pipe(fds);

	int i = crear_fork();
	if (i == 0) {
		buscar_primes(fds);

	} else {
		close(fds[0]);
		int primo = PRIMER_PRIMO;
		printf("primo %i\n", primo);

		for (int i = 2; i <= n; i++) {
			escribir_numero(fds, i, primo);
		}

		close(fds[1]);
		wait(NULL);
	}
	return 0;
}