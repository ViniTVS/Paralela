#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
// macros
#define ALPHABET_LENGTH 4
#define max(x, y) ((x) > (y) ? (x) : (y))
#define MAX_CHAR 256

// global variables
char *string_A;
char *string_B;
char *unique_chars_C; // unique alphabets
int c_len;
int *P_Matrix;
int *DP_Results; // to store the DP values
int *dp_prev_row;

int get_index_of_character(char *str, char x, int len){
    for (int i = 0; i < len; i++){
        if (str[i] == x){
            return i;
        }
    }
    return -1; // not found the character x in str
}


void print_matrix(int *P, int len_b, int len_c){
    printf("\nMatriz:");

    for(int i = 0; i < len_c; i++){
		printf("\n");
        // não faz para j = 0 pois essa coluna possui somente 0s
		for(int j = 0; j < len_b + 1; j++){
			printf("%d ", P[i*(len_b + 1) + j]);
		}
	}
    printf("\n");
}

void calc_P_matrix_v2(int *P, char *b, int len_b, char *c, int len_c, int myrank, int chunk_size, int resto_chunk, int num_procs){
    char receive_array_for_scatter_c[chunk_size];
    int receive_array_for_scatter_p[chunk_size * (len_b + 1)];
    // Scatter the char array chunks by sending each process a particular chunk
    MPI_Scatter(c, chunk_size, MPI_CHAR, &receive_array_for_scatter_c, chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);
    // Scatter the char array chunks by sending each process a particular chunk
    MPI_Scatter(P, chunk_size * (len_b + 1), MPI_INT, &receive_array_for_scatter_p, chunk_size * (len_b + 1), MPI_INT, 0, MPI_COMM_WORLD);
    // Broadcast the whole b  array to everybody
    MPI_Bcast(b, len_b, MPI_CHAR, 0, MPI_COMM_WORLD);
    // parte paralela executada por todas as threads
    int i, j;
    for (i = 0; i < chunk_size; i++){
        for (j = 1; j < len_b + 1; j++){
            if (b[j - 1] == receive_array_for_scatter_c[i]){
                receive_array_for_scatter_p[(i * (len_b + 1)) + j] = j;
            } else {
                receive_array_for_scatter_p[(i * (len_b + 1)) + j] = receive_array_for_scatter_p[(i * (len_b + 1)) + j - 1];
            }
        }
    }

    // now gather all the calculated values of P matrix in process 0
    MPI_Gather(receive_array_for_scatter_p, chunk_size * (len_b + 1), MPI_INT, P, chunk_size * (len_b + 1), MPI_INT, 0, MPI_COMM_WORLD);
    // barreira para garantir que todas as threads chegaram até aqui
    MPI_Barrier(MPI_COMM_WORLD);

    // parte que sobra para computar no último processo 
    if (myrank == 0){        
        for (i = len_c - resto_chunk; i < len_c; i++){
            for (j = 1; j < len_b + 1; j++){
                if (b[j - 1] == c[i]){
                    P[i*(len_b + 1) + j] = j;
                } else {
                    P[i*(len_b + 1) + j] = P[i*(len_b + 1) + j - 1];
                }
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

/*  DP = vetor com resultados,
    prev_row = vetor com resultados anteriores
    P = "matriz" P
    string A 
    string B 
    string C
    m = tam A 
    n = tam B 
    u = tam C
    rank
    chunk_size 
*/ 
int lcs_yang_v2(int *DP, int *prev_row, int *P, char *A, char *B, char *C, int m, int n, int u, int myrank, int chunk_size, int resto_chunk, int num_procs){
    MPI_Bcast(P, (u * (n + 1)), MPI_INT, 0, MPI_COMM_WORLD);
	int *temp;
    int i, j, t, s, c_i;
    // calcula qual parte esta thread deve fazer:
    int start_id = (myrank * chunk_size);
    int end_id = (myrank * chunk_size) + chunk_size;
    int dp_i_receive[chunk_size];

    for (i = 1; i < m + 1; i++){
        // incide do char em C:
        c_i = get_index_of_character(C, A[i - 1], u);
        // vetor que recebe dados:
        //  divide a tarefa entre as threads
        MPI_Scatter(DP, chunk_size, MPI_INT, dp_i_receive, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

        // se j for zero, a gente pula seu cálculo
        if (j == 0)
            j = 1;

        for (j = start_id; j < end_id; j++){
            t = (0 - P[(c_i * (n + 1)) + j]) < 0;
            s = (0 - (prev_row[j] - (t * prev_row[P[(c_i * (n + 1)) + j] - 1])));
            dp_i_receive[j - start_id] = ((t ^ 1) || (s ^ 0)) * (prev_row[j]) + (!((t ^ 1) || (s ^ 0))) * (prev_row[P[(c_i * (n + 1)) + j] - 1] + 1);
        }

        // now gather all the calculated values of P matrix in process 0
        MPI_Allgather(dp_i_receive, chunk_size, MPI_INT, DP, chunk_size, MPI_INT, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        if (myrank == 0){        
            for (j = n + 1 - resto_chunk; j < n + 1; j++){
                if(c_i){
                    DP[j] = max(prev_row[j], prev_row[c_i - 1] + 1);
                } else {
                    // DP[j] = max(prev_row[j], 0);
                    // mas como prev_row[j] >= 0, então basta pegar o da prev_row[j];
                    DP[j] = prev_row[j];
                }
            }
        }
        
        // troca as linhas da coluna
		temp = DP;
		DP = prev_row;
		prev_row = temp;
        // MPI_Barrier(MPI_COMM_WORLD);
    }
    // troca as linhas da coluna
    temp = DP;
    DP = prev_row;
    prev_row = temp;
    return DP[n];
}

char* read_seq(char *fname) {
	//file pointer
	FILE *fseq = NULL;
	//sequence size
	long size = 0;
	//sequence pointer
	char *seq = NULL;
	//sequence index
	int i = 0;

	//open file
	fseq = fopen(fname, "rt");
	if (fseq == NULL ) {
		printf("Error reading file %s\n", fname);
		exit(1);
	}

	//find out sequence size to allocate memory afterwards
	fseek(fseq, 0, SEEK_END);
	size = ftell(fseq);
	rewind(fseq);

	//allocate memory (sequence)
	seq = (char *) calloc(size + 1, sizeof(char));
	if (seq == NULL ) {
		printf("Erro allocating memory for sequence %s.\n", fname);
		exit(1);
	}

	//read sequence from file
	while (!feof(fseq)) {
		seq[i] = fgetc(fseq);
		if ((seq[i] != '\n') && (seq[i] != EOF))
			i++;
	}
	//insert string terminator
	seq[i] = '\0';

	//close file
	fclose(fseq);

	//return sequence pointer
	return seq;
}


/**
 * @brief Cria a string que possui os caracteres únicos presentes nas strings de entrada seqA e seqB.
 * Em vez de percorrer toda a string que está sendo criada a cada vez que temos um caracter candidato
 * a ser incluso, é criado um vetor que possui 0s e 1s para dizer se um caracter está contido em ao
 * menos uma das strings de entrada. Após isso, é criada uma string com as posições do vetor marcadas
 * com 1. 
 * @param seqA String qualquer.
 * @param seqB String qualquer.
 *
 * @return char* String de caracteres únicos.
 */
char *string_char_unicos(char *seqA, char *seqB){
    // Aloca um array com todos os possíveis chars que possamos ter de entrada (2⁸)
    int *charsetC = calloc(MAX_CHAR, sizeof(int));
    int sizeC = 0;
    // leio a seqB e separo seu chars únicos escrevendo 1 em sua posição de mem. equivalente em charsetC
    for(int i = 0; i < strlen(seqA); i++){
        if (charsetC[(int)seqA[i]] == 0){
            sizeC += 1; // contabiliza o num. de chars diferentes existentes 
        }
        charsetC[(int)seqA[i]] = 1;
    }
    // faço o mesmo para a seqB
    for(int i = 0; i < strlen(seqB); i++){
        if (charsetC[(int)seqB[i]] == 0){
            sizeC += 1;
        }
        charsetC[(int)seqB[i]] = 1;
    }

    // aloco espaço para a string C e coloco seus valores (será sempre < 256)
    char *seqC;
    seqC = calloc(sizeC, sizeof(char));
    int j = 0;
    for(int i = 0; i < MAX_CHAR; i++){
        if (charsetC[i] == 1){
            seqC[j] = i;
            j++;
        }
    }

	return seqC;
}

int main(int argc, char *argv[]){
	if (argc < 3){
		printf("Não foram especificados arquivos de entrada.");
		exit(1);
	}
	// if (argc == 4){
	// 	omp_set_num_threads(atoi(argv[3]));
	// }

    int my_rank;
    int num_procs;
    int chunk_size_p, chunk_size_dp; // chunk_size for P matrix and DP matrix
    int res;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);   // grab this process's rank
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // grab the total num of processes

    int len_a, len_b;
    double start_time, stop_time;

    string_A = read_seq(argv[1]);
    string_B = read_seq(argv[2]);
    len_a = strlen(string_A);
    len_b = strlen(string_B);
    unique_chars_C = string_char_unicos(string_A, string_B);
	c_len = strlen(unique_chars_C);
    // printf("Tam c: %d \n", c_len);
    chunk_size_p = (c_len / num_procs);
    int resto_p = (c_len % num_procs);
    chunk_size_dp = ((len_b + 1) / num_procs);
    int resto_dp = ((len_b + 1) % num_procs);

    // if (my_rank == 0){
    //     printf("chunk_p: %d chunk_dp: %d procs: %d\n", chunk_size_p, chunk_size_dp, num_procs);
    // }

    DP_Results = calloc((len_b + 1), sizeof(int));
    dp_prev_row = calloc((len_b + 1), sizeof(int));

    P_Matrix = calloc((c_len * (len_b + 1)), sizeof(int));
    
    start_time = MPI_Wtime();

    calc_P_matrix_v2(P_Matrix, string_B, len_b, unique_chars_C, c_len, my_rank, chunk_size_p, resto_p, num_procs);
    
    
    // MPI_Finalize();
    // if (my_rank == 0){
    //     print_matrix(P_Matrix, len_b, c_len);
    // }
    // return 0;

    res = lcs_yang_v2(DP_Results, dp_prev_row, P_Matrix, string_A, string_B, unique_chars_C, len_a, len_b, c_len, my_rank, chunk_size_dp, resto_dp, num_procs);

    stop_time = MPI_Wtime();

    if (my_rank == 0){
        printf("\nlcs_yang_v2 is: %d\n", res);
        printf("time taken for lcs_yang_v2 is: %lf\n", stop_time - start_time);
    }
    // deallocate pointers
    free(P_Matrix);
    free(DP_Results);
    free(dp_prev_row);

    // Shutdown MPI (important - don't forget!)
    MPI_Finalize();
    return 0;
}
