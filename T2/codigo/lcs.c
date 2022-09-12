#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>

#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#define MAX_CHAR 256

typedef unsigned short mtype;

// #define DEBUGMATRIX

/* Read sequence from a file to a char vector.
 Filename is passed as parameter */

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
    int len_c = 0;
    // leio a seqB e separo seu chars únicos escrevendo 1 em sua posição de mem. equivalente em charsetC
    for(int i = 0; i < strlen(seqA); i++){
        if (charsetC[(int)seqA[i]] == 0){
            len_c += 1; // contabiliza o num. de chars diferentes existentes 
        }
        charsetC[(int)seqA[i]] = 1;
    }
    // faço o mesmo para a seqB
    for(int i = 0; i < strlen(seqB); i++){
        if (charsetC[(int)seqB[i]] == 0){
            len_c += 1;
        }
        charsetC[(int)seqB[i]] = 1;
    }

    // aloco espaço para a string C e coloco seus valores (será sempre < 256)
    char *seqC = calloc(len_c, sizeof(char));
    int j = 0;
    for(int i = 0; i < MAX_CHAR; i++){
        if (charsetC[i] == 1){
            seqC[j] = i;
            j++;
        }
    }

	return seqC;
}


int inidice_em_c(char *str_c,char chr){
	for(int i = 0; i < strlen(str_c); i++){
		if(str_c[i] == chr){
			return i;
		}
	}
	// não encontra char na string c
	return -1;
}


