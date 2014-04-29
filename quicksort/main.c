#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>

unsigned long int SIZE = 1<<20;
#define SEED        6
#define NUM_THREADS 16
#define MAXVAL      31
#define GIG         1000000000
#define NUM_RUNS    1

struct timespec diff(struct timespec start, struct timespec end);
void checkSorted(int *in, unsigned long int mySize);
void printInt(int *in, long int mySize);



int cmpfunc (const void * a, const void * b)
{
return ( *(int*)a - *(int*)b );
}

int main()
{
  int i, k;
  struct timespec time1, time2, time_elapsed;

  float tmpTime, totalTime;

  int *in = (int*)malloc(SIZE * sizeof(int));

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
            
            1<<20, 1<<20, 1<<20, 1<<20,
            };


  srand(SEED);
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
          //_quicksort(in, SIZE, sizeof(int), cmpfunc);
          myqsort(in, SIZE, sizeof(int), cmpfunc);
          clock_gettime(CLOCK_REALTIME, &time2);
          time_elapsed = diff(time1, time2);

          tmpTime = (double)(GIG * time_elapsed.tv_sec + time_elapsed.tv_nsec);
          totalTime += tmpTime;
      }
      totalTime /= NUM_RUNS;
      totalTime /= 1000000;


      if((myCount % 4) == 0) {
        printf("%ld,\t%.4f\n", SIZES[k], totalTime);
        //checkSorted(in, SIZE);
      //printInt(in, SIZE);
      }
      totalTime = 0;
      myCount++;
      

  }
  

return(0);
}

void printInt(int *in, long int mySize) {
  int i;
  for (i=0; i<mySize; i++) {
    printf("%d",in[i]);
    printf(", ");
  }
  printf("\n");
  return;
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