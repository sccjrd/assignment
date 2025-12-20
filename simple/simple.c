/*

REFERENCES USED FOR THE PROJECT:

Error handling:
https://peerdh.com/blogs/programming-insights/custom-error-handling-mechanisms-in-c
Copied code and adjusted with errors to handle provided in the last section of the assignment guide
Inspired also the line classification later with definitions and switch statement


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 1024

typedef enum
{
    INVALID_MEMORY_ACCESS,
    INVALID_SYNTAX,
    DIVISION_BY_ZERO,
    INVALID_INPUT,
    INVALID_OUTPUT,
    MEMORY_ALLOCATION_FAILED
} ErrorCode;

typedef enum
{
    LINE_EMPTY,
    LINE_LABEL,
    LINE_INSTRUCTION
} LineType;

typedef enum
{
    // operand
    REGISTER,
    MEMORY_REFERENCE,
    LITERAL,
    LABEL_REFERENCE,

    // expression
    ASSIGNMENT,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    INTEGER_DIVISION,
    EQUALS,
    NOT_EQUALS,
    LESS_THAN,
    GREATER_THAN,
    LESS_THAN_EQUAL,
    GREATER_THAN_EQUAL,

    // flow
    GOTO,
    IF,
    HALT,
    INPUT,
    OUTPUT,
} TokenType;

struct Token
{
    TokenType type;
    char *string;
    int line_n;
};

/* ----------- Error Handler -----------  */

void handle_error(ErrorCode code, int line_n)
{
    switch (code)
    {
    case INVALID_SYNTAX:
        fprintf(stderr, "Error: Invalid Syntax at line %d\n", line_n);
        break;
    case DIVISION_BY_ZERO:
        fprintf(stderr, "Error: Division by zero\n");
        break;
    case INVALID_MEMORY_ACCESS:
        fprintf(stderr, "Error: Invalid memory access\n");
        break;
    case INVALID_INPUT:
        fprintf(stderr, "Error: Input error\n");
        break;
    case INVALID_OUTPUT:
        fprintf(stderr, "Error: Output error\n");
        break;
    case MEMORY_ALLOCATION_FAILED:
        fprintf(stderr, "Error: Memory allocation failure\n");
        break;
    default:
        fprintf(stderr, "Error: Unknown error occurred.\n");
        break;
    }
    exit(EXIT_FAILURE);
}

/* ----------- handling list of tokens ----------- */

// will be a dynamic array of tokens
struct Token *tokens = NULL;
size_t token_count = 0;
size_t token_capacity = 0;

void init_token_list()
{
    token_capacity = 5;
    tokens = (struct Token *)malloc(token_capacity * sizeof(struct Token));
    if (!tokens)
        handle_error(MEMORY_ALLOCATION_FAILED, 0);
    token_count = 0;
}

// increase the size of the token list if needed
void ensure_token_capacity()
{
    if (token_count >= token_capacity)
    {
        token_capacity *= 2;
        tokens = (struct Token *)realloc(tokens, token_capacity * sizeof(struct Token));
        if (!tokens)
            handle_error(MEMORY_ALLOCATION_FAILED, 0);
    }
}

/* ----------- I/O operations ----------- */

/* read as instructed and handle error*/
int read_stdin()
{
    int value;
    if (scanf("%d", &value) != 1)
        handle_error(INVALID_INPUT, 0);
    return value;
}

/* write as instructed and handle error*/
void print_value(int v)
{
    if (printf("%d\n", v) < 0)
        handle_error(INVALID_OUTPUT, 0);
}

/* ----------- handle labels ----------- */

/* if a valid name, so with no spaces, extract label name */
char *extract_label_name(char *s, int line_n)
{
    size_t n = strlen(s);
    if (strchr(s, " "))
        handle_error(INVALID_SYNTAX, line_n);
    s[n - 1] = '\0';
    trim(s);
    return s;
}

void handle_label(char *s, int line_n)
{
    extract_label_name(s, line_n);
    // now we have a valid label name, we need to save it as categorized label,
    // maybe create a struct ? Dunno
    // the label also has a position in the to be executed code, saved so that
    // it can be access the position at constant position, so maybe a list of labels
    // in the code with the code position
}

/* ----------- handle instruction ----------- */

