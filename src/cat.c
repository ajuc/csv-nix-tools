/*
 * Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"no-header",	no_argument,		NULL, 'H'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-cat [OPTION]... [FILE]...\n");
	printf("Options:\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	size_t *idx = arg;

	for (size_t i = 0; i < nheaders - 1; ++i) {
		fputs(&buf[col_offs[idx[i]]], stdout);
		fputc(',', stdout);
	}
	fputs(&buf[col_offs[idx[nheaders - 1]]], stdout);
	fputc('\n', stdout);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	bool print_header = true;

	while ((opt = getopt_long(argc, argv, "v", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'H':
				print_header = false;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					case 1:
					default:
						usage();
						return 2;
				}
				break;
			case 'h':
			default:
				usage();
				return 2;
		}
	}

	size_t ninputs = argc - optind;

	struct inputs {
		FILE *f;
		struct csv_ctx *s;
		size_t *idx;
	} *inputs = calloc(ninputs, sizeof(inputs[0]));

	bool stdin_used = false;
	size_t nheaders;
	const struct col_header *headers;

	size_t i = 0;
	while (optind < argc) {
		FILE *f;
		if (strcmp(argv[optind], "-") == 0) {
			if (stdin_used) {
				fprintf(stderr,
					"stdin is used more than once\n");
				exit(2);
			}
			f = stdin;
			stdin_used = true;
		} else {
			f = fopen(argv[optind], "r");
			if (!f) {
				fprintf(stderr, "opening '%s' failed: %s\n",
					argv[optind], strerror(errno));
				exit(2);
			}
		}

		struct csv_ctx *s = csv_create_ctx(f, stderr);
		if (!s)
			exit(2);
		if (csv_read_header(s))
			exit(2);

		const struct col_header *headers_cur;
		size_t nheaders_cur = csv_get_headers(s, &headers_cur);

		inputs[i].f = f;
		inputs[i].s = s;
		inputs[i].idx = malloc(nheaders_cur * sizeof(inputs->idx[0]));
		if (!inputs->idx) {
			perror("malloc");
			exit(2);
		}

		if (i == 0) {
			nheaders = nheaders_cur;
			headers = headers_cur;

			for (size_t j = 0; j < nheaders; ++j)
				inputs[i].idx[j] = j;
		} else {
			if (nheaders != nheaders_cur) {
				fprintf(stderr,
					"files have different number of columns\n");
				exit(2);
			}

			for (size_t j = 0; j < nheaders; ++j) {
				size_t idx = csv_find(headers_cur, nheaders_cur,
						headers[j].name);
				if (idx == CSV_NOT_FOUND) {
					fprintf(stderr,
						"column '%s' not found in input %ld\n",
						headers[j].name, i + 1);
					exit(2);
				}

				if (strcmp(headers[j].type,
						headers_cur[idx].type) != 0) {
					fprintf(stderr,
						"column '%s' have different types in different inputs\n",
						headers[j].name);
					exit(2);
				}

				inputs[i].idx[j] = idx;
			}
		}

		optind++;
		i++;
	}

	if (print_header)
		csv_print_header(stdout, headers, nheaders);

	for (i = 0; i < ninputs; ++i) {
		struct inputs *in = &inputs[i];
		if (csv_read_all(in->s, &next_row, in->idx))
			exit(2);
		csv_destroy_ctx(in->s);
		free(in->idx);
		fclose(in->f);
	}

	free(inputs);

	return 0;
}