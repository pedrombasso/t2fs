

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2fs.h"

struct t2fs_inode diretorioAtualInode;
struct t2fs_superbloco superBloco;
struct t2fs_inode diretorioRaizInode;
char * blocoAtual;
char * currentPathName;
int blocoInodesInicial;
int tamanhoBlocoBytes;
int tamanhoBloco; //quantidade de setores por bloco
int numeroRecords;
int initFlag = 0;

HANDLE fileHandleList[10];
HANDLE dirHandleList [10];
// funcao recebe duas strigs retrona na segunda o correspondente ao diretorio atual e na primeira o correspondente ao resto do caminho
void caminhoParcial(char * stringTotal, char * stringParcial){
	int i,shift;
	char stringNovoPath[59];
	
	i = 0;	
	if(stringTotal[0] == '/'){
		i++;
		
		
	}
	else{
		while(stringTotal[i] != '\0' && stringTotal[i] != '/'){
			stringParcial[i] = stringTotal[i];
			i++;
		}
		stringParcial[i] = '\0'; 
		
	}
	
	shift = 0;

	if(stringTotal[i] == '\0'){
		strcpy(stringTotal,"\0");
		return;
	}
	i++;
	
	while(stringTotal[i] != '\0'){
		
		stringNovoPath[shift] = stringTotal[i];
		shift++;i++;
	}
	
	stringNovoPath[shift] = '\0';

	strcpy(stringTotal,stringNovoPath);

}

//carrega o super bloco e faz os calculos iniciais das variaveis globais, deve estar na init()	
void carregaSuperBloco(){
	unsigned char buffer[SECTOR_SIZE];

	read_sector (0, buffer);
	//copia do setor para a estrutura global
	memcpy((void *)&(superBloco.id),                  (void*)&buffer[0] ,4);
	memcpy((void *)&(superBloco.version),             (void*)&buffer[4] ,2);
	memcpy((void *)&(superBloco.superblockSize),      (void*)&buffer[6] ,2);
	memcpy((void *)&(superBloco.freeBlocksBitmapSize),(void*)&buffer[8] ,2);
	memcpy((void *)&(superBloco.freeInodeBitmapSize), (void*)&buffer[10],2);
	memcpy((void *)&(superBloco.inodeAreaSize),       (void*)&buffer[12],2);
	memcpy((void *)&(superBloco.blockSize),           (void*)&buffer[14],2);
	memcpy((void *)&(superBloco.diskSize),            (void*)&buffer[16],4);
	//inicia as varaiveis globais
	blocoInodesInicial  = superBloco.superblockSize + superBloco.freeBlocksBitmapSize + superBloco.freeInodeBitmapSize;
	tamanhoBlocoBytes   = SECTOR_SIZE * superBloco.blockSize;
	tamanhoBloco        = superBloco.blockSize;
	numeroRecords       = tamanhoBlocoBytes/RECORD_SIZE;
	



}

/*funcao que inicia no heap(malloc) o veotr que contera o bloco atualmente carregado, deve estar na init(), acessar o bloco pela variavel
global ablocoAtual*/
void iniciaBloco(){
	blocoAtual  = (char *) malloc (sizeof(char)*SECTOR_SIZE*tamanhoBloco);

}
void iniciaStringPath(){
	currentPathName  = (char *) malloc (sizeof(char)*1024);

}
/*funcao que abstrai a leitura de setores para blocos*/
void carregaBloco(int i){
	unsigned char buffer[SECTOR_SIZE];
	int n,j;
	n = i*tamanhoBloco;
	for(j = 0; j<tamanhoBloco;j++){
		read_sector (n+j, buffer);
		memcpy((void*)&blocoAtual[SECTOR_SIZE*j],            (void*)&buffer[0],SECTOR_SIZE);
	}
}

/*funcao que abstrai a escrita de setores para blocos*/
void escreveBloco(int i){
	unsigned char buffer[SECTOR_SIZE];
	int n,j;
	n = i*tamanhoBloco;
	for(j = 0; j<tamanhoBloco;j++){
		
		memcpy((void*)&buffer[0], (void*)&blocoAtual[SECTOR_SIZE*j],SECTOR_SIZE);
		write_sector (n+j, buffer);
	}
}

void printSuperBloco(){
	
	printf("ID:%c %c %c %c \n",superBloco.id[0],superBloco.id[1],superBloco.id[2],superBloco.id[3]);
	printf("Version:%X \n",superBloco.version);
	printf("SuperBlockBitmap:%d \n",superBloco.superblockSize);
	printf("FreeBlockBitMap:%d \n",superBloco.freeBlocksBitmapSize);
	printf("FreeiNodeBitmap:%d \n",superBloco.freeInodeBitmapSize);
	printf("iNodeArea:%d \n",superBloco.inodeAreaSize);
	printf("BlockSize:%d \n",superBloco.blockSize);
	printf("DiskSize:%d \n",superBloco.diskSize);


}

//retorna uma estrutura inode e le o inode referente ao numero passado como parametro
struct t2fs_inode leInode(int n){
	struct t2fs_inode iNode;	
	int pos;
	carregaBloco(blocoInodesInicial+((int)n/(tamanhoBlocoBytes/sizeof(struct t2fs_inode))));
	pos =(n%(tamanhoBlocoBytes/sizeof(struct t2fs_inode)))*INODE_SIZE;


	//printf("leInode: bloco %d ... Pos %d\n", (blocoInodesInicial+((int)n/32)), pos );
	memcpy((void *)&(iNode.blocksFileSize),            (void*)&blocoAtual[pos]    ,4);
	memcpy((void *)&(iNode.bytesFileSize),             (void*)&blocoAtual[pos+4]  ,4);
	memcpy((void *)&(iNode.dataPtr[0]),                (void*)&blocoAtual[pos+8]  ,4);
	memcpy((void *)&(iNode.dataPtr[1]),                (void*)&blocoAtual[pos+12] ,4);
	memcpy((void *)&(iNode.singleIndPtr),              (void*)&blocoAtual[pos+16] ,4);
	memcpy((void *)&(iNode.doubleIndPtr),              (void*)&blocoAtual[pos+20] ,4);
	memcpy((void *)&(iNode.reservado),                 (void*)&blocoAtual[pos+24] ,8);	
		
	return iNode;
	

}
/*grava o Inode passado por parâmetro na posicao informada
Retorno 0 --> Sem erro*/

/* Ex: escreveInode(iNode, fileHandleList[handle].inodeNumber);	*/
int escreveInode(struct t2fs_inode Inode, int posicao){	
	int InodePorBlocos = ((int)tamanhoBlocoBytes/sizeof(Inode));
	int nBloco = ((int)posicao/InodePorBlocos);
	
//	printf("InodeporBloco %d --- nBloco %d\n", InodePorBlocos, nBloco);
//	printf("escreveInode: blocoInicial %d --- bloco %d ... Pos %d\n", blocoInodesInicial, nBloco, (posicao%InodePorBlocos)*sizeof(Inode) );

	carregaBloco(blocoInodesInicial+nBloco);
	memcpy((void*)&blocoAtual[(posicao%InodePorBlocos)*sizeof(Inode)], (void *)&(Inode),sizeof(Inode));
	escreveBloco(blocoInodesInicial+nBloco);
	//printInode(leInode(posicao));
	return 0;		
}


void printInode(struct t2fs_inode iNode){
	printf("\nSize in Blocks %d\n",iNode.blocksFileSize);
	printf("Size in Bytes %d\n",iNode.bytesFileSize);
	printf("Data ptr 1 %u\n",iNode.dataPtr[0]);
	printf("Data ptr 2 %u\n",iNode.dataPtr[1]);
	printf("Ptr Ind Simples %u\n",iNode.singleIndPtr);
	printf("Ptr Ind Dupla %u\n\n",iNode.doubleIndPtr);


}
//funcao de debug que imprime um diretorio de forma human friendly
void readAndPrintDir(struct t2fs_inode diretorioInode){
	struct t2fs_record record;
	int i;			
	if(diretorioInode.blocksFileSize > 0){
		carregaBloco(diretorioInode.dataPtr[0]);

		for(i = 0; i < numeroRecords; i++) {
			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_REGULAR || record.TypeVal == TYPEVAL_DIRETORIO){
				printf("TypeVal: %X\n",record.TypeVal);
				printf("Name: %s\n",record.name);
				printf("iNodeNumber: %d\n",record.inodeNumber);
		
				printf("----------------------------------------------------------------------\n");
			}
		}
	}
	printf("BLOCO 2\n");
	if(diretorioInode.blocksFileSize > 1){
		int i;
		carregaBloco(diretorioInode.dataPtr[1]);
		if(record.TypeVal == TYPEVAL_REGULAR || record.TypeVal == TYPEVAL_DIRETORIO){
			for(i = 0; i < numeroRecords; i++) {
				memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
				printf("TypeVal: %X\n",record.TypeVal);
				printf("Name: %s\n",record.name);
				printf("iNodeNumber: %d\n",record.inodeNumber);
		
				printf("----------------------------------------------------------------------\n");
			}
		}
	}
	if(diretorioInode.blocksFileSize > 2){

		printf("leitura de indirecao\n");

	}
}
int procuraOpen(int numeroInode,int typeVal){
	int i;
	if(typeVal == TYPEVAL_DIRETORIO){
		for(i = 0; i < 10; i ++){
			if(dirHandleList[i].inodeNumber  == numeroInode && dirHandleList[i].validade == VALIDO)
				return -1;
		}
        }                               
	if(typeVal == TYPEVAL_REGULAR){
		for(i = 0; i < 10; i ++){
			if(fileHandleList[i].inodeNumber  == numeroInode && fileHandleList[i].validade == VALIDO)
				return -1;
		}
	}

       return 0;

}
struct t2fs_record procuraDirEntry(int bloco,int numero){
	struct t2fs_record record;
	carregaBloco(bloco);
	memcpy((void*)&record,(void *)&blocoAtual[numero*sizeof(record)],sizeof(record));
	return record;

}
//numero externo 'e o numero do ptr
//numero interno 'e o numero do record
struct t2fs_record  procuraDirEntryIndirecao(int blocoIndireto,int numeroInterno,int numeroExterno){
	int bloco;
	bloco = 0;
	struct t2fs_record record;	
	carregaBloco(blocoIndireto);
	memcpy((void*)&bloco,(void *)&(blocoAtual[numeroExterno*sizeof(DWORD)]),sizeof(DWORD));
	record =  procuraDirEntry(bloco,numeroInterno);
	return record;
}
struct t2fs_record  procuraDirEntryDuplaIndirecao(int blocoIndireto,int numeroInterno,int numeroExterno){
	int  enderecoBloco;
	struct t2fs_record record;	
	carregaBloco(blocoIndireto);
	memcpy((void*)&enderecoBloco,(void *)&blocoAtual[numeroExterno / (tamanhoBlocoBytes/sizeof(DWORD))],sizeof(DWORD));
	numeroExterno = numeroExterno % (tamanhoBlocoBytes/sizeof(DWORD));
	record =  procuraDirEntryIndirecao(enderecoBloco,numeroInterno,numeroExterno); //calcular numero interno e externo
	return record;
}
//funcao que percore os rescords dentro de um bloco e acha por nome de arquivo
struct t2fs_record  findRecords(int bloco,char* partialPath){
	int i;
	struct t2fs_record recordErro;
	struct t2fs_record record;
	recordErro.inodeNumber = -1;
	carregaBloco(bloco);
	for(i = 0; i < numeroRecords; i++) {
		memcpy((void*)&record,(void *)&blocoAtual[i*sizeof(struct t2fs_record)],sizeof(struct t2fs_record));
		if((record.TypeVal == TYPEVAL_REGULAR || record.TypeVal == TYPEVAL_DIRETORIO) && strcmp(partialPath,record.name) == 0){
			
			return record;
		}
	}
	return recordErro;	

		

}
//funcao que percore os rescords dentro de um bloco e acha por nome de arquivo
struct t2fs_record  findRecords2(int bloco,int deletadoNumero,int * blocoRecord){
	int i;
	struct t2fs_record recordErro;
	struct t2fs_record record;
	recordErro.inodeNumber = -1;
	carregaBloco(bloco);
	for(i = 0; i < numeroRecords; i++) {
		memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
		if((record.TypeVal == TYPEVAL_REGULAR || record.TypeVal == TYPEVAL_DIRETORIO) && record.inodeNumber == deletadoNumero){
			*blocoRecord = bloco;
			
			return record;
		}
	}
	return recordErro;	

		

}

