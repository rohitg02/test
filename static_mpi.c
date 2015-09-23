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

// Number of workloads for each type. CHECK
// Average Sleeptime for each type. CHECK
// Total sleeptime for each type. CHECK
// Number of workloads for each worker. CHECK
// Average Sleeptime for each worker. CHECK
// Total sleep time for each worker. CHECK
// Total execution time for entire program. CHECK

int * receiveWorkload(int * buffer, int numOfElements, MPI_Datatype receiveType, int root, int rank, MPI_Comm comm, MPI_Status status);
int ** distributeWorkload(int * workload, int workloadSize, int numOfWorkers, int senderRank);
void divideWorkloadByType(int type, int * workload, int workloadSize, int numOfWorkers, int ** distributedWorkloads, unsigned int * workerWorkloadIterator);
void sendDistributedWorkloads(int ** workloads, int workloadSize, int numOfWorkers);
void printDistributedWorkloads(int ** distributedWorkloads, int workloadSize, int numOfWorkers);
void printProcessingTimeByType(int type, int numOfType, int numOfWorkers, MPI_Status * status);
void printTotalProcessingTimePerWorker(int numOfWorkers, int workerWorkloadSize, MPI_Status * status);
int getNumberOfJobsByType(int type, int ** distributedWorkloads, int workloadSize, int numOfWorkers);

int main(int argc, char ** argv) {
	// printf("Program start.");
	double startTime, stopTime;
	int rank, size;
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
		int numOfWorkers = size - 1;
		workload = generateRandomWorkloadArray(WORKLOAD_SIZE);
		int ** distributedWorkloads = distributeWorkload(workload, WORKLOAD_SIZE, numOfWorkers, rank);
		sendDistributedWorkloads(distributedWorkloads, WORKLOAD_SIZE, numOfWorkers);
		int numberOfType0Jobs = getNumberOfJobsByType(TYPE0_JOB, distributedWorkloads, WORKLOAD_SIZE, numOfWorkers);
		int numberOfType1Jobs = getNumberOfJobsByType(TYPE1_JOB, distributedWorkloads, WORKLOAD_SIZE, numOfWorkers);
		int numberOfType2Jobs = getNumberOfJobsByType(TYPE2_JOB, distributedWorkloads, WORKLOAD_SIZE, numOfWorkers);
		printf("Workloads for type 0: %d\n", numberOfType0Jobs);
		printf("Workloads for type 1: %d\n", numberOfType1Jobs);
		printf("Workloads for type 2: %d\n", numberOfType2Jobs);
		printTotalProcessingTimePerWorker(size - 1, WORKLOAD_SIZE / numOfWorkers, &status);
		printProcessingTimeByType(TYPE0_JOB, numberOfType0Jobs, size - 1, &status);
		printProcessingTimeByType(TYPE1_JOB, numberOfType1Jobs, size - 1, &status);
		printProcessingTimeByType(TYPE2_JOB, numberOfType2Jobs, size - 1, &status);
		free(distributedWorkloads);
	} else {
		// printf("Process %d receiving workload\n", rank);
		workload = receiveWorkload(workload, WORKLOAD_SIZE / (size - 1), MPI_INT, ROOT_RANK, rank, MPI_COMM_WORLD, status);
		// printf("Process %d received workload\n", rank);
		// printf("Process %d processing workload\n", rank);
		workload_result_t workResultIntegerArray = processWorkload(workload, WORKLOAD_SIZE / (size - 1));
		// printf("Process %d processed workload\n", rank);
		double processingTime = MPI_Wtime() - startTime;
		// printf("Process %d sleep time was: %f\n", rank, processingTime);
		MPI_Send(&processingTime, 1, MPI_DOUBLE, ROOT_RANK, rank, MPI_COMM_WORLD);
		MPI_Send(&workResultIntegerArray.type0ProcessingTime, 1, MPI_DOUBLE, ROOT_RANK, rank, MPI_COMM_WORLD);
		MPI_Send(&workResultIntegerArray.type1ProcessingTime, 1, MPI_DOUBLE, ROOT_RANK, rank, MPI_COMM_WORLD);
		MPI_Send(&workResultIntegerArray.type2ProcessingTime, 1, MPI_DOUBLE, ROOT_RANK, rank, MPI_COMM_WORLD);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	stopTime = MPI_Wtime();
	MPI_Finalize();
	if(rank == 0) {
		printf("Total Execution Time: %f seconds\n", stopTime - startTime);
	}
	free(workload);
	return 0;
}

