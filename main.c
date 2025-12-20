#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
	char *line = NULL;
	size_t len = 0;

	while (1) {
		printf("nautilush:%s$ ", getcwd(NULL, 0));
		ssize_t count = getline(&line, &len, stdin);

		if (count == -1) {
			break;
		}

		pid_t pid = fork();
		if (pid == 0) {
			execlp("bash", "bash", "-c", line, NULL);
		} else {
			wait(&pid);
		}
	}

	free(line);
	return 0;
}