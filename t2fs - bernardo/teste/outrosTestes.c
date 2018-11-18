/*char buffer2 [] = "_________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________Teste de escrita___________________________________					   ___________________________________________________________________________________________________________________________________________________________________________________________________________________________";
=======
	char buffer2 [] = "----11111-----------------------------------------------------------------------------------------";
>>>>>>> 29aaf49a0b6826df459b09947984045e494a38b5
	char dir[80];
	char fileS[80];
	int loop;
	int c;
	int dirHandle;
	FILE2 fileHandle;
	struct t2fs_inode Inode;
	init();


	
	
	
	

	mkdir2(dirPath);

	printf("%d\n",chdir2(novoDirPath));
	create2(filePath);
	readAndPrintDir(diretorioAtualInode);*/

	/*
	file = open2(path);
	Inode = leInode(fileHandleList[0].inodeNumber);	
	read2(file, buffer, Inode.bytesFileSize);
	printf("Arquivo lido: %s\n\n", buffer);

	printf("Write -- Erro: %d\n", write2(file, buffer, 5));
	Inode = leInode(fileHandleList[0].inodeNumber);		
	fileHandleList[0].seekPtr = 0;
	
	read2(file, buffer2, Inode.bytesFileSize);
	printf("Arquivo lido: %s\n\n", buffer2);	
	
	printf("FIM\n");
	*/
/*
	//TESTE WRITE
	FILE2 file;
	char path [] = "/file3";
	file = open2(path);
	Inode = leInode(fileHandleList[0].inodeNumber);	
	printf("tam: %d\n", Inode.bytesFileSize);
	read2(file, buffer, Inode.bytesFileSize);
	for(i = 0; i < Inode.bytesFileSize; i++)
		printf("%c", buffer[i]);
	printf("\n");
	
	seek2(0,-1);
	printf("Write -- Erro: %d\n", write2(file, buffer2,16));
	Inode = leInode(fileHandleList[0].inodeNumber);		
	fileHandleList[0].seekPtr = 0;
	read2(file, buffer3, Inode.bytesFileSize);		
	for(i = 0; i < Inode.bytesFileSize; i++)
		printf("%c", buffer3[i]);
	printf("\n");	
	
	printf("tam: %d\n", Inode.bytesFileSize);	
		
	printf("FIM\n");	
*/

	/*
	//TESTE WRITE multiBlocos
	FILE2 file;
	char path [] = "/file3";
	file = open2(path);
	Inode = leInode(fileHandleList[0].inodeNumber);	
	printf("tam: %d\n", Inode.bytesFileSize);
	read2(file, buffer, Inode.bytesFileSize);
	for(i = 0; i < Inode.bytesFileSize; i++)
		printf("%c", buffer[i]);
	printf("\n");
	
	seek2(0,-1);
	printf("Write -- Erro: %d\n", write2(file, buffer2,500));
	Inode = leInode(fileHandleList[0].inodeNumber);		
	fileHandleList[0].seekPtr = 0;
	read2(file, buffer3, Inode.bytesFileSize);		
	for(i = 0; i < Inode.bytesFileSize; i++)
		printf("%c", buffer3[i]);
	printf("\n");	
	
	printf("tam: %d\n", Inode.bytesFileSize);	
		
	printf("FIM\n");
	*/
	
/*
	//teste truncate, seek e escreveInode
	char path [] = "/file3";
	file = open2(path);
	printf("ARQUIVO ABERTO COM HANDLE %d\n",file);
	Inode = leInode(fileHandleList[0].inodeNumber);
		
	loop = read2(file, buffer, Inode.bytesFileSize);	
	printf("Tamanho Lido %d\n",loop);
	printf("Arquivo lido: %s\n", buffer);
	
	fileHandleList[0].seekPtr = 0;	
	seek2 (file,6);
	printf("Seek realizado na pos %d\n", fileHandleList[0].seekPtr);
	printf("truncate -- qtd erros: %d\n", truncate2(file));
	
	fileHandleList[0].seekPtr = 0; //reposiciona no início do arq		
	Inode = leInode(fileHandleList[0].inodeNumber);	
	loop = read2(file, buffer2, Inode.bytesFileSize);
	printf("Tamanho Lido %d\n",loop);	
	printf("Arquivo lido: %s\n", buffer2);
	
	printf("\nFIM EXECUCAO\n");
*/

/*
//teste truncate2
	FILE2 file;
	char path [] = "/file3";
	file = open2(path);
	Inode = leInode(fileHandleList[0].inodeNumber);	
	
	while(Inode.blocksFileSize < 3){
		read2(file, buffer, Inode.bytesFileSize);
		seek2(0,-1);
		printf("Write -- Erro: %d\n", write2(file, buffer2,80));
		Inode = leInode(fileHandleList[0].inodeNumber);		
	}	
	
	printf("Número de blocos: %d\nNumero bytes: %d\n", Inode.blocksFileSize, Inode.bytesFileSize);
	read2(file, buffer3, Inode.bytesFileSize);		
	for(i = 0; i < Inode.bytesFileSize; i++)
		printf("%c", buffer3[i]);
	printf("\n");	
		
		
	printf("ANTES TRUNCATE\nNúmero de blocos: %d\nNumero bytes: %d\n\n", Inode.blocksFileSize, Inode.bytesFileSize);
	seek2(0, 10);
	truncate2(0);
	seek2(0, 0);
	printf("leitura do arquivo : ");
	Inode = leInode(fileHandleList[0].inodeNumber);		
	read2(file, buffer, Inode.bytesFileSize);		
	for(i = 0; i < Inode.bytesFileSize; i++)
		printf("%c", buffer[i]);
	printf("\n");
	
	printf("DEPOIS TRUNCATE\nNúmero de blocos: %d\nNumero bytes: %d\n\n", Inode.blocksFileSize, Inode.bytesFileSize);
	printf("FIM\n");	

*/

	
	
	
	/*
	printf("%d\n",create2(path));
	readAndPrintDir(leInode(0));
	printInode(leInode(13));
	return 0;
	*/



	/*
	for (j = 0; j < 5; j++ ){
		if (getBitmap2(0,j) == 1){
			inode = leInode(j);
			printf("inode: %d ---> " , j);
			listBloc = getListPointer(inode);
			printf("fim getListPointer...\n");
			//if (inode.blocksFileSize > 2)
			//	printf("inode: %d", i);
			//printf("inode lido...\n");
			//printInode(leInode(i));
		}			
	}
	*/
	
	/*
	createDataBlock(0,2);
	leInode(0);
	carregaBloco((leInode(0).singleIndPtr));
	for(i = 0;i<tamanhoBlocoBytes/sizeof(DWORD);i++)
		printf("%d -> %d\n",blocoAtual[i],i);
	*/