struct t2fs_record  removeRecords(int bloco,char* partialPath,int type){
	int i;
	struct t2fs_record recordErro;
	struct t2fs_record record;
	recordErro.inodeNumber = -1;
	carregaBloco(bloco);
	//printf("current Block %d\n",bloco);
	for(i = 0; i < numeroRecords; i++) {
		//printf("del rec name %s block %d\n",record.name,bloco);
		memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
		if(record.TypeVal == TYPEVAL_DIRETORIO  && strcmp(partialPath,record.name) == 0){
			return record;
		}
		if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
			
			record.TypeVal =  TYPEVAL_INVALIDO;
			memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
			escreveBloco(bloco);
			return record;
			
		}
	}

	return recordErro;	

		

}

//funcao utilizada para percorrer bloco de indirecao, chama funcao que percorre os records dentro dos blocos apontados
struct t2fs_record  procuraRecordsIndirecao(int blocoIndireto,char * partialPath){
	int bloco,i;
	int temp;
	struct t2fs_record record;	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		
		memcpy((void*)&bloco,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR){
			break;
		}
		record =  findRecords(bloco,partialPath);
		temp = record.inodeNumber;
		if(temp >= 0 ){
			return record;
		}
		
		

	}
	return record;
}
//funcao utilizada pel rmdir2 para percorrer bloco de indirecao, chama funcao que percorre os records dentro dos blocos apontados
struct t2fs_record  procuraRecordsIndirecao2(int blocoIndireto,int deletadoNumero,int * blocoRecord){
	int i,bloco,temp;
	struct t2fs_record record;	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void*)&bloco,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR)
			break;
		record =  findRecords2(bloco,deletadoNumero,blocoRecord);
		temp = record.inodeNumber;
		if(temp >= 0 )
			return record;
		
		

	}
	return record;
}
//funcao auxiliar usada para remover records
struct t2fs_record  procuraERemoveRecordsIndirecao(int blocoIndireto,char * partialPath,int type){
	int i,bloco,temp;
	struct t2fs_record record;	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void*)&bloco,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));

		if(bloco == INVALID_PTR)
			break;
		record =  removeRecords(bloco,partialPath,type);
		temp = record.inodeNumber;
		if(temp >= 0 ){
			return record;
		}
		

	}
	return record;
}
//funcao utilizada para percorrer bloco de indirecao, chama funcao que percorre os records dentro dos blocos apontados
struct t2fs_record  procuraRecordsDuplaIndirecao(int blocoIndireto,char * partialPath){
	int i,bloco,temp;
	struct t2fs_record record;	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void*)&bloco,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR)
			break;
		record =  procuraRecordsIndirecao(bloco,partialPath);
		temp = record.inodeNumber;
		if(temp >= 0 )
			return record;
		

	}
	return record;
}
//funcao utilizada pela rmdir para percorrer bloco de indirecao, chama funcao que percorre os records dentro dos blocos apontados
struct t2fs_record  procuraRecordsDuplaIndirecao2(int blocoIndireto,int deletadoNumero,int * blocoRecord){
	int i,bloco,temp;
	struct t2fs_record record;	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void*)&bloco,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR)
			break;
		record =  procuraRecordsIndirecao2(bloco,deletadoNumero,blocoRecord);
		temp = record.inodeNumber;
		if(temp >= 0 ){
			return record;
		}
		

	}
	return record;
}
//funcao auxiliar para remover blocos
struct t2fs_record  procuraERemoveRecordsDuplaIndirecao(int blocoIndireto,char * partialPath, int type){
	int i,bloco;
	int temp;
	struct t2fs_record record;	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void*)&bloco,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR)
			break;
		record =  procuraERemoveRecordsIndirecao(bloco,partialPath,type);
		temp = record.inodeNumber;
		if(temp >= 0 )
			return record;
		

	}
	return record;
}
//usado para achar em algum caminho(absoluto ou relativo) um arquivo regular, retorna o numero do inode do arquivo
int findFile(struct t2fs_inode diretorioInode,char * nome){
	struct t2fs_record record;
	int i;
	char partialPath[59];
	if(nome[0] == '/'){
		diretorioInode = diretorioRaizInode;
		i = 1;
		while(nome[i] != '\0'){	
			nome[i-1] = nome[i];
			i++;	
		}
		nome[i-1] = '\0';
	}
	caminhoParcial(nome,partialPath);
	if(diretorioInode.blocksFileSize > 0){
		carregaBloco(diretorioInode.dataPtr[0]);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
				return record.inodeNumber;

			}
			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
				return findFile(leInode(record.inodeNumber),nome);
			}
		}
	}
	if(diretorioInode.blocksFileSize > 1){	
		carregaBloco(diretorioInode.dataPtr[1]);
		for(i = 0; i < numeroRecords; i++) {
			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
				return record.inodeNumber;
			}
			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
				return findFile(leInode(record.inodeNumber),nome);
			}
		}
	}
	if(diretorioInode.blocksFileSize > 2){
			record = procuraRecordsIndirecao(diretorioInode.singleIndPtr,partialPath);
			if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
				return record.inodeNumber;
			}
			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
				return findFile(leInode(record.inodeNumber),nome);
			}
				
	}
	if(diretorioInode.blocksFileSize > tamanhoBloco+2){
			record = procuraRecordsDuplaIndirecao(diretorioInode.singleIndPtr,partialPath);
			if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
				return record.inodeNumber;
			}
			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
				return findFile(leInode(record.inodeNumber),nome);
			}	
				
	}
		
	return -1;
}




//usado para criar deletar o record referente a um arquivo

int findFileAndRemoveRecord(struct t2fs_inode diretorioInode,char * nome){
	struct t2fs_record record;
	int i;
	char partialPath[59];
	if(nome[0] == '/'){
		diretorioInode = diretorioRaizInode;
		i = 1;
		while(nome[i] != '\0'){	
			nome[i-1] = nome[i];
			i++;	
		}
		nome[i-1] = '\0';
	}
	caminhoParcial(nome,partialPath);
	if(diretorioInode.blocksFileSize > 0){
		carregaBloco(diretorioInode.dataPtr[0]);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
				record.TypeVal = TYPEVAL_INVALIDO;
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(diretorioInode.dataPtr[0]);
				return record.inodeNumber;

			}
			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
				return findFileAndRemoveRecord(leInode(record.inodeNumber),nome);
			}
		}
	}
	if(diretorioInode.blocksFileSize > 1){	
		carregaBloco(diretorioInode.dataPtr[1]);
		for(i = 0; i < numeroRecords; i++) {
			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
				record.TypeVal = TYPEVAL_INVALIDO;
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(diretorioInode.dataPtr[1]);
				return record.inodeNumber;
			}
			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
				return findFileAndRemoveRecord(leInode(record.inodeNumber),nome);
			}
		}
	}
	if(diretorioInode.blocksFileSize > 2){
			record = procuraERemoveRecordsIndirecao(diretorioInode.singleIndPtr,partialPath,TYPEVAL_REGULAR);
			if(record.inodeNumber < 0)
				return -1;
			if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
				return record.inodeNumber;
			}
			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
				return findFileAndRemoveRecord(leInode(record.inodeNumber),nome);
			}
				
	}
	if(diretorioInode.blocksFileSize > tamanhoBloco+2){
			record = procuraERemoveRecordsDuplaIndirecao(diretorioInode.singleIndPtr,partialPath,TYPEVAL_REGULAR);
			if(record.inodeNumber < 0)
				return -1;
			if(record.TypeVal == TYPEVAL_REGULAR && strcmp(partialPath,record.name) == 0){
				return record.inodeNumber;
			}
			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
				return findFileAndRemoveRecord(leInode(record.inodeNumber),nome);
			}	
				
	}
		
	return -1;
}
//usado para achar em algum caminho(absoluto ou relativo) um arquivo diretorio, retorna o numero do inode do diretorio
int findDir(struct t2fs_inode diretorioInode,char * nome){
	struct t2fs_record record;
	int i;
	char partialPath[59];
	
	if(strcmp(nome,"/") == 0)
		return 0;
	if(nome[0] == '/'){
		diretorioInode = diretorioRaizInode;
		i = 1;
		while(nome[i] != '\0'){	
			nome[i-1] = nome[i];
			i++;	
		}
		nome[i-1] = '\0';
	}
	caminhoParcial(nome,partialPath);
	if(diretorioInode.blocksFileSize > 0){
		carregaBloco(diretorioInode.dataPtr[0]);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));

			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){

				if(nome[0] == '\0'){
		
					return record.inodeNumber;
				}
				return findDir(leInode(record.inodeNumber),nome);
			}
		}
	}
	if(diretorioInode.blocksFileSize > 1){	
		carregaBloco(diretorioInode.dataPtr[1]);
		for(i = 0; i < numeroRecords; i++) {
			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));

			if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){

				if(nome[0] == '\0'){
		
					return record.inodeNumber;
				}
				return findDir(leInode(record.inodeNumber),nome);
			}
		}
	}
	if(diretorioInode.blocksFileSize > 2){
		record = procuraRecordsIndirecao(diretorioInode.singleIndPtr,partialPath);
		if(record.inodeNumber < 0)
				return -1;
		if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
			
			
			if(nome[0] == '\0'){
		
				return record.inodeNumber;
			}
			return findDir(leInode(record.inodeNumber),nome);
		}		
	}
	if(diretorioInode.blocksFileSize > tamanhoBloco+2){
		record = procuraRecordsDuplaIndirecao(diretorioInode.singleIndPtr,partialPath);
		
		if(record.inodeNumber < 0)
			return -1;
		if(record.TypeVal == TYPEVAL_DIRETORIO && strcmp(partialPath,record.name) == 0){
			

			if(nome[0] == '\0'){
				
				return record.inodeNumber;
			}
			return findDir(leInode(record.inodeNumber),nome);
		}
	}	
	return -1;
}


