/*
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"column",	required_argument,	NULL, 'c'},
	{"new-name",	required_argument,	NULL, 'n'},
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"table",	required_argument,	NULL, 'T'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-add-exec [OPTION]... -- command\n");
	fprintf(out,
"Read CSV stream from standard input and print it back to standard output with\n"
"a new column produced by reading standard output of an external command whose\n"
"standard input is fed with input column.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c, --column=NAME          use column NAME as an input\n");
	fprintf(out, "  -n, --new-name=NAME        create column NAME as an output\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct subst {
	size_t argv_idx;
	size_t col;
};

struct cb_params {
	char **argv;

	struct subst *substs;
	size_t nsubsts;

	size_t stdin_col;

	size_t table_column;
	char *table;
};

static void
write_all(int fd, const char *buf, size_t len)
{
	int written = 0;

	while (len) {
		int w = write(fd, buf, len);
		if (w < 0) {
			perror("write");
			exit(2);
		}

		written += w;
		buf += w;
		len -= w;
	}
}

static size_t
read_all(int fd, char **buf, size_t *buf_len)
{
	size_t readin = 0;
	if (*buf_len < 1) {
		*buf_len = 1;
		*buf = xrealloc_nofail(*buf, *buf_len, 1);
	}

	ssize_t ret;

	ret = read(fd, (*buf) + readin, *buf_len - readin);
	while (ret > 0) {
		readin += (size_t)ret;

		if (*buf_len - readin == 0) {
			*buf_len *= 2;
			*buf = xrealloc_nofail(*buf, *buf_len, 1);
		}

		ret = read(fd, (*buf) + readin, *buf_len - readin);
	}

	if (ret < 0) {
		perror("read");
		exit(2);
	}

	return readin;
}

static char *input_buf;
static size_t input_buf_len;

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;
	int child_in[2]; /* 0=read, 1=write*/
	int child_out[2];

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, nheaders, false);

			putchar(',');
			putchar('\n');

			return 0;
		}
	}

	if (pipe(child_in) < 0) {
		perror("pipe");
		exit(2);
	}

	if (pipe(child_out) < 0) {
		perror("pipe");
		exit(2);
	}

	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(2);
	}

	if (pid == 0) {
		char **argv = params->argv;

		for (size_t i = 0; i < params->nsubsts; ++i) {
			struct subst *subst = &params->substs[i];
			char *d = (char *)&buf[col_offs[subst->col]];
			if (d[0] == '"')
				d = csv_unquot(d);

			argv[subst->argv_idx] = d;
		}

		if (close(child_in[1])) {
			perror("close");
			exit(2);
		}

		if (dup2(child_in[0], 0) < 0) {
			perror("dup2");
			exit(2);
		}

		if (close(child_out[0])) {
			perror("close");
			exit(2);
		}

		if (dup2(child_out[1], 1) < 0) {
			perror("dup2");
			exit(2);
		}

		execvp(argv[0], argv);
		perror("execvp");
		exit(2);
	}

	if (close(child_in[0])) {
		perror("close");
		exit(2);
	}

	if (close(child_out[1])) {
		perror("close");
		exit(2);
	}

	if (params->stdin_col != SIZE_MAX) {
		char *str = (char *)&buf[col_offs[params->stdin_col]];
		char *unquot = str;
		if (str[0] == '"')
			unquot = csv_unquot(str);

		write_all(child_in[1], unquot, strlen(unquot));

		if (str[0] == '"')
			free(unquot);
	}

	if (close(child_in[1])) {
		perror("close");
		exit(2);
	}

	int wstatus;
	pid_t termindated = wait(&wstatus);
	if (termindated == -1) {
		perror("wait");
		exit(2);
	}

	if (WIFEXITED(wstatus)) {
		int status = WEXITSTATUS(wstatus);
		if (status) {
			fprintf(stderr,
				"child process terminated with status code %d\n",
				status);
			exit(3);
		}
	} else if (WIFSIGNALED(wstatus)) {
		int sig = WTERMSIG(wstatus);
		fprintf(stderr, "child process terminated by signal %d\n", sig);
		exit(4);
	} else {
		fprintf(stderr,
			"unhandled child termination type (wstatus=0x%x)\n",
			wstatus);
		exit(4);
	}

	csv_print_line(stdout, buf, col_offs, nheaders, false);
	fputc(',', stdout);

	size_t readin = read_all(child_out[0], &input_buf, &input_buf_len);
	csv_print_quoted(input_buf, readin);
	fputc('\n', stdout);

	if (close(child_out[0])) {
		perror("close");
		exit(2);
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	char *stdin_colname = NULL;
	char *new_colname = NULL;
	bool show = false;
	bool show_full;

	memset(&params, 0, sizeof(params));
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:n:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				stdin_colname = xstrdup_nofail(optarg);
				break;
			case 'n':
				new_colname = xstrdup_nofail(optarg);
				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
				break;
			case 'T':
				params.table = xstrdup_nofail(optarg);
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (!new_colname) {
		fprintf(stderr, "missing -n/--new-name argument\n");
		usage(stderr);
		exit(2);
	}

	size_t args = argc - optind;
	if (args == 0) {
		usage(stderr);
		exit(2);
	}

	params.argv = xmalloc_nofail(args + 1, sizeof(params.argv[0]));
	params.argv[args] = NULL;

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	for (size_t i = optind; i < argc; ++i) {
		size_t idx = i - optind;

		if (argv[i][0] == '%') {
			size_t col = csv_find_loud(headers, nheaders,
					params.table, argv[i] + 1);
			if (col == CSV_NOT_FOUND)
				exit(2);

			params.substs = xrealloc_nofail(params.substs,
				++params.nsubsts, sizeof(params.substs[0]));

			params.substs[params.nsubsts - 1].argv_idx = idx;
			params.substs[params.nsubsts - 1].col = col;
		} else {
			params.argv[idx] = argv[i];
		}
	}

	if (stdin_colname) {
		params.stdin_col = csv_find_loud(headers, nheaders,
				params.table, stdin_colname);
		if (params.stdin_col == CSV_NOT_FOUND)
			exit(2);
	} else {
		params.stdin_col = SIZE_MAX;
	}

	if (show)
		csv_show(show_full);

	for (size_t i = 0; i < nheaders; ++i)
		printf("%s:%s,", headers[i].name, headers[i].type);

	if (params.table)
		printf("%s.", params.table);
	printf("%s:string\n", new_colname);

	free(new_colname);

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	free(params.substs);
	free(params.argv);
	free(params.table);
	free(stdin_colname);
	free(input_buf);

	return 0;
}
