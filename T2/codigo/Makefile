# Nome do executável
PROG = lcs
# Nome dos arquivos de origem
SRCS = lcs.c 
# Diretório para armazenar arquivos .o
TEMPDIR = temp
# nome dos arquivos .o
OBJS = $(patsubst %.c, $(TEMPDIR)/%.o, $(SRCS)) 
# compilador
CC = gcc
# flags de compilação
CFLAGS = -fopenmp -Wall -O3 

# chamado para fazer compilação
all: $(PROG)

$(OBJS): mkdir $(SRCS)
	$(CC) $(CFLAGS) -c -o $(OBJS) $(SRCS) 
 
# links object files into executable
$(PROG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

# creates directory to store object files
mkdir:
	mkdir -p ./$(TEMPDIR)

# cleans up object files and executable
# type "make clean" to use
# Note: you can add your own commands to remove other things (e.g. output files)
clean:
	rm -rf $(PROG)
	rm -rf $(TEMPDIR)
