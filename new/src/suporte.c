#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/suporte.h"
#include "../include/t2fs.h"

#define LIMITES_ABERTOS 10 

int disco_iniciado = 0;

DISK_DIR diretorios_abertos[LIMITES_ABERTOS];

DWORD converter_para_DWORD(unsigned char* buffer) {
    return (DWORD) ((DWORD)buffer[0] | (DWORD)buffer[1] << 8 |(DWORD)buffer[2] << 16 |(DWORD)buffer[3] << 24 );
}

WORD converter_para_WORD(unsigned char* buffer) {
    return (WORD) ((WORD)buffer[0] | (WORD)buffer[1] << 8);
}

unsigned char* dword_para_endereco(DWORD entry) {
    unsigned char* buffer = malloc(sizeof(unsigned char)*4);

    buffer[0] = entry;
    buffer[1] = entry >> 8;
    buffer[2] = entry >> 16;
    buffer[3] = entry >> 24;

    return buffer;
}

int iniciar_disco() {
    if(!disco_iniciado){
        int i;
        unsigned char buffer[SECTOR_SIZE] = {0};
        
        if (read_sector(0,buffer) != 0) {
            return -1;
        }

        memcpy(super_bloco.id, buffer, 4);
        super_bloco.version = converter_para_WORD(buffer + 4);
        super_bloco.superblockSize = converter_para_WORD(buffer + 6);
        super_bloco.DiskSize = converter_para_DWORD(buffer + 8);
        super_bloco.NofSectors = converter_para_DWORD(buffer + 12);
        super_bloco.SectorsPerCluster = converter_para_DWORD(buffer + 16);
        super_bloco.pFATSectorStart = converter_para_DWORD(buffer + 20);
        super_bloco.RootDirCluster = converter_para_DWORD(buffer + 24);
        super_bloco.DataSectorStart = converter_para_DWORD(buffer + 28); 


        for (i = 0; i < LIMITES_ABERTOS; i++) {
            arquivos_abertos[i].file = -1;
            arquivos_abertos[i].currPointer = -1;
            arquivos_abertos[i].clusterNo = -1;
            diretorios_abertos[i].handle = -1;
            diretorios_abertos[i].noReads=-1;
            diretorios_abertos[i].clusterDir= -1;
            diretorios_abertos[i].directory=setNullDirent();
        }

        caminho_atual.absolute = malloc(sizeof(char)*5); 
        strcpy(caminho_atual.absolute, "/");
        caminho_atual.clusterNo = super_bloco.RootDirCluster;
        
        disco_iniciado = 1;
        
    }
    return 0;
}


int escrever_FAT(int clusterNo, DWORD value) {
    int offset = clusterNo/64;
    unsigned int sector = super_bloco.pFATSectorStart + offset;
    int sectorOffset = (clusterNo % 64)*4;
    unsigned char buffer[SECTOR_SIZE] = {0};
    unsigned char* writeValue = malloc(sizeof(unsigned char)*4);
    DWORD badSectorCheck;

    if (value == 0x00000001) {
        return -1;
    }

    if (sector >= super_bloco.pFATSectorStart && sector < super_bloco.DataSectorStart) { 
        ler_FAT(clusterNo, &badSectorCheck);
        if (badSectorCheck == BAD_SECTOR) { 
            return -1;
        }

        read_sector(sector,buffer);
        writeValue = dword_para_endereco(value);
        memcpy(buffer + sectorOffset, writeValue,4);
        write_sector(sector,buffer);

        free(writeValue);

        
        return 0;
    }
    return -1;
}

int ler_FAT(int clusterNo, DWORD* value) {
    int offset = clusterNo/64;
    unsigned int sector = super_bloco.pFATSectorStart + offset;
    int sectorOffset = (clusterNo % 64)*4;
    unsigned char buffer[SECTOR_SIZE];

    if (sector >= super_bloco.pFATSectorStart && sector < super_bloco.DataSectorStart) { 
        read_sector(sector,buffer);
        *value = converter_para_DWORD(buffer + sectorOffset);
        return 0;
    }
    return -1;
}


int escrever_cluster_pasta(int clusterNo, struct t2fs_record folder) {
    int i;
    int k = 0;
    int written = 0;
    unsigned int sectorToWrite;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    unsigned int sector = super_bloco.DataSectorStart + super_bloco.SectorsPerCluster*clusterNo;
    
    if (sector >= super_bloco.DataSectorStart && sector < super_bloco.NofSectors) {
        ler_cluster(clusterNo, buffer);

        for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
            if ( ((BYTE) buffer[i]) == 0 && !written ) {
                memcpy(buffer + i,&(folder.TypeVal),1);
                memcpy((buffer + i + 1),folder.name,51);
                memcpy((buffer + i + 52),dword_para_endereco(folder.bytesFileSize),4);
                memcpy((buffer + i + 56),dword_para_endereco(folder.clustersFileSize),4);
                memcpy((buffer + i + 60),dword_para_endereco(folder.firstCluster),4);
                written = 1;
            } 
        }

        for(sectorToWrite = sector; sectorToWrite < (sector + super_bloco.SectorsPerCluster); sectorToWrite++) {
            write_sector(sectorToWrite, buffer + k);
            k += 256;
        }
        free(buffer);
        if (written) {
            return 0;
        } else {
            return -1;
        }
    }
    free(buffer);
    return -1;
}

