#include <mpi.h>
#include <stdio.h>
#include <string.h>
#define STD_TAG 0

void main(int argc, char **argv){
    int i, my_rank, n_procs;
    MPI_Status status;
    char msg[100];
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    if(my_rank){
        // envia msg
        sprintf(msg, "I'M ALIVE!");
        MPI_Send(msg, strlen(msg) + 1, MPI_CHAR, 0, STD_TAG, MPI_COMM_WORLD);
        // recebe msg
        MPI_Recv(msg, 100, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("%d recebeu \"%s\" \n", my_rank, msg);
    } else {
        for (i = 1; i < n_procs; i++){
            // recebe msg dos demais processos
            MPI_Recv(msg, 100, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("Proc %d: %s \n", status.MPI_SOURCE, msg);
            // envia resposta
            sprintf(msg, "Obrigado!");
            MPI_Send(msg, strlen(msg) + 1, MPI_CHAR, status.MPI_SOURCE, STD_TAG, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
}