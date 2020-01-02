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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-add-substring [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c name\n");
	fprintf(out, "  -n new-name\n");
	fprintf(out, "  -p start-pos\n");
	fprintf(out, "  -l length\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t col;
	ssize_t start_pos;
	size_t length;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	csv_print_line(stdout, buf, col_offs, nheaders, false);
	fputc(',', stdout);

	const char *str = &buf[col_offs[params->col]];
	const char *unquoted = str;
	if (str[0] == '"')
		unquoted = csv_unquot(str);

	ssize_t start = params->start_pos;
	size_t len = params->length;
	csv_substring_sanitize(unquoted, &start, &len);
	csv_print_quoted(unquoted + (size_t)start, len);

	fputc('\n', stdout);

	if (str[0] == '"')
		free((char *)unquoted);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	char *input_col = NULL;
	char *new_name = NULL;
	struct cb_params params;
	bool show = false;
	bool show_full;

	memset(&params, 0, sizeof(params));
	params.length = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:l:p:sSn:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				input_col = xstrdup_nofail(optarg);
				break;
			case 'n':
				new_name = xstrdup_nofail(optarg);
				break;
			case 'p':
				if (strtol_safe(optarg, &params.start_pos, 0))
					exit(2);
				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
				break;
			case 'l':
				if (strtoul_safe(optarg, &params.length, 0))
					exit(2);
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

	if (!input_col || !new_name) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (csv_find(headers, nheaders, new_name) != CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' already exists in input\n",
				new_name);
		exit(2);
	}

	params.col = csv_find(headers, nheaders, input_col);
	if (params.col == CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' not found in input\n", input_col);
		exit(2);
	}

	free(input_col);
	input_col = NULL;

	for (size_t i = 0; i < nheaders; ++i)
		printf("%s:%s,", headers[i].name, headers[i].type);
	printf("%s:string\n", new_name);

	free(new_name);
	new_name = NULL;

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	return 0;
}