int ler_cluster(int clusterNo, unsigned char* buffer) {
    int i = 0;
    unsigned int sectorToRead;
    unsigned int sector = super_bloco.DataSectorStart + super_bloco.SectorsPerCluster*clusterNo;

    for(sectorToRead = sector; sectorToRead < (sector + super_bloco.SectorsPerCluster); sectorToRead++) {
        read_sector(sectorToRead,buffer + i);
        i += 256;
    }
    return 0;
}

struct t2fs_record* ler_cluster_pasta(int clusterNo) {
    int j;
    int folderSizeInBytes = sizeof(struct t2fs_record)*( (SECTOR_SIZE*super_bloco.SectorsPerCluster) / sizeof(struct t2fs_record) );
    unsigned int sector = super_bloco.DataSectorStart + super_bloco.SectorsPerCluster*clusterNo;
    unsigned char* buffer = malloc(sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster);
    struct t2fs_record* folderContent = malloc(folderSizeInBytes);

    if (sector >= super_bloco.DataSectorStart && sector < super_bloco.NofSectors) {
        ler_cluster(clusterNo, buffer);

        for(j = 0; j < folderSizeInBytes/sizeof(struct t2fs_record); j++) {
            folderContent[j].TypeVal = (BYTE) *(buffer + sizeof(struct t2fs_record)*j);
            memcpy(folderContent[j].name, buffer + 1 + sizeof(struct t2fs_record)*j, 51);
            folderContent[j].bytesFileSize = converter_para_DWORD(buffer + 52 + sizeof(struct t2fs_record)*j);
            folderContent[j].clustersFileSize = converter_para_DWORD(buffer + 56 + sizeof(struct t2fs_record)*j);
            folderContent[j].firstCluster = converter_para_DWORD(buffer + 60 + sizeof(struct t2fs_record)*j);
        }
        free(buffer);
        return folderContent;
    }
    free(buffer);
    return NULL;
}

unsigned char* ler_dado_cluster (int clusterNo){
    unsigned int sector = super_bloco.DataSectorStart + super_bloco.SectorsPerCluster*clusterNo;
    unsigned char* buffer = malloc(sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster); 
    if (sector >= super_bloco.DataSectorStart && sector < super_bloco.NofSectors) {
        ler_cluster(clusterNo, buffer);
        return buffer;
    }
    return NULL;
}

int escrever_cluster(int clusterNo, unsigned char* buffer, int position, int size) {
    int j;
    int k = 0;
    unsigned int sectorToWrite;
    unsigned int sector = super_bloco.DataSectorStart + super_bloco.SectorsPerCluster*clusterNo;
    unsigned char* newBuffer = malloc(sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster);

    if (size > SECTOR_SIZE*super_bloco.SectorsPerCluster || (position + size) > SECTOR_SIZE*super_bloco.SectorsPerCluster) {
        return -1;
    }

    ler_cluster(clusterNo, newBuffer);

    for(j = position; j < size + position; j++){
        newBuffer[j] = buffer[j - position];
    }

    for(sectorToWrite = sector; sectorToWrite < (sector + super_bloco.SectorsPerCluster); sectorToWrite++) {
        write_sector(sectorToWrite, newBuffer + k);
        k += 256;
    }
    free(newBuffer);
    return position + size;
}

int caminho_para_cluster(char* path) {
    int i;
    int found = 0;
    int pathsNo = 0;
    int folderInPath = 1;
    int pathComplete = 0;
    unsigned int currentCluster;
    char* pathTok;
    char* pathcpy = malloc(sizeof(char)*(strlen(path)+1));
    int folderSize = ( (SECTOR_SIZE*super_bloco.SectorsPerCluster) / sizeof(struct t2fs_record) );
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*super_bloco.SectorsPerCluster) / sizeof(struct t2fs_record) ));

    strcpy(pathcpy,path);

    if (pathcpy[0] == '/') {
        currentCluster = super_bloco.RootDirCluster;
    }else {
        currentCluster = caminho_atual.clusterNo;
    }

    if (strcmp(pathcpy,"/") == 0) {
        return super_bloco.RootDirCluster;
    }

    pathTok = strtok(pathcpy,"/");

    while(pathTok != NULL && pathsNo == found && folderInPath) {
        pathsNo += 1;
        folderContent = ler_cluster_pasta(currentCluster);
        for(i = 0; i < folderSize; i++) {
            if (strcmp(folderContent[i].name,pathTok) == 0) {
                currentCluster = folderContent[i].firstCluster;
                found += 1;
                if (folderContent[i].TypeVal != TYPEVAL_DIRETORIO) {
                    folderInPath = 0;
                }
            }
        }
        pathTok = strtok(NULL,"/");
        if (pathTok == NULL) {
            pathComplete = 1;
        }
    }

    if (pathsNo != found) {
        free(pathcpy);
        free(folderContent);
        return -1;
    }

    if (!pathComplete) {
        free(pathcpy);
        free(folderContent);
        return -1;
    }
    free(pathcpy);
    free(folderContent);
    return currentCluster;
}

