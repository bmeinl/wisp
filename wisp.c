#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { FALSE, TRUE } bool;

/******************** MODEL ********************/

typedef enum { FIXNUM, BOOLEAN } object_type;

typedef struct {
    object_type type;
    union {
        struct {
            long value;
        } fixnum;

        struct {
            bool value;
        } boolean;
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

bool is_fixnum(object *obj) {
    return obj->type == FIXNUM;
}

object *true;
object *false;

bool is_boolean(object *obj) {
    return obj->type == BOOLEAN;
}

bool is_false(object *obj) {
    return obj == false;
}

bool is_true(object *obj) {
    return !is_false(obj);
}

void init(void) {
    false = alloc_object();
    false->type = BOOLEAN;
    false->data.boolean.value = FALSE;

    true = alloc_object();
    true->type = BOOLEAN;
    true->data.boolean.value = TRUE;
}

/******************** READ ********************/

bool is_delimiter(int c) {
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

    else if (c == '#') {
        c = getc(in);
        switch (c) {
            case 't':
                return true;
            case 'f':
                return false;
            default:
                fprintf(stderr, "unknown boolean literal\n");
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
        case BOOLEAN:
            printf("#%c", is_true(obj) ? 't' : 'f');
            break;
        default:
            fprintf(stderr, "cannot print unknown type\n");
            exit(1);
    }
}

/******************** REPL ********************/

int main(void) {
    printf("Welcome to Walrus Lisp. Use ctrl-c to exit.\n");

    init();

    while (1) {
        printf("> ");
        print(eval(read(stdin)));
        printf("\n");
    }

    return 0;
}

