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

			if (cwd != NULL) {
				if (home && strncmp(cwd, home, strlen(home)) == 0) {
					snprintf(prompt,sizeof(prompt),"nautilush:~%s$ ", cwd + strlen(home));
				} else {
					snprintf(prompt, sizeof(prompt),"nautilush:%s$ ", cwd);
				}
			} else {
				snprintf(prompt, sizeof(prompt), "nautilush:$ ");
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

		char *cmd = input;

		while (isspace((unsigned char)*cmd)) {
			cmd++;
		}

		size_t length = strlen(cmd);
		while (length > 0 && isspace((unsigned char)cmd[length - 1])) {
			cmd[length - 1] = '\0';
			length--;
		}

		if (*cmd == '\0') {
			free(input);
			continue;
		}

		if (strcmp(cmd, "exit") == 0) {
			free(input);
			break;
		}

		if (strncmp(cmd, "cd ", 3) == 0) {
			cmd += 3;
			while (isspace((unsigned char)*cmd)) {
				cmd++;
			}
			if (chdir(cmd) != 0) {
				perror("nautilush");
			}
			free(input);
			continue;
		}

		if (strcmp(cmd, "cd") == 0) {
			const char *home = getenv("HOME");
			if (home && chdir(home) != 0) {
				perror("nautilush");
			}
			free(input);
			continue;
		}

		pid_t pid = fork();
		if (pid == 0) {
			execlp("bash", "bash", "-c", input, NULL);
			perror("nautilush: exec failed");
			_exit(EXIT_FAILURE);
		} else {
			int status;
			waitpid(pid, &status, 0);
		}

		free(input);
	}
	return EXIT_SUCCESS;
}