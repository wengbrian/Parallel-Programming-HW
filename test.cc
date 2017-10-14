#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>
#include<list>

void readFile(char* filename);
void writeFile(char* filename);
int checkIfSorted(int* checkList);
void even_odd();
float* mergeFromMin(float *arr1, float *arr2, int size1, int size2);
float* mergeFromMax(float *arr1, float *arr2, int size1, int size2);

int debug = 0;
int rank, m;
MPI_Comm comm = MPI_COMM_WORLD;
int n;
MPI_Offset pos;
int readSize;
int bufferSize;
int bufferSize_l = 0; // left for rank-1, right for rank+1
int bufferSize_r = 0;
float* arr;
float* arr_l;
float* arr_r;

int main(int argc, char* argv[]){
    if(argc < 4){
        printf("not enough argument\n");
        exit(0);
    }

    // initialize MPI and variables
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &m);
    n = atoi(argv[1]);

    // calculate parameters
    int re = n % m; // residual pair
    int avg = n / m; // average pair
    pos = sizeof(float)*(rank*avg + ((m-rank <= re) ? re-m+rank : 0));
    readSize = bufferSize = avg + (m-rank <= re ? 1 : 0);
    if(rank > 0) bufferSize_l = avg + (m-rank+1 <= re ? 1 : 0);
    if(rank != m-1) bufferSize_r = avg + (m-rank-1 <= re ? 1 : 0);
    if(debug) printf("rank%d:, %d, %d, %d\n", rank, bufferSize_l, bufferSize, bufferSize_r); 
    // read file to buffer
    arr = (float*)malloc(sizeof(float)*bufferSize);
    arr_l = (float*)malloc(sizeof(float)*bufferSize_l);
    arr_r = (float*)malloc(sizeof(float)*bufferSize_r);
    readFile(argv[2]);

    if(debug){
        printf("rank: %d\n", rank);
        printf("input: %d\n", n);
        printf("num processors: %d\n", m);
        printf("avg: %d\n", avg);
        printf("residual: %d\n", re);
        printf("offset: %d\n", pos);
        printf("buffer size: %d\n", bufferSize);    
        printf("read size: %d\n", readSize);    
        for(int i = 0; i < 10; i++)
            printf("%.0f ", arr[i]);
        printf("\n\n");
    }
    even_odd();
    if(debug){
        for(int i = 0; i < 10; i++)
            printf("%.0f ", arr[i]);
    }

    // write file
    writeFile(argv[3]);
    free(arr);
	MPI_Finalize();
}

void readFile(char* filename){
    MPI_Status status;
    MPI_File in;
    MPI_File_open(comm, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &in);
    MPI_File_read_at(in, pos, arr, readSize, MPI_FLOAT, &status);
    MPI_File_close(&in);
}

void writeFile(char* filename){
    MPI_Status status;
    MPI_File out;
    MPI_File_open(comm, filename, MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &out);
    MPI_File_write_at(out, pos, arr, readSize, MPI_FLOAT, &status);
    MPI_File_close(&out);
}

// check if all processor sorted
int checkIfSorted(int* checkList){
    for(int i = 0; i < m; i++){
        if(checkList[i] != 1)
            return 0;
    }
    return 1;
}

int comp(const void* a, const void* b){
    float c = *(float*) a;
    float d = *(float*) b;
    if(c > d)
        return 1;
    else if(c < d)
        return -1;
    else
        return 0;
}
void even_odd(){
    MPI_Request req;
    MPI_Status status;
    qsort(arr, bufferSize, sizeof(float), comp);
    for(int i = 0; i < m; i++){

        // even phase communication
        if(rank % 2 == 1){
	        MPI_Isend(arr, bufferSize, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, &req);
	        MPI_Recv(arr_l, bufferSize_l, MPI_FLOAT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Wait(&req, &status);
            arr = mergeFromMax(arr, arr_l, bufferSize, bufferSize_l);
        }else if((rank % 2 == 0) && (rank+1 < m)){
	        MPI_Isend(arr, bufferSize, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, &req);
	        MPI_Recv(arr_r, bufferSize_r, MPI_FLOAT, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Wait(&req, &status);
            arr = mergeFromMin(arr, arr_r, bufferSize, bufferSize_r);
        }

        if(debug){
            //for(int i = 0; i < bufferSize; i++)
            for(int i = bufferSize-10; i < bufferSize; i++)
                //printf("%.0f ", arr[i]);
                printf("%.1f ", arr[i]);
            printf("\n");
        }

        // odd phase communication
        if((rank % 2 == 1) && (rank+1 < m)){
	        MPI_Isend(arr, bufferSize, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, &req);
	        MPI_Recv(arr_r, bufferSize_r, MPI_FLOAT, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Wait(&req, &status);
            arr = mergeFromMin(arr, arr_r, bufferSize, bufferSize_r);
        }else if((rank % 2 == 0) && (rank-1 >= 0)){
	        MPI_Isend(arr, bufferSize, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, &req);
	        MPI_Recv(arr_l, bufferSize_l, MPI_FLOAT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Wait(&req, &status);
            arr = mergeFromMax(arr, arr_l, bufferSize, bufferSize_l);
        }

        if(debug){
            //for(int i = 0; i < bufferSize; i++)
            for(int i = bufferSize-10; i < bufferSize; i++)
                //printf("%.0f ", arr[i]);
                printf("%.1f ", arr[i]);
            printf("\n");
        }
    }
}

float* mergeFromMin(float *arr1, float *arr2, int size1, int size2){
    int idx1 = 0, idx2 = 0; 
    float *tmp = (float*) malloc(sizeof(float)*bufferSize);
    for(int i = 0; i < bufferSize; i++){
        if(((idx1 < size1) && (idx2 < size2) && (arr1[idx1] <= arr2[idx2])) || (idx2 >= size2)){
            tmp[i] = arr1[idx1++];
        }else{
            tmp[i] = arr2[idx2++];
        }
    }
    free(arr);
    return tmp;
}

float* mergeFromMax(float *arr1, float *arr2, int size1, int size2){
    int idx1 = size1-1, idx2 = size2-1; 
    float *tmp = (float*) malloc(sizeof(float)*bufferSize);
    for(int i = bufferSize-1; i >= 0 ; i--){
        if(((idx1 >= 0) && (idx2 >= 0) && (arr1[idx1] >= arr2[idx2])) || (idx2 < 0)){
            tmp[i] = arr1[idx1--];
        }else{
            tmp[i] = arr2[idx2--];
        }
    }
    free(arr);
    return tmp;
}
