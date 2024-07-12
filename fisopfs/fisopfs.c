#define FUSE_USE_VERSION 30
#include <fuse.h>
#include "fisopfs.h"

char fs_fisopfs[MAX_PATH] = "fs.fisopfs";

super_block_t superb = {};

const char *
get_last_element(const char *path)
{
	char *last_element = strrchr(path, '/');
	if (last_element != NULL) {
		return last_element + 1;
	}
	return path;
}

int
fetch_free_index(super_block_t *superb)
{
	if (!superb)
		return ERROR;
	int i = 0;
	while (i < (MAX_INODES) && superb->bitmap_inodos[i] == OCCUPIED) {
		i++;
	}
	return i;
}

int
set_inode_in_superblock(inode_t *i)
{
	int free_idx = fetch_free_index(&superb);
	if (!free_idx)
		return ERROR;
	superb.inodes[free_idx] = *i;
	superb.bitmap_inodos[free_idx] = OCCUPIED;
	return free_idx;
}

const char *
get_parent(const char *path)
{
	char path_dup[MAX_PATH];
	strcpy(path_dup, path);
	char *primer_dir = strtok(path_dup, "/");
	char *resto = strtok(NULL, "/");
	if (!resto) {
		return ROOT;
	}
	return primer_dir;
}

static int
create_inode_from_path(const char *path, mode_t mode, int type)
{
	printf("[debug] create_inode_from_path PRE CREACION- path: %s - mode: "
	       "%d - type: %d\n",
	       path,
	       mode,
	       type);

	static inode_t i;
	i.file_size = 0;
	i.uid = getuid();
	i.gid = getgid();
	i.type = type;
	i.mode = (type == IS_FILE) ? __S_IFREG | 0644 : __S_IFDIR | 0755;
	i.atime = time(NULL);
	i.mtime = time(NULL);
	i.ctime = time(NULL);
	i.nlink = (type == IS_FILE) ? 1 : 2;

	strcpy(i.file_name, get_last_element(path));
	strcpy(i.file_path, path);
	strcpy(i.file_parent, get_parent(path));

	int free_idx = set_inode_in_superblock(&i);
	if (free_idx < 0)
		return free_idx;
	memset(i.file_content, 0, sizeof(i.file_content));

	return 0;
}

