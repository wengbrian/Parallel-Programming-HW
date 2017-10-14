#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>
#include<list>

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
int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    int rank, m;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &m);
    MPI_Status status;
    float val = 0;
    int k = 536870911;
    float *arr = (float*)malloc(sizeof(float)*k);
    struct timespec start, end;
    double time_used;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if(rank==0){
        for(int i = 0; i < k; i++){ 
	        MPI_Send(&val, 1, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
        }
    }
    if(rank==1){
        for(int i = 0; i < k; i++){ 
	        MPI_Recv(&val, 1, MPI_FLOAT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    struct timespec temp = diff(start, end);
    time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;

    printf("Time taken %f\n", time_used);
    clock_gettime(CLOCK_MONOTONIC, &start);
    if(rank==0)
        MPI_Send(arr, k, MPI_FLOAT, 1, 0, comm);
    if(rank==1)
        MPI_Recv(arr, k, MPI_FLOAT, 0, MPI_ANY_TAG, comm, &status);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    printf("Time taken %f\n", time_used);
    free(arr);
	MPI_Finalize();
}
