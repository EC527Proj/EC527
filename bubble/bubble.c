//gcc radix.c -o radix -lrt -fopenmp

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SEED 9
#define MAXVAL 1000
//#define MAXCORES SIZE/2
#define MAXCORES 512
#define OUTERSIZE 1
unsigned long int SIZE = 1 << 18;
#define GIG 1000000000

#define OMP
#define DYNAMIC

#ifdef OMP
#define NUM_THREADS 8

#else
#define NUM_THREADS 1
#endif

#ifdef OMP
#include <omp.h>
#endif

#define NUM_RUNS 1

#define BASE 2

void radixSort(int *in, int *out);
int checkSorted(int *in);
void bucketSort(int *in, int radix, int offset, int length, int  *buck[SIZE/NUM_THREADS], int *l1, int *l2);
void combineBuckets(int *in1, int *in2, int offset, int length);
void printInt(int *in);
struct timespec diff(struct timespec start, struct timespec end);

int swap(int *a, int *b) {
	int temp = *a;
	*a = *b;
	*b = temp;
}

void radixSort(int *in, int *out) {
	int i, t;
	int radix;
	int sorted = 0;
	int temp;
	int topIndex = SIZE;
	int topChange = 0;
topSpaghetti:
topChange=0;
topIndex=SIZE;
sorted=0;
while (!sorted) {
		sorted=1;
#ifdef OMP
#pragma omp parallel
		{
#pragma omp parallel for
#endif
			for (i=0; i<topIndex; i+=2) {
				if (in[i] > in[i+1]) {
					temp = in[i];
					in[i]=in[i+1];
					in[i+1]=temp;
					sorted=0;
				}
			}
#ifdef OMP
#pragma omp parallel for
#endif
			for (i=1; i<topIndex; i+=2) {
				if (in[i] > in[i+1]) {
					temp = in[i];
					in[i]=in[i+1];
					in[i+1]=temp;
					sorted=0;
				}
			}

//			topChange++;
//			if (topChange % 100 == 0)
//				topIndex--;
#ifdef OMP
		}
#endif

	}
//	if (!checkSorted(in))
//		goto topSpaghetti;
	for (i=0; i<SIZE; i++) {
		out[i]=in[i];
	}
}


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

	for (i=0; i<SIZE; i++) {
		out[i] = 0;
	}
//	printf("In: ");
//	printInt(in);

#ifdef OMP
	omp_set_num_threads(NUM_THREADS);
#endif
	printf("start count.\n");
	float tmpValue;
	//First two 8s to initialize stuff better.
	long int SIZES[] = {256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144};
	//First few values to even stuff out
	//long int SIZES[] = {8,8,8,8,2097152,4194304,8388608,16777216,33554432,67108864,134217728,268435456,536870912};
	for (k=0; k<(sizeof(SIZES)/sizeof(long int)); k++) {
		SIZE=SIZES[k];
		for (i=0; i<NUM_RUNS; i++) {

			clock_gettime(CLOCK_REALTIME, &time1);
			radixSort(in, out);

			clock_gettime(CLOCK_REALTIME, &time2);
			time_elapsed = diff(time1,time2);
			tmpValue = (double)(GIG * time_elapsed.tv_sec + time_elapsed.tv_nsec);
			//Reset out

			value += tmpValue;
			//reset out
		}
		value /= NUM_RUNS;
	    //Put in ms.
	    value /= 1000000;
		printf("%ld\t%.3f\n", SIZE, value);
		value=0;
//		printf("out: ");
//		printInt(out);
	}

//	printf("out: ");
//	printInt(out);
	checkSorted(out);

#ifdef DYNAMIC
	free(in);
	free(out);
#endif
	return 0;
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

int checkSorted(int *in) {
	int i, j;
	for (i=1; i<SIZE; i++) {
		if (in[i] < in[i-1]) {
//			printf("ARRAY UNSORTED.\n");
			return 0;
		}
	}
//	printf("Array Sorted.\n");
	return 1;
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
