#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

unsigned long int SIZE = 1<<30;
#define SEED        6
#define NUM_THREADS 1
#define MAXVAL      31
#define GIG         1000000000
#define NUM_RUNS    1

void quicksort(int * a, int p, int r);
int partition(int * a, int p, int r);
void checkSorted(int *in, unsigned long int mySize);
struct timespec diff(struct timespec start, struct timespec end);

int main(void) {

   // int a[10] = {5, 3, 8, 4, 0, 9, 2, 1, 7, 6};
    int i, k;
    float tmpTime, totalTime;


    printf("Allocation\n");
    int *in = (int*)malloc(SIZE * sizeof(int));
    
    struct timespec time1, time2, time_elapsed;

    srand(SEED);

    printf("\nNumber of Threads: %d\n", NUM_THREADS);

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
        /*                1<<18, 1<<18, 1<<18, 1<<18,
                        1<<19, 1<<19, 1<<19, 1<<19,
        */
                        };

    int myCount = 0;
    for (k = 0; k < (sizeof(SIZES)/sizeof(long int)); k++) {
        for (i = 0; i < SIZES[k]; i++) {
            in[i] = random() % MAXVAL;
        }
        SIZE = SIZES[k];

        for (i = 0; i < NUM_RUNS; i++) {
            omp_set_num_threads(NUM_THREADS);
            //omp_set_nested(1);

            clock_gettime(CLOCK_REALTIME, &time1);
            quicksort(in, 0, SIZE-1);
            clock_gettime(CLOCK_REALTIME, &time2);
            time_elapsed = diff(time1, time2);

            tmpTime = (double)(GIG * time_elapsed.tv_sec + time_elapsed.tv_nsec);
            totalTime += tmpTime;
        }
        totalTime /= NUM_RUNS;
        totalTime /= 1000000;
        if((myCount % 4) == 0) 
            printf("%ld,\t%.4f\n", SIZES[k], totalTime);
        totalTime = 0;
        //checkSorted(in, SIZE);
        myCount++;

    }

    return 0;
}

int partition(int * a, int p, int r) {
    int lt[r-p];
    int gt[r-p];
    int i;
    int j;
    int key = a[r];
    int lt_n = 0;
    int gt_n = 0;

#pragma omp parallel for shared(i, j)
    for(i = p; i < r; i++) {
        if(a[i] < a[r]){
            lt[lt_n++] = a[i];
        }else{
            gt[gt_n++] = a[i];
        }   
    }   

    for(i = 0; i < lt_n; i++) {
        a[p + i] = lt[i];
    }   

    a[p + lt_n] = key;

    for(j = 0; j < gt_n; j++) {
        a[p + lt_n + j + 1] = gt[j];
    }   

    return p + lt_n;
}

void quicksort(int * a, int p, int r) {
    int div;

    if(p < r){ 
        div = partition(a, p, r); 
#pragma omp parallel sections
        {   
#pragma omp section
            quicksort(a, p, div - 1); 
#pragma omp section
            quicksort(a, div + 1, r); 

        }
    }
}

void checkSorted(int *in, unsigned long int mySize) {
    int i, j;
    for (i=1; i<mySize; i++) {
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

