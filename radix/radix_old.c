//gcc radix.c -o radix -lrt -fopenmp

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SEED 6
#define MAXVAL 31
//#define MAXCORES SIZE/2
#define MAXCORES 512
#define OUTERSIZE 1
unsigned long int SIZE = 1 << 15;

#define GIG 1000000000

#define OMP
#define DYNAMIC

#ifdef OMP
#include <omp.h>
#endif

#ifdef OMP
#define NUM_THREADS 2
#else
#define NUM_THREADS 1
#endif

#define NUM_RUNS 1

#define BASE 2

void radixSort(int *in, int *out);
void checkSorted(int *in);
void bucketSort(int *in, int radix, int offset, int length, int  *buck[SIZE/NUM_THREADS], int *l1, int *l2);
void combineBuckets(int *in1, int *in2, int offset, int length);
void printInt(int *in);
struct timespec diff(struct timespec start, struct timespec end);

int main() {
	printf("Start.\n");

	int i, j, k;
#ifdef DYNAMIC
	int *in;
	int *out;
#else
	int in[SIZE];
	int out[SIZE];
#endif

	int *tmp;

	struct timespec time1, time2;
  	struct timespec time_elapsed;
  	float value;
	printf("Pre allocate.\n");

#ifdef DYNAMIC
	in = malloc(SIZE * sizeof(int));
	out = malloc(SIZE * sizeof(int));
#endif

	srand(SEED);

	for (i=0; i<SIZE; i++) {
		in[i]=random() % MAXVAL;//MAXVAL;
	}

	for (i=0; i<MAXVAL; i++) {
		out[i] = 0;
	}

	printf("start count.\n");
	float tmpValue;
	//First two 8s to initialize stuff better.
	//long int SIZES[] = {SIZE};
	//First few values to even stuff out
	//long int SIZES[] = {8,8,8,8,2097152,4194304,8388608,16777216,33554432,67108864,134217728,268435456,536870912};
	//for (k=0; k<(sizeof(SIZES)/sizeof(long int)); k++) {
		//SIZE=SIZES[k];
		for (i=0; i<NUM_RUNS; i++) {

			omp_set_num_threads(NUM_THREADS);
			clock_gettime(CLOCK_REALTIME, &time1);
			radixSort(in, out);

			clock_gettime(CLOCK_REALTIME, &time2);
			time_elapsed = diff(time1,time2);
			tmpValue = (double)(GIG * time_elapsed.tv_sec + time_elapsed.tv_nsec);
			//Reset out

	//		for (j=0; j<MAXVAL; j++) {
	//			out[j] = 0;
	//		}

	//		printf("time %d: %.4f\n",i,tmpValue);
			value += tmpValue;
			//reset out
		}
		value /= NUM_RUNS;
	    //Put in ms.
	    value /= 1000000;
		printf("%ld\t%.4f ms\n", SIZE, value);
		value=0;
	//}

//	printf("out: ");
//	printInt(out);
	checkSorted(out);

#ifdef DYNAMIC
	free(in);
	free(out);
#endif
	return 0;
}

void radixSort(int *in, int *out) {
	int i;
	int radix;
	//int *buck0[NUM_THREADS];
	//int *buck1[NUM_THREADS];
	int *buck[2][NUM_THREADS/SIZE];

	for (i=0; i<NUM_THREADS; i++) {
		//buck0[i] = malloc(SIZE * sizeof(int));
		//buck1[i] = malloc(SIZE * sizeof(int));
		buck[i][0] = malloc(SIZE * sizeof(int));
		buck[i][1] = malloc(SIZE * sizeof(int));
	}

	int l1[NUM_THREADS][1], l2[NUM_THREADS][1];
	int runLength = SIZE / NUM_THREADS;

	for (radix=0; radix<5; radix++) {
#ifdef OMP
#pragma omp parallel for
#endif
		for (i=0; i<NUM_THREADS; i++) {
			//bucketSort(in, radix, i*runLength, runLength, buck[i], buck0[i], buck1[i], l1[i], l2[i]);
			bucketSort(in, radix, i*runLength, runLength, buck[i], l1[i], l2[i]);
		}

		int inputOffset = i*runLength;
		int sum=0;

		for (i=0; i<NUM_THREADS; i++) {
			combineBuckets(in, buck[i][0], sum, *l1[i]);
			sum += *l1[i];
		}

		for (i=0; i<NUM_THREADS; i++) {
			combineBuckets(in, buck[i][1], sum, *l2[i]);
			sum += *l2[i];
		}
	}
	

	for (i=0; i<NUM_THREADS; i++) {
		//free(buck0[i]);
		//free(buck1[i]);
		free(buck[i][0]);
		free(buck[i][1]);
	}
}


void bucketSort(int *in, int radix, int offset, int length, int *buck[SIZE/NUM_THREADS], int *l1, int *l2) {
	int i=0;
	int counter0 = 0;
	int counter1 = 0;
	int maskVal = 1 << radix;

	for (i=offset; i<length+offset; i++) {
		if (in[i] & maskVal) {
			buck[1][counter1] = in[i];
			counter1++;
		} else if (!(in[i] & maskVal)) {
			buck[0][counter0] = in[i];
			counter0++;
		}
	}
	l1[0] = counter0;
	l2[0] = counter1;
}

void combineBuckets(int *in1, int *in2, int offset, int length) {
	int i;
	for (i=0; i<length; i++)
		in1[offset+i] = in2[i];
}


void printInt(int *in) {
	int i;
	for (i=0; i<SIZE; i++) {
		printf("%d",in[i]);
		printf(", ");
	}
	printf("\n");
	return;
}

void checkSorted(int *in) {
	int i, j;
	for (i=1; i<SIZE; i++) {
		if (in[i] < in[i-1]) {
			printf("ARRAY UNSORTED.\n");
			return;
		}
	}
	printf("Array Sorted.\n");
	return;
}

struct timespec diff(struct timespec start, struct timespec end) {
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}