TokenType recognize_token(char *s, int line_n)
{
    size_t len = strlen(s);

    if (len == 1)
    {
        switch (s[0])
        {
        case '+':
            return ADDITION;
        case '-':
            return SUBTRACTION;
        case '*':
            return MULTIPLICATION;
        case '/':
            return INTEGER_DIVISION;
        case '<':
            return LESS_THAN;
        case '>':
            return GREATER_THAN;
        case '=':
            return ASSIGNMENT;
        }
    }

    if (len == 2)
    {
        switch (s[0])
        {
        case '=':
            if (s[1] == '=')
                return EQUALS;
            break;
        case '!':
            if (s[1] == '=')
                return NOT_EQUALS;
            break;
        case '<':
            if (s[1] == '=')
                return LESS_THAN_EQUAL;
            break;
        case '>':
            if (s[1] == '=')
                return GREATER_THAN_EQUAL;
            break;
        }
    }

    if (strcmp(s, "goto") == 0)
        return GOTO;
    if (strcmp(s, "if") == 0)
        return IF;
    if (strcmp(s, "halt") == 0)
        return HALT;
    if (strcmp(s, "input") == 0)
        return INPUT;
    if (strcmp(s, "output") == 0)
        return OUTPUT;

    handle_error(INVALID_SYNTAX, line_n);
    return -1;
}

// tokenize the instruction line and store the tokens
void handle_instruction(char *s, int line_n, struct Token *tokens)
{
    char *token_str = strtok(s, " ");
    while (token_str != NULL)
    {
        // recognize token type
        TokenType type = recognize_token(token_str, line_n);
        // store token
        token_str = strtok(NULL, " ");
    }
}

/* ----------- logic to clean inputted lines ----------- */

/* replace the last line char if is newline with eol*/
void strip_newline(char *s)
{
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n')
        s[n - 1] = '\0';
}

/* remove comments from a string*/
void strip_comment(char *s)
{
    char *p = strchr(s, '#');
    if (p)
        *p = '\0';
}

/* remove all the whitespaces before and after the string */
void trim(char *s)
{
    char *start = s;
    while (*start == ' ' || *start == '\t')
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t'))
    {
        s[n - 1] = '\0';
        n--;
    }
}

/* simple checking line type*/
LineType get_line_type(const char *s)
{
    if (s[0] == '\0')
        return LINE_EMPTY;
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == ':')
        return LINE_LABEL;
    return LINE_INSTRUCTION;
}

/* ----------- handle expressions ----------- */

void handle_assignment(char *address, char value)
{
}
void handle_addition() {}
void handle_subtraction() {}
void handle_multiplication() {}
void handle_integer_division() {}

int handle_equals(char *a, char *b)
{
    if (strcmp(a, b) == 0)
    {
        return 0;
    }
    return 1;
}
void handle_not_equals(char *a, char *b)
{
    if (strcmp(a, b) != 0)
    {
        return 0;
    }
    return 1;
}
void handle_less_than() {}
void handle_greater_than() {}
void handle_less_than_equal() {}
void handle_greater_than_equal() {}

/* ----------- handle flow ----------- */

void handle_goto(char *Label)
{
    // if label exists

    // jump to label
}
void handle_if(char *condition, char *label)
{
    // evaluate condition

    // if true, jump to label
}
void handle_halt()
{
    // free allocated memory before exiting
    exit(0);
}

/* ----------- execute code ----------- */

void execute_code() {}

/* ----------- main functions ----------- */
void read_code(FILE *file)
{
    char buffer[MAX_STRING_LENGTH];
    int line_n = 0;
    // call the init token list to prepare for token storage
    init_token_list();

    while (fgets(buffer, MAX_STRING_LENGTH, file))
    {

        line_n++;

        strip_newline(buffer);
        strip_comment(buffer);
        trim(buffer);

        LineType kind = get_line_type(buffer);

        switch (kind)
        {
        case LINE_EMPTY:
            continue;

        case LINE_LABEL:
            handle_labels(buffer, line_n);
            break;

        case LINE_INSTRUCTION:
            handle_instruction(buffer, line_n);
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        return EXIT_FAILURE;

    FILE *file = fopen(argv[1], "r");
        if (!file)
        handle_error(INVALID_INPUT, 0);

        read_code(file);
    fclose(file);

    execute_code();
    cleanup();
    return 0;
}