static int
get_inode_index_from_path(const char *path)
{
	if (strcmp(path, ROOT) == 0)
		return 0;

	const char *buff = get_last_element(path);
	for (int i = 0; i < MAX_INODES; i++) {
		if (superb.bitmap_inodos[i] == OCCUPIED &&
		    strcmp(superb.inodes[i].file_name, buff) ==
		            0) {  // bitmap == 1 para saber si accedemos algo valido
			return i;
		}
	}
	return ERROR;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	memset(st, 0, sizeof(struct stat));

	int index = get_inode_index_from_path(path);
	if (index == BAD_INDEX) {
		printf("[debug] fisopfs_getattr - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE O ESTÁ VACÍO\n",
		       path);
		return -ENOENT;
	}
	inode_t i = superb.inodes[index];

	st->st_dev = 0;
	st->st_ino = index;
	st->st_uid = i.uid;
	st->st_gid = i.gid;
	st->st_mode = i.mode;
	st->st_nlink = i.nlink;
	st->st_size = i.file_size;
	st->st_atime = i.atime;
	st->st_mtime = i.mtime;
	st->st_ctime = i.ctime;

	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("\n\n\n\n[debug] fisopfs_readdir - path: %s\n", path);

	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);
	int index = get_inode_index_from_path(path);
	if (index == BAD_INDEX) {
		printf("[debug] fisopfs_READDIR - path: \"%s\" Indice no "
		       "encontrado \n",
		       path);
		errno = -ENOENT;
		return -ENOENT;
	}
	inode_t dir_inode = superb.inodes[index];

	if (dir_inode.type != IS_DIR) {
		fprintf(stderr, "[debug] Error readdir: %s\n", strerror(errno));
		errno = ENOTDIR;
		return -ENOTDIR;
	}
	if (strcmp(path, ROOT) != 0) {
		path++;
	}

	for (int j = 0; j < MAX_INODES; j++) {
		if (superb.bitmap_inodos[j] == OCCUPIED &&
		    strcmp(superb.inodes[j].file_parent, path) == 0) {
			filler(buffer, superb.inodes[j].file_name, NULL, 0);
		} else {
		}
	}
	return 0;
}

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
	if (index == BAD_INDEX) {
		printf("[debug] fisopfs_read - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE \n",
		       path);
		return -ENOENT;
	}

	inode_t i = superb.inodes[index];
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
	return create_inode_from_path(path, mode, IS_DIR);
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);
	int index = get_inode_index_from_path(path);
	if (index == BAD_INDEX) {
		printf("[debug] fisopfs_rmdir - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE \n",
		       path);
		return -ENOENT;
	}
	inode_t i = superb.inodes[index];
	if (i.type != IS_DIR) {
		fprintf(stderr, "[debug] Error rmdir: %s\n", strerror(errno));
		errno = ENOTDIR;
		return -ENOTDIR;
	}

	if (strcmp(path, ROOT) != 0) {
		path++;
	}

	for (int j = 0; j < MAX_INODES; j++) {
		if (superb.bitmap_inodos[j] == OCCUPIED &&
		    strcmp(superb.inodes[j].file_parent, path) == 0) {
			superb.bitmap_inodos[j] = FREE;
		}
	}
	superb.bitmap_inodos[index] = FREE;
	return 0;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);
	int index = get_inode_index_from_path(path);
	if (index == BAD_INDEX) {
		printf("[debug] fisopfs_unlink - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE\n",
		       path);
		return -ENOENT;
	}

	inode_t *inode = &superb.inodes[index];
	if (inode->type != IS_FILE) {
		fprintf(stderr, "[debug] Error unlink: %s\n", strerror(errno));
		errno = EISDIR;
		return -EISDIR;
	}

	inode->nlink--;

	if (inode->nlink == 0) {
		superb.bitmap_inodos[index] = FREE;
	}

	return 0;
}


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
	printf("[debug] fisopfs_write - path: %s - size: %lu - offset: %lu\n",
	       path,
	       size,
	       offset);
	if (size + offset > MAX_CONTENT) {
		printf("[debug] fisopfs_write - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE \n\n\n\n\n",
		       path);
		errno = -ENOENT;
		return -ENOENT;
	}

	int index = get_inode_index_from_path(path);
	if (index == BAD_INDEX) {
		printf("[debug] fisopfs_write - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE \n\n\n\n\n",
		       path);
		errno = -ENOENT;
		return -ENOENT;
	}

	inode_t *i = &superb.inodes[index];
	if (i->file_size < offset) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}
	if (i->type != IS_FILE) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = EACCES;
		return -EACCES;
	}
	strncpy(i->file_content + offset, buffer, size);
	i->atime = time(NULL);
	i->mtime = time(NULL);
	i->file_size = strlen(i->file_content);
	i->file_content[i->file_size] = '\0';
	return (int) size;
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_utimens - path: %s", path);
	int index = get_inode_index_from_path(path);
	if (index == BAD_INDEX) {
		printf("[debug] fisopfs_utimens - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE\n",
		       path);
		return -ENOENT;
	}

	superb.inodes[index].atime = tv[0].tv_sec;
	superb.inodes[index].mtime = tv[1].tv_sec;

	return 0;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);
	if (size > MAX_CONTENT) {
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}
	int index = get_inode_index_from_path(path);
	if (index < 0) {
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	inode_t *inode = &(superb.inodes[index]);
	inode->file_size = size;
	inode->mtime = time(NULL);
	return 0;
}

