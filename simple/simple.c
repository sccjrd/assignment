/*

REFERENCES USED FOR THE PROJECT:

Error handling:
https://peerdh.com/blogs/programming-insights/custom-error-handling-mechanisms-in-c
Copied code and adjusted with errors to handle provided in the last section of the assignment guide
Inspired also the line classification later with definitions and switch statement
Overall design:
https://craftinginterpreters.com/contents.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STRING_LENGTH 1024
#define MAX_LABEL_LENGTH 51
#define MAX_TOKENS_PER_LINE 5

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
    REGISTER,
    MEMORY_REFERENCE,
    LITERAL,
    LABEL_REFERENCE,

    ASSIGNMENT,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    EQUALS,
    NOT_EQUALS,
    LESS_THAN,
    GREATER_THAN,
    LESS_THANK_EQUAL,
    GREATER_THAN_EQUAL,

    GOTO,
    IF,
    HALT,
    INPUT,
    OUTPUT,
} TokenType;

/* ----------- structures ----------- */

typedef struct
{
    TokenType type;
    int value;
} Token;

typedef struct
{
    Token tokens[MAX_TOKENS_PER_LINE];
    int expression_count;
    int source_line;
} Instruction;

typedef struct
{
    char name[MAX_LABEL_LENGTH];
    int instruction_index;
} Label;

/* ----------- initial declarations ----------- */

Instruction *instructions = NULL;
int instruction_count = 0;
int instruction_capacity = 0;

Label *labels = NULL;
int label_count = 0;
int label_capacity = 0;

int registers[26] = {0};
int *memory = NULL;
int memory_size = 0;

/* ----------- error handler  ----------- */

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

/* ----------- memory handlers -----------*/

void init_instructions()
{
    instruction_capacity = 16;
    instructions = malloc(instruction_capacity * sizeof(Instruction));
    if (!instructions)
        handle_error(MEMORY_ALLOCATION_FAILED, 0);
    instruction_count = 0;
}

void ensure_instruction_capacity()
{
    if (instruction_count >= instruction_capacity)
    {
        instruction_capacity *= 2;
        instructions = realloc(instructions, instruction_capacity * sizeof(Instruction));
        if (!instructions)
            handle_error(MEMORY_ALLOCATION_FAILED, 0);
    }
}

void init_labels()
{
    label_capacity = 16;
    labels = malloc(label_capacity * sizeof(Label));
    if (!labels)
        handle_error(MEMORY_ALLOCATION_FAILED, 0);
    label_count = 0;
}

void ensure_label_capacity()
{
    if (label_count >= label_capacity)
    {
        label_capacity *= 2;
        labels = realloc(labels, label_capacity * sizeof(Label));
        if (!labels)
            handle_error(MEMORY_ALLOCATION_FAILED, 0);
    }
}

void ensure_memory(int address, int line_n)
{
    if (address < 0)
        handle_error(INVALID_MEMORY_ACCESS, line_n);

    if (address >= memory_size)
    {
        int new_size = (address + 1) * 2;
        int *new_memory = realloc(memory, new_size * sizeof(int));
        if (!new_memory)
            handle_error(MEMORY_ALLOCATION_FAILED, line_n);

        for (int i = memory_size; i < new_size; i++)
            new_memory[i] = 0;

        memory = new_memory;
        memory_size = new_size;
    }
}

void cleanup()
{
    free(instructions);
    free(labels);
    free(memory);
    instructions = NULL;
    labels = NULL;
    memory = NULL;
}

/* ----------- string helper functions ----------- */

void strip_newline(char *s)
{
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n')
        s[n - 1] = '\0';
}

void strip_comment(char *s)
{
    char *p = strchr(s, '#');
    if (p)
        *p = '\0';
}

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

LineType get_line_type(const char *s)
{
    if (s[0] == '\0')
        return LINE_EMPTY;
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == ':')
        return LINE_LABEL;
    return LINE_INSTRUCTION;
}

