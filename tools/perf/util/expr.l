%option prefix="expr_"
%option reentrant
%option bison-bridge

%{
#include <linux/compiler.h>
#include "expr.h"
#include "expr-bison.h"

char *expr_get_text(yyscan_t yyscanner);
YYSTYPE *expr_get_lval(yyscan_t yyscanner);

static int __value(YYSTYPE *yylval, char *str, int base, int token)
{
	u64 num;

	errno = 0;
	num = strtoull(str, NULL, base);
	if (errno)
		return EXPR_ERROR;

	yylval->num = num;
	return token;
}

static int value(yyscan_t scanner, int base)
{
	YYSTYPE *yylval = expr_get_lval(scanner);
	char *text = expr_get_text(scanner);

	return __value(yylval, text, base, NUMBER);
}

static int str(yyscan_t scanner, int token)
{
	YYSTYPE *yylval = expr_get_lval(scanner);
	char *text = expr_get_text(scanner);

	yylval->str = strdup(text);
	return token;
}
%}

%x multi

number		[0-9]+
symbol		[0-9a-zA-Z_\.:@\\]+
all		[^;]+

%%
	{
		int start_token;

		start_token = parse_events_get_extra(yyscanner);

		if (start_token) {
			parse_events_set_extra(NULL, yyscanner);
			return start_token;
		}
	}

<multi>{

{all}		{ BEGIN(INITIAL); return str(yyscanner, ALL); }

}

max		{ return MAX; }
min		{ return MIN; }
if		{ return IF; }
else		{ return ELSE; }
#smt_on		{ return SMT_ON; }
{number}	{ return value(yyscanner, 10); }
{symbol}	{ return str(yyscanner, ID); }
"|"		{ return '|'; }
"^"		{ return '^'; }
"&"		{ return '&'; }
"-"		{ return '-'; }
"+"		{ return '+'; }
"*"		{ return '*'; }
"/"		{ return '/'; }
"%"		{ return '%'; }
"("		{ return '('; }
")"		{ return ')'; }
","		{ return ','; }
";"		{ return ';'; }
"="		{ BEGIN(multi); return '='; }
.		{ }
%%

int expr_wrap(void *scanner __maybe_unused)
{
	return 1;
}
