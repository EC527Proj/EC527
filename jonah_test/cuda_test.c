#include <cstdio>
#include <cstdlib>
#include <math.h>

// Assertion to check for errors
#define CUDA_SAFE_CALL(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line, bool abort=true)
{
	if (code != cudaSuccess) 
	{
		fprintf(stderr,"CUDA_SAFE_CALL: %s %s %d\n", cudaGetErrorString(code), file, line);
		if (abort) exit(code);
	}
}

#define BLOCK_SIZE				16
#define WIDTH					1024
#define NUM_BLOCKS				WIDTH / 16
#define TILE_WIDTH				16

#define GIG 					1000000000
#define CPG 					3.6

#define PRINT_TIME 				1
#define SM_ARR_LEN				128
#define TOL						11


#define OMEGA 					1.90
#define MINVAL   				0.0
#define MAXVAL  				10.0

#define IMUL(a, b) __mul24(a, b)

void host_bb(float * arr);
void initializeArray1D(float *arr, int len, int seed);
void SOR(float * v, int arrLen);
void print_matrix(float * v, int len);
struct timespec diff(struct timespec start, struct timespec end);

__global__ void mmm_global( float *Md, float *Nd, float *Od ) {
/*
	
	*/
	const int tid = IMUL(blockDim.x, blockIdx.x) + threadIdx.x;
	const int threadN = IMUL(blockDim.x, gridDim.x);
	
	int i;
	
	for(i = tid; i < WIDTH; i += threadN) {
		Od[i] = Md[i] * 2;
	}
}

__global__ void bubblesort_d(float* scratch_global)  { 
    int offset = (blockDim.x*blockIdx.x+threadIdx.x)*numchn;
    int k1, k2, i;
    float temp1;
    float scratch[numchn]; //local memory
    for(i=0;i<numchn;i++)
        scratch[i]=scratch_global[offset+i];
    /* Perform a bubble sort */
    for (k1=0; k1<numchn-1; k1++) {
        for (k2=0; k2<numchn-1-k1; k2++) {
            if (scratch[k2]>scratch[k2+1]) {
                temp1 = scratch[k2];
                scratch[k2]=scratch[k2+1];
                scratch[k2+1]=temp1;
    }   }   }
    //__syncthreads();        
    for(i=0;i<numchn;i++)
        scratch_global[offset+i] = scratch[i];
}


int main(int argc, char **argv) {
	int arrLen = 0, i, j;
		
	// GPU Timing variables
	cudaEvent_t start, stop;
	float elapsed_gpu;
	
	//CPU timing variables
	struct timespec time1, time2, time_elapsed;
	
	
	// Arrays on GPU global memory
	float *d_m;
	float *d_n;
	float *d_o1;
	float *d_o2;
	float *d_o3;

	// Arrays on the host memory
	float *h_m;
	float *h_n;
	float *h_o1;
	float *h_o2;
	float *h_o3;
	float *h_gold;
	
	
	if (argc > 1) {
		arrLen  = atoi(argv[1]);
	}
	else {
		arrLen = WIDTH;
	}

	printf("Size of the matrix = %dx%d\n", arrLen, arrLen);

	// Allocate GPU memory
	size_t allocSize = WIDTH * WIDTH * sizeof(float);
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_m, allocSize));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_n, allocSize));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_o1, allocSize));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_o2, allocSize));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_o3, allocSize));
		
	// Allocate arrays on host memory
	h_m                   = (float *) malloc(allocSize);
	h_n                   = (float *) malloc(allocSize);
	h_o1                   = (float *) malloc(allocSize);
	h_o2                   = (float *) malloc(allocSize);
	h_o3                   = (float *) malloc(allocSize);
	h_gold                 = (float *) malloc(allocSize);
	
	// Initialize the host arrays
	printf("\nInitializing the arrays ...");
	// Arrays are initialized with a known seed for reproducability
	initializeArray1D(h_m, WIDTH, 2453);
	initializeArray1D(h_n, arrLen, 2453);
	initializeArray1D(h_gold, arrLen, 2453);
	printf("\t... done\n\n");
	
	//Make the destination matrix all zeros 
	for(i = 0; i < WIDTH; i++)
		for(j = 0; j < WIDTH; j++) {
			h_o1[i + WIDTH + j] = 0.0;
			h_o2[i + WIDTH + j] = 0.0;
			h_o3[i + WIDTH + j] = 0.0;
		
		}
		
