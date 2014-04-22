#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define SEED 6
#define MAXVAL 100000
//#define MAXCORES SIZE/2
#define MAXCORES 512
#define OUTERSIZE 1
unsigned long int SIZE = 2700000;

#define GIG 1000000000
//#define OMP

#ifdef OMP
#define NUM_THREADS 4
#else
#define NUM_THREADS 1
#endif

#define NUM_RUNS 50

void printInt(int **in);
void swapCounted(long int in[MAXVAL], int **out);
struct timespec diff(struct timespec start, struct timespec end);

void countSort(int **in, long int out[MAXVAL]) {
	long int i, j;
#ifdef OMP
	long int tmpOut[NUM_THREADS][MAXVAL];
	for (i=0; i<NUM_THREADS; i++)
		for (j=0; j<MAXVAL; j++)
			tmpOut[i][j]=0;
	int myThread=-1;
#endif

//#pragma omp parallel for shared (j) private (i)
#ifdef OMP
#pragma omp parallel num_threads(NUM_THREADS)
	{
#endif
		int myId=omp_get_thread_num();
//		printf("myId: %d\n",myId);
#ifdef OMP
#pragma omp parallel for firstprivate(myId)
#endif
#ifdef OMP
		for (j=0; j<SIZE; j+=NUM_THREADS) {
			tmpOut[myId][in[0][j]]++;
		}
#else
		for (j=0; j<SIZE; j++)
			out[in[0][j]]++;
#endif

//		printf("here. j: %d\n",j);
#ifdef OMP
		#pragma omp barrier
	}
	for (i=0; i<NUM_THREADS; i++)
		for (j=0; j<MAXVAL; j++)
			out[j]+=tmpOut[i][j];
#endif
	return;
}

void checkSorted(int **in);

int main() {
	int i, j, k;
	int **in;
	int **out;
	long int mid[MAXVAL];
	int *tmp;

	struct timespec time1, time2;
  	struct timespec diff1;
  	float value;

	in = malloc(OUTERSIZE * sizeof(int*));
	out = malloc(OUTERSIZE * sizeof(int*));
	for (i=0; i<OUTERSIZE; i++) {
		in[i] = malloc(SIZE * sizeof(int));
		out[i] = malloc(SIZE * sizeof(int));
	}
	srand(SEED);
//	printf("Testing array of size %ld\n",SIZE*OUTERSIZE);

	for (i=0; i<OUTERSIZE; i++) {
		for (j=0; j<SIZE; j++) {
			in[i][j]=random() % MAXVAL;
		}
	}
	for (i=0; i<MAXVAL; i++) {
		mid[i] = 0;
	}


	printf("start count.\n");
	float tmpValue;
//First two 8s to initialize stuff better.
long int SIZES[] = {8,8,4,8,16,32,64,128,256,512,1024,2048,
4096,8192,16384,32768,65536,131072,262144,524288,1048576};
//First few values to even stuff out
//long int SIZES[] = {8,8,8,8,2097152,4194304,8388608,16777216,
//33554432,67108864,134217728,268435456,536870912};
for (k=0; k<(sizeof(SIZES)/sizeof(long int)); k++) {
	SIZE=SIZES[k];
	for (i=0; i<NUM_RUNS; i++) {
		clock_gettime(CLOCK_REALTIME, &time1);
		countSort(in, mid);
		clock_gettime(CLOCK_REALTIME, &time2);
		diff1 = diff(time1,time2);
		tmpValue = (double)(GIG * diff1.tv_sec + diff1.tv_nsec);
		//Reset mid
		for (j=0; j<MAXVAL; j++) {
			mid[j] = 0;
		}
//		printf("time %d: %.4f\n",i,tmpValue);
		value += tmpValue;
		//reset mid
	}
	value /= NUM_RUNS;
    value /= 1000000;
	printf("%ld\t%.4f\n",SIZES[k],value);
	value=0;
}
    //Put in ms.
	swapCounted(mid, out);
//	printf("out: ");
//	printInt(out);
	checkSorted(out);

	for (i=0; i<OUTERSIZE; i++) {
		free(in[i]);
		free(out[i]);
	}
	free(in);
//	free(mid);
	free(out);
	return 0;
}

void printInt(int **in) {
	int i,j;
	for (i=0; i<OUTERSIZE; i++) {
		for (j=0; j<SIZE; j++) {
			printf("%d",in[i][j]);
			printf(", ");
		}
	}
	printf("\n");
	return;
}

void swapCounted(long int in[MAXVAL], int **out) {
	int i, j, k, l;
	k=0;
	l=0;
	for (i=0; i<MAXVAL; i++) {	
		int topVal = in[i];
		for (j=0; j<topVal; j++) {
			out[k][l]=i;
			l++;
			if (l==SIZE) {
				l=0;
				k++;
			}
		}
	}
	return;
}

void checkSorted(int **in) {
	int i, j;
	for (j=0; j<OUTERSIZE; j++) {
		for (i=1; i<SIZE; i++) {
			if (in[j][i-1] > in[j][i]) {
				printf("ARRAY UNSORTED AT INDEX [%d][%d]\n",j,i);
				return;
			}
		}
		//Check the edges
		if (j != OUTERSIZE-1) {
			if (in[j+1][0] < in[j][SIZE-1]) {
				printf("in here.\n");
				printf("ARRAY UNSORTED AT INDEX [%d][%d]\n",j,i);
				return;
			}
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
