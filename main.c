#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

		if (line[count - 1] == '\n') {
			line[count - 1] = '\0';
		}

		char *copy = strdup(line);
		char *cmd = copy;

		while (isspace(*cmd)) {
			cmd++;
		}

		size_t length = strlen(cmd);
		while (length > 0 && isspace(cmd[length - 1])) {
			cmd[length - 1] = '\0';
			length--;
		}

		if (strcmp(cmd, "exit") == 0) {
			break;
		}

		if (strncmp(cmd, "cd ", 3) == 0) {
			if (chdir(cmd + 3) != 0) {
				perror("nautilush");
				continue;
			}
		}

		free(copy);

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