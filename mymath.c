#include <stdio.h>
#include <stdlib.h>
#include "mymath.h"

int isPowerOfTwo(unsigned int x) {
	double x2 = (double) x;
	while(x2 > 1)
		x2 /= 2;
	return x2 == 1;
}


double generateRandomDoubleInRange(double min, double max) {
	double scalar = (double) rand() / RAND_MAX;
	return min + scalar * (max - min);
}