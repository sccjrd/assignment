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

#define MAX_CHAR_LINE 1024

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
    MORE_THAN_EQUAL
} ExpressionType;

typedef enum
{
    GOTO,
    IF,
    HALT,
    INPUT,
    OUTPUT,

} FlowType;

handle_error(ErrorCode code, int line_n)
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

/* if a valid name, so with no spaces, extract label name*/
void extract_label_name(char *s, int line_n)
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

int is_valid_instruction(char *s, int line_n)
{
}

void handle_instruction(char *s, int line_n)
{
    char *token = strtok(s, " ");
    while (token)
    {
        puts(token);
        token = strtok(NULL, " ");
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

/* ----------- execute code ----------- */

/* ----------- main functions ----------- */
void read_code(FILE *file)
{
    char buffer[MAX_CHAR_LINE];
    int line_n = 0;

    while (fgets(buffer, MAX_CHAR_LINE, file))
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

            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        FILE *file;
        file = fopen(argv[1], "r");
        if (!file)
            return 1;
        read_code(file);
    }
    return 1;
}