/* Função que retorna o bloco que encontra-se no endereço passado por parametro.
Ex:
bloco = getBloco(iNode.singleIndPtr); */
char * getBloco(int i){
	unsigned char buffer[SECTOR_SIZE];
	char * bloco = (char *) malloc (sizeof(char)*tamanhoBlocoBytes);
	int n,j;
	n = i*tamanhoBloco;
	
	for(j = 0; j<tamanhoBloco;j++){
		read_sector (n+j, buffer);
		memcpy((void*)&bloco[SECTOR_SIZE*j],            (void*)&buffer[0],SECTOR_SIZE);
	}	
	return bloco;
}

/* Retorna o endereço direto do N-ésimo bloco do inode */
DWORD getBlocoN(struct t2fs_inode iNode, int blocoN){	
	char * bloco = (char *) malloc (tamanhoBlocoBytes);	
	char * blocoInd = (char *) malloc (tamanhoBlocoBytes);	
	int  blocoIndirecao;
	int contBloc;	
	int PointerPerBloc = tamanhoBlocoBytes/sizeof(DWORD); //quantidade de ponteiros que cabe em um bloco
	DWORD ender;
	
	//Blocos endereçados diretamente.
	if (blocoN == 0){
		free(bloco);
		free(blocoInd);		
		return iNode.dataPtr[0];
	}
	if (blocoN == 1){
		free(bloco);
		free(blocoInd);		
		return iNode.dataPtr[1];
	}
	contBloc = blocoN - 2;
	if (contBloc < PointerPerBloc){ //blocoN esta no bloco de indireção simples
		if (iNode.singleIndPtr == INVALID_PTR){
			free(bloco);
			free(blocoInd);			
			return -1;
		}
		bloco = getBloco(iNode.singleIndPtr);
		memcpy((void *)&(ender), (void*)&bloco[sizeof(DWORD)*(contBloc)],sizeof(DWORD));	
		free(bloco);
		free(blocoInd);
		return ender;
	}
	else{	

		if (iNode.doubleIndPtr == INVALID_PTR){
			free(bloco);
			free(blocoInd);
			return -1;
		}
		
		blocoInd = getBloco(iNode.doubleIndPtr);		
		contBloc = contBloc - PointerPerBloc;		

		blocoIndirecao = (int)(contBloc/PointerPerBloc);
		memcpy((void *)&(ender), (void*)&blocoInd[sizeof(DWORD)*(blocoIndirecao)],sizeof(DWORD));	
		blocoInd = getBloco(ender);
		contBloc = contBloc%PointerPerBloc;
		memcpy((void *)&(ender), (void*)&blocoInd[sizeof(DWORD)*(contBloc)],sizeof(DWORD));	
		free(bloco);
		free(blocoInd);		
		return ender;
	}
}

//inicia a list de files com valores de nao valido
void initFileHandleList(){
	int i;
	for(i =0;i<10;i++){
		fileHandleList[i].validade = NAO_VALIDO;
	}

}
//inicia a list de handles com valores de nao valido
void initDirHandleList(){
	int i;
	for(i =0;i<10;i++){

		dirHandleList[i].validade = NAO_VALIDO;
	}

}

//faz as inicializacoes da lib, 'e chamada no comeco de todas as funcoes de usuario
void init(){

	if (initFlag == 0){
		
		carregaSuperBloco();
		iniciaBloco();
		iniciaStringPath();
		diretorioRaizInode  = leInode(0);
		diretorioAtualInode = leInode(0);
		strcpy(currentPathName,"/");
		initFlag = 1;
		initFileHandleList();
		initDirHandleList();
	}
}

//funcao auxiliar que bytes dentro de arquivos apontados por blocos de indices(indiretos);
void leBytesBloco(int * bytesRestantes, int size, char * buffer,int bloco, int * contador,int byteInicial){
	int j;
	carregaBloco(bloco);
	
	for(j = byteInicial; j < tamanhoBlocoBytes && (*contador) < size && (*contador) < *bytesRestantes;(*contador)++,j++){
		buffer[*contador] = blocoAtual[j];
		
	} 
}

//funcao auxiliar que le blocos indiretos de arquivos de dados
void readArquivoIndirecao(int ptrIndirecao ,int blocoInicial,int * bytesRestantes, int size, char * buffer,int * contador,int handle){
	int k,byteInicial,numeroBloco;	
	for(k = 0; k < tamanhoBlocoBytes/sizeof(DWORD) && ((*contador) < *bytesRestantes) && (*contador) < size ;k++){
		carregaBloco(ptrIndirecao);

		memcpy((void*)&numeroBloco,(void *)&(blocoAtual[sizeof(DWORD)*k]),sizeof(DWORD));
		if(blocoInicial == k+2 ){
			byteInicial = fileHandleList[handle].seekPtr % tamanhoBlocoBytes;
			leBytesBloco(bytesRestantes,size,buffer,numeroBloco,(contador),byteInicial);
		}
		else if(blocoInicial > k +2){
			byteInicial = 0;
		 	leBytesBloco(bytesRestantes,size,buffer,numeroBloco,(contador),byteInicial);
		}
		//else do nothing

		

	}

}
void readArquivoDuplaIndirecao(int ptrIndirecao ,int blocoInicial,int * bytesRestantes, int size, char * buffer,int * contador,int handle){
	int k,ptrDuplaIndirecao;
	int ptrInicial;
	ptrInicial = blocoInicial/tamanhoBloco;
	blocoInicial = blocoInicial %tamanhoBloco;
	for(k = 0; k < tamanhoBlocoBytes/sizeof(DWORD) && (*contador < *bytesRestantes)  && *contador < size ;k++){
		carregaBloco(ptrIndirecao);
		memcpy((void*)&ptrDuplaIndirecao,(void *)&(blocoAtual[sizeof(DWORD)*k]),sizeof(DWORD));
		if(ptrInicial == (((tamanhoBloco*k) + 2 + tamanhoBloco)/tamanhoBloco)){
			readArquivoIndirecao(ptrDuplaIndirecao,blocoInicial,bytesRestantes,size,buffer,contador,handle);
		}
		else if(ptrInicial > ((tamanhoBloco*k + 2 + tamanhoBloco)/tamanhoBloco))
			//passa 0 como bloco inicial para saber que ja ocorria leitura antes do bloco anterior
			readArquivoIndirecao(ptrDuplaIndirecao,0,bytesRestantes,size,buffer,contador,handle);
		//else do nothing
		
	}

}

//faz o parsing do caminho para criacao de arquivo
// retorna 0 se o path for o diretorio atual ex. file1
//retorn 1 se o path for absoluto
//retorna 2 se o path for apenas o diretorio raiz
//retorn 3 se o path for complexo e relativo
int createFilePathParser(char * filename,char * tempFileName, char * fileLastName){
	int i,j,simplePathFlag;
	i = 0; simplePathFlag = 0;
		
	while(filename[i] != '\0')
		i++;
	
	while(filename[i] != '/' && i>= 0){
		
		i --;
	}
	if(((i == 0) && (filename[i] == '/'))){
		simplePathFlag = 2;
		

	}
	if( i != -1 && !((i == 0) && (filename[i] == '/'))){	
		strcpy(tempFileName,filename);
		tempFileName[i] = '\0';
		simplePathFlag = 3;
		if(filename[0] == '/')
			return 1;
	}

	i++;
	j = 0;
	while(filename[i] != '\0'){
		fileLastName[j] = filename[i];
		i++; j++;
	}
	fileLastName[j] = '\0';
	return simplePathFlag;
}

//acha inode livre a a partir do bitmap, retorna numero do inode
int findInodeLivre(){
	int valorBit,i;
	i = -1;
	do{
		i++;
		valorBit = getBitmap2 (BITMAP_INODE,i);
	
	}while(valorBit == 1);

	return i;



}

//acha bloco livre a a partir do bitmap, retorna numero do inode
int findBlocoLivre(){
	int valorBit,i;
	i = -1;
	do{
		i++;
		valorBit = getBitmap2 (BITMAP_DADOS,i);
	}while(valorBit == 1);

	return i;



}
//procura blocos com espacao para que seja registrado um record
//se acha retorna o numero do bloco
//se nao acha retorna -1
int findDirEntryInd(int bloco){
	int i;
	struct t2fs_record record;
	carregaBloco(bloco);
	for(i = 0; i < numeroRecords; i++) {

		memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
		
		if(record.TypeVal == TYPEVAL_INVALIDO){
			return bloco;

			}
		}
	return -1;

}
//recebe um bloco e transforma esse bloco em um bloco de diretorios vazios
void emptyDir(int bloco,int inodeNumber){
	int i,indiceInodeBloco;
	struct t2fs_record record;
	struct t2fs_inode inode;
	inode = leInode(inodeNumber);
	carregaBloco(bloco);
	for(i = 0; i < numeroRecords; i++) {

		memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
		record.TypeVal = TYPEVAL_INVALIDO;
		memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
		
		
			
	}
	
	escreveBloco(bloco);	
	leInode(inodeNumber);
	inode.bytesFileSize = tamanhoBlocoBytes*inode.blocksFileSize;
	indiceInodeBloco = inodeNumber % (tamanhoBlocoBytes/INODE_SIZE);
	memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(inode),INODE_SIZE);
	escreveBloco(blocoInodesInicial+((int)inodeNumber/(tamanhoBlocoBytes/32)));
}

//funcao auxiliar que aloca um bloco apontado por um ponteiro indireto


