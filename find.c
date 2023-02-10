#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MIN_PARAMETROS 2
#define MAX_PARAMETROS 3
#define DIR_ACTUAL "."
#define DIR_PADRE ".."
#define MAX_PATH 100
#define DIVIDER "/"

static bool
es_directorio_valido(char nombre_dir[])
{
	return ((strcmp(nombre_dir, DIR_ACTUAL) != 0) &&
	        (strcmp(nombre_dir, DIR_PADRE) != 0));
}


static void
concat_file(char path[], char filename[], int *long_path, int long_entry)
{
	strcat(path, filename);
	*(long_path) += long_entry;
	printf("%s\n", path);
	path[strlen(path) - long_entry] = '\0';
}


static void
concat_subdir(char path[], char subdir[], int *long_path, int long_entry)
{
	strcat(path, subdir);
	strcat(path, DIVIDER);
	*(long_path) = long_entry + 1;
}


static void
find(char string[],
     DIR *actual_dir,
     char path[],
     int long_path,
     char *(comparar)(const char str1[], const char str2[]))
{
	struct dirent *entry;

	while ((entry = readdir(actual_dir))) {
		int long_entry = strlen(entry->d_name);

		if (comparar(entry->d_name, string)) {
			concat_file(path, entry->d_name, &long_path, long_entry);
		}

		if (es_directorio_valido(entry->d_name)) {
			DIR *next_dir;
			int fd = dirfd(actual_dir);
			if ((fd = openat(fd, entry->d_name, O_DIRECTORY)) > 0) {
				next_dir = fdopendir(fd);

				concat_subdir(path,
				              entry->d_name,
				              &long_path,
				              long_entry);

				find(string, next_dir, path, long_path, comparar);
				path[strlen(path) - long_entry - 1] = '\0';
			}
		}
	}
	path[strlen(path) - long_path] = '\0';
	closedir(actual_dir);
}


int
main(int argc, char *argv[])
{
	if (argc != MIN_PARAMETROS && argc != MAX_PARAMETROS) {
		fprintf(stderr, "cantidad erronea de parametros\n");
		exit(-1);
	}

	char string[MAX_PATH];
	char path[MAX_PATH] = "";

	DIR *dir;
	dir = opendir(".");
	if (!dir) {
		printf("Error: No se puede abrir el directorio\n");
		exit(-1);
	}

	if (argc == MIN_PARAMETROS) {
		strcpy(string, argv[1]);
		find(string, dir, path, 0, strstr);

	} else {
		strcpy(string, argv[2]);
		find(string, dir, path, 0, strcasestr);
	}
	return 0;
}