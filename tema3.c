#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NO_OF_CLUSTERS 3

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })



void copySubArray(int *srcArray, int *subArray, int start, int end)
{   
    for (int i = start; i < end; i++){
        subArray[i] = srcArray[i];
    }
}


int main (int argc, char *argv[])
{
    int  numtasks, rank, line_count = 0, i,
        next_rank, vec_dim;
    FILE *fp1, *fp2, *fp0;
    ssize_t read;
    size_t len = 0;
    int coord_rank, j = 0, start, end;
    int* topology[NO_OF_CLUSTERS];
    int* finalizedV;
    long worker_count;
    char *line = malloc(100000000000 * sizeof(char));


    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for(i = 0; i < NO_OF_CLUSTERS; i++) 
        topology[i] = (int*)calloc(numtasks, sizeof(int));
    
    int* curr_workers = calloc(numtasks, sizeof(int));

    //**********************************************Process rank 0
    if (rank == 0) {
        fp0 = fopen("cluster0.txt", "r");
        if(fp0 == NULL)         //Read error
            exit(EXIT_FAILURE);
        /*
        //Read every line from file
        while((read = getline(&line, &len, fp0)) != -1){
            if(line_count != 0 ){
                next_rank = atoi(line);
                curr_workers[next_rank] = 1;                

                //Send to each worker the rank of its coordinator
                MPI_Send(&rank, 1, MPI_INT, next_rank, 1, 
                    MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, next_rank);

                //Create topology for rank 0
                topology[rank][line_count-1] = next_rank;
            }
            else
                worker_count = atoi(line);
            line_count++;
        }
        */

        fscanf(fp0, "%ld", &worker_count);
        //Read every line from file
       for(i = 0; i < worker_count; i++){
            fscanf(fp0, "%d", &next_rank);
            curr_workers[next_rank] = 1;

            //Send to each worker the rank of its coordinator
            MPI_Send(&rank, 1, MPI_INT, next_rank, 1, 
                MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, next_rank);


            //Create topology for rank 0
            topology[rank][i] = next_rank;            
        }

        
        //Send current topology to rank 1
        MPI_Send(curr_workers, numtasks, MPI_INT, 1, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);
        
        //Send current topology to rank 2
        MPI_Send(curr_workers, numtasks, MPI_INT, 2, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);

        
        //Recieve from rank 1 its topology
        MPI_Status status;
        MPI_Recv(curr_workers, numtasks, MPI_INT, 1, 1, MPI_COMM_WORLD,
            &status);

        for(i = 0; i < numtasks; i++){
            if(curr_workers[i] == 1){
                topology[1][j] = i;
                j++;
            }
        }
        
        //Recieve from rank 2 its topology
        MPI_Recv(curr_workers, numtasks, MPI_INT, 2, 1, MPI_COMM_WORLD,
            &status);
        j = 0;
        for(i = 0; i < numtasks; i++){
            if(curr_workers[i] == 1){
                topology[2][j] = i;
                j++;
            }
        }
        
        //Print topology from current process
        printf("%d -> ", rank);
        for(i = 0; i < NO_OF_CLUSTERS; i++){
            printf("%d:", i);
            for(j = 0; j < numtasks; j++){
                if(topology[i][j] == 0)
                    break;
                if(j != 0)
                    printf(",");
                printf("%d", topology[i][j]);
            }
            printf(" ");
        }
        printf("\n");

        
        //Send the topology to the workers that this process is the coordinator
        // of
        for(j = 0; j < worker_count; j++){
            for(i = 0; i < NO_OF_CLUSTERS; i++){
                MPI_Send(topology[i], numtasks, MPI_INT, 
                    topology[0][j], 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", 0, topology[0][j]);    
            }
        } 
        

        fclose(fp0);  
    //*******************************Process rank 1
    } else if(rank == 1){
        fp1 = fopen("cluster1.txt", "r");
        if(fp1 == NULL)         //Read error
            exit(EXIT_FAILURE);
        
        fscanf(fp1, "%ld", &worker_count);
        //Read every line from file
        for(i = 0; i < worker_count; i++){
                fscanf(fp1, "%d", &next_rank);
                curr_workers[next_rank] = 1;

                //Send to each worker the rank of its coordinator
                MPI_Send(&rank, 1, MPI_INT, next_rank, 1, 
                    MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, next_rank);


                //Create topology for rank 1
                topology[1][i] = next_rank;            
        }

        
        //Send current topology to rank 0
        MPI_Send(curr_workers, numtasks, MPI_INT, 0, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);

        //Send current topology to rank 2
        MPI_Send(curr_workers, numtasks, MPI_INT, 2, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);
       
       //Recieve from rank 0 its topology
        MPI_Status status;
        MPI_Recv(curr_workers, numtasks, MPI_INT, 0, 1, MPI_COMM_WORLD,
            &status);
        //printf("* (1) The process %d has received the topology 0\n", rank);
        j = 0;
        for(i = 0; i < numtasks; i++){
            if(curr_workers[i] == 1){
                topology[0][j] = i;
                j++;
            }
        }
        
        //Recieve from rank 2 its topology
        MPI_Recv(curr_workers, numtasks, MPI_INT, 2, 1, MPI_COMM_WORLD,
            &status);
        //printf("* (1) The process %d has received the topology 2\n", rank);
        j = 0;
        for(i = 0; i < numtasks; i++){
            if(curr_workers[i] == 1){
                topology[2][j] = i;
                j++;
            }
        }

        //Print topology from current process
        printf("%d -> ", rank);
        for(i = 0; i < NO_OF_CLUSTERS; i++){
            printf("%d:", i);
            for(j = 0; j < numtasks; j++){
                if(topology[i][j] == 0)
                    break;
                if(j != 0)
                    printf(",");
                printf("%d", topology[i][j]);
            }
            printf(" ");
        }
        printf("\n");

        
        //Send the topology to the workers that this process is the coordinator
        // of
        for(j = 0; j < worker_count; j++){
            for(i = 0; i < NO_OF_CLUSTERS; i++){
                MPI_Send(topology[i], numtasks, MPI_INT, 
                    topology[1][j], 1, MPI_COMM_WORLD);        
                printf("M(%d,%d)\n", rank, topology[1][j]);
            }    
        }  
        
        fclose(fp1);

    //------------------------------Process rank 2-----------------------------
    } else if(rank == 2){
        fp2 = fopen("cluster2.txt", "r");
        if(fp2 == NULL)         //Read error
            exit(EXIT_FAILURE);

        //Read every line from file
       fscanf(fp2, "%ld", &worker_count);
       for(i = 0; i < worker_count; i++){
                fscanf(fp2, "%d", &next_rank);
                curr_workers[next_rank] = 1;

                //Send to each worker the rank of its coordinator
                MPI_Send(&rank, 1, MPI_INT, next_rank, 1, 
                    MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, next_rank);

                //Create topology for rank 2
                topology[2][i] = next_rank;            
        }

        
        //Send current topology to rank 0
        MPI_Send(curr_workers, numtasks, MPI_INT, 0, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
 
        //Send current topology to rank 1
        MPI_Send(curr_workers, numtasks, MPI_INT, 1, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);
       
       //Recieve from rank 0 its topology
        MPI_Status status;
        MPI_Recv(curr_workers, numtasks, MPI_INT, 0, 1, MPI_COMM_WORLD,
            &status);
        //printf("* (2) The process %d has received the topology 0\n", rank);
        for(i = 0; i < numtasks; i++){
            if(curr_workers[i] == 1){
                topology[0][j] = i;
                j++;
            }
        }

        //Recieve from rank 1 its topology
        MPI_Recv(curr_workers, numtasks, MPI_INT, 1, 1, MPI_COMM_WORLD,
            &status);
        j = 0;
        for(i = 0; i < numtasks; i++){
            if(curr_workers[i] == 1){
                topology[1][j] = i;
                j++;
            }
        }

        //Print topology from current process
        printf("%d -> ", rank);
        for(i = 0; i < NO_OF_CLUSTERS; i++){
            printf("%d:", i);
            for(j = 0; j < numtasks; j++){
                if(topology[i][j] == 0)
                    break;
                if(j != 0)
                    printf(",");
                printf("%d", topology[i][j]);
            }
            printf(" ");
        }
        printf("\n");

        
        //Send the topology to the workers that this process is the coordinator
        // of
        for(j = 0; j < worker_count; j++){
            if(topology[2][j] == 0)
                break;

            for(i = 0; i < NO_OF_CLUSTERS; i++){
                MPI_Send(topology[i], numtasks, MPI_INT, 
                    topology[2][j], 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", 2, topology[2][j]);
            }      
        }

        fclose(fp2);
    //Workers
    } else {
        
        for(i = 0; i < NO_OF_CLUSTERS; i++) 
            topology[i] = (int*)calloc(numtasks, sizeof(int));
        MPI_Status status;

        //Recieve the rank of its coordinator
        MPI_Recv(&coord_rank, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD,
            &status);
        
        
        //Recieve topology from coordinator
        for(i = 0; i < NO_OF_CLUSTERS; i++){
            MPI_Recv(topology[i], numtasks, MPI_INT, MPI_ANY_SOURCE, 1,
                MPI_COMM_WORLD, &status);
        }    
        
        //Print topology from current process
        printf("%d -> ", rank);
        for(i = 0; i < 3; i++){
            printf("%d:", i);
            for(j = 0; j < numtasks; j++){
                if(topology[i][j] == 0)
                    break;
                if(j != 0)
                    printf(",");
                printf("%d", topology[i][j]);
            }
            printf(" ");
        }
        printf("\n"); 
        
    }
    

    MPI_Barrier(MPI_COMM_WORLD);
    //--------------------------------PART 1-----------------------------------

    if(rank == 0){

        char *first_arg = argv[1];
        vec_dim = atoi(first_arg);  
        int* V = calloc(vec_dim, sizeof(int));
        finalizedV = calloc(vec_dim, sizeof(int));

        //Generate V vector
        for(i = 0; i < vec_dim; i++){
            V[i] = i;
        }

        //Send each worker the vector and the interval that it has to double
        for(i = 0; i < worker_count; i++){
            int start = (topology[rank][i] - 3)* (double)vec_dim / (numtasks-3);
            int end = min((topology[rank][i] - 2) * (double)vec_dim / (numtasks-3),
                vec_dim);

            //Send start position to worker
            MPI_Send(&start, 1, MPI_INT, topology[rank][i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);

            //Send end position to worker
            MPI_Send(&end, 1, MPI_INT, topology[rank][i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);

            //printf("(to:%d) s-%d e-%d \n", topology[rank][i], start, end);

            //Send subarray to worker
            MPI_Send(V+start, end-start, MPI_INT, topology[rank][i], 2, 
                MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);

            //Get doubled portion of vector
            MPI_Recv(V+start, end-start, MPI_INT, topology[rank][i], 2, MPI_COMM_WORLD,
                &status);

            /*
            printf("(0) aVec from %d: ", topology[rank][i]);
            for(int k = 0; k < vec_dim ;k++)
                printf("%d ", V[k]);
            printf("\n");
            */

            copySubArray(V, finalizedV, start, end);
        }
        int worker_count1 = 0;
        int worker_count2 = 0;
        while(1){
            if(topology[1][worker_count1] == 0)
                break;
            worker_count1++;
        }
        while(1){
            if(topology[2][worker_count2] == 0)
                break;
            worker_count2++;
        }
        //printf("*********wks %d, %d\n", worker_count1, worker_count2);
    
        //Send and recieve vector  to rank 1
        for(i = 0; i < worker_count1; i++){
            start = (topology[1][i] - 3)* (double)vec_dim / (numtasks-3);
            end = min((topology[1][i] - 2) * (double)vec_dim / (numtasks-3),
                vec_dim);

            MPI_Send(&start, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);

            MPI_Send(&end, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);
            
            MPI_Send(V+start, end-start, MPI_INT, 1, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);

            //Recieve finalized vector from rank 1
            MPI_Recv(V+start, end-start, MPI_INT, 1, 2, MPI_COMM_WORLD, &status);

            /*
            printf("(0) 1Vec from %d: ", topology[1][i]);
            for(int k = 0; k < vec_dim ;k++)
                printf("%d ", V[k]);
            printf("\n");
            */

            copySubArray(V, finalizedV, start, end);
        }

        //Send and recieve vector  to rank 2
        for(i = 0; i < worker_count2; i++){
            start = (topology[2][i] - 3)* (double)vec_dim / (numtasks-3);
            end = min((topology[2][i] - 2) * (double)vec_dim / (numtasks-3),
                vec_dim);

            MPI_Send(&start, 1, MPI_INT, 2, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);

            MPI_Send(&end, 1, MPI_INT, 2, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);
            
            MPI_Send(V+start, end-start, MPI_INT, 2, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);

            //Recieve finalized vector from rank 2
            MPI_Recv(V+start, end-start, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);

            /*
            printf("(0) 2Vec from %d: ", topology[2][i]);
            for(int k = 0; k < vec_dim ;k++)
                printf("%d ", V[k]);
            printf("\n");
            */

            copySubArray(V, finalizedV, start, end);
        }
        
        free(V);

    } else if((rank == 1) || (rank == 2)){

        for(i = 0; i < worker_count; i++){
            //Recieve from rank 0 start, end and vector
            MPI_Recv(&start, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

            MPI_Recv(&end, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

            int* V = calloc(end-start, sizeof(int));


            MPI_Recv(V, end-start, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
            //printf("(%d) recv from 0 s-e: %d - %d, w %d\n", rank, start, end, topology[rank][i]);
            
            //Send to worker start end and vector
            MPI_Send(&start, 1, MPI_INT, topology[rank][i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);

            MPI_Send(&end, 1, MPI_INT, topology[rank][i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);
            
            MPI_Send(V, end-start, MPI_INT, topology[rank][i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);

            //Recieve doubled vector from worker
            MPI_Recv(V, end-start, MPI_INT, topology[rank][i], 2, MPI_COMM_WORLD, &status);
            
            //Send to rank 0 finalized vector
            MPI_Send(V, end-start, MPI_INT, 0, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);

            free(V);
        }
        
    } else{

        //Recieve start of vector from coordinator
        MPI_Recv(&start, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD,
            &status);

        //Recieve end of vector from coordinator
        MPI_Recv(&end, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD,
            &status);

        //printf("(%d) recv from coord s-e: %d - %d\n", rank, start, end);

        int* subV = calloc(end-start, sizeof(int));
        
        //Recieve vector from coordinator
        MPI_Recv(subV, end - start, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD,
            &status);
        

        //Double each element in vector;
        for(i = 0; i < end-start; i++){
            //printf("%d ",subV[i]);
            subV[i] *= 2;
        }
        //printf("\n");
        
        //Send doubled vector back to coordinator
        MPI_Send(subV, end - start, MPI_INT, coord_rank, 2, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, coord_rank);
        
        free(subV);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0){
        //Print finalized vector
        printf("Rezultat: ");
        for(i = 0; i < vec_dim; i++){
            printf("%d ", finalizedV[i]);
        }
        printf("\n");
        
        
        free(finalizedV);
    }
    
    for (i = 0; i < NO_OF_CLUSTERS; i++)
        free(topology[i]);
    free(line);
    line_count = 0;
    free(curr_workers);

    MPI_Finalize();  
}


