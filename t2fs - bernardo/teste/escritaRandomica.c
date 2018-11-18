#include <stdio.h>
#include "../include/t2fs.h"
int main(){

	struct t2fs_inode iNode;
	char buffer[256] = "HEYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY   MEIO   DO   ARQUIVO    YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYFIM";
	char buffer2[100] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	char bufferRead[256];
	int i, handleNumber;
	HANDLE fileHandleList[10];
	
	handleNumber = create2 ("Arquivo1");
	seek2(handleNumber,0);
	printf("Escrevendo 2 blocos...\n");
	
	for(i = 0; i < 8; i++)
		write2(handleNumber, buffer,256);

		
	printf("Posiciona na pos 300\n");
	seek2(handleNumber, 300);
	printf("escreve 600 bytes\n");
		
		
	for(i = 0; i < 6; i++)
		write2(handleNumber, buffer2,100);		
		
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	//printInode(iNode);		
		
	
	seek2(handleNumber, 0);
	while(fileHandleList[handleNumber].seekPtr < iNode.bytesFileSize - 1 ){
		read2(handleNumber, bufferRead, 256);
		for(i = 0; i < 256; i++)
			printf("%c", bufferRead[i]);
	}		
		
	printf("FIM\n\n");
	
}
