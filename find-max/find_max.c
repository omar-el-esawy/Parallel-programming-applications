#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define max(a,b) (((a)>(b))?(a):(b))





int main(int argc, char* argv[])
{

	int processID, numOfProcess,
		elements_per_process,
		n_elements_recieved;
	// numOfProcess -> number of processes
	// processID -> process id

	MPI_Status status;

	// Creation of parallel processes
	MPI_Init(&argc, &argv);

	// find out process ID,
	MPI_Comm_rank(MPI_COMM_WORLD, &processID);
    // and how many processes were started
    MPI_Comm_size(MPI_COMM_WORLD, &numOfProcess);
	// master process
	if (processID == 0) {
        printf("Hello from master process.\n\n");
        printf("Number of slave processes is %d\n",numOfProcess);
        int n ;
        printf("Please enter size of array...\n");
        fflush(stdout);
        scanf("%d",&n);
		printf("\n");
        printf("Please enter array elements ...\n");
        fflush(stdout);
        int *arr = (int*)malloc(n* sizeof(int));
        
        int i;
        for(i =0;i<n;i++)
            scanf("%d",&arr[i]);
		printf("\n");
            
            
        
        int index;
        
		elements_per_process = n / numOfProcess;
		// check if more than 1 processes are run
		if (numOfProcess > 1) {
			// distributes the portion of array
			// to child processes to calculate
			// their partial maxes
			for (i = 1; i < numOfProcess - 1; i++) {
				index = i * elements_per_process;

				MPI_Send(&elements_per_process,1, MPI_INT, i, 0,MPI_COMM_WORLD);
				MPI_Send(&arr[index],elements_per_process,MPI_INT, i, 0,MPI_COMM_WORLD);
			}

			// last process adds remaining elements
			index = i * elements_per_process;
			int elements_left = n - index;

			MPI_Send(&elements_left,1, MPI_INT,i, 0,MPI_COMM_WORLD);
			MPI_Send(&arr[index],elements_left,MPI_INT, i, 0,MPI_COMM_WORLD);
		}

		// master process add its own max of sub array
        int ind; 
		int currentMAX = -1000000000;
		for (i = 0; i < elements_per_process; i++)
			if(currentMAX < arr[i])
                currentMAX = arr[i],ind =i;
            
        printf("Hello from slave#%d Max number in my partiton is %d and index is %d\n",processID+1,currentMAX,ind);
        fflush(stdout);
        // collects partial max from other processes
		int tempMax;
        int tempID;
		for (i = 1; i < numOfProcess; i++) {
			MPI_Recv(&tempMax, 1, MPI_INT,i, 0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

			MPI_Recv(&tempID, 1, MPI_INT,i, 0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			
            if(currentMAX < tempMax){
                currentMAX = tempMax;
                ind = ((i)*elements_per_process)+tempID;
            }
		}

		// prints the final max of array
        printf("Master process announce the final max which is %d and its index is %d.\n\n",currentMAX,ind);
        printf("Thanks for using our program\n");
        fflush(stdout);
    }
	else {
		MPI_Recv(&n_elements_recieved,1, MPI_INT, 0, 0,MPI_COMM_WORLD,&status);

		// stores the received array segment
		// in local array a2
        int *a2 = (int*)malloc(n_elements_recieved* sizeof(int));
        MPI_Recv(a2, n_elements_recieved,MPI_INT, 0, 0,MPI_COMM_WORLD,&status);

		// calculates its partial max
        int index=0;
		int currentMAX = -1000000000;
		int i ; 
		for (i = 0; i < n_elements_recieved; i++)
            if(currentMAX < a2[i])currentMAX = a2[i],index = i ;

		// sends the partial max to the root process
        printf("Hello from slave#%d Max number in my partiton is %d and index is %d.\n",processID+1,currentMAX,index);
		MPI_Send(&currentMAX, 1, MPI_INT,0, 0, MPI_COMM_WORLD);
		MPI_Send(&index, 1, MPI_INT,0, 0, MPI_COMM_WORLD);
	}

	// cleans up all MPI state before exit of process
	MPI_Finalize();

	return 0;
}
