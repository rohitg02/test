#ifndef PROCESS_WORKLOAD_H
#define PROCESS_WORKLOAD_H

typedef struct {
	int * integerArray;
	unsigned int arraySize;
	double type0ProcessingTime;
	double type1ProcessingTime;
	double type2ProcessingTime;
} workload_result_t;

typedef struct {
	int * job_results;
	unsigned int results_size;
} job_result_t;

typedef struct {
	job_result_t job_result;
	double sleepTime;
} job_eval_result_t;

workload_result_t processWorkload(int * workload, int workloadSize);
job_eval_result_t processJob(int workloadType);
void preciseSleep(double sleepTime);
int * appendToArray(int * toArray, int * fromArray, int fromArraySize, int toArraySize);

#endif