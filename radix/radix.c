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
//unsigned long int SIZE = 1048576;
//unsigned long int SIZE = 524288;
//unsigned long int SIZE = 262144;

#define GIG 1000000000

#define OMP
#define DYNAMIC

#ifdef OMP
#include <omp.h>
#endif
#ifdef OMP
#define NUM_THREADS 8
#else
#define NUM_THREADS 1
#endif

#define NUM_RUNS 1

//This is the base, in terms of 2^(BASE-1), so BASE 2 means 2^(BASE-1), or base 2. BASE 3 would mean 2^(BASE-1) or 4
#define BASE 2
#define POW2 2

void printInt(int *in);
struct timespec diff2(struct timespec start, struct timespec end);

void checkSorted(int *in);
void bucketSort(int *in, int radix, int offset, int length, int bucket[POW2][SIZE], int *l1, int *l2);
void combineBuckets(int *in1, int *in2, int offset, int length);


void radixSort(int *in, int *out) {
	int i;
	int radix;
	int *buck[NUM_THREADS];
//	int *buck1[NUM_THREADS];
	for (i=0; i<NUM_THREADS; i++) {
		buck[0][i] = malloc(SIZE * sizeof(int));
		buck[1][i] = malloc(SIZE * sizeof(int));
	}
	int l1[NUM_THREADS][1], l2[NUM_THREADS][1];
	int runLength = SIZE / NUM_THREADS;
	for (radix=0; radix<5; radix++) {
#ifdef OMP
#pragma omp parallel for
#endif
		for (i=0; i<NUM_THREADS; i++) {
			bucketSort(in,radix,i*runLength,runLength,l1[i],l2[i]);
		}
	int inputOffset = i*runLength;
		int sum=0;
		for (i=0; i<NUM_THREADS; i++) {
			combineBuckets(in, buck[0][i], sum, *l1[i]);
			sum += *l1[i];
		}

		for (i=0; i<NUM_THREADS; i++) {
			combineBuckets(in, buck[1][i], sum, *l2[i]);
			sum += *l2[i];
		}
	}
	for (i=0; i<SIZE; i++) {
		out[i]=in[i];
	}
	for (i=0; i<NUM_THREADS; i++) {
		free(buck[0][i]);
		free(buck[1][i]);
	}
}


void bucketSort(int *in, int radix, int offset, int length, int bucket[POW2][SIZE], int *l1, int *l2) {
	int i=0;
	int counter0 = 0;
	int counter1 = 0;
	int maskVal = 1 << radix;
	for (i=offset; i<length+offset; i++) {
		if (in[i] & maskVal) {
			bucket[0][counter1] = in[i];
			counter1++;
		} else if (!(in[i] & maskVal)) {
			bucket[1][counter0] = in[i];
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

int main() {
printf("Start.\n");

	int i, j, k;
#ifdef DYNAMIC
	int *in;
	int *mid;
#else
	int in[SIZE];
	int mid[SIZE];
#endif

	int *tmp;

	struct timespec time1, time2;
  	struct timespec diff1;
  	float value;
printf("Pre allocate.\n");
#ifdef DYNAMIC
	in = malloc(SIZE * sizeof(int));
	mid = malloc(SIZE * sizeof(int));
#endif
	srand(SEED);

	for (i=0; i<SIZE; i++) {
		in[i]=random() % MAXVAL;//MAXVAL;
	}

	for (i=0; i<MAXVAL; i++) {
		mid[i] = 0;
	}

	printf("start count.\n");
	float tmpValue;
//First two 8s to initialize stuff better.
long int SIZES[] = {SIZE};
//First few values to even stuff out
//long int SIZES[] = {8,8,8,8,2097152,4194304,8388608,16777216,
//33554432,67108864,134217728,268435456,536870912};
for (k=0; k<(sizeof(SIZES)/sizeof(long int)); k++) {
	SIZE=SIZES[k];
	for (i=0; i<NUM_RUNS; i++) {
		omp_set_num_threads(NUM_THREADS);
		clock_gettime(CLOCK_REALTIME, &time1);
		radixSort(in, mid);
		clock_gettime(CLOCK_REALTIME, &time2);
		diff1 = diff2(time1,time2);
		tmpValue = (double)(GIG * diff1.tv_sec + diff1.tv_nsec);
		//Reset mid

//		for (j=0; j<MAXVAL; j++) {
//			mid[j] = 0;
//		}

//		printf("time %d: %.4f\n",i,tmpValue);
		value += tmpValue;
		//reset mid
	}
	value /= NUM_RUNS;
    //Put in ms.
    value /= 1000000;
	printf("%ld\t%.4f\n",SIZES[k],value);
	value=0;
}

//	printf("out: ");
//	printInt(mid);
	checkSorted(mid);

#ifdef DYNAMIC
	free(in);
	free(mid);
#endif
	return 0;
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

struct timespec diff2(struct timespec start, struct timespec end) {
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
