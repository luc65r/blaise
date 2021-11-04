#define _GNU_SOURCE

#include <argp.h>
#include <jansson.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "eval.h"
#include "vec.h"

#include "parse.h"
#include "scan.h"

#define IGNORE_MISSING_FIELD(X...)                                      \
    _Pragma("GCC diagnostic push")                                      \
    _Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")  \
    X                                                                   \
    _Pragma("GCC diagnostic pop")

extern ASTProg *ast;

const char *argp_program_version = "pc 0.1.0";
const char *argp_program_bug_address = "<lucas@ransan.tk>";

static char doc[] = "pc -- pseudocode interpreter";
static char args_doc[] = "[FILE]";

IGNORE_MISSING_FIELD (
static struct argp_option options[] = {
    { "ast", 'a', 0, 0, "Dump abstract syntax tree" },
    { 0 }
};
)

struct args {
    char *file;
    bool ast;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct args *args = state->input;
    switch (key) {
    case 'a':
        args->ast = true;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 1)
            argp_usage(state);
        args->file = arg;
        break;
    case ARGP_KEY_END:
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

IGNORE_MISSING_FIELD (
static struct argp argp = { options, parse_opt, args_doc, doc };
)

int main(int argc, char **argv) {
    int err;
    struct args args = {
        .file = "-",
        .ast = false,
    };

    argp_parse(&argp, argc, argv, 0, 0, &args);

    bool read_stdin = strcmp(args.file, "-") == 0;

    FILE *f = read_stdin ? stdin : fopen(args.file, "r");
    if (f == NULL) {
        fprintf(stderr, "cannot access '%s': %s\n", args.file, strerror(errno));
        return 1;
    }

    yyin = f;
    err = yyparse();
    if (err)
        goto error;
    yylex_destroy();

    if (args.ast) {
        json_t *j = ast_json(ast);
        json_dumpf(j, stdout, JSON_INDENT(2));
        printf("\n");
        json_decref(j);
    } else {
        eval(ast);
    }

    ast_free(ast);

error:
    if (!read_stdin)
        fclose(f);

    return !!err;
}
