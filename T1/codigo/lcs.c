#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include<time.h>
#include "omp.h"

#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#define MAX_CHAR 256

typedef unsigned short mtype;


typedef union{
    mtype num;
    char chr;
} charToShort;

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
    char *seqC = calloc(sizeC, sizeof(char));
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


void inicia_matriz_P(int *P, char *b, int len_b, char *c, int len_c){
	#pragma omp parallel for
	for(int i = 0; i < len_c; i++){
		// não faz para j = 0 pois essa coluna possui somente 0s
		for(int j = 1; j < len_b + 1; j++){
			if( b[j - 1] == c[i]){
				P[i*(len_b + 1) + j] = j;
			} else {
				P[i*(len_b + 1) + j] = P[i*(len_b + 1) + j - 1];
			}
		}
	}
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
 * @param sizeA Tamanho da string A.
 * @param sizeB Tamanho da string B.
 * @param sizeC Tamanho da string C.
 * @param matrizP Matriz P do algoritmo de Yang et al armazenada em forma de array. Deve ser de tamanho 
 * sizeC * (sizeB + 1).
 * 
 * @return int Tamanho da maior subsequência comum entre seqA e seqB.
 */
int calcula_lcs(int *linha_atual, int *linha_anterior, char *seqA, char *seqB, char *seqC, 
						int sizeA, int sizeB, int sizeC, int *matrizP){
	// O tamanho da matriz R seria de (sizeA + 1) x (sizeB + 1) 
	// Como utilizamos somente a linha atual e a linha anterior daquilo que seria a matriz R,
	// não é necessário alocar ela inteira. Então só usamos estas 2 linhas 
	int *temp;
	// quando i = 0, seu valor é sempre 0
	for(int i = 1; i < sizeA; i++){
		// c denota o índice do char A[i - 1] na string C.
		int c = inidice_em_c( seqC, seqA[i - 1]);
		int p_c_j, j;
		// quando j = 0, seu valor é sempre 0
		#pragma omp parallel for private(p_c_j) schedule(static)
		for(j = 1; j < sizeB + 1; j++){
			p_c_j = matrizP[c*(sizeB + 1) + j];
			if(p_c_j){
				linha_atual[j] = max(linha_anterior[j], linha_anterior[p_c_j - 1] + 1);
			} else {
				// linha_atual[j] = max(linha_anterior[j], 0);
				// mas como linha_anterior[j] >= 0, então basta pegar o da linha_anterior[j];
				linha_atual[j] = linha_anterior[j];
			}
		}
		// Aqui é como se tivesse uma barreira, espera todas as threads acabarem de executar 
		// para eviar uma racing condition
		
		// A linha anterior passa a ser a linha atual e vice-versa. Não tem porblema a linha anterior
		//  ter os mesmos dados da anteriror pois serão sobre-escritos 
		temp = linha_atual;
		linha_atual = linha_anterior;
		linha_anterior = temp;
	}
	// o algoritmo faz uma troca entre as linhas a mais que devia na última iteração, então a corrigimos
	temp = linha_atual;
	linha_atual = linha_anterior;
	linha_anterior = temp;

	return linha_atual[sizeB];
}

int main(int argc, char **argv) {
	double start_time, midi_time, stop_time;

	if (argc < 3){
		printf("Não foram especificados arquivos de entrada.");
		exit(1);
	}
	if (argc == 4){
		omp_set_num_threads(atoi(argv[3]));
	}
	start_time = omp_get_wtime();
	// ponteiros p/ as strings
	char *seqA, *seqB, *seqC;
	// tamanho das strings
	int sizeA, sizeB, sizeC;
	// lê os arquivos especificados na entrada
	seqA = read_seq(argv[1]);
	seqB = read_seq(argv[2]);
	
	sizeA = strlen(seqA);
	sizeB = strlen(seqB);
	
    // Criando string C 
    seqC = string_char_unicos(seqA, seqB);    
    if (!seqC){
        printf("Erro ao alocar matriz string C");
        return 1;
    } 
	sizeC = strlen(seqC);

	int *linha_atual = calloc( sizeB + 1, sizeof(int)); 
	int *linha_anterior = calloc( sizeB + 1, sizeof(int));

	// inicia a matriz P
	int *matrizP = calloc(sizeC * (sizeB + 1), sizeof(int));
	// int sizeP = sizeC * (sizeB + 1);
    if (!matrizP){
        printf("Erro ao alocar matriz R");
        return 1;    
    }

	midi_time= omp_get_wtime();
	inicia_matriz_P(matrizP, seqB, sizeB, seqC, sizeC);
	int res = calcula_lcs(linha_atual, linha_anterior, seqA, seqB, seqC, sizeA, sizeB, sizeC, matrizP);
	stop_time = omp_get_wtime();

	printf("%d\n", res);	
	printf("%lf\n", stop_time - start_time);	
	printf("%lf\n", stop_time - midi_time);	


    return 0;
}
