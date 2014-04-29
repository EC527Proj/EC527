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
unsigned long int SIZE = 1 << 24;
//unsigned long int SIZE = 1048576; //2^20
//unsigned long int SIZE = 524288;
//unsigned long int SIZE = 262144;

#define GIG 1000000000

#define OMP
#define DYNAMIC

#ifdef OMP
#include <omp.h>
#endif

#ifdef OMP
#define NUM_THREADS 32
#else
#define NUM_THREADS 1
#endif

#define NUM_RUNS 1

#define BASE 1024
#define LOG_BASE log(BASE)/log(2)

//Function prototypes
void radixSort(int *in, int *out);
void checkSorted(int *in);
void bucketSort(int *in, int radix, int offset, int length, int **bucket, int *bucketLengths);
void combineBuckets(int *in1, int *in2, int offset, int length);
void printInt(int *in);
struct timespec diff(struct timespec start, struct timespec end);


int main() {
	printf("Start.\n");

	int i, j, k;
#ifdef DYNAMIC
	int *in;
#else
	int in[SIZE];
#endif

	int *tmp;

	struct timespec time1, time2;
  	struct timespec diff1;
  	float value;
	printf("Pre allocate.\n");
#ifdef DYNAMIC
	in = malloc(SIZE * sizeof(int));
#endif
	srand(SEED);

	printf("start count.\n");

	printf("\nNumber of Threads: %d\n", NUM_THREADS);
	printf("Base: %d\n\n", BASE);

	float tmpValue;
	//First two 8s to initialize better.
	//First few values to even it out
	long int SIZES[] = {
		
						1<<2, 1<<2, 1<<2, 1<<2,
						1<<3, 1<<3, 1<<3, 1<<3,
						1<<4, 1<<4, 1<<4, 1<<4,
						1<<5, 1<<5, 1<<5, 1<<5,
						1<<6, 1<<6, 1<<6, 1<<6,
						1<<7, 1<<7, 1<<7, 1<<7,
						1<<8, 1<<8, 1<<8, 1<<8,
						1<<9, 1<<9, 1<<9, 1<<9,
						1<<10, 1<<10, 1<<10, 1<<10,
		
						1<<11, 1<<11, 1<<11, 1<<11,
						1<<12, 1<<12, 1<<12, 1<<12,
						1<<13, 1<<13, 1<<13, 1<<13,
						1<<14, 1<<14, 1<<14, 1<<14,
						1<<15, 1<<15, 1<<15, 1<<15,
						1<<16, 1<<16, 1<<16, 1<<16,
						1<<17, 1<<17, 1<<17, 1<<17,
						1<<18, 1<<18, 1<<18, 1<<18,
						1<<19, 1<<19, 1<<19, 1<<19,
			
						1<<21, 1<<21, 1<<21, 1<<21,
						1<<22, 1<<22, 1<<22, 1<<22,
						1<<23, 1<<23, 1<<23, 1<<23,
						1<<24, 1<<24, 1<<24, 1<<24,
	//					1<<25, 1<<25, 1<<25, 1<<25,
	//					1<<26, 1<<26, 1<<26, 1<<26,
	//					1<<27, 1<<27, 1<<27, 1<<27,
	//					1<<28, 1<<28, 1<<28, 1<<28,
	//					1<<29, 1<<29, 1<<29, 1<<29,
						};

	int myCount = 1;
	for (k=0; k<(sizeof(SIZES)/sizeof(long int)); k++) {

		for (i=0; i<SIZE; i++) {
			in[i]=random() % MAXVAL;//MAXVAL;
		}

		SIZE=SIZES[k];

		for (i=0; i<NUM_RUNS; i++) {
			omp_set_num_threads(NUM_THREADS);
			clock_gettime(CLOCK_REALTIME, &time1);
			radixSort(in,in);
			clock_gettime(CLOCK_REALTIME, &time2);
			diff1 = diff(time1,time2);
			tmpValue = (double)(GIG * diff1.tv_sec + diff1.tv_nsec);
			//Reset out

			value += tmpValue;
			//reset out
		}
		value /= NUM_RUNS;
	    //Put in ms.
	    value /= 1000000;
		

		if((myCount % 4) == 0) 
			printf("%ld,\t%.4f\n",SIZES[k],value);
		value=0;
		myCount++;
	}

	//	printf("out: ");
	//	printInt(out);
	checkSorted(in);

#ifdef DYNAMIC
	free(in);
#endif
	return 0;
}

void radixSort(int *in, int *out) {

	int i, j;
	int radix;
	int ***buck; //[NUM_THREADS][BASE][SIZE/NUM_THREADS]
	int bucketLengths[NUM_THREADS][BASE];
	int runLength = SIZE / NUM_THREADS;

	buck = (int***)malloc(NUM_THREADS * sizeof(int**));
	for (i = 0; i < NUM_THREADS; i++) {
		buck[i] = (int**) malloc(BASE * sizeof(int *));
		for (j = 0; j < BASE; j++) {
			buck[i][j] = (int *) malloc( (SIZE/NUM_THREADS) * sizeof(int));
		}
	}



	for (radix=0; radix<5; radix++) {

		for (i = 0; i < NUM_THREADS; i++) {
			for (j = 0; j < BASE; j++)
			bucketLengths[i][j] = 0;
		}

#ifdef OMP
#pragma omp parallel for
#endif
		for (i = 0; i < NUM_THREADS; i++) {
			bucketSort(in, radix, i*runLength, runLength, buck[i], bucketLengths[i]);
		}

		int inputOffset = i*runLength;
		int sum=0;
		


		for (j = 0; j < BASE; j++) {
			for (i = 0; i < NUM_THREADS; i++) {
				combineBuckets(in, buck[i][j], sum, bucketLengths[i][j]);
				sum += bucketLengths[i][j];
			}
		}

	}

	for (i=0; i<SIZE; i++) {
		out[i]=in[i];
	}

	for (i = 0; i < NUM_THREADS; i++) {
		for (j = 0; j < BASE; j++) {
			free(buck[i][j]);
		}
		free(buck[i]);
	}
	free(buck);

}


void bucketSort(int *in, int radix, int offset, int length, int **bucket, int *bucketLengths) {
	int i = 0, buckNum;

	int maskVal = BASE - 1;
	int shift = radix * LOG_BASE;

	for (i = offset; i < length + offset; i++) {
		buckNum = ((in[i] >> shift) & maskVal);
		bucket[buckNum][bucketLengths[buckNum]] = in[i];
		bucketLengths[buckNum]++;

	}
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
	} 
	else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
