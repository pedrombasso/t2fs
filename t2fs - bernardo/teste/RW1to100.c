#include<stdio.h>
#include "../include/t2fs.h"
int main(){

	struct t2fs_inode iNode;
	char buffer[256] = "HEYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY   MEIO   DO   ARQUIVO    YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYFIM";
	char bufferRead[256];
	int i;
	HANDLE fileHandleList[256];
	create2 ("Arquivo1");
	int handleNumber = open2 ("Arquivo1");
	//printf("Validade:%d\n", fileHandleList[handleNumber].validade);
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);	
	//printf("Tamanho do arquivo:%d\n", iNode.bytesFileSize);
	seek2(handleNumber,0);
	//printf("seek %d\n", fileHandleList[handleNumber].seekPtr);
	printf("Escrevendo no arquivo criado... Write -- size wrote: %d\n", write2(handleNumber, buffer,256));
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);	
	//printf("Tamanho do arquivo:%d\n", iNode.bytesFileSize);

	
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);	
	seek2(handleNumber,0);
	//printf("seek %d\ninodeNumber %d\n", fileHandleList[handleNumber].seekPtr, fileHandleList[handleNumber].inodeNumber);
	printf("READ: %d\n", read2(handleNumber, bufferRead, 256));
	printf("INICIO DA LEITURA\n\n");
	//printf("Tamanho do arquivo:%d -- add: %d\n", iNode.bytesFileSize, iNode.dataPtr[0]);
	for(i = 0; i < 256; i++)
		printf("%c", bufferRead[i]);
	printf("\n");
	//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	printf("FIM DA LEITURA --- Total de blocos lidos = %d \n\n", iNode.blocksFileSize);
	printf("Fechando o arquivo %d\n",close2(handleNumber) );
	
	
	
	
	
	
	printf("Tentando ler dados de uma arquivo fechado...Espera-se que de erro %d\n", read2(handleNumber, bufferRead, 256));
	printf("Abrindo arquivo\n");
	handleNumber = open2 ("Arquivo1");
	printf("READ: %d\n", read2(handleNumber, bufferRead, 256));
	printf("INICIO DA LEITURA\n\n");
	for(i = 0; i < 256; i++)
		printf("%c", bufferRead[i]);
	printf("\n");
//	iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	printf("FIM DA LEITURA --- Total de blocos lidos = %d \n\n", iNode.blocksFileSize);	
	
	
	printf("Posiciona ao final dele... %d\n", seek2(handleNumber, -1));
	printf("Escrevo até fechar 100 blocos\n");
	
//	iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	while(iNode.blocksFileSize < 100){
		write2(handleNumber, buffer,256);
		//iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	}
	
	printf("Fim escrita de 100 blocos\n");
	
	
	printf("Agora ler 100 blocos...\nPress Enter");
	scanf("...", &i);
	seek2(handleNumber, 0);
//	iNode = leInode(fileHandleList[handleNumber].inodeNumber);
	
	while(fileHandleList[handleNumber].seekPtr < iNode.bytesFileSize - 1 ){
		read2(handleNumber, bufferRead, 256);
		for(i = 0; i < 256; i++)
			printf("%c", bufferRead[i]);
	}
	
	printf("\n\n\n\nFIM da Leitura dos 100 Blocos");
	
	
	
	
}
