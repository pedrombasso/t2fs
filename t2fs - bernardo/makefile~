#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
# 

CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/

all: t2fs.o 
	 gcc -o t2fs.exe t2fs.o $(LIB_DIR)apidisk.o $(LIB_DIR)bitmap2.o
	 
t2fs.o: $(SRC_DIR)t2fs.c
		gcc -c $(SRC_DIR)t2fs.c

clean:
	rm -rf t2fs.o