int createDataBlockDoubleIndir(int numeroBlocoInd, int numeroInternoBloco,int inodeNumber){
	int numeroNovoBloco;
	int i,indiceInodeBloco;
	int numeroBlocoSingleIndir;
	DWORD invalidPtrTemp;
	struct t2fs_inode inode;
	inode = leInode(inodeNumber);
	invalidPtrTemp = INVALID_PTR;	
	
	
	carregaBloco(numeroBlocoInd);
	if(numeroInternoBloco  %(tamanhoBlocoBytes/sizeof(DWORD)) == 0){
		numeroNovoBloco = findBlocoLivre();
		if(numeroNovoBloco < 0)
			return -1;	
		setBitmap2(BITMAP_DADOS,numeroNovoBloco,1);
		memcpy((void*) &(blocoAtual[(((numeroInternoBloco/(tamanhoBlocoBytes/sizeof(DWORD))))-1)*sizeof(DWORD)]),
		(void*)&numeroNovoBloco,
		sizeof(DWORD));
		escreveBloco(numeroBlocoInd);
		carregaBloco(numeroNovoBloco);


		for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
			memcpy((void*) &(blocoAtual[i*sizeof(DWORD)]),(void*)&invalidPtrTemp,sizeof(DWORD));
			escreveBloco(numeroNovoBloco);
			
		}
		carregaBloco(numeroBlocoInd);
	}

	memcpy((void*)&numeroBlocoSingleIndir,
	&(blocoAtual[((numeroInternoBloco-(tamanhoBlocoBytes/sizeof(DWORD)))/(tamanhoBlocoBytes/sizeof(DWORD)))*sizeof(DWORD)]),
	sizeof(DWORD));
	numeroInternoBloco =((numeroInternoBloco - ((tamanhoBlocoBytes/sizeof(DWORD)))%tamanhoBlocoBytes ))%(tamanhoBlocoBytes/sizeof(DWORD));
	leInode(inodeNumber);
	inode.blocksFileSize += 1;
	indiceInodeBloco = inodeNumber % (tamanhoBlocoBytes/INODE_SIZE);
	memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(inode),INODE_SIZE);
	escreveBloco(blocoInodesInicial+((int)inodeNumber/(tamanhoBlocoBytes/32)));
	

	return createDataBlockSingleIndir(numeroBlocoSingleIndir,(numeroInternoBloco%(tamanhoBlocoBytes/sizeof(DWORD)))+2,inodeNumber);
	       
}
//funcao auxiliar que cria um bloco de indirecao dupla
int createDataBlockSingleIndir(int numeroBlocoInd, int numeroInternoBloco,int inodeNumber){
	int numeroNovoBloco;
	int indiceInodeBloco;
	struct t2fs_inode inode;
	inode = leInode(inodeNumber);
	numeroNovoBloco = findBlocoLivre();
	if(numeroNovoBloco < 0)
		return -1;
	setBitmap2(BITMAP_DADOS,numeroNovoBloco,1);
	carregaBloco(numeroBlocoInd);	
	memcpy((void*) &(blocoAtual[(numeroInternoBloco-2)*sizeof(DWORD)]),(void*)&numeroNovoBloco,sizeof(DWORD));
	escreveBloco(numeroBlocoInd);
	leInode(inodeNumber);

	indiceInodeBloco = inodeNumber % (tamanhoBlocoBytes/INODE_SIZE);
	memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(inode),INODE_SIZE);
	escreveBloco(blocoInodesInicial+((int)inodeNumber/(tamanhoBlocoBytes/32)));
	return numeroNovoBloco;

}

