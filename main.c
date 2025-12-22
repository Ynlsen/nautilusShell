#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>

int main(void) {
	while (1) {
		char *input = NULL;

		if (isatty(STDIN_FILENO)) {
			char *cwd = getcwd(NULL, 0);
			const char *home = getenv("HOME");
			char prompt[PATH_MAX + 13];  // Length of "nautilush:~$ " is 13

			if (home && strncmp(cwd, home, strlen(home)) == 0) {
				snprintf(prompt,sizeof(prompt),"nautilush:~%s$ ", cwd + strlen(home));
			} else {
				snprintf(prompt, sizeof(prompt),"nautilush:%s$ ", cwd);
			}

			free(cwd);

			input = readline(prompt);
			if (!input) {
				break;
			}
			if (*input) {
				add_history(input);
			}
		} else {
			size_t len = 0;
			const ssize_t count = getline(&input, &len, stdin);

			if (count == -1) {
				free(input);
				break;
			}

			if (input[count - 1] == '\n') {
				input[count - 1] = '\0';
			}
		}

		char *copy = strdup(input);
		char *cmd = copy;

		while (isspace((unsigned char)*cmd)) {
			cmd++;
		}

		size_t length = strlen(cmd);
		while (length > 0 && isspace((unsigned char)cmd[length - 1])) {
			cmd[length - 1] = '\0';
			length--;
		}

		if (strcmp(cmd, "exit") == 0) {
			free(copy);
			free(input);
			break;
		}

		if (strncmp(cmd, "cd ", 3) == 0) {
			if (chdir(cmd + 3) != 0) {
				perror("nautilush");
			}
			free(copy);
			free(input);
			continue;
		}

		pid_t pid = fork();
		if (pid == 0) {
			execlp("bash", "bash", "-c", input, NULL);
		} else {
			int status;
			waitpid(pid, &status, 0);
		}

		free(copy);
		free(input);
	}
	return 0;
}