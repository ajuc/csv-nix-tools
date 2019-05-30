%{

#include <stdarg.h>
#include <stdio.h>
#include "sql_parser.h"

#define DBG 0

static inline void
dbg_printf(const char *format, ...)
{
#if DBG
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
#endif
}

void error(const char *str);

%}

%option noinput
%option nounput
%option noyywrap
%option case-insensitive

bin_number 0b[01]+
hex_number 0x[0-9a-fA-F]+
oct_number 0[0-7]*
dec_number [1-9][0-9]*
alpha [a-zA-Z]+[a-zA-Z0-9_]*
literal1 '[^']*'
literal2 \"[^"]*\"

%%

[ \t\n]		;
"select"	{ dbg_printf("LEX: matching SELECT\n"); return SELECT; }
"as"		{ dbg_printf("LEX: matching AS\n"); return AS; }
"from"		{ dbg_printf("LEX: matching FROM\n"); return FROM; }
"where"		{ dbg_printf("LEX: matching WHERE\n"); return WHERE; }
"or"		{ dbg_printf("LEX: matching OR\n"); return OR; }
"and"		{ dbg_printf("LEX: matching AND\n"); return AND; }
"xor"		{ dbg_printf("LEX: matching XOR\n"); return XOR; }
"not"		{ dbg_printf("LEX: matching NOT\n"); return NOT; }
"length"	{ dbg_printf("LEX: matching LENGTH\n"); return LENGTH; }
"substr"	{ dbg_printf("LEX: matching SUBSTR\n"); return SUBSTR; }
"fmt_hex"	{ dbg_printf("LEX: matching FMT_HEX\n"); return FMT_HEX; }
"fmt_oct"	{ dbg_printf("LEX: matching FMT_OCT\n"); return FMT_OCT; }
"fmt_dec"	{ dbg_printf("LEX: matching FMT_DEC\n"); return FMT_DEC; }
"fmt_bin"	{ dbg_printf("LEX: matching FMT_BIN\n"); return FMT_BIN; }
"("		{ dbg_printf("LEX: matching LPAREN\n"); return LPAREN; }
")"		{ dbg_printf("LEX: matching RPAREN\n"); return RPAREN; }
"if"		{ dbg_printf("LEX: matching IF\n"); return IF; }
"\+"		{ dbg_printf("LEX: matching ADD\n"); return ADD; }
"-"		{ dbg_printf("LEX: matching SUB\n"); return SUB; }
"\*"		{ dbg_printf("LEX: matching MUL\n"); return MUL; }
"/"		{ dbg_printf("LEX: matching DIV\n"); return DIV; }
"%"		{ dbg_printf("LEX: matching MOD\n"); return MOD; }
"||"		{ dbg_printf("LEX: matching CONCAT\n"); return CONCAT; }

"|"		{ dbg_printf("LEX: matching BIT_OR\n"); return BIT_OR; }
"^"		{ dbg_printf("LEX: matching BIT_XOR\n"); return BIT_XOR; }
"&"		{ dbg_printf("LEX: matching BIT_AND\n"); return BIT_AND; }
"~"		{ dbg_printf("LEX: matching BIT_NEG\n"); return BIT_NEG; }
"<<"		{ dbg_printf("LEX: matching BIT_LSHIFT\n"); return BIT_LSHIFT; }
">>"		{ dbg_printf("LEX: matching BIT_RSHIFT\n"); return BIT_RSHIFT; }

{dec_number}	{ dbg_printf("LEX: matching dec_number\n"); yylval.number = strtoll(yytext,     NULL, 10); return NUMBER; }
{hex_number}	{ dbg_printf("LEX: matching hex_number\n"); yylval.number = strtoll(yytext + 2, NULL, 16); return NUMBER; }
{oct_number}	{ dbg_printf("LEX: matching oct_number\n"); yylval.number = strtoll(yytext + 1, NULL, 8); return NUMBER; }
{bin_number}	{ dbg_printf("LEX: matching bin_number\n"); yylval.number = strtoll(yytext + 2, NULL, 2); return NUMBER; }
{alpha}		{ dbg_printf("LEX: matching alpha '%s'\n", yytext); yylval.name = strdup(yytext); return STRING; }
{literal1}	{ dbg_printf("LEX: matching literal1 '%s'\n", yytext); yylval.name = strndup(yytext + 1, strlen(yytext) - 2); return LITERAL; }
{literal2}	{ dbg_printf("LEX: matching literal2 '%s'\n", yytext); yylval.name = strndup(yytext + 1, strlen(yytext) - 2); return LITERAL; }
","		{ dbg_printf("LEX: matching COMMA\n"); return COMMA; }
"!="		{ dbg_printf("LEX: matching NE\n"); return NE; }
"<>"		{ dbg_printf("LEX: matching NE\n"); return NE; }
"<="		{ dbg_printf("LEX: matching LE\n"); return LE; }
">="		{ dbg_printf("LEX: matching GE\n"); return GE; }
"="		{ dbg_printf("LEX: matching EQ\n"); return EQ; }
"<"		{ dbg_printf("LEX: matching LT\n"); return LT; }
">"		{ dbg_printf("LEX: matching GT\n"); return GT; }
.		{ error(yytext); return OTHER; }

%%

void
error(const char *str)
{
	fprintf(stderr, "LEX: unknown token '%s'\n", str);
	exit(2);
}

#if 0
int
main(int argc, char *argv[])
{
	FILE *in = fmemopen(argv[1], strlen(argv[1]), "r");
	if (!in)
		abort();
	yyin = in;
	while (yylex())
		;
	fclose(in);

	return 0;
}
#endif