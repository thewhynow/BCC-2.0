#ifdef LEXER_TYPES
#ifndef LEXER_TYPES_INCLUDE
#define LEXER_TYPES_INCLUDE
    #include <stdlib.h>
    typedef enum {
        TOK_NULL,
        TOK_INTEGER,
        TOK_OPEN_BRACE,
        TOK_CLOSE_BRACE,
        TOK_OPEN_PAREN,
        TOK_CLOSE_PAREN,
        TOK_SEMICOLON,
        TOK_IDENTIFIER,
        TOK_ADD,
        TOK_SUB,
        TOK_MUL,
        TOK_DIV,
        TOK_MOD,
        TOK_BITWISE_NOT,
        TOK_LOGICAL_NOT,
        TOK_BITWISE_AND,
        TOK_BITWISE_OR,
        TOK_BITWISE_XOR,
        TOK_SHIFT_LEFT,
        TOK_SHIFT_RIGHT,
        TOK_LOGICAL_AND,
        TOK_LOGICAL_OR,
        TOK_EQUAL_TO,
        TOK_NOT_EQUAL_TO,
        TOK_LESS_THAN,
        TOK_GREATER_THAN,
        TOK_LESS_THAN_OR_EQUAL,
        TOK_GREATER_THAN_OR_EQUAL,
        /* KEYWORD START */
        KEYW_int,
        KEYW_return,
    } token_type_t;

    typedef struct {
        token_type_t type;
        size_t line;
        union {
            char* identifier;
            unsigned int uint_literal;
            /* all token types */
        };
    } token_t;

    #define is_identifier_char(c) (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z') || ((c) == '_')) 

    void token_dstr(void* token);
#endif
#endif

#ifdef PARSER_TYPES
#ifndef PARSER_TYPES_INCLUDE
#define PARSER_TYPES_INCLUDE
    typedef struct node_t node_t;

    typedef enum {
        NOD_NULL,
        NOD_INTEGER,
        NOD_RETURN,
        NOD_BLOCK,
        NOD_FUNC,
        NOD_LOGICAL_NOT,
        NOD_BITWISE_NOT,
        NOD_UNARY_SUB,

        /* binary operations */
        NOD_BINARY_ADD,
        NOD_BINARY_SUB,
        NOD_BINARY_MUL,
        NOD_BINARY_DIV,
        NOD_BINARY_MOD,
        NOD_BITWISE_AND,
        NOD_BITWISE_OR,
        NOD_BITWISE_XOR,
        NOD_SHIFT_LEFT,
        NOD_SHIFT_RIGHT,
        NOD_LOGICAL_AND,
        NOD_LOGICAL_OR,
        NOD_EQUAL_TO,
        NOD_NOT_EQUAL_TO,
        NOD_LESS_THAN,
        NOD_GREATER_THAN,
        NOD_LESS_THAN_OR_EQUAL,
        NOD_GREATER_THAN_OR_EQUAL,

    } node_type_t;

    typedef struct {
        node_t* value;
    } nod_unary_t;

    typedef struct {
        node_t* value_a;
        node_t* value_b;
    } nod_binary_t;

    typedef struct {
        unsigned int val;
    } nod_integer_t;

    typedef struct {
        node_t* value;
    } nod_return_t;
    
    typedef enum {
        SYM_VAR,
        SYM_FUNC,
    } symbol_types_t;

    typedef struct {
        symbol_types_t type;
        char* name;
    } symbol_t;
    
    typedef struct {
        symbol_t* locals;
        node_t* code;
    } nod_block_t;
    
    typedef struct {
        char* name;
        void* args;
        nod_block_t body;
    } nod_function_t;

    struct node_t {
        node_type_t type;
        union {
            nod_integer_t int_node;
            nod_return_t return_node;
            nod_block_t block_node;
            nod_function_t func_node;
            nod_unary_t unary_node;
            nod_binary_t binary_node;
        };
    };

    void parse_node_dstr(void* node);
#endif
#endif

#ifdef IR_GEN_TYPES
#ifndef IR_GEN_TYPES_INCLUDE
#define IR_GEN_TYPES_INCLUDE

typedef enum {                  /* dst, op_1, op_2 */
    INST_NOP,
    INST_RET,                   /* NULL, val, NULL */
    /* unary instructions */    /* dst, val, NULL */
    INST_NEGATE, 
    INST_LOGICAL_NOT,
    INST_BITWISE_NOT,
    /* binary instructions */   /* dst, val_a, val_b */
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_MOD,
    INST_BITWISE_AND,
    INST_BITWISE_OR,
    INST_BITWISE_XOR,
    INST_SHIFT_LEFT,
    INST_SHIFT_RIGHT,
    /* conditional set */       /* byte, NULL, NULL*/
    INST_SET_E,
    INST_SET_NE,
    INST_SET_G,
    INST_SET_L,
    INST_SET_GE,
    INST_SET_LE,
    /* miscellanous instructions */
    INST_COPY,                  /* dst, val, NULL */
    INST_JUMP,                  /* label, NULL, NULL */
    INST_JUMP_IF_Z,             /* label, val, NULL */
    INST_JUMP_IF_NZ,            /* label, val, NULL */
    INST_LABEL,                 /* label, NULL, NULL */
} ir_inst_t;

typedef enum {
    OP_NULL,
    REG_AX,
    REG_BX,
    REG_CX,
    REG_DX,
    REG_SI,
    REG_DI,
    REG_BP,
    REG_SP,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15,
    ___IMMEDIATE_TYPE_START___,
    IMM_U32,
    ___LVALUE_TYPE_START___,
    STK_OFFSET,
} operand_t;

#define is_immediate(operand_type) (((operand_type) > ___IMMEDIATE_TYPE_START___) && ((operand_type) < ___LVALUE_TYPE_START___))

typedef struct {
    operand_t type;
    union {
        signed long offset;
        unsigned int immediate;
        char* identifier;
        size_t label_id;
    };
} ir_operand_t;

typedef struct {
    ir_inst_t instruction;
    ir_operand_t op_1;
    ir_operand_t op_2;
    ir_operand_t dst;
} ir_node_t;

typedef struct {
    char* name;
    ir_node_t* code;
    size_t frame_size;
} ir_func_t;

void ir_node_dstr(void* _node);

#endif
#endif