static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy - destroying root\n");

	FILE *fs = fopen(fs_fisopfs, MODE_WRITE);
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

	memset(superb.inodes, 0, sizeof(superb.inodes));
	memset(superb.bitmap_inodos, 0, sizeof(superb.bitmap_inodos));

	static inode_t root;
	root.file_size = MAX_CONTENT;
	root.uid = 1717;
	root.gid = getgid();
	root.type = IS_DIR;
	root.nlink = (root.type == IS_FILE) ? 1 : 2;
	root.mode = __S_IFDIR | 0755;
	root.atime = time(NULL);
	root.mtime = time(NULL);
	root.ctime = time(NULL);

	strcpy(root.file_name, ROOT);
	strcpy(root.file_path, ROOT);
	strcpy(root.file_parent, NO_PARENT);

	superb.inodes[0] = root;
	superb.bitmap_inodos[0] = OCCUPIED;

	return;
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisop_init - Starting init\n");

	FILE *fs = fopen(fs_fisopfs, "r");
	if (!fs) {
		FILE *fs_new = fopen(fs_fisopfs, "w");
		fclose(fs_new);
		create_root();
	} else {
		int bytes = fread(&superb, sizeof(superb), 1, fs);
		if (bytes < 0) {
			printf("[debug] fisop_init - error reading fs\n");
			return NULL;
		}
		fclose(fs);
	}

	return 0;
}


static int
fisopfs_chmod(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_chmod - path: %s - mode: %o\n", path, mode);
    int index = get_inode_index_from_path(path);
    if (index == BAD_INDEX) {
        printf("[debug] fisopfs_chmod - path: \"%s\" FALLÓ POR NO "
               "ENCONTRAR INDICE \n",
               path);
        return -ENOENT;
    }

    if (superb.inodes[index].mode & S_IWUSR) {
        printf("[debug] fisopfs_chmod - path: %s - Modo de archivo antes de cambiar: %o\n",
               path,
               superb.inodes[index].mode);
        superb.inodes[index].mode = (superb.inodes[index].mode & ~S_IRWXU) | (mode & S_IRWXU);

        printf("[debug] fisopfs_chmod - path: %s - Modo actualizado: %o\n",
               path,
               superb.inodes[index].mode);
    } else {
        printf("[debug] fisopfs_chmod - path: %s - No se puede escribir en un archivo de solo lectura.\n",
               path);
        return -EACCES; 
    }

    return EXIT_SUCCESS;
}

static int
fisopfs_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("[debug] fisopfs_chown - path: %s - uid: %d - gid: %d\n",
	       path,
	       uid,
	       gid);
	int index = get_inode_index_from_path(path);
	if (index == BAD_INDEX) {
		printf("[debug] fisopfs_chown - path: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE \n",
		       path);
		return -ENOENT;
	}
	printf("[debug] fisopfs_chown - path: %s - uid anterior: %d - uid "
	       "nuevo: %d - gid anterior: %d - gid nuevo: %d\n",
	       path,
	       superb.inodes[index].uid,
	       uid,
	       superb.inodes[index].gid,
	       gid);
	superb.inodes[index].uid = uid;
	superb.inodes[index].gid = gid;

	return EXIT_SUCCESS;
}


static int
fisopfs_link(const char *from, const char *to)
{
	printf("[debug] fisopfs_link - from: %s - to: %s\n", from, to);
	int from_idx = get_inode_index_from_path(from);
	if (from_idx == BAD_INDEX) {
		printf("[debug] fisopfs_link - from: \"%s\" FALLÓ POR NO "
		       "ENCONTRAR INDICE\n",
		       from);
		return -ENOENT;
	}

	inode_t *inode_from = &superb.inodes[from_idx];
	if (inode_from->type != IS_FILE) {
		printf("[debug] fisopfs_link - from: \"%s\" NO ES UN ARCHIVO\n",
		       from);
		return -EPERM;
	}

	int to_idx = get_inode_index_from_path(to);
	if (to_idx != BAD_INDEX) {
		printf("[debug] fisopfs_link - to: \"%s\" YA EXISTE\n", to);
		return -EEXIST;
	}

	inode_from->nlink++;

	strcpy(superb.inodes[from_idx].file_name, get_last_element(to));
	strcpy(superb.inodes[from_idx].file_path, to);
	strcpy(superb.inodes[from_idx].file_parent, get_parent(to));

	superb.bitmap_inodos[from_idx] = OCCUPIED;

	return 0;
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
	.truncate = fisopfs_truncate,

	// challenges
	.chmod = fisopfs_chmod,
	.chown = fisopfs_chown,
	.link = fisopfs_link,
};


int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