//funcao auxiliar que acha records em blocos indiretos
//retorna numero do bloco com records em branco para serem usados
int singleIndirRecordBlock(int inodeNumber,int blocoIndireto){
	int blocoNovoRecord;
	int i,	ptrAtual;
	struct t2fs_inode inode;
	
	inode = leInode(inodeNumber);
	carregaBloco(blocoIndireto);
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void*)&ptrAtual,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
				
			if(ptrAtual != INVALID_PTR)
				memcpy((void*)&blocoNovoRecord,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));	
				blocoNovoRecord = findDirEntryInd(ptrAtual) ;
				if(blocoNovoRecord > 0)
					return blocoNovoRecord;
			if(ptrAtual == INVALID_PTR){
				blocoNovoRecord = createDataBlockSingleIndir(inode.singleIndPtr,i+2,inodeNumber);		
				
				inode = leInode(inodeNumber);
				inode.blocksFileSize += 1;
				escreveInode(inode,inodeNumber);
				emptyDir(blocoNovoRecord,inodeNumber);
				return blocoNovoRecord;
			}	
	}
	return -1;
}
int doubleIndirRecordBlock(int inodeNumber,int blocoIndireto){
	int i;
	int blocoSingleIndir;
	int blocoNovoRecord;
	struct t2fs_inode inode;
	inode = leInode(inodeNumber);
		for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
			
			if(blocoAtual[i*sizeof(DWORD)] != INVALID_PTR)
				memcpy((void *)&(blocoSingleIndir),(void*)&blocoAtual[i * sizeof(DWORD)],sizeof(DWORD));
				blocoNovoRecord = singleIndirRecordBlock(inodeNumber,blocoSingleIndir);
				if(blocoNovoRecord > 0)
					return blocoNovoRecord;
			if(blocoAtual[i*sizeof(DWORD)] == INVALID_PTR){
				blocoNovoRecord = createDataBlock(inodeNumber,inode.blocksFileSize);			
				
				return blocoNovoRecord;
			}	
	}
	return -1;

}	
//funcao que aloca um bloco de dados
int createDataBlock(int numeroInode,int numeroInternoBloco){
	int numeroNovoBloco,i;
	int indiceInodeBloco;
	DWORD invalidPtrTemp;
	invalidPtrTemp = INVALID_PTR;
	struct t2fs_inode inode;
	if(!(numeroInternoBloco >= (tamanhoBlocoBytes/sizeof(DWORD) + 1))){
		if(!((numeroInternoBloco < (tamanhoBlocoBytes/sizeof(DWORD) + 1)) && (numeroInternoBloco >2))){
			numeroNovoBloco = findBlocoLivre();
			if(numeroNovoBloco < 0)
				return -1;
			setBitmap2(BITMAP_DADOS,numeroNovoBloco,1);
		}
		
	}


	inode = leInode(numeroInode);
	
	if(numeroInternoBloco == 0){
		inode.dataPtr[0] = numeroNovoBloco;
		inode.blocksFileSize = 1;
		//recarrega inode na memoria de trabalho	
		leInode(numeroInode);
		//sobreescreve inode
		indiceInodeBloco = numeroInode % (tamanhoBlocoBytes/INODE_SIZE);
		memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(inode)    ,INODE_SIZE);
		escreveBloco(blocoInodesInicial+((numeroInode/(tamanhoBlocoBytes/32))));		
	}
	if(numeroInternoBloco == 1){
		inode.dataPtr[1] = numeroNovoBloco;
		inode.blocksFileSize = 2;
		//recarrega inode na memoria de trabalho	
		leInode(numeroInode);
		//sobreescreve inode
		indiceInodeBloco = numeroInode % (tamanhoBlocoBytes/INODE_SIZE);
		memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(inode)    ,INODE_SIZE);
		escreveBloco(blocoInodesInicial+(((int)numeroInode/(tamanhoBlocoBytes/32))));			
	}
	//criaBlocoDeIndirecoes e inicia ele com ptrs nulos tamanho em blocos vai ser incrementado depois
	if(numeroInternoBloco == 2){
		inode.singleIndPtr = numeroNovoBloco;
		carregaBloco(numeroNovoBloco);
		for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
			memcpy((void*) &(blocoAtual[i*sizeof(DWORD)]),(void*)&invalidPtrTemp,sizeof(DWORD));
			

		}
		escreveBloco(inode.singleIndPtr);
	}
	if((numeroInternoBloco < (tamanhoBlocoBytes/sizeof(DWORD) + 1)) && (numeroInternoBloco >=2)){
		numeroNovoBloco = createDataBlockSingleIndir(inode.singleIndPtr,numeroInternoBloco,numeroInode);
		inode.blocksFileSize += 1;
		//recarrega inode na memoria de trabalho	
		leInode(numeroInode);
		//sobreescreve inode
		indiceInodeBloco = numeroInode % (tamanhoBlocoBytes/INODE_SIZE);
		memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(inode)    ,INODE_SIZE);
		escreveBloco(blocoInodesInicial+(((int)numeroInode/(tamanhoBlocoBytes/32))));	
	}
	//criaBlocoDuplaIndirecao
	if(numeroInternoBloco == (tamanhoBlocoBytes/sizeof(DWORD) + 1)){
		numeroNovoBloco = findBlocoLivre();
		if(numeroNovoBloco < 0)
			return -1;
		setBitmap2(BITMAP_DADOS,numeroNovoBloco,1);
		inode.doubleIndPtr = numeroNovoBloco;
		carregaBloco(numeroNovoBloco);
		for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
			memcpy((void*) &(blocoAtual[i*sizeof(DWORD)]),(void*)&invalidPtrTemp,sizeof(DWORD));
			

		}
		escreveBloco(inode.doubleIndPtr);		
	}
	if((numeroInternoBloco >= (tamanhoBlocoBytes/sizeof(DWORD) + 1))){
		inode.blocksFileSize += 1;
		numeroNovoBloco = createDataBlockDoubleIndir(inode.doubleIndPtr,numeroInternoBloco-1,numeroInode);	
	}
	//recarrega inode na memoria de trabalho	
	leInode(numeroInode);
	//sobreescreve inode
	indiceInodeBloco = numeroInode % (tamanhoBlocoBytes/INODE_SIZE);
	memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(inode)    ,INODE_SIZE);
	escreveBloco(blocoInodesInicial+(((int)numeroInode/(tamanhoBlocoBytes/32))));
	return numeroNovoBloco;
	

}
//cria uma entrada de diretorio dado o passado dir number 
//TYPEVAL_REGULAR como ultimo argumento para criar files
//TYPEVAL_DIRETORIO como ultimo argumento para criar diretorios
int createDirEntry(int dirInodeNumber,char * fileLastName,int numeroInode,int typeVal){
	struct t2fs_inode diretorioInode;
	struct t2fs_record record;
	int i;
	int singleIndirFreeRecord;
	int doubleIndirFreeRecord;
	diretorioInode = leInode(dirInodeNumber);
	if(diretorioInode.blocksFileSize == 0){
		createDataBlock(dirInodeNumber,0);
		diretorioInode = leInode(dirInodeNumber);
		emptyDir(leInode(dirInodeNumber).dataPtr[0],dirInodeNumber);
		diretorioInode = leInode(dirInodeNumber);
	}
	if(diretorioInode.blocksFileSize > 0){
		carregaBloco(diretorioInode.dataPtr[0]);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_INVALIDO){
				if(typeVal == TYPEVAL_REGULAR)
					record.TypeVal     = TYPEVAL_REGULAR;
				if(typeVal == TYPEVAL_DIRETORIO)
					record.TypeVal = TYPEVAL_DIRETORIO;
				record.inodeNumber = numeroInode;
				strcpy(record.name,fileLastName);
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(diretorioInode.dataPtr[0]);
				
				return 1;

			}
		}
	}
	if(diretorioInode.blocksFileSize == 1){
		createDataBlock(dirInodeNumber,1);
		diretorioInode = leInode(dirInodeNumber);
		emptyDir(leInode(dirInodeNumber).dataPtr[1],dirInodeNumber);
		diretorioInode = leInode(dirInodeNumber);
	}
	if(diretorioInode.blocksFileSize > 1){	
		carregaBloco(diretorioInode.dataPtr[1]);
		for(i = 0; i < numeroRecords; i++) {

		memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_INVALIDO){
				if(typeVal == TYPEVAL_REGULAR)
					record.TypeVal     = TYPEVAL_REGULAR;
				if(typeVal == TYPEVAL_DIRETORIO)
					record.TypeVal = TYPEVAL_DIRETORIO;
				record.inodeNumber = numeroInode;
				strcpy(record.name,fileLastName);
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(diretorioInode.dataPtr[1]);
				return 1;

			}
		}
	}
	if(diretorioInode.blocksFileSize == 2)
		createDataBlock(dirInodeNumber,2);
	diretorioInode = leInode(dirInodeNumber);
	
	if(diretorioInode.blocksFileSize > 2){
		//printf("\nACHO QUE ESTE 'E O PTR %d ",diretorioInode.singleIndPtr);
		singleIndirFreeRecord = singleIndirRecordBlock(dirInodeNumber,diretorioInode.singleIndPtr);
		carregaBloco(singleIndirFreeRecord);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_INVALIDO){
				if(typeVal == TYPEVAL_REGULAR)
					record.TypeVal     = TYPEVAL_REGULAR;
				if(typeVal == TYPEVAL_DIRETORIO)
					record.TypeVal = TYPEVAL_DIRETORIO;
				record.inodeNumber = numeroInode;
				strcpy(record.name,fileLastName);
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(singleIndirFreeRecord);
				return 1;
			}
		}
	}
	if(diretorioInode.blocksFileSize == (tamanhoBlocoBytes/sizeof(DWORD) + 2))
		createDataBlock(dirInodeNumber,(tamanhoBlocoBytes/sizeof(DWORD) + 2));
	diretorioInode = leInode(dirInodeNumber);

	if(diretorioInode.blocksFileSize > (tamanhoBlocoBytes/sizeof(DWORD) + 2)){
		doubleIndirFreeRecord = doubleIndirRecordBlock(dirInodeNumber,diretorioInode.singleIndPtr);
		carregaBloco(doubleIndirFreeRecord);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_INVALIDO){
				if(typeVal == TYPEVAL_REGULAR)
					record.TypeVal     = TYPEVAL_REGULAR;
				if(typeVal == TYPEVAL_DIRETORIO)
					record.TypeVal = TYPEVAL_DIRETORIO;
				record.inodeNumber = numeroInode;
				strcpy(record.name,fileLastName);
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(doubleIndirFreeRecord);
				return 1;
			}
		}
	}


	return -1;
}
//funcao utilizada para remover bloco de indirecao, chama funcao que percorre os records dentro dos blocos apontados
void  deletaBlocoSingleIndir(int blocoIndireto){
	int i,bloco;	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void *)&bloco,(void*)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR)
			break;
		setBitmap2 (BITMAP_INODE, bloco,0);

	}
}
//funcao utilizada para percorrer remover blocos de dupla indirecao
void  deletaBlocoDoubleIndir(int blocoIndireto){
	int i,bloco;	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void *)&bloco,(void*)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR)
			break;
		deletaBlocoSingleIndir(bloco);
		setBitmap2 (BITMAP_INODE, bloco,0);
		

	}
}
int  entradasAbertasAux(int bloco,char * nome){
	int i;
	struct t2fs_record record;
	carregaBloco(bloco);
	for(i = 0; i < numeroRecords; i++) {
		memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
		if(record.TypeVal == TYPEVAL_REGULAR){
				

			if(procuraOpen(record.inodeNumber,TYPEVAL_REGULAR) < 0)
				return -1;

			}
			if(record.TypeVal == TYPEVAL_DIRETORIO){
				if(procuraOpen(record.inodeNumber,TYPEVAL_DIRETORIO) == 0){
					if(verificaEntradasAbertas(leInode(record.inodeNumber),nome) < 0);
						return -1;
				}
				else{
					return -1;
				}
			}
		
	}
	return 0;	

		

}
//funcao utilizada para percorrer bloco de indirecao, chama funcao que percorre os records dentro dos blocos apontados
int  verificaEntradasAbertasIndirecao(int blocoIndireto,char * nome){
	int i,bloco,arquivoAberto;
	char nomeCpy [1024];	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void*)&bloco,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR)
			break;
		strcpy(nomeCpy,nome);
		arquivoAberto =  entradasAbertasAux(bloco,nomeCpy);
		if(arquivoAberto <  0 )
			return -1;
		

	}
	return 0;
}
int verificaEntradasAbertasDuplaIndirecao(int blocoIndireto,char * nome){
	int i,bloco,arquivoAberto;
	char nomeCpy[1024];	
	for(i = 0; i < tamanhoBlocoBytes/sizeof(DWORD);i++){
		carregaBloco(blocoIndireto);
		memcpy((void*)&bloco,(void *)&blocoAtual[i*sizeof(DWORD)],sizeof(DWORD));
		if(bloco == INVALID_PTR)
			break;
		strcpy(nomeCpy,nome);
		arquivoAberto =  verificaEntradasAbertasIndirecao(bloco,nomeCpy);
		if(arquivoAberto <  0 )
			return -1;
		

	}
	return 0;
}
int verificaEntradasAbertas(struct t2fs_inode diretorioInode,char * nome){
	struct t2fs_record record;
	int i;
	char partialPath[59];
	char nomeCpy[1024];
	if(nome[0] == '/'){
		diretorioInode = diretorioRaizInode;
		i = 1;
		while(nome[i] != '\0'){	
			nome[i-1] = nome[i];
			i++;	
		}
		nome[i-1] = '\0';
	}
	caminhoParcial(nome,partialPath);
	if(diretorioInode.blocksFileSize > 0){
		carregaBloco(diretorioInode.dataPtr[0]);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_REGULAR){
				

				if(procuraOpen(record.inodeNumber,TYPEVAL_REGULAR) < 0)
					return -1;

			}
			if(record.TypeVal == TYPEVAL_DIRETORIO){
				if(procuraOpen(record.inodeNumber,TYPEVAL_DIRETORIO) == 0){
					strcpy(nomeCpy,nome);
					if(verificaEntradasAbertas(leInode(record.inodeNumber),nome) < 0);
						return -1;
				}
				else{
					return -1;
				}
			}
		}
	}
	if(diretorioInode.blocksFileSize > 1){	
		carregaBloco(diretorioInode.dataPtr[1]);
		for(i = 0; i < numeroRecords; i++) {
			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_REGULAR){
				

				if(procuraOpen(record.inodeNumber,TYPEVAL_REGULAR) < 0)
					return -1;

			}
			if(record.TypeVal == TYPEVAL_DIRETORIO){
				if(procuraOpen(record.inodeNumber,TYPEVAL_DIRETORIO) == 0){
					strcpy(nomeCpy,nome);
					if(verificaEntradasAbertas(leInode(record.inodeNumber),nome) < 0);
						return -1;
				}
				else{
					return -1;
				}
			}
		}
	}
	if(diretorioInode.blocksFileSize > 2){
		if(verificaEntradasAbertasIndirecao(diretorioInode.singleIndPtr,nome)< 0)
				return -1;		
	}
	if(diretorioInode.blocksFileSize > tamanhoBloco+2){
		if(verificaEntradasAbertasDuplaIndirecao(diretorioInode.singleIndPtr,nome)< 0)
			return -1;		
				
	}
		
	return 0;	
}
void refreshCurrentPath(){
	char thisDir[] = ".";
	int inodeNumber;
	inodeNumber = findDir(diretorioAtualInode,thisDir);
	diretorioAtualInode = leInode(inodeNumber);
	



}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////FUNCOES DE USUARIO/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int identify2 (char *name, int size){
	init();
	name = "Bernardo Neuhaus Lignati - 230159 \nCleiton Souza Lima - 262511\nLucas Augusto Tansini - 265038 \n";
	if (sizeof(name) > size)
		return -1;
	else
		return 0;
}
FILE2 create2 (char *filename){

	int numeroInode,indiceInodeBloco,dirInodeNumber;
	char tempFileName [1024];
	char dirName[1024];
	char fileLastName[1024];
	char thisDir [] = ".";
	init();
	strcpy(tempFileName, filename);
	struct t2fs_inode novoInode;
	//verifica se o nome do arquivo ja existe no disco
	if(findFile(diretorioAtualInode,tempFileName) >= 0)
		return -3;
	strcpy(tempFileName, filename);
	if(findDir(diretorioAtualInode,tempFileName) >= 0)
		return -3;

	int i;
	int HandleAvail = 0;
	for(i = 0; i<10; i++){
		if(fileHandleList[i].validade == NAO_VALIDO)
			HandleAvail = 1;		
	}
	if (HandleAvail == 0)
		return -8; //Nao há espaço no handle
	


	//to do
	numeroInode = findInodeLivre();
	if(numeroInode < 0)
		return -7;
	setBitmap2 (BITMAP_INODE, numeroInode,1);


	novoInode.blocksFileSize = 0;
	novoInode.bytesFileSize  = 0;	
	novoInode.dataPtr[0]     = INVALID_PTR;
	novoInode.dataPtr[1]     = INVALID_PTR;
	novoInode.singleIndPtr   = INVALID_PTR;
	novoInode.doubleIndPtr   = INVALID_PTR;
	//le propositalmente o lixo no local de memoria onde esta no inode para carregar o bloco certo na memoria
	leInode(numeroInode);
	indiceInodeBloco = numeroInode % (tamanhoBlocoBytes/INODE_SIZE);

	memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(novoInode)   ,INODE_SIZE);
	
	escreveBloco(blocoInodesInicial+((int)numeroInode/(tamanhoBlocoBytes/32)));
	
	switch(createFilePathParser(filename,dirName,fileLastName)){
		case 0:
			dirInodeNumber = findDir(diretorioAtualInode,thisDir);
		break;
		case 1:
			dirInodeNumber = findDir(diretorioRaizInode,dirName);
			break;
		case 2:
			dirInodeNumber = 0;
		break;
		case 3:
			dirInodeNumber = findDir(diretorioAtualInode,dirName);
		if(dirInodeNumber < 0)
			return -6;

	}
	
	if(createDirEntry(dirInodeNumber, fileLastName, numeroInode,TYPEVAL_REGULAR) < 0)
		return -5;
	refreshCurrentPath();
	strcpy(tempFileName, filename);
	return open2(tempFileName);
}


