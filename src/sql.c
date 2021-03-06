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
#include "sql.h"
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
	fprintf(out, "Usage: csv-sql [OPTION] sql-query\n");
	fprintf(out,
"Read CSV stream from standard input, process it using simplified SQL query\n"
"and print back to standard output its result.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct column {
	struct rpn_expression expr;
	char *name;
};

struct cb_params {
	struct column *columns;
	size_t columns_count;

	struct rpn_expression *where;
};

static void
process_exp(struct rpn_expression *exp, const char *buf, const size_t *col_offs,
		const struct col_header *headers, char sep)
{
	struct rpn_variant ret;

	if (rpn_eval(exp, buf, col_offs, headers, &ret))
		exit(2);

	if (ret.type == RPN_LLONG)
		printf("%lld%c", ret.llong, sep);
	else if (ret.type == RPN_PCHAR) {
		csv_print_quoted(ret.pchar, strlen(ret.pchar));
		fputc(sep, stdout);
		free(ret.pchar);
	} else {
		fprintf(stderr, "unknown type %d\n", ret.type);
		exit(2);
	}
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;
	struct rpn_expression *exp;
	struct rpn_variant ret;

	if (params->where) {
		if (rpn_eval(params->where, buf, col_offs, headers, &ret))
			exit(2);

		if (ret.type != RPN_LLONG) /* shouldn't be possible - XXX? */
			abort();

		if (ret.llong == 0)
			return 0;
	}

	for (size_t i = 0; i < params->columns_count - 1; ++i) {
		exp = &params->columns[i].expr;

		process_exp(exp, buf, col_offs, headers, ',');
	}

	exp = &params->columns[params->columns_count - 1].expr;

	process_exp(exp, buf, col_offs, headers, '\n');

	return 0;
}

static struct cb_params Params;

#include "sql-shared.h"

void
sql_column_done(void)
{
	struct rpn_expression exp;
	exp.tokens = Tokens;
	exp.count = Ntokens;

	Params.columns = xrealloc_nofail(Params.columns,
			Params.columns_count + 1,
			sizeof(Params.columns[0]));
	struct column *col = &Params.columns[Params.columns_count];

	col->expr = exp;
	col->name = NULL;

	if (Ntokens == 1 && Tokens[0].type == RPN_COLUMN)
		col->name = xstrdup_nofail(Headers[Tokens[0].colnum].name);

	Params.columns_count++;
	Tokens = NULL;
	Ntokens = 0;
}

void
sql_named_column_done(char *name)
{
	struct rpn_expression exp;
	exp.tokens = Tokens;
	exp.count = Ntokens;

	Params.columns = xrealloc_nofail(Params.columns,
			Params.columns_count + 1,
			sizeof(Params.columns[0]));

	struct column *col = &Params.columns[Params.columns_count];
	col->expr = exp;
	col->name = name;

	Params.columns_count++;
	Tokens = NULL;
	Ntokens = 0;
}

void
sql_all_columns(void)
{
	Params.columns = xmalloc_nofail(Nheaders, sizeof(Params.columns[0]));
	Params.columns_count = Nheaders;

	for (size_t i = 0; i < Nheaders; ++i) {
		struct column *col = &Params.columns[i];
		col->name = xstrdup_nofail(Headers[i].name);
		col->expr.count = 1;
		col->expr.tokens = xmalloc_nofail(1, sizeof(col->expr.tokens[0]));
		col->expr.tokens[0].type = RPN_COLUMN;
		col->expr.tokens[0].colnum = i;
	}
}

void
sql_from(char *name)
{
	if (strcmp(name, "input") != 0) {
		fprintf(stderr, "unknown table '%s'\n", name);
		exit(2);
	}

	free(name);
}

void
sql_where()
{
	if (Ntokens > 0) {
		fprintf(stderr, "invalid state\n");
		exit(2);
	}
}

void
sql_end_of_conditions()
{
	struct rpn_expression *exp = xmalloc_nofail(1, sizeof(*exp));

	exp->tokens = Tokens;
	exp->count = Ntokens;

	Params.where = exp;

	Tokens = NULL;
	Ntokens = 0;
}

static void
print_column_header(size_t i, char sep)
{
	const struct column *col = &Params.columns[i];
	if (col->name)
		printf("%s", col->name);
	else
		printf("unnamed%lu", i);

	printf(":%s%c", rpn_expression_type(&col->expr, Headers), sep);
}

int
main(int argc, char *argv[])
{
	int opt;
	bool show = false;
	bool show_full;

	while ((opt = getopt_long(argc, argv, "sS", opts, NULL)) != -1) {
		switch (opt) {
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
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

	if (optind != argc - 1) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	Nheaders = csv_get_headers(s, &Headers);

	FILE *in = fmemopen(argv[optind], strlen(argv[optind]), "r");
	if (!in) {
		perror("fmemopen");
		exit(2);
	}

	yyin = in;

	yyparse();
	yylex_destroy();

	fclose(in);

	for (size_t i = 0; i < Params.columns_count - 1; ++i)
		print_column_header(i, ',');

	print_column_header(Params.columns_count - 1, '\n');

	if (Params.columns_count < 1)
		abort();

	csv_read_all_nofail(s, &next_row, &Params);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < Params.columns_count; ++i) {
		rpn_free(&Params.columns[i].expr);
		free(Params.columns[i].name);
	}

	free(Params.columns);
	if (Params.where) {
		rpn_free(Params.where);
		free(Params.where);
	}
	rpn_fini();

	return 0;
}
