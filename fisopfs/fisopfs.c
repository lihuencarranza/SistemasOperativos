#define FUSE_USE_VERSION 30

#include <fuse.h>
#include "fisopfs.h"

char fs_fisopfs[MAX_PATH] = "fs.fisopfs";

struct super_block superb = {};

static int
create_inode_from_path(const char *path, mode_t mode, int type)
{
	// Hardcodeamos un solo archivo
	static struct inode i;
	i.file_size = 15;                           // SETEAR A 0
	strcpy(i.file_content, "hola fisopfs!\n");  // BORRRARLO!
	i.uid = getuid();
	i.type = type;
	i.nlink =
	        (type == IS_FILE)
	                ? 1
	                : 2;  // Un archivo comienza con un link y un directorio con 2.
	i.mode = mode;


	superb.inodes[0] = i;

	strcpy(i.file_name, path);  // SACARLE LA / AL PATH
	return 0;
}

static int
get_inode_index_from_path(const char *path)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(superb.inodes[i].file_name, path) == 0) {
			return i;
		}
	}
	return -1;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	int index = get_inode_index_from_path(path);
	if (index == -1) {
		printf("[debug] fisopfs_getattr - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR "
		       "INDICE \n",
		       path);
		return -ENOENT;
	}

	struct inode i = superb.inodes[index];
	st->st_uid = i.uid;
	st->st_mode = i.mode;
	st->st_nlink = i.nlink;
	st->st_size = i.file_size;
	st->st_atime = i.atime;
	st->st_mtime = i.mtime;


	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	if (strcmp(path, "/") == 0) {
		filler(buffer, "fisop", NULL, 0);
		return 0;
	}

	return -ENOENT;
}

#define MAX_CONTENIDO 100
static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	int index = get_inode_index_from_path(path);
	if (index == -1) {
		printf("[debug] fisopfs_read - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR "
		       "INDICE \n",
		       path);
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
	printf("[debug] fisopfs_mkdir - path: %s - mode: %d \n", path, mode);
	return 0;
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
	return create_inode_from_path(path, mode, IS_DIR);
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

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush - path: %s\n", path);

	return 0;
}

static int
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy - destroying root\n");
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
	printf("[debug] fisop_init - creating root\n");

	FILE *fs = fopen(fs_fisopfs, "r");
	if (!fs) {
		printf("[debug] fisop_init - creating root\n");
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
