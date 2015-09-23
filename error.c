#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void dieWithError(char * msg) {
	fprintf(stderr, "Error: %s\n", msg);
	exit(1);
}