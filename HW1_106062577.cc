#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>

void readFile(char* filename);
void writeFile(char* filename);
int sort(int n, int start);
int checkIfSorted(int* checkList);
void even_odd();

int debug = 0;
int rank, m;
MPI_Comm comm = MPI_COMM_WORLD;
int n;
MPI_Offset pos;
int readSize;
int bufferSize;
float* arr;

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
    int num_pair = n/2;
    int re = num_pair % m; // residual pair
    int avg = num_pair / m; // average pair
    pos = sizeof(float)*2*(rank*avg + ((m-rank <= re) ? re-m+rank : 0));
    bufferSize = 2*(avg + (m-rank <= re ? 1 : 0)) + (rank==m-1 ? n%2 : 1);
    readSize = 2*(avg + (m-rank <= re ? 1 : 0)) + (rank==m-1 ? n%2: 0);

    // read file to buffer
    arr = (float*)malloc(sizeof(float)*bufferSize);
    readFile(argv[2]);

    if(debug){
        printf("rank: %d\n", rank);
        printf("input: %d\n", n);
        printf("num pair: %d\n", num_pair);
        printf("num processors: %d\n", m);
        printf("avg: %d\n", avg);
        printf("residual: %d\n", re);
        printf("offset: %d\n", pos);
        printf("buffer size: %d\n", bufferSize);    
        printf("read size: %d\n", readSize);    
        for(int i = 0; i < readSize; i++)
            printf("%.0f ", arr[i]);
        printf("\n\n");
    }
    even_odd();
    if(debug){
        for(int i = 0; i < readSize; i++)
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
int sort(int n, int start){
    int sorted = 1;
    for(int i = start; i < n; i+=2){
        if(arr[i] > arr[i+1]){
            float temp = arr[i];
            arr[i] = arr[i+1];
            arr[i+1] = temp;
            sorted = 0;
        }
    }
    return sorted;
}

// check if all processor sorted
int checkIfSorted(int* checkList){
    for(int i = 0; i < m; i++){
        if(checkList[i] != 1)
            return 0;
    }
    return 1;
}

void even_odd(){
    MPI_Request req;
    MPI_Status status;
    int i = 0;
    while(true){
        // even sort
        int sorted = sort(bufferSize-1, 0);

        // even phase communication
        if(rank > 0)
	        MPI_Isend(arr, 1, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, &req);
        if(rank < m-1){
	        MPI_Recv(arr+bufferSize-1, 1, MPI_FLOAT, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
        if(rank > 0)
            MPI_Wait(&req, &status);

        if(debug){
            for(int i = 0; i < readSize; i++)
                printf("%.0f ", arr[i]);
            printf("\n");
        }
        
        // odd sort
        sorted = sorted && sort(bufferSize-1, 1);

        // odd phase communication
        if(rank < m-1)
	        MPI_Isend(arr+bufferSize-1, 1, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, &req);
        if(rank > 0)
	        MPI_Recv(arr, 1, MPI_FLOAT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(rank < m-1)
            MPI_Wait(&req, &status);

        if(debug){
            for(int i = 0; i < readSize; i++)
                printf("%.0f ", arr[i]);
            printf("\n");
        }
        // check if sorted
        int checkList[m];
        memset(checkList, 1, m);
        checkList[rank] = sorted;
        MPI_Request reqs[m];
        MPI_Status stas[m];
        for(int p = 0; p < m; p++)
	        MPI_Ibcast(&checkList[p], 1, MPI_INT, p, MPI_COMM_WORLD, &reqs[p]);
        for(int p = 0; p < m; p++)
            MPI_Wait(&reqs[p], &stas[p]);

        // check if all processors sorted
        int conti = !checkIfSorted(checkList);
        if(conti)
            continue;
        else
            break;
    }
}
