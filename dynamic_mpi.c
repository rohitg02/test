#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include "generate_workload.h"
#include "process_workload.h"
#include "error.h"
#include "mymath.h"

#define WORKLOAD_SIZE 1024
#define ROOT_RANK 0
#define JOB_DISTRIBUTED -1
#define TYPE0_JOB 0
#define TYPE1_JOB 1
#define TYPE2_JOB 2

double totalSleepTimeOfType(int type, int numberOfWorkers, MPI_Status * status);
int totalOfType(int type, int numOfWorkers, MPI_Status * status);

int main(int argc, char ** argv) {
	double startTime, stopTime;
	int rank, size, i,j;
	int * workload = NULL;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	startTime = MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	assert(size > 1);
	if(!isPowerOfTwo(size - 1)) {
		dieWithError("Please provide a process count to the power of 2.");
	}

	if(rank == ROOT_RANK) {
		int firstTime = 1;
		int numberOfWorkers = size - 1;
		int latestWorker = -1;
		workload = generateRandomWorkloadArray(WORKLOAD_SIZE);
		for(i = 0; i < WORKLOAD_SIZE; i++) {
			if(firstTime) {
				firstTime = 0;
				for(j = 0; j < numberOfWorkers; j++) {
					MPI_Send(&workload[i++], 1, MPI_INT, j + 1, j + 1, MPI_COMM_WORLD);
				}
			}
			MPI_Recv(&latestWorker, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			MPI_Send(&workload[i], 1, MPI_INT, latestWorker, 1000, MPI_COMM_WORLD);
		}
		int stop = -1;
		for(i = 0; i < numberOfWorkers; i++) {
			MPI_Send(&stop, 1, MPI_INT, i + 1, 1000, MPI_COMM_WORLD);
		}

		double totalSleepTime = 0;
		double tempSleepTime = 0;
		for(i = 0; i < numberOfWorkers; i++) {
			MPI_Recv(&tempSleepTime, 1, MPI_DOUBLE, i + 1, 500, MPI_COMM_WORLD, &status);
			printf("Total sleeptime for worker %d: %f\n", i+1, tempSleepTime);
			totalSleepTime += tempSleepTime;
		}
		printf("Average worker sleeptime: %f\n", totalSleepTime / WORKLOAD_SIZE);
		int numType0 = totalOfType(TYPE0_JOB, numberOfWorkers, &status);
		int numType1 = totalOfType(TYPE1_JOB, numberOfWorkers, &status);
		int numType2 = totalOfType(TYPE2_JOB, numberOfWorkers, &status);
		printf("Total Type 0 workloads: %d\n", numType0);
		printf("Total Type 1 workloads: %d\n", numType1);
		printf("Total Type 2 workloads: %d\n", numType2);
		double type0SleepTime = totalSleepTimeOfType(TYPE0_JOB, numberOfWorkers, &status);
		double type1SleepTime = totalSleepTimeOfType(TYPE1_JOB, numberOfWorkers, &status);
		double type2SleepTime = totalSleepTimeOfType(TYPE2_JOB, numberOfWorkers, &status);
		printf("Average sleep time for type: %d:%f\n", 0, type0SleepTime / numType0);
		printf("Average sleep time for type: %d:%f\n", 1, type1SleepTime / numType1);
		printf("Average sleep time for type: %d:%f\n", 2, type2SleepTime / numType2);
		printf("Total Sleep Time for Type: %d: %f\n", 0, type0SleepTime);
		printf("Total Sleep Time for Type: %d: %f\n", 1, type1SleepTime);
		printf("Total Sleep Time for Type: %d: %f\n", 2, type2SleepTime);

	} else {
		int buffer = -1;
		int numOfType0, numOfType1, numOfType2;
		double type0SleepTime, type1SleepTime, type2SleepTime;
		type0SleepTime = type1SleepTime = type2SleepTime = 0;
		double totalSleepTime = 0;
		numOfType0 = numOfType1 = numOfType2 = 0;
		job_eval_result_t result;
		while(1) {
			MPI_Recv(&buffer, 1, MPI_INT, ROOT_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(buffer == -1) {
				MPI_Send(&totalSleepTime, 1, MPI_DOUBLE, 0, 500, MPI_COMM_WORLD);
				MPI_Send(&numOfType0, 1, MPI_INT, 0, 600, MPI_COMM_WORLD);
				MPI_Send(&numOfType1, 1, MPI_INT, 0, 600, MPI_COMM_WORLD);
				MPI_Send(&numOfType2, 1, MPI_INT, 0, 600, MPI_COMM_WORLD);
				MPI_Send(&type0SleepTime, 1, MPI_DOUBLE, 0, 700, MPI_COMM_WORLD);
				MPI_Send(&type1SleepTime, 1, MPI_DOUBLE, 0, 701, MPI_COMM_WORLD);
				MPI_Send(&type2SleepTime, 1, MPI_DOUBLE, 0, 702, MPI_COMM_WORLD);

				break;
			} else {
				result = processJob(buffer);
				switch(buffer) {
					case 0: 
						numOfType0++;
						type0SleepTime += result.sleepTime;
						break;
					case 1: 
						numOfType1++; 
						type1SleepTime += result.sleepTime;
						break;
					case 2: 
						type2SleepTime += result.sleepTime;
						numOfType2++;
						break;
				}
				totalSleepTime += result.sleepTime;
				MPI_Send(&rank, 1, MPI_INT, ROOT_RANK, rank, MPI_COMM_WORLD);
			}
		}
	}

	stopTime = MPI_Wtime();
	MPI_Finalize();
	if(rank == 0) {
		printf("Total execution time: %f\n", stopTime - startTime);	
	}

	return 0;
}

double totalSleepTimeOfType(int type, int numberOfWorkers, MPI_Status * status) {
	int i;
	double temp, totalSleepTime;
	temp = totalSleepTime = 0;
	for(i = 0; i < numberOfWorkers; i++) {
		MPI_Recv(&temp, 1, MPI_DOUBLE, i + 1, 700 + type, MPI_COMM_WORLD, status);
		totalSleepTime += temp;
	}
	return totalSleepTime;
}

int totalOfType(int type, int numOfWorkers, MPI_Status * status) {
	int i;
	int totalOfType = 0;
	int temp = 0;
	for(i = 0; i < numOfWorkers; i++) {
		MPI_Recv(&temp, 1, MPI_INT, i + 1, 600, MPI_COMM_WORLD, status);
		totalOfType += temp;
	}
	return totalOfType;
}