int procurar_cluster(int* clusterReturn) { 
    int functionReturn = 0;
    int clusterNo = 1;
    DWORD value = BAD_SECTOR;
    while(functionReturn == 0 && value != 0) {
        clusterNo += 1;
        functionReturn = ler_FAT(clusterNo,&value);
    }
    if(functionReturn == -1) {
        *clusterReturn = -1;
    } else {
        *clusterReturn = clusterNo;
    }
    return functionReturn;
}

int tokenizePath(char* path, char*** tokenized) {
    int i;
    int countFolders = 1;
    char * pathcpy = malloc(sizeof(char)*(strlen(path)+1));
    char * pathTok;

    strcpy(pathcpy, path);

    pathTok = strtok(pathcpy,"/");

    while(pathTok != NULL) {
        pathTok = strtok(NULL,"/");
        if (pathTok != NULL) {
            countFolders += 1;
        }
    }

    *tokenized = malloc(sizeof(char*)*countFolders);

    strcpy(pathcpy, path);

    pathTok = strtok(pathcpy,"/");

    i = 0;
    while(pathTok != NULL) {
        (*tokenized)[i] = malloc(sizeof(char)*(strlen(pathTok)+1));
        strcpy((*tokenized)[i], pathTok);
        pathTok = strtok(NULL,"/");
        i += 1;
    }
    free(pathcpy);
    return countFolders;

}

int converter_caminho_absoluto(char * path, char * currPath, char ** output) {
    int i;
    int numTokens;
    char ** tokenizedPath;
    char * cutToken;
    int bufferSize = (strlen(path) + 1 + strlen(currPath) + 1);
    char * buffer = malloc(sizeof(char)*bufferSize);
    char * pathcpy = malloc(sizeof(char)*(strlen(path) + 1));

    strcpy(pathcpy,path);


    if(pathcpy[0] == '/'){
        buffer[0] = '\0';
    } else {
        strcpy(buffer,currPath);
    }

    numTokens = tokenizePath(pathcpy, &tokenizedPath);

    for(i = 0; i < numTokens; i++) {
        if (strcmp(tokenizedPath[i],"..") == 0) {
            if(strcmp(buffer,"/") != 0){
                cutToken = strrchr(buffer, '/');
                *cutToken = '\0';
            }
            if(strcmp(buffer,"") == 0) {
                strcpy(buffer,"/");
            }
        } else{ 
            if (strcmp(tokenizedPath[i],".") != 0) {
                if(strcmp(buffer,"/") != 0){
                    strcat(buffer,"/");
                }
                strcat(buffer,tokenizedPath[i]);
            }
        }
    }

    *output = malloc(sizeof(char)*(strlen(buffer)+ 1));

    strcpy(*output, buffer);

    free(buffer);
    free(pathcpy);

    return 0;

}
int separar_caminho(char * path, char ** FristStringOutput, char ** SecondStringOutput) {
    const char dir_div = '/';
    int lenghtAux;
    int lenghtPath = strlen(path);
    char *aux =  malloc(lenghtPath);    
    *SecondStringOutput = malloc(lenghtPath);
    memset(*SecondStringOutput,'\0',lenghtPath);
    *FristStringOutput = malloc(lenghtPath);
    memset(*FristStringOutput,'\0',lenghtPath);

    aux = strrchr(path, dir_div);
    lenghtAux = strlen(aux);
    memcpy(*SecondStringOutput,aux+1,lenghtAux);
    memcpy(*FristStringOutput, path, lenghtPath-lenghtAux);
    strcat(*FristStringOutput,"/");
    return 0;
}


