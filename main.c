#include <stdio.h>
#include <stdlib.h>

int main(void) {
	char *line = NULL;
	size_t len = 0;

	while (1) {
		getline(&line, &len, stdin);
		printf(line);
	}

	free(line);
	return 0;
}