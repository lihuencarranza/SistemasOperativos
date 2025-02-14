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
#define MAX_INODES 10

#define OCCUPIED 1
#define FREE 0

#define NO_PARENT ""

#define MODE_WRITE "w"

#define ROOT "/"

#define ERROR -1
#define BAD_INDEX -1

// Esta es la estructura principal para guardar un archivo en memoria.
// Agregaremos lo necesario para que el filesystem funcione como esperamos.
//
typedef struct inode {
	size_t file_size;  // Tamaño del archivo
	int uid;           // User ID
	int gid;           // Group ID
	int type;          // Tipo de archivo (IS_FILE o IS_DIR)
	int nlink;         // Cantidad de links
	mode_t mode;       // Modo

	time_t atime;  // Último acceso
	time_t mtime;  // Última
	time_t ctime;  // Última modificación

	char file_name[MAX_PATH];  // Nombre del archivo/directorio. Si es root, es "/".
	char file_parent[MAX_PATH];  // Path del directorio que lo contiene o path
	                             // del directorio padre,  o si es root está vacío.
	char file_path[MAX_PATH];  // Path completo del archivo/directorio.
	char file_content[MAX_CONTENT];  // Contenido del archivo

} inode_t;

// Esta estructura guarda los inodos de los archivos que tenemos en memoria.
typedef struct super_block {
	struct inode inodes[MAX_INODES];  // Inodos
	int bitmap_inodos[MAX_INODES];
	size_t max;  // todo set with MAX_INODES
} super_block_t;
