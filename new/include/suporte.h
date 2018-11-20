#ifndef __DISK___
#define __DISK___
#include "t2fs.h"

#define END_OF_FILE 0xFFFFFFFF

#define BAD_SECTOR 0xFFFFFFFE

#define MAX_NUM_FILES 10

#define LIMITES_ABERTOS 10 


struct t2fs_superbloco super_bloco;

DWORD converter_para_DWORD(unsigned char* buffer);

WORD converter_para_WORD(unsigned char* buffer);

unsigned char* word_para_endereco(WORD entry);

unsigned char* dword_para_endereco(DWORD entry);

int iniciar_disco();

int escrever_FAT(int clusterNo, DWORD value);

int ler_FAT(int clusterNo, DWORD* value);

struct t2fs_record* ler_cluster_pasta(int clusterNo);

int escrever_cluster_pasta(int clusterNo,struct t2fs_record folder);

int ler_cluster(int clusterNo, unsigned char* buffer);

unsigned char* ler_dado_cluster (int clusterNo);

int escrever_cluster(int clusterNo, unsigned char* buffer, int position, int size);

int caminho_para_cluster(char* path);

int procurar_cluster(int* clusterReturn);

/* TODO */
int tokenizePath(char* path, char*** tokenized);


int converter_caminho_absoluto(char * path, char * currPath, char ** output);

int separar_caminho(char * path, char ** FristStringOutput, char ** SecondStringOutput) ;

int mudar_diretorio(char * path);

int mkdir(char * path);

int esta_no_cluser(int clusterNo, char * fileName, BYTE TypeValEntrada);

int nome_correto(char * name);

int zerar(int clusterNo, struct t2fs_record folder, char * fileName, BYTE TypeValEntrada);

DIRENT2 searchDirByHandle(DIR2 handle);

DIR2 openDir(char *path);

DIRENT2 setNullDirent();

int closeDir(DIR2 handle);

int link(char * path, char ** output);

FILE2 createFile(char * filename);

int deletar_arquivo(char * filename);

int novo_handle();

FILE2 openFile (char * filename);


int ler_arquivo (FILE2 handle, char *buffer, int size);

int escreve_arquivo(FILE2 handle, char * buffer, int size);

int fechar_arquivo(FILE2 handle);

int fechar_arquivo_cluster(int clusterToClose);

int update_tamanho(FILE2 handle,DWORD newFileSize);

int setar_tamanho(FILE2 handle);

int tamanho_real (FILE2 handle);

DWORD get_tipo(char *absolute);

typedef struct diskf {
    FILE2 file;
    int currPointer;
    int clusterNo;
    int clusterDir;
} DISK_FILE;

typedef struct currp {
    char* absolute;
    int clusterNo;
} CURRENT_PATH;

CURRENT_PATH caminho_atual;

typedef struct diskd {
    DIR2 handle;
    int noReads;
    int clusterDir;
    DIRENT2 directory;
} DISK_DIR;

DISK_FILE arquivos_abertos[LIMITES_ABERTOS];

void liberar_diretorio(DISK_DIR *opendirectory);

#endif