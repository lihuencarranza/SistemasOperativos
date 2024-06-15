#define FUSE_USE_VERSION 30
#define ROOT "/"
#include <fuse.h>
#include "fisopfs.h"

char fs_fisopfs[MAX_PATH] = "fs.fisopfs";

struct super_block superb = {};

const char* obtenerUltimoElemento(const char *path) {
    char *ultimoElemento = strrchr(path, '/');
    if (ultimoElemento != NULL) {
        return ultimoElemento + 1; // Se suma 1 para no incluir el '/'
    }
    return path; // Si no hay '/', se devuelve la ruta completa
}

int fetch_free_index(struct super_block *superb){
	if(!superb)
		return -1;
	int i = 0;
	while (i<(MAX_INODES) && superb->bitmap_inodos[i] == 1)
	{
		i++;
	}
	return i;
}

int set_inode_in_superblock(struct inode *i){
	int free_idx = fetch_free_index(&superb);
	if(!free_idx)
		return -1;
	superb.inodes[free_idx] = *i;
	superb.bitmap_inodos[free_idx] = 1;
	return free_idx;
}

static int
create_inode_from_path(const char *path, mode_t mode, int type)
{
	printf("[debug] create_inode_from_path PRE CREACION- path: %s - mode: %d - type: %d\n",
	       path,
	       mode,
	       type);

	static struct inode i;
	i.file_size = 16;                           // SETEAR A 0
	i.uid = getuid();
	i.gid = getgid();
	i.type = type;
	i.mode = (type == IS_FILE)?  __S_IFREG | 0644 : __S_IFDIR | 0755;  // Un archivo comienza con un link y un directorio con 2.
	i.atime = time(NULL);
	i.mtime = time(NULL);
	i.ctime = time(NULL);
	i.nlink = (type == IS_FILE)? 1: 2;  // Un archivo comienza con un link y un directorio con 2.
	(type == IS_FILE)? memcpy(i.file_content, "CONTENIDO DEFAULT",16):NULL;
	strcpy(i.file_name, obtenerUltimoElemento(path));  // SACARLE LA / AL NAME
	strcpy(i.file_path, path);  // SACARLE LA / AL PATH
	strcpy(i.file_parent, "/"); // SACARLE LA / AL PARENT

	int free_idx = set_inode_in_superblock(&i);
	if (free_idx < 0)
		return free_idx;
	memset(i.file_content, 0, sizeof(i.file_content)); // contenido vacío.

	printf("[debug] create_inode_from_path POST CREACION - path: %s - mode: %d - type: %d  \n",
	       superb.inodes[free_idx].file_name,
	       superb.inodes[free_idx].mode,
	       superb.inodes[free_idx].type);
	return 0;
}

