#define FUSE_USE_VERSION 30

#include <fuse.h>
#include "fisopfs.h"

char fs_fisopfs[MAX_PATH] = "fs.fisopfs";

struct super_block superb = {.next_free_index = 0};

static int
create_inode_from_path(const char *path, mode_t mode, int type)
{
	printf("[debug] create_inode_from_path PRE CREACION- path: %s - mode: %d - type: %d\n",
	       path,
	       mode,
	       type);

	static struct inode i;
	i.file_size = 0;                           // SETEAR A 0
	i.uid = getuid();
	i.gid = getgid();
	i.type = type;
	i.nlink =
	        (type == IS_FILE)
	                ? 1
	                : 2;  // Un archivo comienza con un link y un directorio con 2.
	i.mode = mode;

	strcpy(i.file_name, path);  // SACARLE LA / AL NAME
	strcpy(i.file_path, path);  // SACARLE LA / AL PATH

	superb.inodes[superb.next_free_index] = i;
	superb.next_free_index++;

	printf("[debug] create_inode_from_path POST CREACION - path: %s - mode: %d - type: %d - "
	       "new_index: %d\n",
	       superb.inodes[superb.next_free_index-1].file_name,
	       superb.inodes[superb.next_free_index-1].mode,
	       superb.inodes[superb.next_free_index-1].type,
	       superb.next_free_index);
	return 0;
}

char* obtenerUltimoElemento(char *path) {
    char *ultimoElemento = strrchr(path, '/');
    if (ultimoElemento != NULL) {
        return ultimoElemento + 1; // Se suma 1 para no incluir el '/'
    }
    return path; // Si no hay '/', se devuelve la ruta completa
}

static int
get_inode_index_from_path(const char *path)
{
	if(strcmp(path, "/") == 0)
		return 0;
	
	printf("[debug] get_inode_index_from_path SPLIT DEL PATH: %s\n", path);
	char *buff = obtenerUltimoElemento(path);
	printf("[debug] get_inode_index_from_path RESULTADO DE SPLIT: %s\n", buff);
	
/*
	char* primer_dir = strtok(path, "/");
	char* resto = strtok(NULL, "/");
	printf("[debug] get_inode_index_from_path SPLIT DEL PATH: %s\n", path);
	printf("primer_dir: %s\n", primer_dir);
	printf("resto: %s\n", resto);*/

	printf("[debug] get_inode_index_from_path - buscando el path \"%s\" en el super bloque\n", buff);
	int len_inodes = sizeof(superb.inodes)/sizeof(superb.inodes[0]);

	for (int i = 0; i < superb.next_free_index; i++) {
		if (strcmp(superb.inodes[i].file_name, path) == 0) {
			printf("[debug] get_inode_index_from_path - matcheo el path \"%s\" con el inodo [%i] \n", buff, i);
			return i;
		}
		
		printf("[debug] get_inode_index_from_path(for[%i]) - No matcheó [\"%s\" != \"%s\"]\n", i, buff, superb.inodes[i].file_name);
	}
	return -1;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	memset(st, 0, sizeof(struct stat));

	int index = get_inode_index_from_path(path);
	if (index == -1) {
		printf("[debug] fisopfs_getattr - path: \"%s\" FALLÓ POR NO ENCONTRAR INDICE \n", path);
		return -ENOENT;
	}

	printf("[debug] fisopfs_getattr - encontró el path : %s en el indice: [%i]\n", path, index);

	struct inode i = superb.inodes[index];
	
	st->st_uid = i.uid;
	st->st_gid = i.gid;
	st->st_mode = i.mode;
	st->st_nlink = i.nlink;
	st->st_size = i.file_size;
	st->st_atime = i.atime;
	st->st_mtime = i.mtime;


	return 0;
}

static int
fisopfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	printf("\n\n\n\n[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	int index = get_inode_index_from_path(path);
	if (index == -1) { 
		printf("[debug] fisopfs_READDIR - path: \"%s\" Indice no encontrado \n", path); 
		return -ENOENT;
 	}

	struct inode i = superb.inodes[index];
	printf("[debug] fisopfs_readdir - a fillear: %s\n\n\n\n\n", path);
	//filler(buffer, superb.inodes[1].file_path, NULL, 0);
	
	for (int j = 0; j < superb.next_free_index; j++) {
		if (strcmp(superb.inodes[j].file_path, path) == 0) {
			printf("[debug] fisopfs_readdir - filleo match: %s == %s \n\n\n\n",superb.inodes[j].file_path, path);
			filler(buffer, superb.inodes[j].file_path, NULL, 0);
		}
	}


	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	// if (strcmp(path, "/") == 0) {
	// 	filler(buffer, "fisop", NULL, 0);
	// 	return 0;
	// }

	return 0;
}

#define MAX_CONTENIDO 100
static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

static int
fisopfs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("\n\n\n\n[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n", path, offset, size);

	int index = get_inode_index_from_path(path);
	if (index == -1) {
		printf("[debug] fisopfs_read - path: \"%s\" FALLÓ POR NO ENCONTRAR INDICE \n", path);
		return -ENOENT;
	}

	struct inode i = superb.inodes[index];
	if (offset >= i.file_size) {
		return 0;
	}

	size_t len = i.file_size - offset;
	if (len > size) {
		len = size;
	}

	memcpy(buffer, i.file_content + offset, len);

	i.atime = time(NULL);


	return size;
}


static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("\n\n\n\n[debug] fisopfs_mkdir - path: %s - mode: %d \n", path, mode);
	return create_inode_from_path(path, mode, IS_DIR);
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);
	return 0;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);
	return 0;
}

// struct fuse_file_info se descarta ya que nos aporta info extra para la creación.
static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *file_info)
{
	printf("[debug] fisopfs_create - path: %s - mode: %d \n", path, mode);
	return create_inode_from_path(path, mode, IS_FILE);
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s\n", path);
	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);
	return 0;
}


static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy - destroying root\n");

	FILE *fs = fopen(fs_fisopfs, "w");
	if (!fs) {
		printf("[debug] fisopfs_destroy - error destroying root\n");
		exit(1);
	}

	if (fwrite(&superb, sizeof(superb), 1, fs) != 1) {
		printf("[debug] fisopfs_destroy - error writing superb\n");
		exit(1);
	}

	fflush(fs);
	fclose(fs);
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush - path: %s\n", path);

	fisopfs_destroy(NULL);

	return 0;
}

static void
create_root()
{
	printf("[debug] create_root - creating root\n");
	FILE *fs = fopen(fs_fisopfs, "w");
	if (!fs) {
		printf("[debug] create_root - error creating root\n");
		exit(1);
	}
	create_inode_from_path("/", __S_IFDIR | 0755, IS_DIR);
	fwrite(&superb, sizeof(superb), 1, fs);
	fclose(fs);
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisop_init - Starting init\n");

	FILE *fs = fopen(fs_fisopfs, "r");
	if (!fs) {
		// Si no existe el archivo, lo creamos
		create_root();
	} else {
		fread(&superb, sizeof(superb), 1, fs);
		fclose(fs);
	}

	return conn;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,

	// new implementations
	.create = fisopfs_create,
	.mkdir = fisopfs_mkdir,
	.write = fisopfs_write,
	.utimens = fisopfs_utimens,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.flush = fisopfs_flush,
	.destroy = fisopfs_destroy,
	.init = fisopfs_init,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