int delete2 (char *filename){
	int numeroInode;
	struct t2fs_inode inode;
	init();
	char pathNameCpy[1024];
	strcpy(pathNameCpy,filename);
	numeroInode = findFile(diretorioAtualInode,pathNameCpy);
	if(procuraOpen(numeroInode,TYPEVAL_REGULAR) < 0)
		return -2;
	strcpy(pathNameCpy,filename);
	findFile(diretorioAtualInode,pathNameCpy);
	if(numeroInode < 0)
		return -1;
	inode = leInode(numeroInode);
	strcpy(pathNameCpy,filename);
	numeroInode = findFileAndRemoveRecord(diretorioAtualInode,pathNameCpy);
	if(inode.blocksFileSize > 0){
		setBitmap2 (BITMAP_DADOS,inode.dataPtr[0], 0);
		inode.dataPtr[0] = INVALID_PTR;
	}
	if(inode.blocksFileSize > 1){
		setBitmap2 (BITMAP_DADOS,inode.dataPtr[1], 0);
		inode.dataPtr[0] = INVALID_PTR;
	}
	if(inode.blocksFileSize > 2){
		deletaBlocoSingleIndir(inode.singleIndPtr);
		setBitmap2 (BITMAP_DADOS,inode.singleIndPtr, 0);
		inode.singleIndPtr = INVALID_PTR;
	}
	if(inode.blocksFileSize > (tamanhoBloco + 2)){
		deletaBlocoDoubleIndir(inode.doubleIndPtr);
		setBitmap2 (BITMAP_DADOS,inode.doubleIndPtr, 0);
		inode.doubleIndPtr = INVALID_PTR;		
		leInode(numeroInode);
		escreveInode(inode,numeroInode);
	}
	setBitmap2 (BITMAP_INODE,numeroInode, 0);
	refreshCurrentPath();
	return 0;
}

FILE2 open2 (char *filename){
	int i,numeroInode;
	char pathNameCpy[1024];
	init();
	strcpy(pathNameCpy,filename);
	numeroInode = findFile(diretorioAtualInode,pathNameCpy);
	if(numeroInode < 0)
		return -1;
	i = 0;

	while(i<10){
		if(fileHandleList[i].validade == NAO_VALIDO){
			fileHandleList[i].validade = VALIDO;
			fileHandleList[i].seekPtr  = 0;
			fileHandleList[i].inodeNumber    = numeroInode;
			refreshCurrentPath();
			return i;
		}
		i++;	

	}
	return -2;
}
int close2 (FILE2 handle){
	init();
	
	fileHandleList[handle].validade = NAO_VALIDO;
	fileHandleList[handle].inodeNumber = -1;
	refreshCurrentPath();
	return 0;

}
int read2(FILE2 handle, char *buffer, int size){
	struct t2fs_inode inode;
	int bytesRestantes,blocoInicial,i,j;
	init();
	
	if(fileHandleList[handle].validade == NAO_VALIDO)
		return -1;
	
	if(fileHandleList[handle].inodeNumber < 0)
		return -2;
	


	inode =  leInode(fileHandleList[handle].inodeNumber);
	bytesRestantes = inode.bytesFileSize - fileHandleList[handle].seekPtr  ;
	blocoInicial = fileHandleList[handle].seekPtr/tamanhoBlocoBytes;
	//verifica se o arquivo nao 'e vazio
	if(inode.bytesFileSize == 0){
		refreshCurrentPath();
		return 0;
	}
	

	i = 0; 
	

	if(blocoInicial == 0){
		if(i < tamanhoBlocoBytes)
			j = fileHandleList[handle].seekPtr % tamanhoBlocoBytes;
		else 
			j = 0;
	
		if(i<bytesRestantes && i <size) {
			carregaBloco(inode.dataPtr[0]);
		}
		for(; j < tamanhoBlocoBytes && i < size && i < bytesRestantes;i++,j++){
			buffer[i] = blocoAtual[j];
		} 
	}
	
	bytesRestantes -= i;
	if(blocoInicial <= 1){
		if(i < tamanhoBlocoBytes)
			j = fileHandleList[handle].seekPtr % tamanhoBlocoBytes;
		else 
			j = 0;
	
		if(i<bytesRestantes && i <size) 
			carregaBloco(inode.dataPtr[1]);
		for(j=0; j < tamanhoBlocoBytes && i < size &&  i < bytesRestantes;i++,j++){
			buffer[i] = blocoAtual[j];
		} 
		
	}
	bytesRestantes -= i;
	
	//leitura bytes indirecao
	if (bytesRestantes > 0 && i < size){
		//read
		readArquivoIndirecao(inode.singleIndPtr,blocoInicial,&bytesRestantes,size,buffer,&i,handle);
		
	}
	if(bytesRestantes > 0 && i < size){
		readArquivoDuplaIndirecao(inode.singleIndPtr,blocoInicial,&bytesRestantes,size,buffer,&i,handle);

	}
	fileHandleList[handle].seekPtr += i;
	refreshCurrentPath();
	return i;		

}

/*
retorno -1: handle inválido
retorno -2: Inconsistencia de tamanhos
*/
int write2 (FILE2 handle,char *buffer, int size) {
	
	int addrBloc, posIniWri;
	int blocIni = 0;
	int restSize = size;
	struct t2fs_inode iNode;
	
	
	init();
	if(fileHandleList[handle].validade == NAO_VALIDO)
		return -1;	
	iNode = leInode(fileHandleList[handle].inodeNumber);
	

	posIniWri = fileHandleList[handle].seekPtr;
	while (posIniWri >= tamanhoBlocoBytes){
		//printf("posIniWri(%d) > tamanhoBlocoBytes(%d)\n", posIniWri,tamanhoBlocoBytes  );
		posIniWri -= tamanhoBlocoBytes;	
		blocIni++;
	}		
  
	
	while(restSize > 0){

		if(blocIni < iNode.blocksFileSize){ //escrevo em blocos já existentes
			//printf("bloco existente\n");
	
			addrBloc = getBlocoN(iNode, blocIni);
			carregaBloco(addrBloc);
		}
		//printf("restSize(%d) + posIniWri(%d)) > tamanhoBlocoBytes(%d)\n",restSize,posIniWri, tamanhoBlocoBytes  );
		if((restSize + posIniWri) > tamanhoBlocoBytes){	
	   
			memcpy((void*)&blocoAtual[posIniWri],(void*)&buffer[0],tamanhoBlocoBytes - posIniWri);
			restSize = restSize - (tamanhoBlocoBytes - posIniWri);
			buffer = &buffer[tamanhoBlocoBytes - posIniWri];
			posIniWri = 0;
		}
		else{
	  
			memcpy((void*)&blocoAtual[posIniWri],(void*)&buffer[0],restSize);
			restSize = 0;
			posIniWri = 0;				
		}
				
		if(blocIni < iNode.blocksFileSize){
			escreveBloco(addrBloc);
		}
		else{
			char *bufferBlocAux = malloc(tamanhoBlocoBytes);			
			memcpy((void*)&bufferBlocAux[0],(void*)&blocoAtual[0],tamanhoBlocoBytes);	
			addrBloc = createDataBlock(fileHandleList[handle].inodeNumber, blocIni);		
			memcpy((void*)&blocoAtual[0],(void*)&bufferBlocAux[0],tamanhoBlocoBytes);		
			escreveBloco(addrBloc);

			//free(bufferBlocAux);
		}			
		  
	
	}

	iNode = leInode(fileHandleList[handle].inodeNumber);
	if(blocIni + 1> iNode.blocksFileSize)
		iNode.blocksFileSize = blocIni + 1;
	if(fileHandleList[handle].seekPtr + size > iNode.bytesFileSize )
		iNode.bytesFileSize = fileHandleList[handle].seekPtr + size;	
	
	fileHandleList[handle].seekPtr = fileHandleList[handle].seekPtr + size;
	//printf("--->>>>>> Inode.sizeBloco:%d\n", iNode.blocksFileSize);
	escreveInode(iNode, fileHandleList[handle].inodeNumber);
	refreshCurrentPath();
	return size;
} 



int truncate2 (FILE2 handle) {
	
	struct t2fs_inode iNode;
	int i;
	iNode = leInode(fileHandleList[handle].inodeNumber);	
	int qtdBlocOld = iNode.blocksFileSize;
	int qtdBlocNew;
	init();
	if(fileHandleList[handle].validade == NAO_VALIDO)
		return -1;	
	if((fileHandleList[handle].seekPtr%tamanhoBlocoBytes) == 0)
		qtdBlocNew = fileHandleList[handle].seekPtr/tamanhoBlocoBytes;
	else 
		qtdBlocNew = (fileHandleList[handle].seekPtr/tamanhoBlocoBytes) + 1;
	
	
	for(i = qtdBlocNew; i < qtdBlocOld ; i++ ){
		setBitmap2(1,getBlocoN(iNode, i),0);
	}
	
	iNode.bytesFileSize = fileHandleList[handle].seekPtr;
	iNode.blocksFileSize = qtdBlocNew;
		
	fileHandleList[handle].seekPtr = fileHandleList[handle].seekPtr - 1;	

	escreveInode(iNode, fileHandleList[handle].inodeNumber);	
	//iNode = leInode(fileHandleList[handle].inodeNumber);
	refreshCurrentPath();	
	return 0;
	
}

/*
Return = -1 : FileHandle inválido
Return = -2 : offset resulta em uma posição fora do arquivo
*/
int seek2 (FILE2 handle,unsigned int offset){
	struct t2fs_inode iNode;
	init();
	if(fileHandleList[handle].validade == NAO_VALIDO)
		return -1;
	
	iNode = leInode(fileHandleList[handle].inodeNumber);	
	if(offset == -1){ //posiciona no final do arquivo
		fileHandleList[handle].seekPtr = iNode.bytesFileSize;
		return 0;
	}
	else{
		fileHandleList[handle].seekPtr = offset;
		refreshCurrentPath();
		return 0;		
	}	
}

