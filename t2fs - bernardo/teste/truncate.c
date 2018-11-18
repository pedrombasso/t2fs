#include <stdio.h>
#include "../include/t2fs.h"
int main(){

	struct t2fs_inode iNode;
	char buffer[256] = "HEYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY   MEIO   DO   ARQUIVO    YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYFIM";
	char bufferRead[256];
	int i;
	HANDLE fileHandleList[10];
	
	create2 ("Arquivo1");
	int handleNumber = open2 ("Arquivo1");
	
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	while(iNode.blocksFileSize < 25){
		write2(handleNumber, buffer,256);
		//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	}
	
	printf("Fim escrita de 25 blocos\nTamanho total %d\n", iNode.bytesFileSize);
	
	
	printf("Agora ler 25 blocos...\nPress Enter");
	scanf("...", &i);
	seek2(handleNumber, 0);
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	
	while(fileHandleList[handleNumber].seekPtr < iNode.bytesFileSize - 1 ){
		read2(handleNumber, bufferRead, 256);
		for(i = 0; i < 256; i++)
			printf("%c", bufferRead[i]);
	}	
	printf("\n\n\n\nFIM da Leitura dos 25 Blocos");	
	
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	printf("Fazendo truncate da metado do arquivo de %d vai para %d\n", iNode.bytesFileSize, iNode.bytesFileSize/2 );
	seek2(handleNumber, iNode.bytesFileSize/2);
	printf("truncate...%d\n ", truncate2(handleNumber));
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	//printf("Novo tamanho: %d\n", iNode.bytesFileSize);
	
	
	
	
	
	
	char buffer2[10] = "0123456789";
	char bufferRead2[10];
	char bufferRead3[10];
	char bufferRead4[10];
	
	printf("Criando novo arquivo\n");
	create2 ("Arquivo2");
	handleNumber = open2 ("Arquivo2");
	write2(handleNumber, buffer2,10);
	seek2(handleNumber, 0);
	read2(handleNumber, bufferRead2, 10);
	for(i = 0; i < 10; i++)
		printf("%c", bufferRead2[i]);	
	printf("\n");
	
	seek2(handleNumber, 5);
	truncate2(handleNumber);
	printf("truncate metade.\n");
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	//printf("Novo tamanho: %d\n", iNode.bytesFileSize);
	seek2(handleNumber, 0);
	read2(handleNumber, bufferRead3, 5);
	for(i = 0; i < 5; i++)
		printf("%c", bufferRead3[i]);	
	printf("\n");	
	
	
	seek2(handleNumber, 2);
	truncate2(handleNumber);
	printf("truncate de novo.\n");
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	//printf("Novo tamanho: %d\n", iNode.bytesFileSize);
	seek2(handleNumber, 0);
	read2(handleNumber, bufferRead4, 2);
	for(i = 0; i < 2; i++)
		printf("%c", bufferRead4[i]);	
	printf("\n");	
	
}
