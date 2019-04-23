/*
 * st-ls.c
 *
 *  Created on: 22.04.2019
 *      Author: marcin
 */

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void print_stat(const char *dirpath, const char *path, struct stat *st)
{
#if 1
	fprintf(stdout, "mode 0%06o nlink %ld uid %d gid %d size %6ld mtime %ld ",
		st->st_mode, st->st_nlink, st->st_uid, st->st_gid, st->st_size,
		st->st_mtime);
#else
	fprintf(stdout, "dev 0x%lx ino %ld mode 0%06o nlink %ld uid %d gid %d rdev 0x%lx size %6ld blksize %ld blocks %3ld atime %ld mtime %ld ctime %ld ",
		st->st_dev, st->st_ino, st->st_mode, st->st_nlink,
		st->st_uid, st->st_gid, st->st_rdev, st->st_size,
		st->st_blksize, st->st_blocks, st->st_atime, st->st_mtime,
		st->st_ctime);
#endif

	if (dirpath)
		printf("parent %s ", dirpath);

	printf("name %s\n", path);
	// TODO: nsec
	// TODO: user, group
	// TODO: translate mode
	// TODO: separarate parent & name
}

int alphasort_caseinsensitive(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

int list(const char *dirpath, int dirfd, int recursive, int all)
{
	int ret = 0;
	struct dirent **namelist;
	const char **dirs;
	int numdirs = 0;
	char *path;
	size_t pos = strlen(dirpath);

	if (pos < 1)
		return -1;

	int entries = scandirat(dirfd, ".", &namelist, NULL,
			alphasort_caseinsensitive);
	if (entries < 0) {
		fprintf(stderr, "reading %s failed: %s\n", dirpath,
				strerror(errno));
		return -1;
	}

	dirs = malloc(entries * sizeof(dirs[0]));
	if (!dirs) {
		perror("malloc");
		ret = -1;
		goto dirs_alloc_fail;
	}

	path = malloc(pos + 1 + sizeof(namelist[0]->d_name));
	if (!path) {
		perror("malloc");
		ret = -1;
		goto path_alloc_fail;
	}

	strcpy(path, dirpath);
	if (path[pos - 1] != '/')
		path[pos++] = '/';

	for (int i = 0; i < entries; ++i) {
		struct stat statbuf;
		/* skip hidden files */
		if (!all && namelist[i]->d_name[0] == '.')
			continue;

		if (fstatat(dirfd, namelist[i]->d_name, &statbuf,
				AT_SYMLINK_NOFOLLOW)) {
			ret = 1;
			continue;
		}

		print_stat(dirpath, namelist[i]->d_name, &statbuf);

		if (!recursive)
			continue;
		if (!S_ISDIR(statbuf.st_mode))
			continue;
		if (strcmp(namelist[i]->d_name, ".") == 0)
			continue;
		if (strcmp(namelist[i]->d_name, "..") == 0)
			continue;

		dirs[numdirs++] = namelist[i]->d_name;
	}


	for (int j = 0; j < numdirs; ++j) {
		int fd = openat(dirfd, dirs[j], O_PATH);
		if (fd < 0) {
			perror("openat");
			continue;
		}

		strcpy(path + pos, dirs[j]);

		if (list(path, fd, recursive, all))
			ret = 1;

		close(fd);
	}

	free(path);
path_alloc_fail:
	free(dirs);
dirs_alloc_fail:
	for (int i = 0; i < entries; ++i)
		free(namelist[i]);
	free(namelist);

	return ret;
}

int main(int argc, char *argv[])
{
	int opt, ret = 0;
	int dir = 0, recursive = 0, all = 0;

	while ((opt = getopt(argc, argv, "adR")) != -1) {
		// TODO: -U
		switch (opt) {
			case 'a':
				all = 1;
				break;
			case 'd':
				dir = 1;
				break;
			case 'R':
				recursive = 1;
				break;
			default:
				fprintf(stderr,
					"Usage: %s [-a] [-d] [-R] [paths ...]\n",
					argv[0]);
				break;
		}
	}

	for (int i = optind; i < argc; ++i) {
		/* TODO: O_NOFOLLOW */
		int fd = openat(AT_FDCWD, argv[i], O_PATH);
		if (fd < 0) {
			perror("open");
			continue;
		}

		struct stat buf;
		int r = fstat(fd, &buf);
		if (r < 0) {
			perror("fstat");
			close(fd);
			continue;
		}

		if (S_ISDIR(buf.st_mode)) {
			if (dir) {
				print_stat(NULL, argv[i], &buf);
			} else {
				if (list(argv[i], fd, recursive, all))
					ret = 1;
			}
		} else {
			print_stat(NULL, argv[i], &buf);
		}

		close(fd);
	}

	return ret;
}