int mkdir2 (char *pathname) {

	int numeroInode,indiceInodeBloco,dirInodeNumber;
	char tempFileName [1024];
	char dirName[1024];
	char fileLastName[1024];
	char thisDir [] = ".";
	char fatherDir [] = "..";
	init();	
	strcpy(tempFileName, pathname);
	struct t2fs_inode novoInode;
	//verifica se o nome do arquivo ja existe no disco
	if(findDir(diretorioAtualInode,tempFileName) >= 0)
		return -10;
	strcpy(tempFileName, pathname);
	if(findFile(diretorioAtualInode,tempFileName) >= 0)
		return -3;
	
	
	int i;
	int HandleAvail = 0;
	for(i = 0; i<10; i++){
		if(dirHandleList[i].validade == NAO_VALIDO)
			HandleAvail = 1;		
	}
	if (HandleAvail == 0)
		return -8; //Nao há espaço no handle	

	
	
	//to do
	//init();
	numeroInode = findInodeLivre();
	if(numeroInode < 0)
		return -7;
	setBitmap2 (BITMAP_INODE, numeroInode,1);


	novoInode.blocksFileSize = 0;
	novoInode.bytesFileSize  = 0;	
	novoInode.dataPtr[0]     = INVALID_PTR;
	novoInode.dataPtr[1]     = INVALID_PTR;
	novoInode.singleIndPtr   = INVALID_PTR;
	novoInode.doubleIndPtr   = INVALID_PTR;
	//le propositalmente o lixo no local de memoria onde esta no inode para carregar o bloco certo na memoria
	leInode(numeroInode);
	indiceInodeBloco = numeroInode % (tamanhoBlocoBytes/INODE_SIZE);

	memcpy((void*)&blocoAtual[INODE_SIZE * indiceInodeBloco],(void *)&(novoInode)   ,INODE_SIZE);
	
	escreveBloco(blocoInodesInicial+((int)numeroInode/(tamanhoBlocoBytes/32)));
	
	switch(createFilePathParser(pathname,dirName,fileLastName)){
		case 0:
			dirInodeNumber = findDir(diretorioAtualInode,thisDir);
			strcpy(thisDir,".");

		break;
		case 1:
			dirInodeNumber = findDir(diretorioRaizInode,dirName);

			break;
		case 2:

			dirInodeNumber = 0;
		break;
		case 3:

			dirInodeNumber = findDir(diretorioAtualInode,dirName);
		break;

	}
	
		if(dirInodeNumber < 0)
			return -6;



	
	if(createDirEntry(dirInodeNumber, fileLastName, numeroInode,TYPEVAL_DIRETORIO) < 0)
		return -5;
	if(createDirEntry(numeroInode,thisDir ,numeroInode,TYPEVAL_DIRETORIO) < 0)
		return -8;
	if(createDirEntry(numeroInode,fatherDir,dirInodeNumber,TYPEVAL_DIRETORIO) < 0)
		return -9;
	


	refreshCurrentPath();

	strcpy(tempFileName,pathname);
	return opendir2(tempFileName);


}
int deleteThisInodeRecord(int  deletadoInodeNumero){
	struct t2fs_inode deletadoInode,diretorioInode;
	int i,diretorioInodeNumero,blocoRecords;
	struct t2fs_record record;	
	char  fatherInode [] = "..";
	deletadoInode = leInode(deletadoInodeNumero);
	
	diretorioInodeNumero = findDir(deletadoInode,fatherInode);
	if(diretorioInodeNumero < 0){
		return -1;
		
	}
	
	diretorioInode = leInode(diretorioInodeNumero);

	if(diretorioInode.blocksFileSize > 0){
		carregaBloco(diretorioInode.dataPtr[0]);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_DIRETORIO && record.inodeNumber  == deletadoInodeNumero){
				record.TypeVal = TYPEVAL_INVALIDO;
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(diretorioInode.dataPtr[0]);
				return 1;

			}
		}
	}

	if(diretorioInode.blocksFileSize > 1){	
		carregaBloco(diretorioInode.dataPtr[1]);
		for(i = 0; i < numeroRecords; i++) {
			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));
			if(record.TypeVal == TYPEVAL_DIRETORIO && record.inodeNumber  == deletadoInodeNumero){
				record.TypeVal = TYPEVAL_INVALIDO;
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(diretorioInode.dataPtr[1]);
				return 1;

			}
		}
	}
	if(diretorioInode.blocksFileSize > 2){
		record = procuraRecordsIndirecao2(diretorioInode.singleIndPtr,deletadoInodeNumero,&blocoRecords);
		carregaBloco(blocoRecords);

		for(i = 0; i < numeroRecords; i++) {
			
			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));

			if(record.TypeVal == TYPEVAL_DIRETORIO && record.inodeNumber  == deletadoInodeNumero){
				record.TypeVal = TYPEVAL_INVALIDO;
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));
				escreveBloco(blocoRecords);
				return 1;

			}
		}
	}
	if(diretorioInode.blocksFileSize > tamanhoBloco+2){
			record = procuraRecordsDuplaIndirecao2(diretorioInode.singleIndPtr,deletadoInodeNumero,&blocoRecords);
			if(record.inodeNumber < 0)
				return -1;
		carregaBloco(blocoRecords);
		for(i = 0; i < numeroRecords; i++) {

			memcpy((void*)&record,(void *)&blocoAtual[i*64],sizeof(struct t2fs_record));

			if(record.TypeVal == TYPEVAL_DIRETORIO && record.inodeNumber  == deletadoInodeNumero){
				record.TypeVal = TYPEVAL_INVALIDO;
				memcpy((void *)&blocoAtual[i*64],(void*)&record,sizeof(struct t2fs_record));	
				escreveBloco(blocoRecords);
				return 1;

			}
		
		}
	}	
	return -1;
	


}
int readdirAux (int inodeNumber,int *seekPointer, DIRENT2 *dentry) {
	struct t2fs_record record;
	struct t2fs_inode diretorioInode,recordInode;
	init();
	int numeroInternoRecord;
	int numeroBlocoRecord;
	numeroBlocoRecord   = *seekPointer   /(tamanhoBlocoBytes/sizeof(struct t2fs_record));
	numeroInternoRecord = *seekPointer % (tamanhoBlocoBytes/sizeof(struct t2fs_record));
	diretorioInode = leInode(inodeNumber);		
	if(numeroBlocoRecord >= 0){
		carregaBloco(diretorioInode.dataPtr[0]);
			do{

			memcpy((void*)&record,(void *)&blocoAtual[numeroInternoRecord*64],sizeof(struct t2fs_record));
			numeroInternoRecord ++;
			}while(!(record.TypeVal == TYPEVAL_REGULAR || record.TypeVal == TYPEVAL_DIRETORIO)
			&& numeroInternoRecord < (tamanhoBlocoBytes/sizeof(record)));
			
			if(numeroInternoRecord < tamanhoBlocoBytes/sizeof(record)){
				*seekPointer = numeroBlocoRecord * (tamanhoBlocoBytes/sizeof(record)) + numeroInternoRecord;

				//monta estrutura de retorno
				strcpy(dentry->name,record.name);

				recordInode      = leInode(record.inodeNumber);
				dentry->fileSize = recordInode.bytesFileSize;
				dentry->fileType = record.TypeVal;
				
				return 0;
			}else if(numeroInternoRecord >= tamanhoBlocoBytes/sizeof(record)){
				return -1;

			}
			else{ 
				numeroInternoRecord = 0;
				numeroBlocoRecord = 1;
			}
			
	}
	if(numeroBlocoRecord >= 1){
		carregaBloco(diretorioInode.dataPtr[1]);

			do{
			memcpy((void*)&record,(void *)&blocoAtual[numeroInternoRecord*64],sizeof(struct t2fs_record));
			numeroInternoRecord ++;
			}while(!(record.TypeVal == TYPEVAL_REGULAR || record.TypeVal == TYPEVAL_DIRETORIO) 
			&& numeroInternoRecord < (tamanhoBlocoBytes/sizeof(record)));
			
			if(numeroInternoRecord < tamanhoBlocoBytes/sizeof(record)){
				*seekPointer  = numeroBlocoRecord * (tamanhoBlocoBytes/sizeof(record)) + numeroInternoRecord;
				//monta estrutura de retorno
				strcpy(dentry->name,record.name);
				recordInode      = leInode(record.inodeNumber);
				dentry->fileSize = recordInode.bytesFileSize;
				dentry->fileType = record.TypeVal;
				
				return 0;
			
			}else if(numeroInternoRecord >= tamanhoBlocoBytes/sizeof(record)){
				return -1;

			}
			else{ 
				numeroInternoRecord = 0;
				numeroBlocoRecord = 2;
			}
			
	}
	if(numeroBlocoRecord >= 2 && numeroBlocoRecord <(tamanhoBlocoBytes/sizeof(DWORD)) + 2){
		
		do{
			
			record = procuraDirEntryIndirecao(diretorioInode.singleIndPtr,numeroInternoRecord,numeroBlocoRecord-2);
			//printf("%d\n",record.TypeVal);
			(*seekPointer) ++;
			numeroBlocoRecord   = (*seekPointer)   /(tamanhoBlocoBytes/sizeof(struct t2fs_record));
			numeroInternoRecord = (*seekPointer) % (tamanhoBlocoBytes/sizeof(struct t2fs_record));

		}while(record.TypeVal == TYPEVAL_INVALIDO &&
		( *seekPointer) <= (diretorioInode.bytesFileSize/ sizeof(struct t2fs_record))&&
		 (*seekPointer )* sizeof(struct t2fs_record) <= tamanhoBlocoBytes * (tamanhoBlocoBytes/sizeof(DWORD)));
		if(record.TypeVal != TYPEVAL_INVALIDO &&  (*seekPointer) < (diretorioInode.bytesFileSize/sizeof(struct t2fs_record))){
			//monta estrutura de retorno
			strcpy(dentry->name,record.name);
			recordInode      = leInode(record.inodeNumber);
			dentry->fileSize = recordInode.bytesFileSize;
			dentry->fileType = record.TypeVal;
			refreshCurrentPath();	
			return 0;
		
		}else if(numeroInternoRecord > tamanhoBlocoBytes/sizeof(record)){
				return -1;

		}
	}
	if(numeroBlocoRecord >=(tamanhoBlocoBytes/sizeof(DWORD) + 2)){
		do{
			record = procuraDirEntryDuplaIndirecao(diretorioInode.singleIndPtr,numeroInternoRecord,numeroBlocoRecord-2);
			(*seekPointer) ++;
			numeroBlocoRecord   = (*seekPointer)   /(tamanhoBlocoBytes/sizeof(struct t2fs_record));
			numeroInternoRecord = (*seekPointer) % (tamanhoBlocoBytes/sizeof(struct t2fs_record));
			
		}while(record.TypeVal == TYPEVAL_INVALIDO &&
		 (*seekPointer)   <= (diretorioInode.bytesFileSize/sizeof(struct t2fs_record))&&
		 (*seekPointer) * sizeof(struct t2fs_record) <= tamanhoBlocoBytes * (tamanhoBlocoBytes/sizeof(DWORD)) * (tamanhoBlocoBytes/sizeof(DWORD)));
		if(record.TypeVal != TYPEVAL_INVALIDO &&  (*seekPointer) < (diretorioInode.bytesFileSize/sizeof(struct t2fs_record))){
			//monta estrutura de retorno
			strcpy(dentry->name,record.name);
			recordInode      = leInode(record.inodeNumber);
			dentry->fileSize = recordInode.bytesFileSize;
			dentry->fileType = record.TypeVal;
			refreshCurrentPath();	
			return 0;
		
		}else if(numeroInternoRecord > tamanhoBlocoBytes/sizeof(record)){
				return -1;

		}	

	}

	
	return -1;





}
int diretorioVazio(int numeroInode){
	int i,seekPtr;
	seekPtr = 0;
	i = 0;
	DIRENT2 dirEntry;
	while(readdirAux(numeroInode,&seekPtr,&dirEntry) == 0 ){
		if(i > 1)
			return -1;
		i++;
		
	}
	//o diretorio vazio tera apena as entradas "." e ".."
	refreshCurrentPath();
	return 0;

}
int rmdir2 (char *pathname) {
	char tempPathName[1024];
	int numeroInode;
	struct t2fs_inode inode;
	init();
	strcpy(tempPathName,pathname);
	numeroInode = findDir(diretorioAtualInode,tempPathName);
	if(numeroInode < 0)
		return -1;

	inode = leInode(numeroInode);
	if(diretorioVazio < 0){

		return -1;
	}

	strcpy(tempPathName,pathname);
	if(procuraOpen(numeroInode,TYPEVAL_DIRETORIO) < 0)
		return -1;
	

	if(strcmp(pathname,"/") != 0){
		strcpy(tempPathName,pathname);
		deleteThisInodeRecord(numeroInode);
	}
	strcpy(tempPathName,pathname);
		inode = leInode(numeroInode);
	if(inode.blocksFileSize > 0){
		setBitmap2 (BITMAP_DADOS,inode.dataPtr[0], 0);
		inode.dataPtr[0] = INVALID_PTR;
	}
	if(inode.blocksFileSize > 1){
		setBitmap2 (BITMAP_DADOS,inode.dataPtr[1], 0);
		inode.dataPtr[0] = INVALID_PTR;
	}
	if(inode.blocksFileSize > 2){
		deletaBlocoSingleIndir(inode.singleIndPtr);
		setBitmap2 (BITMAP_DADOS,inode.singleIndPtr, 0);
		inode.singleIndPtr = INVALID_PTR;
	}
	if(inode.blocksFileSize > (tamanhoBloco + 2)){
		deletaBlocoDoubleIndir(inode.doubleIndPtr);
		setBitmap2 (BITMAP_DADOS,inode.doubleIndPtr, 0);
		inode.doubleIndPtr = INVALID_PTR;		
		leInode(numeroInode);
		escreveInode(inode,numeroInode);
	}
	setBitmap2 (BITMAP_INODE,numeroInode, 0);
	refreshCurrentPath();
	return 0;

}
int chdir2(char *pathname){
	int numeroInode;
	char tempPathName[1024];
	init();
	strcpy(tempPathName,pathname);
	numeroInode = findDir(diretorioAtualInode,tempPathName);

	if(numeroInode < 0)
		return -1;
	diretorioAtualInode = leInode(numeroInode);
	if(pathname[0] == '/'){
		strcpy(currentPathName,pathname);
	}
	else{	
		if(strcmp(currentPathName,"/") != 0)
			strcat(currentPathName,"/");
		strcat(currentPathName,pathname);
	}
	return 0;
}
int getcwd2 (char *pathname, int size) {
	init();
	//printf("%s %d \n ",currentPathName,strlen(currentPathName));
	if(strlen(currentPathName) > size)
		return -1;
	
	else
		strcpy(pathname,currentPathName);

	return 0;
}
DIR2 opendir2 (char *pathname){
	int i,numeroInode;
	char pathNameCpy[1024];
	strcpy(pathNameCpy,pathname);
	init();
	
	numeroInode = findDir(diretorioAtualInode,pathNameCpy);
	if(numeroInode < 0)
		return -1;
	i = 0;
	while(i<10){
		if(dirHandleList[i].validade == NAO_VALIDO){
			dirHandleList[i].validade = VALIDO;
			dirHandleList[i].seekPtr  = 0;
			dirHandleList[i].inodeNumber    = numeroInode;
			refreshCurrentPath();
			return i;
		}
		i++;	

	}
	return -2;


}
int readdir2 (DIR2 handle, DIRENT2 *dentry) {
	struct t2fs_record record;
	struct t2fs_inode diretorioInode,recordInode;	
	init();
	int numeroInternoRecord;
	int numeroBlocoRecord;
	numeroBlocoRecord   = dirHandleList[handle].seekPtr   /(tamanhoBlocoBytes/sizeof(struct t2fs_record));
	numeroInternoRecord = dirHandleList[handle].seekPtr % (tamanhoBlocoBytes/sizeof(struct t2fs_record));
	diretorioInode = leInode(dirHandleList[handle].inodeNumber);
	if(handle < 0)
		return -1;		
	if(numeroBlocoRecord == 0){
		carregaBloco(diretorioInode.dataPtr[0]);	
			do{
			
			memcpy((void*)&record,(void *)&blocoAtual[numeroInternoRecord*64],sizeof(struct t2fs_record));
			numeroInternoRecord ++;
			}while(!(record.TypeVal == TYPEVAL_REGULAR || record.TypeVal == TYPEVAL_DIRETORIO)
			&& numeroInternoRecord <=(tamanhoBlocoBytes/sizeof(record)));
			if(numeroInternoRecord <= tamanhoBlocoBytes/sizeof(record)){
				dirHandleList[handle].seekPtr = numeroBlocoRecord * (tamanhoBlocoBytes/sizeof(record)) + numeroInternoRecord;

				//monta estrutura de retorno
				strcpy(dentry->name,record.name);

				recordInode      = leInode(record.inodeNumber);
				dentry->fileSize = recordInode.bytesFileSize;
				dentry->fileType = record.TypeVal;
				refreshCurrentPath();
				return 0;
			}else if(numeroInternoRecord > tamanhoBlocoBytes/sizeof(record)){
				return -1;

			}
			else{ 
				numeroInternoRecord = 0;
				numeroBlocoRecord = 1;
			}
			
	}
	if(numeroBlocoRecord == 1){
		carregaBloco(diretorioInode.dataPtr[1]);

			do{
			
			memcpy((void*)&record,(void *)&blocoAtual[numeroInternoRecord*64],sizeof(struct t2fs_record));
			numeroInternoRecord ++;
			}while(!(record.TypeVal == TYPEVAL_REGULAR || record.TypeVal == TYPEVAL_DIRETORIO) 
			&& numeroInternoRecord <= (tamanhoBlocoBytes/sizeof(record)));
			if(numeroInternoRecord <= tamanhoBlocoBytes/sizeof(record)){
				dirHandleList[handle].seekPtr  = numeroBlocoRecord * (tamanhoBlocoBytes/sizeof(record)) + numeroInternoRecord;
				//monta estrutura de retorno
				strcpy(dentry->name,record.name);
				recordInode      = leInode(record.inodeNumber);
				dentry->fileSize = recordInode.bytesFileSize;
				dentry->fileType = record.TypeVal;
				refreshCurrentPath();
				return 0;
			
			}else if(numeroInternoRecord >= tamanhoBlocoBytes/sizeof(record)){
				return -1;

			}
			else{ 
				numeroInternoRecord = 0;
				numeroBlocoRecord = 2;
			}
			
	}
	if(numeroBlocoRecord >= 2 && numeroBlocoRecord <(tamanhoBlocoBytes/sizeof(DWORD)) + 2){
		
		do{
			
			record = procuraDirEntryIndirecao(diretorioInode.singleIndPtr,numeroInternoRecord,numeroBlocoRecord-2);
			//printf("%d\n",record.TypeVal);
			dirHandleList[handle].seekPtr ++;
			numeroBlocoRecord   = dirHandleList[handle].seekPtr   /(tamanhoBlocoBytes/sizeof(struct t2fs_record));
			numeroInternoRecord = dirHandleList[handle].seekPtr % (tamanhoBlocoBytes/sizeof(struct t2fs_record));

		}while(record.TypeVal == TYPEVAL_INVALIDO &&
		 dirHandleList[handle].seekPtr <= (diretorioInode.bytesFileSize/ sizeof(struct t2fs_record))&&
		 dirHandleList[handle].seekPtr * sizeof(struct t2fs_record) <= tamanhoBlocoBytes * (tamanhoBlocoBytes/sizeof(DWORD)));
		if(record.TypeVal != TYPEVAL_INVALIDO &&  dirHandleList[handle].seekPtr < (diretorioInode.bytesFileSize/sizeof(struct t2fs_record))){
			//monta estrutura de retorno
			strcpy(dentry->name,record.name);
			recordInode      = leInode(record.inodeNumber);
			dentry->fileSize = recordInode.bytesFileSize;
			dentry->fileType = record.TypeVal;
			refreshCurrentPath();	
			return 0;
		
		}else if(numeroInternoRecord > tamanhoBlocoBytes/sizeof(record)){
				return -1;

		}
	}
	if(numeroBlocoRecord >=(tamanhoBlocoBytes/sizeof(DWORD) + 2)){
		do{
			record = procuraDirEntryDuplaIndirecao(diretorioInode.singleIndPtr,numeroInternoRecord,numeroBlocoRecord-2);
			dirHandleList[handle].seekPtr ++;
			numeroBlocoRecord   = dirHandleList[handle].seekPtr   /(tamanhoBlocoBytes/sizeof(struct t2fs_record));
			numeroInternoRecord = dirHandleList[handle].seekPtr % (tamanhoBlocoBytes/sizeof(struct t2fs_record));
			
		}while(record.TypeVal == TYPEVAL_INVALIDO &&
		 dirHandleList[handle].seekPtr   <= (diretorioInode.bytesFileSize/sizeof(struct t2fs_record))&&
		 dirHandleList[handle].seekPtr * sizeof(struct t2fs_record) <= tamanhoBlocoBytes * (tamanhoBlocoBytes/sizeof(DWORD)) * (tamanhoBlocoBytes/sizeof(DWORD)));
		if(record.TypeVal != TYPEVAL_INVALIDO &&  dirHandleList[handle].seekPtr < (diretorioInode.bytesFileSize/sizeof(struct t2fs_record))){
			//monta estrutura de retorno
			strcpy(dentry->name,record.name);
			recordInode      = leInode(record.inodeNumber);
			dentry->fileSize = recordInode.bytesFileSize;
			dentry->fileType = record.TypeVal;
			refreshCurrentPath();	
			return 0;
		
		}else if(numeroInternoRecord > tamanhoBlocoBytes/sizeof(record)){
				return -1;

		}	

	}

	
	return -1;




}

int closedir2 (DIR2 handle) {
	init();
	dirHandleList[handle].validade = NAO_VALIDO;
	dirHandleList[handle].inodeNumber = -1;
	refreshCurrentPath();
	return 0;
}

////////////////////////////MAIN/////////////////////////////////////////


