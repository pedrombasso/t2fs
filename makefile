CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: suporte t2fs
	ar crs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/suporte.o $(BIN_DIR)/t2fs.o $(LIB_DIR)/apidisk.o

suporte:
	$(CC) -c $(SRC_DIR)/suporte.c -o $(BIN_DIR)/suporte.o -Wall

t2fs:
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o -Wall

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~


