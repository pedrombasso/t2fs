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
	init_disk();
	if(strcmp(filename,"") ==0 || strcmp(filename,"/") ==0 )
		return -1;
	return createFile(filename);
}

int delete2 (char *filename) {
	init_disk();
	if(strcmp(filename,"") ==0 || strcmp(filename,"/") ==0 )
		return -1;
    return deleteFile(filename);
}

FILE2 open2 (char *filename) {
	init_disk();
	if(strcmp(filename,"") ==0 || strcmp(filename,"/") ==0)
		return -1;
    return openFile(filename);
}

int close2 (FILE2 handle) {
	init_disk();
	if(handle < 0)
	 	return -1;
    return closeFile(handle);
}

int read2 (FILE2 handle, char *buffer, int size) {
	init_disk();
	if(handle < 0)
	 	return -1;
	return readFile(handle,buffer,size);
}

int write2 (FILE2 handle, char *buffer, int size) {
	init_disk();
	int bytesWritten;

	if(handle < 0)
	 	return -1;
	bytesWritten = writeFile(handle,buffer,size);

	//TODO: MODIFICAR SIZEOFFILE
    return bytesWritten;
}

int truncate2 (FILE2 handle) {
	return -1;
}

int seek2 (FILE2 handle, DWORD offset) {
	return -1;
}

int mkdir2 (char *pathname) {
	init_disk();
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
	if((strlen(currentPath.absolute) + 1) > size){
		return -1;
	}
	else{
		memset(pathname,'\0',size);
		strcpy(pathname, currentPath.absolute);
		return 0;
	}
}

DIR2 opendir2 (char *pathname) {
	init_disk();
	if(strcmp(pathname,"") ==0 )
		return -1;
	return openDir(pathname);
}

int readdir2 (DIR2 handle, DIRENT2 *dentry) {
	return -1;
}

int closedir2 (DIR2 handle) {
	init_disk();
	if(handle < 0)
		return -1;
    return closeDir(handle);
}

int ln2(char *linkname, char *filename) {
	return -1;
}