void inicia_matriz_P(int *P, char *b, int len_b, char *c, int len_c, int myrank, int chunk, int resto_chunk){
	char string_c_scatter[chunk];
    int vetor_p_scatter[chunk * (len_b + 1)];
    // Scatter the char array chunks by sending each process a particular chunk
    MPI_Scatter(c, chunk, MPI_CHAR, &string_c_scatter, chunk, MPI_CHAR, 0, MPI_COMM_WORLD);
    // Divide partes de P igualmente entre as threads
    MPI_Scatter(P, chunk * (len_b + 1), MPI_INT, &vetor_p_scatter, chunk * (len_b + 1), MPI_INT, 0, MPI_COMM_WORLD);
    // Compartilha a string B para todas as threads
    MPI_Bcast(b, len_b, MPI_CHAR, 0, MPI_COMM_WORLD);
    // parte paralela 
    int i, j;
    for (i = 0; i < chunk; i++){
        for (j = 1; j < len_b + 1; j++){
            if (b[j - 1] == string_c_scatter[i]){
                vetor_p_scatter[i*(len_b + 1) + j] = j;
            } else {
                vetor_p_scatter[i*(len_b + 1) + j] = vetor_p_scatter[i*(len_b + 1) + j - 1];
            }
        }
    }

    // agrega todos os valores de P na thread 0
    MPI_Gather(vetor_p_scatter, chunk * (len_b + 1), MPI_INT, P, chunk * (len_b + 1), MPI_INT, 0, MPI_COMM_WORLD);
    // barreira para garantir que todas as threads chegaram até aqui
    MPI_Barrier(MPI_COMM_WORLD);

    // parte que sobra para computar por último no processo 0 
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

/**
 * @brief Algoritmo para cálculo do Problema da Maior Subsequência Comum utilizando como base o algoritmo
 * desenvolvido por Yang et al. Durante a execução os vetores de entrada linha_atual e linha_anterior
 * passam a ter novos valores escritos.
 *  
 * @param linha_atual Vetor para armazenar os valores da linha anterior da matriz. Deve ter seus valores
 * iniciais zerados.
 * @param linha_anterior Vetor para armazenar os valores da linha atual da matriz. Deve ter seus valores
 * iniciais zerados.
 * @param seqA String A do algoritmo de Yang et al.
 * @param seqB String B do algoritmo de Yang et al.
 * @param seqC String C do algoritmo de Yang et al de caracteres únicos presentes em seqA e seqB.
 * @param len_a Tamanho da string A.
 * @param len_b Tamanho da string B.
 * @param len_c Tamanho da string C.
 * @param matrizP Matriz P do algoritmo de Yang et al armazenada em forma de array. Deve ser de tamanho 
 * len_c * (len_b + 1).
 * 
 * @return int Tamanho da maior subsequência comum entre seqA e seqB.
 */
int calcula_lcs(int *linha_atual, int *linha_anterior, char *seqA, char *seqB, char *seqC, 
						int len_a, int len_b, int len_c, int *matrizP, int myrank, int chunk_size, int resto_chunk){

	// compartiha entre as threads a matriz P
	MPI_Bcast(matrizP, (len_c*(len_b + 1)), MPI_INT, 0, MPI_COMM_WORLD);
	int *temp;
    int i;
    // calcula qual parte esta thread deve fazer:
    int start_id = (myrank * chunk_size);
    int end_id = (myrank * chunk_size) + chunk_size;
	// variável que cada thread terá p/ guardar a parte que será calculada
    int dp_i_receive[chunk_size];
	// O tamanho da matriz R seria de (len_a + 1) x (len_b + 1) 
	// Como utilizamos somente a linha atual e a linha anterior daquilo que seria a matriz R,
	// não é necessário alocar ela inteira. Então só usamos estas 2 linhas 
	
	// quando i = 0, seu valor é sempre 0
	for(i = 1; i < len_a; i++){
		// c denota o índice do char A[i - 1] na string C.
		int c = inidice_em_c( seqC, seqA[i - 1]);
		int p_c_j, j;
		// quando j = 0, seu valor é sempre 0
        //  divide a tarefa entre as threads
        MPI_Scatter(linha_atual, chunk_size, MPI_INT, dp_i_receive, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
        // MPI_Barrier(MPI_COMM_WORLD);
		j = start_id;
		// não precisa calcular j = 0
		if (j == 0)
			j = 1;
		
		for(; j < end_id; j++){
			p_c_j = matrizP[c*(len_b + 1) + j];
			if(p_c_j){
				dp_i_receive[j - start_id] = max(linha_anterior[j], linha_anterior[p_c_j - 1] + 1);
			} else {
				dp_i_receive[j - start_id] = linha_anterior[j];
			}
		}
		// Aqui é como se tivesse uma barreira, espera todas as threads acabarem de executar 
		// para eviar uma racing condition
        MPI_Allgather(dp_i_receive, chunk_size, MPI_INT, linha_atual, chunk_size, MPI_INT, MPI_COMM_WORLD);
		
        if (myrank == 0){
            for (j = len_b + 1 - resto_chunk; j < len_b + 1; j++){
                p_c_j = matrizP[c*(len_b + 1) + j];
                if(p_c_j){
                    linha_atual[j] = max(linha_anterior[j], linha_anterior[p_c_j - 1] + 1);
                } else {
                    // linha_atual[j] = max(linha_anterior[j], 0);
                    // mas como linha_anterior[j] >= 0, então basta pegar o da linha_anterior[j];
                    linha_atual[j] = linha_anterior[j];
                }
            }
        }
		// A linha anterior passa a ser a linha atual e vice-versa. Não tem porblema a linha anterior
		//  ter os mesmos dados da anteriror pois serão sobre-escritos 
		temp = linha_atual;
		linha_atual = linha_anterior;
		linha_anterior = temp;
		// espera todos os processos realizarem a troca das linhas para garantir a informação correta
        MPI_Barrier(MPI_COMM_WORLD);
	}
	// o algoritmo faz uma troca entre as linhas a mais que devia na última iteração, então a corrigimos
	temp = linha_atual;
	linha_atual = linha_anterior;
	linha_anterior = temp;

	return linha_atual[len_b];
}

int main(int argc, char **argv) {
	double start_time, midi_time, stop_time;

	if (argc < 3){
		printf("Não foram especificados arquivos de entrada.");
		exit(1);
	}
	
	start_time = MPI_Wtime();
	// ponteiros p/ as strings
	char *seqA, *seqB, *seqC;
	// tamanho das strings
	int len_a, len_b, len_c;
	// lê os arquivos especificados na entrada
	seqA = read_seq(argv[1]);
	seqB = read_seq(argv[2]);
	
	len_a = strlen(seqA);
	len_b = strlen(seqB);
	
    // Criando string C 
    seqC = string_char_unicos(seqA, seqB);    
    if (!seqC){
        printf("Erro ao alocar matriz string C");
        return 1;
    } 
	len_c = strlen(seqC);

	int *linha_atual = calloc( len_b + 1, sizeof(int)); 
	int *linha_anterior = calloc( len_b + 1, sizeof(int));
    if (linha_atual == NULL || linha_anterior == NULL){
        printf("Erro ao alocar linhas de R");
        return 1;
    }
	// inicia a matriz P
	int *matrizP = calloc(len_c * (len_b + 1), sizeof(int));
	// int sizeP = len_c * (len_b + 1);
    if (!matrizP){
        printf("Erro ao alocar matriz P");
        return 1;    
    }

    int my_rank, num_procs; // chunk_size for P matrix and linha_atual matrix
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);   // grab this process's rank
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // grab the total num of processes

    int chunk_size_p = (len_c / num_procs);
    int resto_p = (len_c % num_procs);
    int chunk_r = ((len_b + 1) / num_procs);
    int resto_r = ((len_b + 1) % num_procs);

	midi_time = MPI_Wtime();
	inicia_matriz_P(matrizP, seqB, len_b, seqC, len_c, my_rank, chunk_size_p, resto_p);

	int res = calcula_lcs(linha_atual, linha_anterior, seqA, seqB, seqC, len_a, len_b, len_c, matrizP, my_rank, chunk_r, resto_r);
	stop_time = MPI_Wtime();
	if (my_rank == 0){
		printf("%d\n", res);	
		printf("%lf\n", stop_time - start_time);	
		printf("%lf\n", stop_time - midi_time);	
	}
	free(linha_atual);
	free(linha_anterior);
	free(matrizP);

	MPI_Finalize();
    return 0;
}