/* ----------- Label Management ----------- */

void is_valid_label(const char *name, int line_n)
{
    if (strchr(name, ' ') || strchr(name, '\t') || strlen(name) >= MAX_LABEL_LENGTH)
        handle_error(INVALID_SYNTAX, line_n);

    for (int i = 0; i < label_count; i++)
    {
        if (strcmp(labels[i].name, name) == 0)
            handle_error(INVALID_SYNTAX, line_n);
    }
}

void add_label(const char *name, int instruction_index, int line_n)
{

    ensure_label_capacity();

    strcpy(labels[label_count].name, name);
    labels[label_count].instruction_index = instruction_index;
    label_count++;
}

int find_label(const char *name)
{
    for (int i = 0; i < label_count; i++)
    {
        if (strcmp(labels[i].name, name) == 0)
            return labels[i].instruction_index;
    }
    return -1;
}

void handle_label(char *s, int line_n)
{
    size_t n = strlen(s);
    s[n - 1] = '\0';
    trim(s);

    is_valid_label(s, line_n);
    add_label(s, instruction_count, line_n);
}

/* ----------- Token Recognition ----------- */

int is_register(const char *s)
{
    return strlen(s) == 1 && s[0] >= 'a' && s[0] <= 'z';
}

int is_memory_ref(const char *s)
{
    return strlen(s) == 2 && s[0] == '@' && s[1] >= 'a' && s[1] <= 'z';
}

int is_literal(const char *s)
{
    if (*s == '-' || *s == '+')
        s++;
    if (*s == '\0')
        return 0;
    while (*s)
    {
        if (!isdigit(*s))
            return 0;
        s++;
    }
    return 1;
}

Token parse_token(const char *s, int line_n)
{
    Token tk = {0};
    size_t len = strlen(s);

    // operators tokens
    if (len == 1)
    {
        switch (s[0])
        {
        case '+':
            tk.type = ADDITION;
            return tk;
        case '-':
            tk.type = SUBTRACTION;
            return tk;
        case '*':
            tk.type = MULTIPLICATION;
            return tk;
        case '/':
            tk.type = DIVISION;
            return tk;
        case '<':
            tk.type = LESS_THAN;
            return tk;
        case '>':
            tk.type = GREATER_THAN;
            return tk;
        case '=':
            tk.type = ASSIGNMENT;
            return tk;
        }
    }

    if (len == 2)
    {
        if (s[0] == '=' && s[1] == '=')
        {
            tk.type = EQUALS;
            return tk;
        }
        if (s[0] == '!' && s[1] == '=')
        {
            tk.type = NOT_EQUALS;
            return tk;
        }
        if (s[0] == '<' && s[1] == '=')
        {
            tk.type = LESS_THANK_EQUAL;
            return tk;
        }
        if (s[0] == '>' && s[1] == '=')
        {
            tk.type = GREATER_THAN_EQUAL;
            return tk;
        }
    }

    // flow tokens
    if (strcmp(s, "goto") == 0)
    {
        tk.type = GOTO;
        return tk;
    }
    if (strcmp(s, "if") == 0)
    {
        tk.type = IF;
        return tk;
    }
    if (strcmp(s, "halt") == 0)
    {
        tk.type = HALT;
        return tk;
    }
    if (strcmp(s, "input") == 0)
    {
        tk.type = INPUT;
        return tk;
    }
    if (strcmp(s, "output") == 0)
    {
        tk.type = OUTPUT;
        return tk;
    }

    // Register (a-z)
    if (is_register(s))
    {
        tk.type = REGISTER;
        tk.value = s[0] - 'a'; // 0-25
        return tk;
    }

    // Memory reference (@a-@z)
    if (is_memory_ref(s))
    {
        tk.type = MEMORY_REFERENCE;
        tk.value = s[1] - 'a'; // Register index for address
        return tk;
    }

    // Literal integer
    if (is_literal(s))
    {
        tk.type = LITERAL;
        tk.value = atoi(s);
        return tk;
    }

    // Must be a label reference (for goto/if)
    tk.type = LABEL_REFERENCE;
    tk.value = -1; // Will be resolved later
    // Store label name temporarily - we'll resolve it after parsing
    // For now, we need to handle this differently

    return tk;
}

