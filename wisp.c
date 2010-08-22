#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_MAX 1000

typedef enum { FALSE, TRUE } bool;

/******************** MODEL ********************/

typedef enum { FIXNUM, BOOLEAN, STRING, CHARACTER, NIL, CONS } object_type;

typedef struct object {
    object_type type;
    union {
        struct {
            long value;
        } fixnum;

        struct {
            bool value;
        } boolean;

        struct {
            char value;
        } character;

        struct {
            char *value;
        } string;

        struct {
            struct object *car;
            struct object *cdr;
        } cons;
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
object *nil;

bool is_nil(object *obj) {
    return obj == nil;
}

bool is_boolean(object *obj) {
    return obj->type == BOOLEAN;
}

bool is_false(object *obj) {
    return obj == false;
}

bool is_true(object *obj) {
    return !is_false(obj);
}

object* make_string(char *value) {
    object *obj;

    obj = alloc_object();
    obj->type = STRING;
    obj->data.string.value = malloc(strlen(value) + 1); /* 1 extra for \0 */
    if (obj->data.string.value == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    strcpy(obj->data.string.value, value);
    return obj;
}

bool is_string(object *obj) {
    return obj->type == STRING;
}

object* make_character(char value) {
    object *obj;

    obj = alloc_object();
    obj->type = CHARACTER;
    obj->data.character.value = value;

    return obj;
}

bool is_character(object *obj) {
    return obj->type == CHARACTER;
}

object* make_cons(object *car, object *cdr) {
    object *obj;

    obj = alloc_object();
    obj->type = CONS;
    obj->data.cons.car = car;
    obj->data.cons.cdr = cdr;
    return obj;
}

bool is_cons(object *obj) {
    return obj->type == CONS;
}

object* car(object *obj) {
    return obj->data.cons.car;
}

void set_car(object *obj, object *value) {
    obj->data.cons.car = value;
}

object* cdr(object *obj) {
    return obj->data.cons.cdr;
}

void set_cdr(object *obj, object *value) {
    obj->data.cons.cdr = value;
}

void init(void) {
    false = alloc_object();
    false->type = BOOLEAN;
    false->data.boolean.value = FALSE;

    true = alloc_object();
    true->type = BOOLEAN;
    true->data.boolean.value = TRUE;

    nil = alloc_object();
    nil->type = NIL;
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

void peek_expected_delimiter(FILE *in) {
    if (!is_delimiter(peek(in))) {
        fprintf(stderr, "was expecting delimiter\n");
        exit(1);
    }
}

bool is_expected_string(FILE *in, char *str) {
    int c;

    while (*str != '\0') {
        c = getc(in);
        if (*str != c) {
            return FALSE;
        }
        str++;
    }

    return TRUE;
}

object* read_character(FILE *in) {
    int c;
    c = getc(in);    

    switch(c) {
        case EOF:
            fprintf(stderr, "incomplete character literal\n");
            exit(1);
        case '\\':
            c = getc(in);
            if (c == 's' && is_expected_string(in, "pace")) {
                        c = ' ';
                        return make_character(c);
                    } else if (c == 'n' && is_expected_string(in, "ewline")) {
                        c = '\n';
                        return make_character(c);
                    }
        default:
            return make_character(c);
    }
}

/* declaration required because `read` and `read_cons` are mutually recursive */
object* read(FILE *in);

object* read_cons(FILE *in) {
    int c;
    object *car, *cdr;

    eat_whitespace(in);

    c = getc(in);
    if (c == ')') {
        return nil;
    }
    ungetc(c, in);

    car = read(in);

    eat_whitespace(in);

    c = getc(in);
    if (c == '.') {
        /* read improper list */
        c = peek(in);
        if (!is_delimiter(c)) {
            fprintf(stderr, "was expecting delimiter\n");
            exit(1);
        }

        cdr = read(in);

        eat_whitespace(in);
        c = getc(in);
        if (c != ')') {
            fprintf(stderr, "couldn't find matching )\n");
            exit(1);
        }

        return make_cons(car, cdr);
    }
    else {
        /* read list */
        ungetc(c, in);
        cdr = read_cons(in);
        return make_cons(car, cdr);
    }
}

object* read(FILE *in) {
    int c;
    short sign = 1;
    long num = 0;
    int i;
    char buffer[BUFFER_MAX];

    eat_whitespace(in);

    c = getc(in);

    if (isdigit(c) || (c == '-' && (isdigit(peek(in))))) {
        /* read a fixnum */
        if (c == '-') {
            sign = -1;
        } else {
            ungetc(c, in);
        }

        while (isdigit(c = getc(in))) {
            num = (num * 10) + (c - '0');
        }

        num *= sign;

        if (is_delimiter(c)) {
            ungetc(c, in);
            return make_fixnum(num);
        } else {
            fprintf(stderr, "number not followed by delimiter\n");
            exit(1);
        }
    }

    else if (c == '#') {
        /* read a boolean */
        c = getc(in);

        switch (c) {
        case 't':
            return true;
        case 'f':
            return false;
        case '\\':
            ungetc(c,in);
            return read_character(in);
        default:
            fprintf(stderr, "unknown boolean literal\n");
            exit(1);
        }
    }
    
    else if (c == '"') {
        /* read a string */
        i = 0;
        while ((c = getc(in)) != '"') {
            if (c == '\\') {
                c = getc(in);
                if (c == 'n') {
                    c = '\n';
                }
            }

            if (c == EOF) {
                fprintf(stderr, "non-terminated string literal\n");
                exit(1);
            }

            if (i < BUFFER_MAX - 1) {
                /* - 1 becaue of the \0 */
                buffer[i++] = c;
            }

            else {
                fprintf(stderr,
                        "string too long; maximum length is %d\n", BUFFER_MAX);
                exit(1);
            }
        }
        buffer[i] = '\0';
        return make_string(buffer);
    }

    else if (c == '"') {
        /* read a string */
        i = 0;
        while ((c = getc(in)) != '"') {
            if (c == '\\') {
                c = getc(in);
                if (c == 'n') {
                    c = '\n';
                }
            }

            if (c == EOF) {
                fprintf(stderr, "non-terminated string literal\n");
                exit(1);
            }

            if (i < BUFFER_MAX - 1) {
                /* - 1 becaue of the \0 */
                buffer[i++] = c;
            }

            else {
                fprintf(stderr,
                        "string too long; maximum length is %d\n", BUFFER_MAX);
                exit(1);
            }
        }
        buffer[i] = '\0';
        return make_string(buffer);
    }

    else if (c == '(') {
        /* read cons/list */
        return read_cons(in);
    }

    else if (c == 'n' && is_expected_string(in, "il")) {
        peek_expected_delimiter(in);
        return nil;
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

/* declaration needed because `print` and `print_cons` are mutually recursive */
void print(object *obj);

void print_cons(object *obj) {
    object *car_obj;
    object *cdr_obj;

    car_obj = car(obj);
    cdr_obj = cdr(obj);

    print(car_obj);

    if (cdr_obj->type == CONS) {
        putchar(' ');
        print_cons(cdr_obj);
    }
    else if (cdr_obj->type == NIL) {
        return;
    }
    else {
        printf(" . ");
        print(cdr_obj);
    }
}

void print(object *obj) {
    char *str;

    switch (obj->type) {
            case FIXNUM:
            printf("%ld", obj->data.fixnum.value);
            break;
        case BOOLEAN:
            printf("#%c", is_true(obj) ? 't' : 'f');
            break;
        case CHARACTER:            
            printf("#\\%c", obj->data.character.value);
            break;
        case STRING:
            str = obj->data.string.value;
            putchar('"');
            while (*str != '\0') {
                switch (*str) {
                    case '\n':
                        printf("\\n");
                        break;
                    case '\\':
                        printf("\\\\");
                        break;
                    case '"':
                        printf("\\\"");
                        break;
                    default:
                        putchar(*str);
                }
                str++;
            }
            putchar('"');
            break;
        case CONS:
            putchar('(');
            print_cons(obj);
            putchar(')');
            break;
        case NIL:
            printf("nil");
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
