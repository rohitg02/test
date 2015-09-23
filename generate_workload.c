#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "generate_workload.h"

int generateRandomWorkload() {
	return rand() % 3;
}

int * generateRandomWorkloadArray(int numOfElements) {
	int * workload = (int *)malloc(sizeof(int) * numOfElements);
	int i;
	for(i = 0; i < numOfElements; i++) {
		workload[i] = generateRandomWorkload();
	}
	return workload;
}

void printWorkloadArray(int * workload, int numOfElements) {
	int i;
	for(i = 0; i < numOfElements; i++) {
		printf("%d  ", workload[i]);
	}
	printf("\n");
	return;
}