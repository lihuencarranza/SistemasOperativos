#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define IS_FILE 1
#define IS_DIR 2

#define MAX_PATH 100
#define MAX_CONTENT 1024
#define MAX_INODES 100

// Esta es la estructura principal para guardar un archivo en memoria.
// Agregaremos lo necesario para que el filesystem funcione como esperamos.
//
typedef struct inode {
	size_t file_size;  // Tamaño del archivo
	int uid;           // User ID
	int type;          // Tipo de archivo (IS_FILE o IS_DIR)
	int nlink;         // Cantidad de links
	mode_t mode;       // Modo

	char file_name[MAX_PATH];  // Nombre del archivo/directorio. Si es root, es "/".
	char file_parent[MAX_PATH];  // Path del directorio que lo contiene o path
	                             // del directorio padre,  o si es root está vacío.
	char file_path[MAX_PATH];  // Path completo del archivo/directorio.
	char file_content[MAX_CONTENT];  // Contenido del archivo
	
	struct inode *children[MAX_INODES]; // Hijos de un directorio

} inode_t;

// Esta estructura guarda los inodos de los archivos que tenemos en memoria.
typedef struct super_block {
	struct inode inodes[MAX_INODES];  // Inodos
} super_block_t;
