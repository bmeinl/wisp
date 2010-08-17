#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/******************** MODEL ********************/

typedef enum { FIXNUM } object_type;

typedef struct {
    object_type type;
    union {
        struct {
            long value;
        } fixnum;
    } data;
} object;

object* alloc_object(void) {
    object *obj;

    obj = malloc(sizeof(object));
    if(obj == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }

    return obj;
}

object* make_fixnum(long value) {
    object *obj;

    obj = alloc_object();
    obj->type = FIXNUM;
    obj->data.fixnum.value = value;
    return obj;
}

char is_fixnum(object *obj) {
    return obj->type == FIXNUM;
}

/******************** READ ********************/

char is_delimiter(int c) {
    return isspace(c) || c == EOF ||
        c == '(' || c == ')' ||
        c == '"' || c == ';';
}

int peek(FILE *in) {
    int c;

    c = getc(in);
    ungetc(c, in);
    return c;
}

void eat_whitespace(FILE *in) {
    int c;

    while ((c = getc(in)) != EOF) {
        if (isspace(c)) {
            continue;
        }
        else if (c == ';') {
            while ((c = getc(in)) != EOF && (c != '\n'));
            continue;
        }
        ungetc(c, in);
        break;
    }
}

object* read(FILE *in) {
    int c;
    short sign = 1;
    long num = 0;

    eat_whitespace(in);

    c = getc(in);

    if(isdigit(c) || (c == '-' && (isdigit(peek(in))))) {
        /* read a fixnum */
        if (c == '-') {
            sign = -1;
        }
        else {
            ungetc(c, in);
        }

        while (isdigit(c = getc(in))) {
            num = (num * 10) + (c - '0');
        }

        num *= sign;

        if (is_delimiter(c)) {
            ungetc(c, in);
            return make_fixnum(num);
        }
        else {
            fprintf(stderr, "number not followed by delimiter\n");
            exit(1);
        }
    }
    else {
        fprintf(stderr, "bad input. unexpected '%c'\n", c);
        exit(1);
    }

    fprintf(stderr, "read illegal state\n");
    exit(1);
}

/******************** EVAL ********************/

object* eval(object *exp) {
    return exp;
}

/******************** PRINT ********************/

void print(object *obj) {
    switch (obj->type) {
        case FIXNUM:
            printf("%ld", obj->data.fixnum.value);
            break;
        default:
            fprintf(stderr, "cannot write unknown type\n");
            exit(1);
    }
}

/******************** REPL ********************/

int main(void) {
    printf("Welcome to Walrus Lisp. Use ctrl-c to exit.\n");

    while (1) {
        printf("> ");
        print(eval(read(stdin)));
        printf("\n");
    }

    return 0;
}

