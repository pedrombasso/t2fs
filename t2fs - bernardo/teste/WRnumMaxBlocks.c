#include <stdio.h>
#include "../include/t2fs.h"
int main(){

	struct t2fs_inode iNode;
	char buffer[256] = "HEYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY   MEIO   DO   ARQUIVO    YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYFIM";
	char bufferRead[256];
	int i,j;
	//Verificar aqui!
	int tamanhoBlocoBytes = 1024;
	//Verificar aqui!
	HANDLE fileHandleList[10];
	//Verificar aqui!
	create2 ("ArquivoMuitoGrande");
	int handleNumber = open2 ("ArquivoMuitoGrande");
	int maxBlocos = tamanhoBlocoBytes/sizeof(DWORD) * tamanhoBlocoBytes/sizeof(DWORD);
		
	printf("Escrevendo o numero maximo de blocos: %d\n", maxBlocos);
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);	
	i = 0;
	while(i < 3000){
		write2(handleNumber, buffer,256);
		i++;
		//iNode = leInode(fileHandleList[handleNumber].inodeNumber);		
	}
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);	
	printf("Total de blocos utilizados %d\n", iNode.blocksFileSize);
	
	printf("Agora ler tudo...\nPress Enter");
	scanf("...", &i);
	seek2(handleNumber, 0);
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	j = 0;
	while(read2(handleNumber, bufferRead, 256) == 256 ){
		
		for(i = 0; i < 256; i++)
			printf("%c", bufferRead[i]);
		printf("bloco %d\n",j/4);
		j++;
	}
	
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	printf("\n\n\n\nFIM da Leitura dos %d Blocos\n", iNode.blocksFileSize);	
	
}