//mmm_global****************************************	
printf("\n*****************************\nmm_global");

	// Transfer the arrays to the GPU memory
	CUDA_SAFE_CALL(cudaMemcpy(d_m, h_m, allocSize, cudaMemcpyHostToDevice));
	CUDA_SAFE_CALL(cudaMemcpy(d_n, h_n, allocSize, cudaMemcpyHostToDevice));
	CUDA_SAFE_CALL(cudaMemcpy(d_o1, h_o1, allocSize, cudaMemcpyHostToDevice));
	  
	dim3 myBlock1(NUM_BLOCKS, NUM_BLOCKS);
	dim3 myThreads1(WIDTH / myBlock1.x, WIDTH / myBlock1.y);
#if PRINT_TIME
	// Create the cuda events
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	// Record event on the default stream
	cudaEventRecord(start, 0);
#endif	
	// Launch the kernel 
	mmm_global<<<16, 256>>>(d_m, d_n, d_o1);
#if PRINT_TIME
	// Stop and destroy the timer
	cudaEventRecord(stop,0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&elapsed_gpu, start, stop);
	printf("\nGPU time: %f (msec)\n", elapsed_gpu);
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
#endif
	
	// Transfer the results back to the host
	CUDA_SAFE_CALL(cudaMemcpy(h_o1, d_o1, allocSize, cudaMemcpyDeviceToHost));
	
//mm_host****************************************	
printf("\n*****************************\nmm_host");
	// Compute the results on the host
	clock_gettime(CLOCK_REALTIME, &time1);
	
	host_bb(h_m, h_n, h_gold);
	clock_gettime(CLOCK_REALTIME, &time2);
	
	time_elapsed = diff(time1,time2);
	printf("\nCPU time: %ld (nsec)\n", (long int)((double)(CPG)*(double)(GIG * time_elapsed.tv_sec + time_elapsed.tv_nsec)) ); 
	
	printf("Calculations done.\n");
	

//Checking mmm_global****************************************	
printf("\n*****************************\nChecking mmm_global...");	
	int errCount = 0, zeroCount = 0;
	// Compare the results
	for(i = 0; i < WIDTH; i++) {
		if (abs(h_gold[i] - h_o1[i]) > TOL) {
			errCount++;
			printf("Failure at [%d][%d]: %.4f vs %.4f\n",i,j,h_o1[i],h_gold[i]);
		}
		if (h_gold[i] == 0) {
			zeroCount++;
			
		}
	} 
	
	if (errCount > 0) {
		printf("@ERROR: TEST FAILED: %d results did not matched\n", errCount);
	}
	else if (zeroCount > 0){
		printf("@ERROR: TEST FAILED: %d results (from GPU) are zero\n", zeroCount);
	}
	else {
		printf("...TEST PASSED: All results matched\n");
	}

	// Free-up device and host memory
	//CUDA_SAFE_CALL(cudaFree(d_P2));
	//CUDA_SAFE_CALL(cudaFree(d_P3));
	
	cudaFree(d_m);
	cudaFree(d_n);
	cudaFree(d_o1);
	cudaFree(d_o2);
	cudaFree(d_o3);
	free(h_m);
	free(h_n);
	free(h_o1);
	free(h_o2);
	free(h_o3);
	free(h_gold);
		
	return 0;
}

void host_bb(float * arr) {
	bool swapped = true;
	int j = 0;
	int tmp;
	while (swapped) {
		swapped = false;
		j++;
		for (int i = 0; i < n - j; i++) {
			if (arr[i] > arr[i + 1]) {
				tmp = arr[i];
				arr[i] = arr[i + 1];
				arr[i + 1] = tmp;
				swapped = true;
			}
		}
	}

}

void initializeArray1D(float *arr, int len, int seed) {
	int i;
	float fRand(float fMin, float fMax);
	srand(seed);

	for (i = 0; i < len; i++) {
			arr[i] = (fRand((float)(MINVAL),(float)(MAXVAL)));
		
	}
}
 
float fRand(float fMin, float fMax)
{
    float f = (float)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/* print vector */
void print_matrix(float * v, int len)
{
  long int i;

  printf("\n length = %ld", len);
  for (i = 0; i < len; i++) {
  }
}

struct timespec diff(struct timespec start, struct timespec end)
{
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
