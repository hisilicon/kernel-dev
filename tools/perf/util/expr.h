/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PARSE_CTX_H
#define PARSE_CTX_H 1

#define EXPR_MAX_OTHER 100
#define MAX_PARSE_ID EXPR_MAX_OTHER
#define EXPR_MAX 20

struct parse_id {
	const char *name;
	double val;
};

struct parse_ctx {
	union {
		struct {
			int		num_ids;
			struct parse_id	ids[MAX_PARSE_ID];
		};
		struct {
			int	 cnt;
			char	*id[EXPR_MAX];
			char	*expr[EXPR_MAX];
		};
	};
};

void expr__ctx_init(struct parse_ctx *ctx);
void expr__add_id(struct parse_ctx *ctx, const char *id, double val);
int expr__parse(double *final_val, struct parse_ctx *ctx, const char *expr);
int expr__parse_multi(struct parse_ctx *ctx, const char *expr);
int expr__find_other(const char *expr, const char *one, const char ***other,
		int *num_other);

#endif
