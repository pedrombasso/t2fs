#include "../include/t2fs.h"
#include <stdio.h>
int main(){

	struct t2fs_inode iNode;
	char buffer[80] = "TTTTTTTTTTTTTTEEEEEEEEEEEEEEEEESSSSSSSSSSSSSSSSSSSTTTTTTTTTTTTTTEEEEEEEEEEEEEEEe";
	char bufferRead[80];
	int i;

	int handleNumber = create2 ("Arquivo1");
	//int handleNumber = open2 ("Arquivo1");

	seek2(handleNumber,0);
	printf("Escrevendo no arquivo criado... Write -- size wrote: %d\n", write2(handleNumber, buffer,256));

	
	seek2(handleNumber,0);
	printf("READ: %d\n", read2(handleNumber, bufferRead, 80));
	printf("INICIO DA LEITURA\n\n");
	//printf("Tamanho do arquivo:%d -- add: %d\n", iNode.bytesFileSize, iNode.dataPtr[0]);
	for(i = 0; i < 80; i++)
		printf("%c", bufferRead[i]);
	printf("\n");
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	//printf("FIM DA LEITURA --- Total de blocos lidos = %d \n\n", iNode.blocksFileSize);
	
	/*
	printf("Handles\n");
	for (i = 0; i < 10; i++){
		printf("handle %d ---- validade %d ---- inode %d\n",i, fileHandleList[i].validade, fileHandleList[i].inodeNumber );
		
	}*/
	
	
	printf("Fechando o arquivo %d\n",close2(handleNumber) );	
	//printf("\n\n\nfileHandleList[handleNumber] == VALIDO: %d\nfileHandleList[handleNumber].inodeNumber(%d) ", fileHandleList[handleNumber].validade == VALIDO, fileHandleList[handleNumber].inodeNumber);	
	printf("Apagando arquivo... %d\n", delete2("Arquivo1"));
	printf("Tentando abrir arquivo deletado %d\n",open2 ("Arquivo1") );
	printf("FIM \n\n");
	
	


}
