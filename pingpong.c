#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

static int
crear_fork()
{
	int i = fork();
	if (i < 0) {
		fprintf(stderr, "Error en el fork");
		_exit(-1);
	}
	return i;
}

static void
crear_pipe(int fds[])
{
	int r = pipe(fds);
	if (r < 0) {
		perror("Error en pipe");
		_exit(-1);
	}
}

int
main(void)
{
	srand(time(NULL));
	int fds[2];   // hijo lee, padre escribe
	int fds2[2];  // hijo escribe, padre lee

	crear_pipe(fds);
	crear_pipe(fds2);

	printf("Hola, soy PID <%d>:\n", getpid());
	printf("- primer pipe me devuelve: [%d, %d]\n", fds[0], fds[1]);
	printf("- segundo pipe me devuelve: [%d, %d]\n\n", fds2[0], fds2[1]);

	int receiver = 0;
	int i = crear_fork();

	if (i == 0) {
		close(fds[1]);
		close(fds2[0]);

		if (read(fds[0], &receiver, sizeof(receiver)) < 0) {
			perror("[hijo] error en read");
			close(fds[0]);
			close(fds2[1]);
			_exit(-1);
		}

		printf("Donde fork me devuelve 0:\n");
		printf("- getpid me devuelve: <%d>\n", getpid());
		printf("- getppid me devuelve: <%d>\n", getppid());
		printf("- recibo valor <%d> vía fd=%d\n", receiver, fds[0]);
		close(fds[0]);

		if (write(fds2[1], &receiver, sizeof(receiver)) < 0) {
			perror("[hijo] error en write");
			close(fds2[1]);
			_exit(-1);
		}
		printf("- reenvío valor en fd=%d y termino\n\n", fds2[1]);
		close(fds2[1]);
		wait(NULL);

	} else {
		int msg = rand() % 100;
		close(fds[0]);   // el padre no va a leer del pipe 1
		close(fds2[1]);  // el padre no va a escribir en el pipe 2

		printf("Donde fork me devuelve <%d>:\n", i);
		printf("- getpid me devuelve: <%d>\n", getpid());
		printf("- getppid me devuelve: <%d>\n", getppid());
		printf("- random me devuelve: <%d>\n", msg);
		printf("- envío valor <%d> a través de fd=%d\n\n", msg, fds[1]);

		if (write(fds[1], &msg, sizeof(msg)) < 0) {
			perror("[padre] error en write");
			close(fds[1]);
			close(fds2[0]);
			_exit(-1);
		}
		close(fds[1]);
		wait(NULL);

		receiver = 0;
		if (read(fds2[0], &receiver, sizeof(receiver)) < 0) {
			perror("[padre] error en read");
			close(fds2[0]);
			_exit(-1);
		}

		printf("Hola, de nuevo PID <%d>:\n", getpid());
		printf("- recibí valor <%d> vía fd=%d\n", receiver, fds2[0]);
		close(fds2[0]);
	}

	return 0;
}