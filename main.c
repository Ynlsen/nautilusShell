#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include <signal.h>

void sigint_handler(int sig) {
	write(STDOUT_FILENO, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
			execlp("sh", "sh", "-c", argv[i+1], NULL);
			perror("nautilush: exec failed");
			_exit(EXIT_FAILURE);
		}
	}
	signal(SIGINT, sigint_handler);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	while (1) {
		char *input = NULL;
		size_t len = 0;

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

		len = strlen(cmd);
		while (len > 0 && isspace((unsigned char)cmd[len - 1])) {
			cmd[len - 1] = '\0';
			len--;
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
		if (pid == -1) {
			perror("nautilush: fork failed");
		} else if (pid == 0) {
			setpgid(0, 0);
			signal(SIGINT, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			signal(SIGTTOU, SIG_DFL);

			execlp("bash", "bash", "-c", input, NULL);
			perror("nautilush: exec failed");
			_exit(EXIT_FAILURE);
		} else {
			setpgid(pid, pid);

			if (isatty(STDIN_FILENO)) {
				tcsetpgrp(STDIN_FILENO, pid);
			}

			signal(SIGINT, SIG_IGN);

			int status;
			waitpid(pid, &status, WUNTRACED);

			if (isatty(STDIN_FILENO)) {
				tcsetpgrp(STDIN_FILENO, getpgrp());
			}

			signal(SIGINT, sigint_handler);
			if ((WIFSIGNALED(status) && (WTERMSIG(status) == SIGINT || WTERMSIG(status) == SIGQUIT)) || WIFSTOPPED(status)) {
				write(STDOUT_FILENO, "\n", 1);
			}
		}

		free(input);
	}
	return EXIT_SUCCESS;
}