#include <stdio.h>
#include "../include/t2fs.h"
int main(){
	char dir[] = "/AAA";	
	//char dir2[] = "/AAA/BBB";	
	//char dir3[] = "./CCC";	
	char  buffer[5];
	int i;
	printf("criando um novo diretório (ERRO %d)\n", mkdir2(dir));
	
	printf("Diretório Atual(ERRO %d):", getcwd2 (buffer,5));	
	for (i = 0; i< 4; i++){
		if(buffer[i] == '\0')
			break;
		printf("%s", buffer);
	}
	printf("\n");	
	
	
	printf("Alterando diretório atual(ERRO %d)\n", chdir2(dir));
	printf("Criando 9 diretórios...\n");
	printf("A0 --- (ERRO %d)\n", mkdir2("/A0"));
	printf("A1 --- (ERRO %d)\n", mkdir2("/A1"));
	printf("A2 --- (ERRO %d)\n", mkdir2("/A2"));
	printf("A3 --- (ERRO %d)\n", mkdir2("/A3"));
	printf("A4 --- (ERRO %d)\n", mkdir2("/A4"));
	printf("A5 --- (ERRO %d)\n", mkdir2("/A5"));
	printf("A6 --- (ERRO %d)\n", mkdir2("/A6"));
	printf("A7 --- (ERRO %d)\n", mkdir2("/A7"));
	printf("A8 --- (ERRO %d)\n", mkdir2("/A8"));
	
	printf("Diretório Atual(ERRO %d):", getcwd2 (buffer,5));	
	for (i = 0; i< 4; i++){
		if(buffer[i] == '\0')
			break;
		printf("%s", buffer);
	}
	printf("\n");	
	
	printf("Tentando criar mais 1\n");
	printf("B --- (ERRO %d)\n", mkdir2("/B"));
	printf("Fechando... %d\n", closedir2(5));
	printf("Fechando um...\nTentando criar um de novo\n");
	printf("B --- (ERRO %d)\n", mkdir2("/B"));
	
	
	
	
	
	printf("FIM TESTES\n");
	return 0;
	
	
}