/* ----------- Instruction Parsing ----------- */

// Temporary storage for unresolved label references
typedef struct
{
    int instruction_index;
    int token_index;
    char label_name[MAX_LABEL_LENGTH];
} UnresolvedLabel;

UnresolvedLabel *unresolved_labels = NULL;
int unresolved_count = 0;
int unresolved_capacity = 0;

void add_unresolved_label(int instr_idx, int idx, const char *name, int line_n)
{
    if (unresolved_count >= unresolved_capacity)
    {
        unresolved_capacity = unresolved_capacity == 0 ? 16 : unresolved_capacity * 2;
        unresolved_labels = realloc(unresolved_labels, unresolved_capacity * sizeof(UnresolvedLabel));
        if (!unresolved_labels)
            handle_error(MEMORY_ALLOCATION_FAILED, line_n);
    }

    if (strlen(name) >= MAX_LABEL_LENGTH)
        handle_error(INVALID_SYNTAX, line_n);

    unresolved_labels[unresolved_count].instruction_index = instr_idx;
    unresolved_labels[unresolved_count].token_index = idx;
    strcpy(unresolved_labels[unresolved_count].label_name, name);
    unresolved_count++;
}

void handle_instruction(char *s, int line_n)
{
    ensure_instruction_capacity();

    Instruction *instr = &instructions[instruction_count];
    instr->expression_count = 0;
    instr->source_line = line_n;

    // Tokenize the line
    char *token = strtok(s, " \t");
    while (token != NULL && instr->expression_count < MAX_TOKENS_PER_LINE)
    {
        // Check for too many tokens
        if (instr->expression_count >= MAX_TOKENS_PER_LINE)
            handle_error(INVALID_SYNTAX, line_n);

        Token tk = parse_token(token, line_n);

        // Check if this is a label reference that needs resolving
        if (tk.type == LABEL_REFERENCE)
        {
            add_unresolved_label(instruction_count, instr->expression_count, token, line_n);
        }

        instr->tokens[instr->expression_count++] = tk;
        token = strtok(NULL, " \t");
    }

    if (instr->expression_count == 0)
        handle_error(INVALID_SYNTAX, line_n);

    instruction_count++;
}

void resolve_labels(int line_n)
{
    for (int i = 0; i < unresolved_count; i++)
    {
        int instr_idx = unresolved_labels[i].instruction_index;
        int idx = unresolved_labels[i].token_index;
        const char *name = unresolved_labels[i].label_name;

        int target = find_label(name);
        if (target == -1)
            handle_error(INVALID_SYNTAX, line_n);

        instructions[instr_idx].tokens[idx].value = target;
    }

    free(unresolved_labels);
    unresolved_labels = NULL;
    unresolved_count = 0;
    unresolved_capacity = 0;
}

/* ----------- validation ----------- */

void validate_program()
{
    // Check that no label points past the last instruction
    for (int i = 0; i < label_count; i++)
    {
        if (labels[i].instruction_index >= instruction_count)
            handle_error(INVALID_SYNTAX, 0);
    }

    // label followed by label without instruction)
    for (int i = 0; i < label_count - 1; i++)
    {
        if (labels[i + 1].instruction_index == labels[i].instruction_index)
            handle_error(INVALID_SYNTAX, 0);
    }
}

/* ----------- Code Reading ----------- */

void read_code(FILE *file)
{
    char buffer[MAX_STRING_LENGTH];
    int line_n = 0;

    init_instructions();
    init_labels();

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
            handle_label(buffer, line_n);
            break;

        case LINE_INSTRUCTION:
            handle_instruction(buffer, line_n);
            break;
        }
    }

    resolve_labels(line_n);
    validate_program();
}

