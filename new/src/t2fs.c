#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/suporte.h"

int identify2 (char *name, int size){
	strncpy(name, "Julio Cesar de Azeredo - 285682\nJuan Suzano da Fonseca - 285689\nPedro Martins Basso - 285683\n", size);
	return 0;
}

FILE2 create2 (char *filename) {
	iniciar_disco();
	if(strcmp(filename,"") ==0 || strcmp(filename,"/") ==0 )
		return -1;
	return createFile(filename);
}

int delete2 (char *filename) {
	iniciar_disco();
	if(strcmp(filename,"") ==0 || strcmp(filename,"/") ==0 )
		return -1;
    return deletar_arquivo(filename);
}

FILE2 open2 (char *filename) {
	iniciar_disco();
	if(strcmp(filename,"") ==0 || strcmp(filename,"/") ==0)
		return -1;
    return openFile(filename);
}

int close2 (FILE2 handle) {
	iniciar_disco();
	if(handle < 0)
	 	return -1;
    return fechar_arquivo(handle);
}

int read2 (FILE2 handle, char *buffer, int size) {
	iniciar_disco();
	if(handle < 0)
	 	return -1;
	return ler_arquivo(handle,buffer,size);
}

int write2 (FILE2 handle, char *buffer, int size) {
	iniciar_disco();
	int bytesWritten;

	if(handle < 0)
	 	return -1;
	bytesWritten = escreve_arquivo(handle,buffer,size);
    return bytesWritten;
}

int truncate2 (FILE2 handle) {
	return -1;
}

int seek2 (FILE2 handle, DWORD offset) {
	return -1;
}

int mkdir2 (char *pathname) {
	iniciar_disco();
	if(strcmp(pathname,"") ==0 )
		return -1;
    return mkdir(pathname);
}

int rmdir2 (char *pathname) {
	return -1;
}

int chdir2 (char *pathname) {
	return -1;
}

int getcwd2 (char *pathname, int size) {
	if((strlen(caminho_atual.absolute) + 1) > size){
		return -1;
	}
	else{
		memset(pathname,'\0',size);
		strcpy(pathname, caminho_atual.absolute);
		return 0;
	}
}

DIR2 opendir2 (char *pathname) {
	iniciar_disco();
	if(strcmp(pathname,"") ==0 )
		return -1;
	return openDir(pathname);
}

int readdir2 (DIR2 handle, DIRENT2 *dentry) {
	return -1;
}

int closedir2 (DIR2 handle) {
	iniciar_disco();
	if(handle < 0)
		return -1;
    return closeDir(handle);
}

int ln2(char *linkname, char *filename) {
	return -1;
}