void printTotalProcessingTimePerWorker(int numOfWorkers, int workerWorkloadSize, MPI_Status * status) { // DONE
	int i;
	double processingTime = 0;
	for(i = 0; i < numOfWorkers; i++) {
		printf("Waiting to receive data from process %d, tag is %d\n", i+1, i+1);
		MPI_Recv(&processingTime, 1, MPI_DOUBLE, i+1, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		printf("Total Processing Time for worker %d: %f\n", i+1, processingTime);
		printf("Average Processing Time for worker %d: %f\n", i+1, processingTime / workerWorkloadSize);
	}
}

void printProcessingTimeByType(int type, int numOfType, int numOfWorkers, MPI_Status * status) { // DONE
	int i;
	double totalProcessingTime = 0;
	double processingTime = 0;
	for(i = 0; i < numOfWorkers; i++) {
		MPI_Recv(&processingTime, 1, MPI_DOUBLE, i+1, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		totalProcessingTime += processingTime;
	}
	printf("Total Processing Time for Type %d jobs: %f\n", type, totalProcessingTime);
	double averageProcessingTime = totalProcessingTime / (double)numOfType;
	printf("Average Processing Time for Type %d jobs: %f\n", type, averageProcessingTime);
}


int * receiveWorkload(int * buffer, int numOfElements, MPI_Datatype receiveType, int root, int rank, MPI_Comm comm, MPI_Status status) {
	buffer = malloc(sizeof(int) * numOfElements);
	MPI_Recv(buffer, numOfElements, receiveType, root, rank, comm, &status);
	// printf("Process %d received workload\n", rank);
	return buffer;
}

int ** distributeWorkload(int * workload, int workloadSize, int numOfWorkers, int senderRank) {
	int i;
	int ** distributedWorkloads = malloc(sizeof(int *) * numOfWorkers);
	unsigned int * workerWorkloadIterator = malloc(sizeof(unsigned int) * numOfWorkers);

	// Since the worker's workloads are empty, clear the iterators as the first available slot is the first slot.
	for(i = 0; i < numOfWorkers; i++) {
		workerWorkloadIterator[i] = 0;
	}

	// Initialize the worker's workload arrays.
	for(i = 0; i < numOfWorkers; i++) {
		distributedWorkloads[i] = malloc(sizeof(int) * (workloadSize / numOfWorkers));
		printf("Processor %d number of workloads: %d\n", i+1, workloadSize / numOfWorkers);
	}

	divideWorkloadByType(TYPE0_JOB, workload, workloadSize, numOfWorkers, distributedWorkloads, workerWorkloadIterator);
	divideWorkloadByType(TYPE1_JOB, workload, workloadSize, numOfWorkers, distributedWorkloads, workerWorkloadIterator);
	divideWorkloadByType(TYPE2_JOB, workload, workloadSize, numOfWorkers, distributedWorkloads, workerWorkloadIterator);
	// printDistributedWorkloads(distributedWorkloads, workloadSize, numOfWorkers);
	free(workerWorkloadIterator);
	return distributedWorkloads;
}

void divideWorkloadByType(int type, int * workload, int workloadSize, int numOfWorkers, int ** distributedWorkloads, unsigned int * workerWorkloadIterator) {
	int i;
	static int currentWorkerNumber = 0;
	for(i = 0; i < workloadSize; i++) {
		int job = workload[i];
		int * currentWorker = distributedWorkloads[currentWorkerNumber];
		unsigned int workerAvailableSpot = workerWorkloadIterator[currentWorkerNumber];
		assert(workerAvailableSpot < (workloadSize / numOfWorkers));
		if(job != JOB_DISTRIBUTED && job == type) {
			workload[i] = JOB_DISTRIBUTED; // indicate the job was distributed.
			currentWorker[workerAvailableSpot] = job;
			workerWorkloadIterator[currentWorkerNumber]++; // For next reference of the worker.
			currentWorkerNumber++;
			currentWorkerNumber %= numOfWorkers; // iterating through 0 -> numOfWorkers.
			continue;
		}
	}
}

int getNumberOfJobsByType(int type, int ** distributedWorkloads, int workloadSize, int numOfWorkers) {
	int i, j;
	int numOfType = 0;
	int * workload;
	for(i = 0; i < numOfWorkers; i++) {
		workload = distributedWorkloads[i];
		for(j = 0; j < (workloadSize / numOfWorkers); j++) {
			if(workload[j] == type) {
				numOfType++;
			}
		}
	}
	return numOfType;
}

void printDistributedWorkloads(int ** distributedWorkloads, int workloadSize, int numOfWorkers) {
	int i, j;
	int numOfZeros, numOfOnes, numOfTwos;
	int * workload;
	printf("\n");
	for(i = 0; i < numOfWorkers; i++) {
		numOfZeros = numOfOnes = numOfTwos = 0;
		workload = distributedWorkloads[i];
		for(j = 0; j < (workloadSize / numOfWorkers); j++) {
			if (workload[j] == 0) {
				numOfZeros++;
			} else if(workload[j] == 1) {
				numOfOnes++;
			} else {
				numOfTwos++;
			}
			printf("%d", workload[j]);
		}
		printf("\nTotal Type 0: %d\nTotal Type 1: %d\nTotal Type 2: %d\n", numOfZeros, numOfOnes, numOfTwos);
	}
}

void sendDistributedWorkloads(int ** workloads, int workloadSize, int numOfWorkers) {
	int worker;
	for(worker = 0; worker < numOfWorkers; worker++) {
		int dest = worker + 1;
		if(MPI_Send(workloads[worker], workloadSize / numOfWorkers, MPI_INT, dest, dest, MPI_COMM_WORLD) != MPI_SUCCESS) {
			dieWithError("Workload sending failed to receive");
		}
	}
}