int mkdir(char * path){
    char * absolute;
    char * firstOut;
    char * secondOut;
    int firstClusterFreeInFAT;
    int clusterDotDot;
    converter_caminho_absoluto(path, caminho_atual.absolute, &absolute);
    separar_caminho(absolute, &firstOut, &secondOut);

    if(procurar_cluster(&firstClusterFreeInFAT) == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
    if(strlen(secondOut) == 0){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
    if(!(nome_correto(secondOut))){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
    if(strlen(firstOut) == 1 && firstOut[0]== '/'){
        clusterDotDot = super_bloco.RootDirCluster;
    }
    else{
        clusterDotDot = caminho_para_cluster(firstOut);
            if(clusterDotDot == -1){
                free(absolute);
                free(firstOut);
                free(secondOut);
                return -1;
            }
        }

    if(esta_no_cluser(clusterDotDot, secondOut, TYPEVAL_DIRETORIO)){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    struct t2fs_record one_dot;
    struct t2fs_record two_dot;
    struct t2fs_record folder;


    one_dot.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(one_dot.name, ".");
    one_dot.bytesFileSize = SECTOR_SIZE*super_bloco.SectorsPerCluster;
    one_dot.clustersFileSize = 1;
    one_dot.firstCluster = firstClusterFreeInFAT;
    escrever_cluster_pasta(firstClusterFreeInFAT, one_dot);


    two_dot.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(two_dot.name, "..");
    two_dot.bytesFileSize = SECTOR_SIZE*super_bloco.SectorsPerCluster;
    two_dot.clustersFileSize = 1;
    two_dot.firstCluster = clusterDotDot;
    escrever_cluster_pasta(firstClusterFreeInFAT, two_dot);


    folder.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(folder.name, secondOut);
    folder.bytesFileSize = SECTOR_SIZE*super_bloco.SectorsPerCluster;;
    folder.clustersFileSize = 1;
    folder.firstCluster = firstClusterFreeInFAT;
    if(escrever_cluster_pasta(clusterDotDot, folder) == -1){
        return -1;
    }

    escrever_FAT(firstClusterFreeInFAT, END_OF_FILE);

    free(absolute);
    free(firstOut);
    free(secondOut);

    return 0;
}
int zerar(int clusterNo, struct t2fs_record folder, char * fileName, BYTE TypeValEntrada) {
    int i;
    int k = 0;
    int written = 0;
    unsigned int sectorToWrite;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    unsigned int sector = super_bloco.DataSectorStart + super_bloco.SectorsPerCluster*clusterNo;
    
    if (sector >= super_bloco.DataSectorStart && sector < super_bloco.NofSectors) {
        ler_cluster(clusterNo, buffer);
        for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
            if ( (strcmp((char *)buffer+i+1, fileName) == 0) && (((BYTE) buffer[i]) == TypeValEntrada) && !written ) {
                memcpy(buffer + i,&(folder.TypeVal),1);
                memcpy((buffer + i + 1),folder.name,51);
                memcpy((buffer + i + 52),dword_para_endereco(folder.bytesFileSize),4);
                memcpy((buffer + i + 56),dword_para_endereco(folder.clustersFileSize),4);
                memcpy((buffer + i + 60),dword_para_endereco(folder.firstCluster),4);
                written = 1;
            } 
        }

        for(sectorToWrite = sector; sectorToWrite < (sector + super_bloco.SectorsPerCluster); sectorToWrite++) {
            write_sector(sectorToWrite, buffer + k);
            k += 256;
        }
        free(buffer);
        if (written) {
            return 0;
        } else {
            return -1;
        }
    }
    free(buffer);
    return -1;
}

int esta_no_cluser(int clusterNo, char * fileName, BYTE TypeValEntrada) {
    int i;
    int wasFound = 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    unsigned int sector = super_bloco.DataSectorStart + super_bloco.SectorsPerCluster*clusterNo;
    
    if (sector >= super_bloco.DataSectorStart && sector < super_bloco.NofSectors) {
        ler_cluster(clusterNo, buffer);

        for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
            if ( (strcmp((char *)buffer+i+1, fileName) == 0) && (((BYTE) buffer[i]) == TypeValEntrada) && !wasFound ) {
                wasFound = 1;
            } 
        }
        free(buffer);
        if (wasFound) {
            return 1;
        } else {
            return 0;
        }
    }
    free(buffer);
    return 0;
}

int nome_correto(char * name){
    if(strcmp(name, ".") == 0){
        return 0;
    }
    if(strcmp(name, "..") == 0){
        return 0;
    }
    if(name[0] == '/'){
        return 0;
    }

    return 1;
}

DIRENT2 setNullDirent(){
    DIRENT2 dir;
    strcpy(dir.name,"");
    dir.fileType=(DWORD)6;
    dir.fileSize=(DWORD)0;

    return dir;
}

DWORD get_tipo(char *absolute){

    char *firstOut;
    char *secondOut;
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*super_bloco.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    int folderSize = ( (SECTOR_SIZE*super_bloco.SectorsPerCluster) / sizeof(struct t2fs_record) );
    int i;
    int clusterDir;

    separar_caminho(absolute, &firstOut, &secondOut);

    clusterDir=caminho_para_cluster(firstOut);
    folderContent=ler_cluster_pasta(clusterDir);
    for(i=0;i<folderSize;i++){
        if(strcmp(folderContent[i].name,secondOut) ==0 ){
            return folderContent[i].TypeVal;
        }
    }
    return TYPEVAL_INVALIDO;
}

DIR2 openDir(char *path){
    int i;
    char *absolute=malloc(sizeof(char)*2);
    int dirCluster;
    char *linkOutput;
    int retornoLink;

    if(strcmp(path,"/") == 0){
        strcpy(absolute,"/");
    }else{
        retornoLink=link(path, &linkOutput);
        if(retornoLink == -1)
            return -1; 
        if(retornoLink < 0){
            return -4;
        }else if(retornoLink ==1){
            if(converter_caminho_absoluto(linkOutput, caminho_atual.absolute, &absolute)){
            return -1;
            } 
        }else{
            if(converter_caminho_absoluto(path, caminho_atual.absolute, &absolute) == -1)
            return -2;
        }
        if(get_tipo(absolute) != TYPEVAL_DIRETORIO){
            return -5;
        }
    }
    
     dirCluster=caminho_para_cluster(absolute);
    for(i=0; i<LIMITES_ABERTOS ;i++){
        if(diretorios_abertos[i].handle == -1){
            diretorios_abertos[i].handle = i;
            diretorios_abertos[i].noReads=0;
            if(strcmp(absolute,"/") == 0 || strcmp(path,"/") ==0 ){
            diretorios_abertos[i].clusterDir=super_bloco.RootDirCluster;
            }
            else{
                diretorios_abertos[i].clusterDir=dirCluster;
            }
            return diretorios_abertos[i].handle;
        }
    }
    return -1;
}

void liberar_diretorio(DISK_DIR *opendirectory){
    opendirectory->handle=-1;
    opendirectory->noReads=-1;
    opendirectory->clusterDir=-1;
    opendirectory->directory=setNullDirent();
}

int closeDir(DIR2 handle){
    int i;

    for(i=0;i< LIMITES_ABERTOS ;i++){
        if(diretorios_abertos[i].handle==handle){
            liberar_diretorio(&diretorios_abertos[diretorios_abertos[i].handle]);
            return 0;
        }
    }
    return -1;
}

FILE2 createFile(char * filename){
    char * absolute;
    char * firstOut;
    char * secondOut;
    int firstClusterFreeInFAT;
    int handle;
    handle = novo_handle();
    int clusterToRecordFile;
    char *linkOutput;
    

    if(link(filename, &linkOutput)== -1)
        return -1; 

    if(converter_caminho_absoluto(linkOutput, caminho_atual.absolute, &absolute)){       
        return -1;
    }

    if(separar_caminho(absolute, &firstOut, &secondOut)){        
        return -1;
    }

    if(!nome_correto(secondOut)){
        return -1;
    }  

    clusterToRecordFile = caminho_para_cluster(firstOut);    
    if(clusterToRecordFile == -1){
        return -1;
    }


    if(strlen(secondOut) == 0){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
    
    if(!(nome_correto(secondOut))){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
   
    if(handle == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1; 
    }

    if(esta_no_cluser(clusterToRecordFile, secondOut, TYPEVAL_REGULAR)){
        deletar_arquivo(filename);
    }
   
    if(procurar_cluster(&firstClusterFreeInFAT) == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    struct t2fs_record toRecord;

    
    toRecord.TypeVal = TYPEVAL_REGULAR;
    strcpy(toRecord.name, secondOut);
    toRecord.bytesFileSize = 0;
    toRecord.clustersFileSize = 1;
    toRecord.firstCluster = firstClusterFreeInFAT;

   
    if(escrever_cluster_pasta(clusterToRecordFile, toRecord) == - 1){
        return -1;
    }    
    escrever_FAT(firstClusterFreeInFAT, END_OF_FILE);
    
    return (openFile (filename));
}

int novo_handle(){
    int i;
    
    for(i = 0; i < MAX_NUM_FILES; i++){
        if(arquivos_abertos[i].file == -1){
            return (i+1);
        }
    }    
    return -1;
}

FILE2 openFile (char * filename){
    char * absolute;
    char * firstOut;
    char * secondOut;    
    int handle;
    handle = novo_handle();
    int firstClusterOfFile;
    char *linkOutput;

    int i;
    int isFile= 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    int clusterOfDir;

    if(link(filename, &linkOutput)== -1)
            return -1; 

    if(converter_caminho_absoluto(linkOutput, caminho_atual.absolute, &absolute)){        
        return -2;
    }

    if(separar_caminho(absolute, &firstOut, &secondOut)){        
        return -2;
    }

    if(!nome_correto(secondOut)){ 
        return -1;
    }    
    clusterOfDir = caminho_para_cluster(firstOut);

    ler_cluster(clusterOfDir, buffer);
    if(strlen(secondOut) > 0){
        for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
            if ( (strcmp((char *)buffer+i+1, secondOut) == 0) && (((BYTE) buffer[i]) == TYPEVAL_REGULAR) && !isFile ) {
                isFile = 1;
            } 
        }
        if(isFile == 0){
            return -3;
        }
    }
    firstClusterOfFile = caminho_para_cluster(absolute);    
    if(firstClusterOfFile == -1){
        return -4;
    }
    if(handle == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -5; 
    }
    struct diskf newFileToRecord;
   
    newFileToRecord.clusterNo = firstClusterOfFile;
    newFileToRecord.currPointer = 0;
    newFileToRecord.file = handle;    
    newFileToRecord.clusterDir=caminho_para_cluster(firstOut);
    memcpy(&arquivos_abertos[handle-1], &newFileToRecord, sizeof(struct diskf));
    
    return newFileToRecord.file;
}

int fechar_arquivo(FILE2 handle){
    int i;
    for(i = 0; i < MAX_NUM_FILES; i++){
        if(arquivos_abertos[i].file == handle){
            arquivos_abertos[i].file = -1;
            arquivos_abertos[i].clusterNo = -1;
            arquivos_abertos[i].currPointer = -1;
            return 0;
        }
    }
    return -1;
}

int deletar_arquivo(char * filename){
    char * absolute;
    char * firstOut;
    char * secondOut;
    int clusterOfDir;
    int clusterToDelete;
    unsigned char* bufferWithNulls = malloc(sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster);
    DWORD FATrepresentation = 0;
    BYTE typeToDelete = TYPEVAL_REGULAR;
    char *linkOutput;  
    int i;
    int isFile = 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    
    if(link(filename, &linkOutput)== -1)
            return -1; 

    if(link(filename, &linkOutput)== 1){
        typeToDelete = TYPEVAL_LINK;
    }

    memset(bufferWithNulls,'\0',SECTOR_SIZE*super_bloco.SectorsPerCluster);

    if(converter_caminho_absoluto(filename, caminho_atual.absolute, &absolute)){        
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    if(separar_caminho(absolute, &firstOut, &secondOut)){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    if(!nome_correto(secondOut)){
        return -1;
    }  

    if((clusterOfDir = caminho_para_cluster(firstOut)) == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
         return -1;
    }

    if((clusterToDelete = caminho_para_cluster(absolute))== -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
    ler_cluster(clusterOfDir, buffer);
    for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
        if ( (strcmp((char *)buffer+i+1, secondOut) == 0) && (((BYTE) buffer[i]) == typeToDelete) && !isFile ) {
            isFile = 1;
        } 
    }
    if(isFile == 0){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }    
    struct t2fs_record folderContent;

    folderContent.TypeVal = TYPEVAL_INVALIDO;
    strcpy(folderContent.name, "\0");
    folderContent.bytesFileSize = 0;
    folderContent.clustersFileSize = 0;
    folderContent.firstCluster = 0;

    if(zerar(clusterOfDir, folderContent, secondOut, typeToDelete) == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    fechar_arquivo_cluster(clusterToDelete);

    while( FATrepresentation != END_OF_FILE && FATrepresentation != BAD_SECTOR){

        ler_FAT(clusterToDelete,&FATrepresentation);
        escrever_FAT(clusterToDelete, 0);
        
        escrever_cluster(clusterToDelete,bufferWithNulls,0,SECTOR_SIZE*super_bloco.SectorsPerCluster);
        
        if(FATrepresentation != END_OF_FILE && FATrepresentation != BAD_SECTOR){
            clusterToDelete = (int) FATrepresentation;
        }

    }
    free(absolute);
    free(firstOut);
    free(secondOut);
    return 0;
}

int fechar_arquivo_cluster(int clusterToClose){
    int i;
    for(i = 0; i < MAX_NUM_FILES; i++){
        if(arquivos_abertos[i].clusterNo == clusterToClose){
            arquivos_abertos[i].file = -1;
            arquivos_abertos[i].clusterNo = -1;
            arquivos_abertos[i].currPointer = -1;
            return 0;
        }
    }
    return -1;
}

int link(char * path, char ** output) {
    int i;
    int isLink = 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    char * absolute;
    char * pathToFile;
    char * fileName;
    int pathClusterNo;
    int linkClusterNo;


    converter_caminho_absoluto(path, caminho_atual.absolute, &absolute);    
    separar_caminho(absolute, &pathToFile, &fileName);

    pathClusterNo = caminho_para_cluster(pathToFile);

    if(pathClusterNo == -1) {
        free(buffer);
        free(absolute);
        free(fileName);
        free(pathToFile);
        return -1;
    }

    ler_cluster(pathClusterNo, buffer);

   
    for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {        
        if ( (strcmp((char *)buffer+i+1, fileName) == 0) && (((BYTE) buffer[i]) == TYPEVAL_LINK) && !isLink ) {
            isLink = 1;
        } 
    }

    if(!isLink) {
        free(buffer);
        free(absolute);
        free(fileName);
        free(pathToFile);
        *output = malloc(sizeof(char)*(strlen(path)+1));
        strcpy(*output,path);
        return 0;
    }
    
    linkClusterNo = caminho_para_cluster(path);

    memset(buffer,0,clusterByteSize);

    ler_cluster(linkClusterNo,buffer);

    *output = malloc(sizeof(char)*(strlen((char*)buffer)+1));
    strcpy(*output,(char*)buffer);

    free(buffer);
    free(absolute);
    free(fileName);
    free(pathToFile);    
    return 1;
}

int escreve_arquivo(FILE2 handle, char * buffer, int size) {
    int i = 0;
    int fileNo;
    int found = 0;
    int remainingSize = size;
    int bytesWritten = 0;
    int currentPointerInCluster;
    int currentCluster;
    int nextCluster;    
    int clusterSize = super_bloco.SectorsPerCluster*SECTOR_SIZE;
     

    DWORD value;

    while(i < MAX_NUM_FILES && !found){
        if (handle == arquivos_abertos[i].file) {
            fileNo = i;
            found = 1;
        }
        i += 1;
    }

    if(!found) {
        return -1;
    }

    currentPointerInCluster = arquivos_abertos[fileNo].currPointer;
    nextCluster = arquivos_abertos[fileNo].clusterNo;
    currentCluster = nextCluster;
    
    while(currentPointerInCluster >= clusterSize) {
        if(ler_FAT(nextCluster,&value) != 0) {
            return -1;
        }
        nextCluster = (int)value;
        if((DWORD)nextCluster != END_OF_FILE) {
            currentCluster = nextCluster;
        }
        currentPointerInCluster -= (clusterSize);
    }

    if((remainingSize + currentPointerInCluster) <= (clusterSize)){
        escrever_cluster(currentCluster,(unsigned char*)(buffer),currentPointerInCluster,size);
        bytesWritten += size;
        remainingSize -= size;
    } else {
        escrever_cluster(currentCluster,(unsigned char*)(buffer),currentPointerInCluster,(clusterSize - currentPointerInCluster));
        remainingSize -= (clusterSize - currentPointerInCluster);
        bytesWritten += (clusterSize - currentPointerInCluster);
    }

    while((DWORD)nextCluster != END_OF_FILE && remainingSize > 0) {
        if(ler_FAT(nextCluster,&value) != 0) {
            return -1;
        }
        nextCluster = (int)value;
        if((DWORD)nextCluster != END_OF_FILE) {
            currentCluster = nextCluster;

            if(remainingSize <= (clusterSize)){
                escrever_cluster(currentCluster,(unsigned char*)(buffer + bytesWritten),0,remainingSize);
                bytesWritten += remainingSize;
                remainingSize -= remainingSize;
            } else {
                escrever_cluster(currentCluster,(unsigned char*)(buffer + bytesWritten),0,(clusterSize));
                remainingSize -= (clusterSize);
                bytesWritten += (clusterSize);
            }
        }
    }
    while(remainingSize > 0) {
        if(remainingSize <= (clusterSize)){
            if(procurar_cluster(&nextCluster) != 0) {
                return -1;
            }
            escrever_FAT(currentCluster,nextCluster);
            escrever_FAT(nextCluster,END_OF_FILE);
            escrever_cluster(nextCluster,(unsigned char*)(buffer + bytesWritten),0,remainingSize);
            bytesWritten += remainingSize;
            remainingSize -= remainingSize;
        } else {
            if(procurar_cluster(&nextCluster) != 0) {
                return -1;
            }
            escrever_FAT(currentCluster,nextCluster);
            escrever_FAT(nextCluster,END_OF_FILE);
            escrever_cluster(nextCluster,(unsigned char*)(buffer + bytesWritten),0,(clusterSize));
            currentCluster = nextCluster;
            remainingSize -= (clusterSize);
            bytesWritten += (clusterSize);
        }
    }

    arquivos_abertos[fileNo].currPointer += bytesWritten; 

    if(setar_tamanho(handle) != 0)
        return -2;

    
    return bytesWritten;
}

int ler_arquivo (FILE2 handle, char *buffer, int size){

    int found=0;
    int currentPointerInCluster;
    int currentCluster;
    int nextCluster;
    int fileNo;
    DWORD value;
    int j;
    int i=0;
    int clusterCount=0;
    unsigned char *prebuffer=malloc(sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster+1);

    for(j=0;j<MAX_NUM_FILES && found==0;j++){
        if(arquivos_abertos[j].file == handle){
            found=1;
            fileNo=j;
        }

    }
    if(found==0){        
        return -1;
    }

    currentPointerInCluster = arquivos_abertos[fileNo].currPointer;
    currentCluster = arquivos_abertos[fileNo].clusterNo;
    prebuffer=ler_dado_cluster(currentCluster);

    while((DWORD)currentCluster != END_OF_FILE && i<size && (DWORD)currentCluster != BAD_SECTOR){
        
        while(currentPointerInCluster < SECTOR_SIZE*super_bloco.SectorsPerCluster  && prebuffer[currentPointerInCluster] != '\0' && i<size){
            buffer[i]=(unsigned char)prebuffer[currentPointerInCluster];            
            currentPointerInCluster++;
            i++;
        }
        if(i>=size){            
            return -1;
        }
         
        if(i<size || i>=clusterCount*SECTOR_SIZE*super_bloco.SectorsPerCluster){
            if(ler_FAT(currentCluster,&value) != 0) {
                return -2;
            }else{
                    nextCluster = (int)value;
                    free(prebuffer);
                    prebuffer=malloc(sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster+1);
                    prebuffer=ler_dado_cluster(nextCluster);
                    currentPointerInCluster=0;
                    currentCluster = nextCluster;
            }
            }
                clusterCount++;
        }
    free(prebuffer);
    if(i == 0)
    return -3;
    arquivos_abertos[fileNo].currPointer +=i;
    return i;
}

int tamanho_real (FILE2 handle){ 

    int found=0;
    int currentPointerInCluster;
    int currentCluster;
    int nextCluster;
    int fileNo;
    DWORD value;
    int j;
    int i=0;
    unsigned char *prebuffer=malloc(sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster+1);

    for(j=0;j<MAX_NUM_FILES && found==0;j++){
        if(arquivos_abertos[j].file == handle){
            found=1;
            fileNo=j;
        }

    }
    if(found==0){
        return -1;
    }
    currentPointerInCluster = 0;
    currentCluster = arquivos_abertos[fileNo].clusterNo;
    prebuffer=ler_dado_cluster(currentCluster);

    while((DWORD)currentCluster != END_OF_FILE && (DWORD)currentCluster != BAD_SECTOR){

        while(currentPointerInCluster <  SECTOR_SIZE*super_bloco.SectorsPerCluster && prebuffer[currentPointerInCluster] != '\0') {
            currentPointerInCluster++;
            i++;
        }
        if(ler_FAT(currentCluster,&value) != 0) {
            return -2;
        }
        nextCluster = (int)value;
        free(prebuffer);
        prebuffer=malloc(sizeof(unsigned char)*SECTOR_SIZE*super_bloco.SectorsPerCluster);
        if((DWORD)nextCluster != END_OF_FILE){
            prebuffer=ler_dado_cluster(nextCluster);
        }
        currentPointerInCluster=0;
        currentCluster = nextCluster;
        }
    
    free(prebuffer);
    if(i == 0)
    return -3;
    return i;
}

int setar_tamanho(FILE2 handle){

    int filesize;
    filesize=tamanho_real(handle);
    if(filesize <0)
    {
        return -2;
    }
    if(update_tamanho(handle,(DWORD)filesize) != 0){
        return -3;
    }

    return 0;
}

int update_tamanho(FILE2 handle,DWORD newFileSize){
    int found=0;
    int fileNo;
    int j;
    struct t2fs_record newStruct;
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*super_bloco.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    int folderSize = ( (SECTOR_SIZE*super_bloco.SectorsPerCluster) / sizeof(struct t2fs_record) );
    int i;
    int foundinfolder =0;
    int count;
    unsigned char * buffer = malloc(sizeof(struct t2fs_record));
    for(j=0;j<MAX_NUM_FILES && found==0;j++){
        if(arquivos_abertos[j].file == handle){
            found=1;
            fileNo=j;
        }

    }
    if(found==0){
        return -1;
    }
    folderContent=ler_cluster_pasta(arquivos_abertos[fileNo].clusterDir);
    for(i=0;i<folderSize && foundinfolder ==0;i++){

        if(folderContent[i].firstCluster == arquivos_abertos[fileNo].clusterNo){
            newStruct.bytesFileSize=newFileSize;
            newStruct.clustersFileSize= (int)(newFileSize % (super_bloco.SectorsPerCluster*SECTOR_SIZE)) == 0 ? (int)newFileSize/(super_bloco.SectorsPerCluster*SECTOR_SIZE) : (int)newFileSize/(super_bloco.SectorsPerCluster*SECTOR_SIZE) + 1;
            newStruct.firstCluster=arquivos_abertos[fileNo].clusterNo;
            strcpy(newStruct.name,folderContent[i].name);
            newStruct.TypeVal=folderContent[i].TypeVal;
            foundinfolder=1;
            count=(i*sizeof(struct t2fs_record));
        }
    }

    if (!foundinfolder)
        return -1;

    memcpy(buffer,&(newStruct.TypeVal),1);
    memcpy((buffer + 1),newStruct.name,51);
    memcpy((buffer + 52),dword_para_endereco(newStruct.bytesFileSize),4);
    memcpy((buffer + 56),dword_para_endereco(newStruct.clustersFileSize),4);
    memcpy((buffer + 60),dword_para_endereco(newStruct.firstCluster),4);
    escrever_cluster(arquivos_abertos[fileNo].clusterDir,buffer,count,sizeof(struct t2fs_record));

return 0;      
}
