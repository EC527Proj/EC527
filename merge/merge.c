#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define SEED 6
#define MAXVAL 14
//#define MAXCORES SIZE/2
#define MAXCORES 512
#define LEVELS 11
//#define SIZE 512
long int SIZE;// = pow(2,LEVELS);

void printInt(int *in);

void mergeSort(int *in, int *merged) {
	//where are left/right split up for the current level?
	int split=1;
	int i,j,k,l,m,n;
	int off;
	int go;
	int tid;
	int val;
	int halfSize=SIZE/2;
	int nStart, nEnd;
	//Get the first 8 values.
	omp_set_num_threads(512);
	nEnd=2048;
	for (k=10; k<LEVELS; k++) {
//#pragma omp parallel for private (off,i,j,l,go)
//	for (m=0; m<1; m++) {
//		for (n=0; n<512; n++) { //we're currently on the nth thread
//#pragma omp parallel private (n,off,i,j,l,go)
		nEnd=nEnd << 1;
#pragma omp for private (n,off,i,j,l,go,val)
		for (n=0; n<nEnd; n++) { //we're currently on the nth thread
			val=k;

			printf("n: %d, k: %d\n",n,LEVELS);
			off = 2 * n * split; //which portion are we working on?
			//turn off a thread at a certain point.
			if (off > (SIZE - 2*split)) {
				n=MAXCORES;
//				n=nEnd;
				go=0;
//				printf("cutoff!\n");
			} else {
//				printf("I'm fine though!");
				go=1;
 			}

			j=0;
			i=0;
			//Do merge-sort on the current level.
			while (go) {
				printf("in here??? k: %d\n",val);
				if (k==LEVELS) {
					printf("DONE");
					go=0;
				}

				//Finish the array if right half is all sorted.
				if (j==split) {
					for (; i<split; i++)
						merged[off+i+j] = in[off+i];
					break;
				}
				//Finish the array if left half is all sorted
				if (i==split) {
					for (; j<split; j++)
						merged[off+i+j]=in[off+split+j];
					break;
				}
				//Check if left or right side is less
				if (in[off+i] < in[off+split+j]) { //left side less
					merged[off+i+j]=in[off+i];
					i++;
				} else { //right side less, increment j
					merged[off+i+j]=in[off+split+j];
					j++;
				}

			}

		}
		printf("out here?\n");
		int *tmp;
		//we're all done here, don't flip pointers on the last run.
		if (k>=LEVELS-1) {
			printf("DONE");
			return;
		}
		//flip the pointers, so we keep working on the "more sorted" data
		tmp=in;
		in=merged;
		merged=tmp;

		split = split << 1; //Go onto the next level!
//	}
	}
	return;
}

void checkSorted(int *in);

int main() {
	int i;
	int *in;
	int *out;
	int *tmp;

	SIZE=pow(2,LEVELS);
	srand(SEED);
	printf("Testing array of size %d\n",SIZE);
	in = (int*)malloc(SIZE*sizeof(int));
	out = (int*)malloc(SIZE*sizeof(int));
	for (i=0; i<SIZE; i++) {
		in[i]=random() % MAXVAL;
	}
//	printf("in: ");
//	printInt(in);
	for (i=0; i<SIZE; i++) {
		out[i] = in[i];
	}

	mergeSort(in, out);

//	printf("out: ");
//	printInt(out);
	checkSorted(out);

	free(in);
	free(out);
	return 0;
}

void printInt(int *in) {
	int i;
	for (i=0; i<SIZE; i++) {
		printf("%d",in[i]);
		if (i!=(SIZE-1))
			printf(", ");
	}
	printf("\n");
	return;
}

void checkSorted(int *in) {
	int i;
	for (i=1; i<SIZE; i++) {
		if (in[i-1] > in[i]) {
			printf("ARRAY UNSORTED AT INDEX %d\n",i);
			return;
		}
	}
	printf("Array Sorted.\n");
	return;
}