static int
get_inode_index_from_path(const char *path)
{
	if(strcmp(path, "/") == 0)
		return 0;
	
	printf("[debug] get_inode_index_from_path SPLIT DEL PATH: %s\n", path);
	const char *buff = obtenerUltimoElemento(path);
	printf("[debug] get_inode_index_from_path RESULTADO DE SPLIT: %s\n", buff);
	

	printf("[debug] get_inode_index_from_path - buscando el path \"%s\" en el super bloque\n", buff);

	for (int i = 0; i < MAX_INODES; i++) {
		if (superb.bitmap_inodos[i] == 1 && strcmp(superb.inodes[i].file_name, buff) == 0) { // bitmap == 1 para saber si accedemos algo valido
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

	st->st_dev = 0;
	st->st_ino = index;
	st->st_uid = i.uid;
	printf("[debug] fisopfs_getattr - UID PASADO AL STAT %i\n", st->st_uid);
	st->st_gid = i.gid;
	printf("[debug] fisopfs_getattr - GID PASADO AL STAT %i\n", st->st_gid);
	st->st_mode = i.mode;
	printf("[debug] fisopfs_getattr - mode %i\n", st->st_mode);
	printf("[debug] fisopfs_getattr - mode constant DIR:  %i\n", __S_IFDIR | 0755);
	printf("[debug] fisopfs_getattr - mode constant FILE:  %i\n", __S_IFREG | 0644);
	st->st_nlink = i.nlink;
	printf("[debug] nlink: %li\n", st->st_nlink);
	st->st_size = i.file_size;
	st->st_atime = i.atime;
	st->st_mtime = i.mtime;
	st->st_ctime = i.ctime;

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
	if ( index == -1) { 
		printf("[debug] fisopfs_READDIR - path: \"%s\" Indice no encontrado \n", path);
		errno = -ENOENT;
		return -ENOENT;
 	}
	struct inode dir_inode = superb.inodes[index];

	if (dir_inode.type != IS_DIR) {
		fprintf(stderr, "[debug] Error readdir: %s\n", strerror(errno));
		errno = ENOTDIR;
		return -ENOTDIR;
	}
	// struct inode i = superb.inodes[index];
	printf("[debug] fisopfs_readdir - a fillear: %s\n\n\n\n\n", path);
	//filler(buffer, superb.inodes[1].file_path, NULL, 0);
	
	for (int j = 0; j < MAX_INODES; j++) {
		if (superb.bitmap_inodos[j] == 1 && strcmp(superb.inodes[j].file_parent, path) == 0) {   // bitmap == 1 para saber si accedemos algo valido
			printf("[debug] fisopfs_readdir - filleo match=> dir: \"%s\" - parent: \"%s\" \n\n\n\n",superb.inodes[j].file_path, path);
			filler(buffer, superb.inodes[j].file_name, NULL, 0);
		}
		else{
			printf("[debug] fisopfs_readdir - NO MATCHEÓ: %s != %s \n\n\n\n",superb.inodes[j].file_path, path);
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
//static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

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
fisopfs_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("\n\n\n\n [debug] fisopfs_write SIN IMPLEMENTAR- path: %s\n\n\n\n", path);

	int index = get_inode_index_from_path(path);
	if (index == -1) {
		printf("[debug] fisopfs_write - path: \"%s\" FALLÓ POR NO ENCONTRAR INDICE \n\n\n\n\n", path);
		errno = -ENOENT;
		return -ENOENT;
	}

	struct inode i = superb.inodes[index];


	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("\n\n\n\n[debug] fisopfs_utimens NO IMPLEMENTADO!!! - path: %s", path);
	int index = get_inode_index_from_path(path);
	if (index == -1) {
		printf("[debug] fisopfs_utimens - path: \"%s\" FALLÓ POR NO ENCONTRAR INDICE \n\n\n\n\n", path);
		return -ENOENT;
	}

	printf("[debug] fisopfs_utimens - actualiza los times del path: \"%s\" (INODO[%i]) \n\n\n\n\n", path, index);
	struct inode i = superb.inodes[index];
	i.atime = tv[0].tv_sec;
	i.mtime = tv[1].tv_sec;

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
	
	//create_inode_from_path("/", __S_IFDIR | 0755, IS_DIR);
	memset(superb.inodes, 0, sizeof(superb.inodes));
	memset(superb.bitmap_inodos, 0, sizeof(superb.bitmap_inodos));

	static struct inode root;
	root.file_size = MAX_CONTENT; // maximo tamaño estático por ahora.
	root.uid = 1717;
	root.gid = getgid();
	root.type = IS_DIR;
	root.nlink = (root.type == IS_FILE) ? 1 : 2;  // Un archivo comienza con un link y un directorio con 2.
	root.mode = __S_IFDIR | 0755;
	root.atime = time(NULL);
	root.mtime = time(NULL);
	root.ctime = time(NULL);

	strcpy(root.file_name, "/");  // root se llama '/'
	strcpy(root.file_path, ROOT); 
	strcpy(root.file_parent, ""); 

	superb.inodes[0] = root; // guardamos el root? TODO checkear en otra función si se guarda
	superb.bitmap_inodos[0] = 1; // ocupado. TODO hacer constantes para ocupado desocupado.
	
	return;
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisop_init - Starting init\n");

	//FILE *fs = fopen(fs_fisopfs, "r");
	void* fs = NULL;
	if (!fs) {
		// Si no existe el archivo, lo creamos
		create_root(); //esto todavia no crea el archivo
	} else {
		// fread(&superb, sizeof(superb), 1, fs); 
		// fclose(fs);
	}

	return 0;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr, //lista
	.readdir = fisopfs_readdir, //lista
	.read = fisopfs_read,

	// new implementations
	.create = fisopfs_create, //lista
	.mkdir = fisopfs_mkdir, //lista
	.write = fisopfs_write,
	.utimens = fisopfs_utimens, //lista
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.flush = fisopfs_flush, //lista
	.destroy = fisopfs_destroy, //lista
	.init = fisopfs_init, //lista
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
