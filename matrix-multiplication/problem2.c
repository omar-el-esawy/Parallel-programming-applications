//make sure to import these libraries to avoid errors.
#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_2d(int **ary,int row,int col)
{
    for (int i = 0; i <row; ++i) {
        for (int j = 0; j < col; ++j) {
            printf("%d ",ary[i][j]);
        }
        printf("\n");
    }
}
int main(int argc, char** argv) {

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    //make variables for all processes.
    int rows1,cols1, rows2, cols2; int **mat1, **mat2;

    // take variable input at master process.
    if(rank==0)
    {

        int choice;
        printf("Welcome to vector Matrix multiplication program!\n");
        printf("To read dimensions and values from file press 1\n");
        printf("To read dimensions and values from console press 2\n");
        scanf("%d",&choice);
        fflush(stdout);
        while (choice!=1 &&choice!=2){
            printf("Incorrect\n");
            printf("To read dimensions and values from file press 1\n");
            printf("To read dimensions and values from console press 2\n");
            fflush(stdout);
            scanf("%d",&choice);

        }
        fflush(stdout); //flush after printing so it appeares before input.
        FILE *file;
        if(choice ==1){
            file = fopen("problem2.txt", "r");
        }


        if(choice == 2) {
            printf("Please enter dimensions of the first matrix: ");
            fflush(stdout); //flush after printing so it appeares before input.
        }
        if(choice==1 )
            fscanf(file, "%d %d",&rows1,&cols1);
        else
            scanf("%d %d",&rows1,&cols1);

        //building the 2d array.
        mat1 = (int**)malloc(rows1 * sizeof(int*));
        for (int i = 0; i < rows1; i++)
            mat1[i] = (int*)malloc(cols1 * sizeof(int));

        if(choice == 2) {
            //taking array elements as input.
            printf("Please enter its elements:\n");
            fflush(stdout);
        }

        for (size_t i = 0; i < rows1; i++)
        {
            for (size_t j = 0; j < cols1; j++)
            {
                if(choice==1)
                    fscanf(file, "%d",&mat1[i][j]);
                else
                    scanf("%d",&mat1[i][j]);
            }

        }
        printf("\n");


        if(choice == 2) {
            printf("Please enter dimensions of the second matrix: ");
            fflush(stdout); //flush after printing so it appeares before input.
        }
        while (1){
            if(choice==1)
                fscanf(file, "%d %d",&rows2,&cols2);
            else
                scanf("%d %d",&rows2,&cols2);
            if(cols1 == rows2){
                break;
            }
            printf("Please Enter a valid matrices such as (r1 c1 X c2 r2) && c1 == c2:\n");
            fflush(stdout); //flush after printing so it appeares before input.
        }
        //building the 2d array.
        mat2 = (int**)malloc(rows2 * sizeof(int*));
        for (int i = 0; i < rows2; i++)
            mat2[i] = (int*)malloc(cols2 * sizeof(int));


        if(choice == 2) {
            //taking array elements as input.
            printf("Please enter its elements:\n");
            fflush(stdout);
        }
        for (size_t i = 0; i < rows2; i++)
        {
            for (size_t j = 0; j < cols2; j++)
            {
                if(choice==1)
                    fscanf(file, "%d",&mat2[i][j]);
                else
                    scanf("%d",&mat2[i][j]);
            }

        }
        printf("\n");



        //sending rows and columns to all processes.
        for (size_t i = 1; i < size; i++)
        {
            MPI_Send( &rows2 , 1 , MPI_INT, i ,0, MPI_COMM_WORLD);
            MPI_Send( &cols2 , 1 , MPI_INT, i ,0, MPI_COMM_WORLD);
        }

        //because you can't send a 2d array in one go
        //you will send each individual row in the array.

        for (size_t i = 1; i < size; i++) //loops and sends to each other proccess.
        {
            for (size_t j = 0; j < rows2; j++)//loops through the rows of the 2d array and sends each row.
            {
                MPI_Send(mat2[j], cols2, MPI_INT, i, 0, MPI_COMM_WORLD); //send (1d array of current row, which has a size = number of columns).
            }
        }
        int rowsTaken = rows1 / size, i = 1, j=rowsTaken, k;
        if(size > 1){
            for (i = 1; i < size - 1; ++i) {
                MPI_Send(&rowsTaken, 1 , MPI_INT, i ,0, MPI_COMM_WORLD);
                MPI_Send( &cols1 , 1 , MPI_INT, i ,0, MPI_COMM_WORLD);
                for (j = rowsTaken * i; j < rowsTaken * (i + 1); ++j) {
                    MPI_Send(mat1[j], cols1, MPI_INT, i, 0, MPI_COMM_WORLD); //send (1d array of current row, which has a size = number of columns).
                }
            }
            int reminder = rows1 - j;
            MPI_Send(&reminder, 1 , MPI_INT, i ,0, MPI_COMM_WORLD);
            MPI_Send( &cols1 , 1 , MPI_INT, i ,0, MPI_COMM_WORLD);
            for(; j < rows1; j++){
                MPI_Send(mat1[j], cols1, MPI_INT, i, 0, MPI_COMM_WORLD); //send (1d array of current row, which has a size = number of columns).
            }
        }

        int **result;
        //now you need to build the array again for each individual proccess.
        result = (int**)malloc(rows1 * sizeof(int*));
        for (i = 0; i < rows1; i++)
            result[i] = (int*)malloc(cols2 * sizeof(int));

        for(i = 0; i < rowsTaken; i++){
            for(j = 0; j < cols2; j++){
                int sum = 0;
                for(k = 0; k < cols1; k++){
                    sum += mat1[i][k] * mat2[k][j];
                }
                result[i][j] = sum;
            }
        }

        int id;
        int cur = rowsTaken;
        for (id = 1; id < size ; id++) {
            int recRows;
            MPI_Recv(&recRows, 1, MPI_INT, id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(i = 0; i < recRows; i++){
                MPI_Recv(result[cur], cols1, MPI_INT, id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                cur++;
            }

        }
        //we are printing the result of mat1 X mat2
        printf("Result Matrix is (%dx%d):\n", rows1, cols2);
        print_2d(result, rows1, cols2);
    }
    else
    {
        //now each other process receives the values of rows and columns.
        MPI_Recv(&rows2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&cols2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //now you need to build the array again for each individual proccess.
        mat2 = (int**)malloc(rows2 * sizeof(int*));
        for (int i = 0; i < rows2; i++)
            mat2[i] = (int*)malloc(cols2 * sizeof(int));

        //now we are going to receive the value of each row in the array in order to form the 2d array.
        for (size_t j = 0; j <rows2; j++)
        {
            MPI_Recv(mat2[j], cols2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        int rowsTaken;
        //now each other process receives the values of rows and columns.
        MPI_Recv(&rowsTaken, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&cols1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //now you need to build the array again for each individual proccess.
        mat1 = (int**)malloc(rowsTaken * sizeof(int*));
        for (int i = 0; i < rowsTaken; i++)
            mat1[i] = (int*)malloc(cols1 * sizeof(int));

        for (size_t j = 0; j <rowsTaken; j++)
        {
            MPI_Recv(mat1[j], cols1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }


        int **result;
        //now you need to build the array again for each individual proccess.
        result = (int**)malloc(rowsTaken * sizeof(int*));
        for (int i = 0; i < rowsTaken; i++)
            result[i] = (int*)malloc(cols2 * sizeof(int));


        int i, j, k;
        for(i = 0; i < rowsTaken; i++){
            for(j = 0; j < cols2; j++){
                int sum = 0;
                for(k = 0; k < cols1; k++){
                    sum += mat1[i][k] * mat2[k][j];
                }
                result[i][j] = sum;
            }
        }

        MPI_Send( &rowsTaken , 1 , MPI_INT, 0 ,0, MPI_COMM_WORLD);
        for (j = 0; j < rowsTaken; j++)//loops through the rows of the 2d array and sends each row.
        {
            MPI_Send(result[j], cols2, MPI_INT, 0, 0, MPI_COMM_WORLD); //send (1d array of current row, which has a size = number of columns).
        }

    }



    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}