/* ----------- access values ----------- */

int get_value(Token *tk, int line_n)
{
    switch (tk->type)
    {
    case REGISTER:
        return registers[tk->value];
    case MEMORY_REFERENCE:
    {
        int addr = registers[tk->value];
        if (addr < 0)
            handle_error(INVALID_MEMORY_ACCESS, line_n);
        if (addr >= memory_size)
            return 0;
        return memory[addr];
    }
    case LITERAL:
        return tk->value;
    default:
        handle_error(INVALID_SYNTAX, line_n);
        return 0;
    }
}

void set_value(Token *tk, int value, int line_n)
{
    switch (tk->type)
    {
    case REGISTER:
        registers[tk->value] = value;
        break;
    case MEMORY_REFERENCE:
    {
        int addr = registers[tk->value];
        ensure_memory(addr, line_n);
        memory[addr] = value;
    }
    break;
    default:
        handle_error(INVALID_SYNTAX, line_n);
    }
}

/* ----------- execute code ----------- */

void execute_code()
{
    int pc = 0;

    while (pc < instruction_count)
    {
        Instruction *instr = &instructions[pc];
        Token *tks = instr->tokens;
        int line_n = instr->source_line;

        switch (tks[0].type)
        {
        case HALT:
            cleanup();
            exit(0);

        case GOTO: // goto label
            pc = tks[1].value;
            continue;

        case IF: // if cond label
            if (get_value(&tks[1], line_n))
            {
                pc = tks[2].value;
                continue;
            }
            break;

        case INPUT: // input ref_v ref_e
        {
            int value;
            int eof = (scanf("%d", &value) != 1);
            if (!eof)
                set_value(&tks[1], value, line_n);
            set_value(&tks[2], eof, line_n);
        }
        break;

        case OUTPUT: // output value
            if (printf("%d\n", get_value(&tks[1], line_n)) < 0)
                handle_error(INVALID_OUTPUT, line_n);
            break;

        case REGISTER:
        case MEMORY_REFERENCE:
            if (instr->expression_count == 3)
            {
                // dest = src
                set_value(&tks[0], get_value(&tks[2], line_n), line_n);
            }
            else if (instr->expression_count == 5)
            {
                // dest = a op b
                int a = get_value(&tks[2], line_n);
                int b = get_value(&tks[4], line_n);
                int result = 0;

                switch (tks[3].type)
                {
                case ADDITION:
                    result = a + b;
                    break;
                case SUBTRACTION:
                    result = a - b;
                    break;
                case MULTIPLICATION:
                    result = a * b;
                    break;
                case DIVISION:
                    if (b == 0)
                        handle_error(DIVISION_BY_ZERO, line_n);
                    result = a / b;
                    break;
                case EQUALS:
                    result = (a == b);
                    break;
                case NOT_EQUALS:
                    result = (a != b);
                    break;
                case LESS_THAN:
                    result = (a < b);
                    break;
                case GREATER_THAN:
                    result = (a > b);
                    break;
                case LESS_THANK_EQUAL:
                    result = (a <= b);
                    break;
                case GREATER_THAN_EQUAL:
                    result = (a >= b);
                    break;
                default:
                    handle_error(INVALID_SYNTAX, line_n);
                }

                set_value(&tks[0], result, line_n);
            }
            else
            {
                handle_error(INVALID_SYNTAX, line_n);
            }
            break;

        default:
            handle_error(INVALID_SYNTAX, line_n);
        }

        pc++;
    }
}

/* ----------- main ----------- */

int main(int argc, char *argv[])
{
    if (argc != 2)
        handle_error(INVALID_INPUT, 0);

    FILE *file = fopen(argv[1], "r");
    if (!file)
        handle_error(INVALID_INPUT, 0);

    read_code(file);
    fclose(file);

    execute_code();

    cleanup();
    return 0;
}