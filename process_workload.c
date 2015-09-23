#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "error.h"
#include "mymath.h"
#include "process_workload.h"
#include "generate_workload.h"

#define TYPE0_ARRAYSIZE_MIN 10
#define TYPE0_ARRAYSIZE_MAX 20
#define TYPE0_SLEEP_MIN 0.1 /* seconds */
#define TYPE0_SLEEP_MAX 1.5 /* seconds */

#define TYPE1_ARRAYSIZE_MIN 20
#define TYPE1_ARRAYSIZE_MAX 50
#define TYPE1_SLEEP_MIN 2.0 /* seconds */
#define TYPE1_SLEEP_MAX 4.0 /* seconds */

#define TYPE2_ARRAYSIZE_MIN 50
#define TYPE2_ARRAYSIZE_MAX 100
#define TYPE2_SLEEP_MIN 3.5 /* seconds */
#define TYPE2_SLEEP_MAX 7.0 /* seconds */


#define MICROSECONDS_IN_SECOND 1000000

int isProperWorkloadType(int workloadType);
job_result_t generateRandomIntegerArray(int workloadType);
int generateRandomIntegerInRange(int minimum, int maximum);
double generateRandomSleepTime(int workloadType);

job_eval_result_t processJob(int workloadType) {
	job_eval_result_t result;
	job_result_t jobProduct;
	if (!isProperWorkloadType(workloadType)) {
		dieWithError("Invalid workload type\n");
	}
	double sleepTime = generateRandomSleepTime(workloadType);
	preciseSleep(sleepTime);
	jobProduct = generateRandomIntegerArray(workloadType);
	result.job_result = jobProduct;
	result.sleepTime = sleepTime;
	return result;
}

workload_result_t processWorkload(int * workload, int workloadSize) {
	workload_result_t result;
	job_eval_result_t jobResult;
	memset(&result, 0, sizeof(workload_result_t));
	// Worst case scenario we get all type 2 jobs and they give us a maximum number of integers.
	int i;
	for(i = 0; i < workloadSize; i++) {
		jobResult = processJob(workload[i]);
		result.integerArray = appendToArray(result.integerArray, jobResult.job_result.job_results, jobResult.job_result.results_size, result.arraySize);
		result.arraySize += jobResult.job_result.results_size;
		switch(workload[i]) {
			case 0:
				result.type0ProcessingTime += jobResult.sleepTime;
				break;
			case 1:
				result.type1ProcessingTime += jobResult.sleepTime;
				break;
			case 2:
				result.type2ProcessingTime += jobResult.sleepTime;
				break;
		}
	}
	return result;
}

// This function will append all the values of the from integer array to end of the to array.
int * appendToArray(int * toArray, int * fromArray, int fromArraySize, int toArraySize) {
	int i, j;
	j = 0;
	toArray = realloc(toArray, sizeof(int) * (toArraySize + fromArraySize));
	for(i = toArraySize; i < toArraySize + fromArraySize; i++) {
		toArray[i] = fromArray[j++];
	}
	return toArray;
}

double generateRandomSleepTime(int workloadType) {
	double min, max;
	if (workloadType == 0) {
		min = TYPE0_SLEEP_MIN;
		max = TYPE0_SLEEP_MAX;
	} else if(workloadType == 1) {
		min = TYPE1_SLEEP_MIN;
		max = TYPE1_SLEEP_MAX;
	} else {
		min = TYPE2_SLEEP_MIN;
		max = TYPE2_SLEEP_MAX;
	}
	return generateRandomDoubleInRange(min, max);
}

int isProperWorkloadType(int workloadType) {
	if(workloadType > 2 || workloadType < 0) {
		return -1;
	} else {
		return 1;
	}
}

job_result_t generateRandomIntegerArray(int workloadType) {
	job_result_t result;
	int i;
	if(workloadType == 0) {
		result.results_size = generateRandomIntegerInRange(TYPE0_ARRAYSIZE_MIN, TYPE0_ARRAYSIZE_MAX);
	} else if(workloadType == 1) {
		result.results_size = generateRandomIntegerInRange(TYPE1_ARRAYSIZE_MIN, TYPE1_ARRAYSIZE_MAX);
	} else {
		result.results_size = generateRandomIntegerInRange(TYPE2_ARRAYSIZE_MIN, TYPE2_ARRAYSIZE_MAX);
	}
	result.job_results = malloc(sizeof(int) * result.results_size);
	for(i = 0; i < result.results_size; i++) {
		result.job_results[i] = rand();
	}
	return result;
}

int generateRandomIntegerInRange(int minimum, int maximum) {
	return rand() % (maximum + 1 - minimum) + minimum;
}

void preciseSleep(double sleepTime) {
	usleep(sleepTime * MICROSECONDS_IN_SECOND);
}