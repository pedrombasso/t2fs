#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
# 

#NESTE MAKE FILE 
#regras all e clean dizem respeito a criacao e exclusao de arquivos realtivos a criacao da lib 
#regras teste e clean-test dizem respeito a criacao e exclusao de arquivos relativos aos testes
#
#
CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/
TES_DIR=./teste/

EXEMP_DIR=./exemplo/
LIB_NAME = libt2fs.a

all:	lib
	

lib:	t2fs.o
	ar crs $(LIB_DIR)libt2fs.a $(BIN_DIR)libt2fs.o $(LIB_DIR)apidisk.o  $(LIB_DIR)bitmap2.o 

t2fs.o: 
	$(CC) -c $(SRC_DIR)t2fs.c -o $(BIN_DIR)libt2fs.o   -Wall


tests:	test1 test2 test3 test4 test5 test6 test7 test8 cp-disk-exemplos

test1: 
	$(CC) -o $(EXEMP_DIR)test1 $(TES_DIR)delete2.c -L$(LIB_DIR) -lt2fs 
test2:
	$(CC) -o $(EXEMP_DIR)test2 $(TES_DIR)DezDiretorios.c -L$(LIB_DIR) -lt2fs
test3: 
	$(CC) -o $(EXEMP_DIR)test3 $(TES_DIR)diretorios.c -L$(LIB_DIR) -lt2fs
test4: 
	$(CC) -o $(EXEMP_DIR)test4 $(TES_DIR)RW1to100.c -L$(LIB_DIR) -lt2fs
test5: 
	$(CC) -o $(EXEMP_DIR)test5 $(TES_DIR)escritaRandomica.c -L$(LIB_DIR) -lt2fs
test6: 
	$(CC) -o $(EXEMP_DIR)test6 $(TES_DIR)main.c -L$(LIB_DIR) -lt2fs
test7: 
	$(CC) -o $(EXEMP_DIR)test7 $(TES_DIR)truncate.c -L$(LIB_DIR) -lt2fs
test8: 
	$(CC) -o $(EXEMP_DIR)test8 $(TES_DIR)WRnumMaxBlocks.c -L$(LIB_DIR) -lt2fs
cp-disk-exemplos:
	cp t2fs_disk.dat $(EXEMP_DIR)
clean-tests:
		rm -rf $(TES_DIR)*.o
		rm -rf $(EXEMP_DIR)*

clean:
	rm $(BIN_DIR)libt2fs.o
	rm $(LIB_DIR)libt2fs.a
