#ifdef LEXER_TYPES
#ifndef LEXER_TYPES_INCLUDE
#define LEXER_TYPES_INCLUDE
    #include <stdlib.h>
    typedef enum {
        TOK_NULL,
        TOK_INTEGER,
        TOK_LONG,
        TOK_SHORT,
        TOK_CHAR,
        TOK_UINTEGER,
        TOK_ULONG,
        TOK_USHORT,
        TOK_UCHAR,
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
        TOK_ASSIGN,
        TOK_ADD_ASSIGN,
        TOK_SUB_ASSIGN,
        TOK_MUL_ASSIGN,
        TOK_DIV_ASSIGN,
        TOK_MOD_ASSIGN,
        TOK_AND_ASSIGN,
        TOK_ORR_ASSIGN,
        TOK_XOR_ASSIGN,
        TOK_LSH_ASSIGN,
        TOK_RSH_ASSIGN,
        TOK_INCREMENT,
        TOK_DECREMENT,
        TOK_QUESTION_MARK,
        TOK_COLON,
        TOK_LABEL,
        TOK_COMMA,
        /* KEYWORD START */
        KEYW_int,
        KEYW_return,
        KEYW_if,
        KEYW_else,
        KEYW_while,
        KEYW_for,
        KEYW_break,
        KEYW_continue,
        KEYW_do,
        KEYW_switch,
        KEYW_goto,
        KEYW_case,
        KEYW_default,
        KEYW_static,
        KEYW_extern,
        KEYW_long,
        KEYW_short,
        KEYW_char,
        KEYW_unsigned,
        KEYW_signed,
    } token_type_t;

    typedef struct {
        token_type_t type;
        size_t line;
        union {
            char* identifier;
            union {
                unsigned int uint;
                unsigned long ulong;
                unsigned short ushort;
                unsigned char uchar;
            } literals;
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
    #include <stdbool.h>

    typedef struct node_t node_t;

    typedef enum {
        NOD_NULL,
        NOD_INTEGER,
        NOD_LONG,
        NOD_SHORT,
        NOD_CHAR,
        NOD_UINTEGER,
        NOD_ULONG,
        NOD_USHORT,
        NOD_UCHAR,
        NOD_RETURN,
        NOD_BLOCK,
        NOD_FUNC,
        NOD_FUNC_CALL,
        NOD_LOGICAL_NOT,
        NOD_BITWISE_NOT,
        NOD_UNARY_SUB,
        NOD_IF_STATEMENT,
        NOD_WHILE_LOOP,
        NOD_DO_WHILE_LOOP,
        NOD_FOR_LOOP,
        NOD_SWITCH_STATEMENT,
        NOD_SWITCH_CASE,
        NOD_BREAK,
        NOD_CONTINUE,
        NOD_LABEL,
        NOD_GOTO,
        NOD_DEFAULT_CASE,

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
        NOD_TERNARY_EXPRESSION,
        NOD_STATIC_INIT,
        NOD_TYPE_CAST,

        /* other epxressions */
        NOD_VARIABLE_ACCESS,
        NOD_ASSIGN,
        NOD_POST_INCREMENT,
        NOD_PRE_INCREMENT,
        /* not implemented yet */
        NOD_POST_DECREMENT,
        /* not implemented yet */
        NOD_PRE_DECREMENT,
    } node_type_t;
    
    typedef enum {
        TYPE_NULL = 0,
        TYPE_VOID,
        TYPE_INT,
        TYPE_LONG,
        TYPE_SHORT,
        TYPE_CHAR,
        __UNSIGNED_TYPE_START__,
        TYPE_UNSIGNED_INT,
        TYPE_UNSIGNED_LONG,
        TYPE_UNSIGNED_SHORT,
        TYPE_UNSIGNED_CHAR,
        __UNSIGNED_TYPE_END__,
    } base_type_t;

    typedef enum {
        STORAGE_NULL = 0,
        STORAGE_STATIC,
        STORAGE_EXTERN,
    } storage_class_t;

    typedef struct {
        base_type_t base_type;
        storage_class_t  storage_class;
        bool is_signed;
    } data_type_t;

    typedef struct {
        node_t* value;
    } nod_unary_t;

    typedef struct {
        node_t* value_a;
        node_t* value_b;
    } nod_binary_t;

    typedef struct {
        union {
            unsigned int int_literal;
            unsigned long long_literal;
            unsigned short short_literal;
            unsigned char char_literal;
        };
    } nod_constant_t;

    typedef struct {
        node_t* value;
    } nod_return_t;

    /* returns NULL if value not found within array */
    void* array_search(void* array, void* target, bool(*comparison_func)(void* val_a, void* val_b));


    typedef enum {
        SYM_FUNCTION,
        SYM_VARIABLE,
    } symbol_type_t;

    typedef struct symbol_t symbol_t;    
    struct symbol_t {
        symbol_type_t symbol_type;
        data_type_t data_type;
        char* name;
        /*
        * optional - only used for runction declarations 
        * i dont wanna do more than that man gimme a break
        */
        symbol_t* args;
        /* optional - only used for static memory objects */
        bool init;
    };

    typedef struct scope_t scope_t;

    struct scope_t {
        symbol_t* top_scope;
        scope_t* above_scope;
        size_t scope_id;
    };

    typedef struct {
        size_t scope_id;
        size_t var_num;
    } var_info_t;
    
    typedef struct {
        /* symbol info is stored in a different structure */
        size_t scope_id;
        node_t* code;
    } nod_block_t;
    
    typedef struct {
        char* name;
        symbol_t* args;
        nod_block_t body;
        symbol_t** function_scope;
        data_type_t type;
    } nod_function_t;

    typedef struct {
        node_t* destination;
        node_t* source;
    } nod_assign_t;

    typedef struct {
        var_info_t var_info;
        /* data_type_t is stored in symbol map */
    } nod_var_access_t;

    typedef struct {
        node_t* condition;
        node_t* true_block;
        node_t* false_block;
    } nod_conditional_t;

    typedef struct {
        node_t* condition;
        node_t* code;
    } nod_while_t;

    typedef struct {
        node_t* init;
        node_t* condition;
        node_t* repeat;
        node_t* code;
        /* since for-loops start their own scope */
        size_t scope_id;
    } nod_for_loop_t;

    typedef struct {
        char* label;
    } nod_label_t;

    typedef struct {
        node_t* cases;
        node_t* value;
        nod_block_t code;
        bool default_case_defined;
    } nod_switch_case_t;

    typedef struct {
        size_t case_id;
    } nod_case_t;

    typedef struct {
        char* identifier;
        node_t* args;
        storage_class_t storage_class;
    } nod_function_call_t;

    typedef struct {
        symbol_t sym_info;
        node_t* value;
        bool global;
    } nod_static_init_t;

    typedef struct {
        node_t* src;
        data_type_t dst_type;
    } nod_type_cast_t;

    struct node_t {
        node_type_t type;
        union {
            nod_constant_t const_node;
            nod_return_t return_node;
            nod_block_t block_node;
            nod_function_t func_node;
            nod_unary_t unary_node;
            nod_binary_t binary_node;
            nod_assign_t assign_node;
            nod_var_access_t var_access_node;
            nod_conditional_t conditional_node;
            nod_while_t while_node;
            nod_for_loop_t for_node;
            nod_label_t label_node;
            nod_switch_case_t switch_node;
            nod_case_t case_node;
            nod_function_call_t func_call_node;
            nod_static_init_t static_init_node;
            nod_type_cast_t type_cast_node;
        };
        /* used for type-annotating the AST */
        data_type_t d_type;
    };

    void parse_node_dstr(void* node);
#endif
#endif

#ifdef IR_GEN_TYPES
#ifndef IR_GEN_TYPES_INCLUDE
#define IR_GEN_TYPES_INCLUDE

typedef enum {                  /* dst, op_1, op_2 */
    INST_NOP,                   /* NULL, NULL, NULL */
    INST_RET,                   /* NULL, val, NULL */
    /* unary instructions */    /* dst, val, NULL */
    INST_NEGATE,                /* dst, val_a, val_b */
    INST_LOGICAL_NOT,           /* dst, val_a, val_b */
    INST_BITWISE_NOT,           /* dst, val_a, val_b */
    INST_INCREMENT,             /* dst, NULL, NULL */
    /* binary instructions */   /* dst, val_a, val_b */
    INST_ADD,                   /* dst, val_a, val_b */
    INST_SUB,                   /* dst, val_a, val_b */
    INST_MUL,                   /* dst, val_a, val_b */
    INST_DIV,                   /* dst, val_a, val_b */
    INST_MOD,                   /* dst, val_a, val_b */
    INST_BITWISE_AND,           /* dst, val_a, val_b */
    INST_BITWISE_OR,            /* dst, val_a, val_b */
    INST_BITWISE_XOR,           /* dst, val_a, val_b */
    INST_SHIFT_LEFT,            /* dst, val_a, val_b */
    INST_SHIFT_RIGHT,           /* dst, val_a, val_b */
    /* conditional set */       /* byte, NULL, NULL*/
    INST_SET_E,                 /* byte, NULL, NULL*/
    INST_SET_NE,                /* byte, NULL, NULL*/
    INST_SET_G,                 /* byte, NULL, NULL*/
    INST_SET_L,                 /* byte, NULL, NULL*/
    INST_SET_GE,                /* byte, NULL, NULL*/
    INST_SET_LE,                /* byte, NULL, NULL*/
    /* control flow instructions */
    INST_JUMP,                  /* label, NULL, NULL */
    INST_JUMP_IF_Z,             /* label, val, NULL */
    INST_JUMP_IF_NZ,            /* label, val, NULL */
    INST_JUMP_IF_EQ,            /* label, val, val */
    INST_LABEL,                 /* label, NULL, NULL */
    INST_USER_LABEL,            /* identifier, NULL, NULL */
    INST_GOTO,                  /* identifier, NULL, NULL */
    INST_CALL,                  /* identifier, NULL, NULL */
    INST_SAVE_CALLER_REGS,      /* NULL, NULL, NULL */
    INST_LOAD_CALLER_REGS,      /* NULL, NULL, NULL */
    /* memory instructions */
    INST_STACK_ALLOC,           /* NULL, bytes, NULL */
    INST_STACK_DEALLOC,         /* NULL, bytes, NULL */
    INST_STACK_PUSH,            /* NULL, val, NULL */
    INST_COPY,                  /* dst, val, NULL */
    /* storage class specifier - related */
    INST_INIT_STATIC,           /* identifier, val, size */
    INST_INIT_STATIC_Z,         /* identifier, NULL, size */
    INST_INIT_STATIC_PUBLIC,    /* identifier, NULL, size */
    INST_INIT_STATIC_Z_PUBLIC,  /* identifier, NULL, size */
    INST_INIT_STATIC_LOCAL,     /* identifier, val, size */
    INST_INIT_STATIC_Z_LOCAL,   /* identifier, NULL, size */
    INST_PUBLIC_FUNC,           /* identifier, frame_size, NULL */
    INST_STATIC_FUNC,           /* identifier, frame_size, NULL */
    /* long-int related instructions */
    INST_SIGN_EXTEND_LQ,        /* dst, src, NULL*/
    INST_ZERO_EXTEND_LQ,        /* dst, src, NULL */
    /* unsigned comparison instructions */
    INST_SET_A,                 /* byte, NULL, NULL*/
    INST_SET_B,                 /* byte, NULL, NULL*/
    INST_SET_AE,                /* byte, NULL, NULL*/
    INST_SET_BE,                /* byte, NULL, NULL*/
    /* other unsigned instructions */
    INST_UDIV,                  /* dst, val_a, val_b */
    INST_UMOD,                  /* dst, val_a, val_b */
    /* short-int and char related instructions */
    INST_SIGN_EXTEND_BW,        /* dst, src, NULL */
    INST_SIGN_EXTEND_BL,        /* dst, src, NULL */
    INST_SIGN_EXTEND_BQ,        /* dst, src, NULL */
    INST_SIGN_EXTEND_WL,        /* dst, src, NULL */
    INST_SIGN_EXTEND_WQ,        /* dst, src, NULL */
    /* unsigned conversion instructions */
    INST_ZERO_EXTEND_BW,        /* dst, src, NULL */
    INST_ZERO_EXTEND_BL,        /* dst, src, NULL */
    INST_ZERO_EXTEND_BQ,        /* dst, src, NULL */
    INST_ZERO_EXTEND_WL,        /* dst, src, NULL */
    INST_ZERO_EXTEND_WQ,        /* dst, src, NULL */
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
    IMM_U8,
    IMM_U16,
    IMM_U32,
    IMM_U64,
    ___LVALUE_TYPE_START___,
    STK_OFFSET,
    STATIC_MEM,
    STATIC_MEM_LOCAL,
} operand_t;

#define is_immediate(operand_type) (((operand_type) > ___IMMEDIATE_TYPE_START___) && ((operand_type) < ___LVALUE_TYPE_START___))

typedef struct {
    operand_t type;
    union {
        signed long offset;
        unsigned long immediate;
        char* identifier;
        size_t label_id;
    };
} ir_operand_t;

typedef enum {
    INST_TYPE_LONGWORD = 4,
    INST_TYPE_QUADWORD = 8,
    INST_TYPE_WORDWORD = 2,
    INST_TYPE_BYTEWORD = 1,
} ir_inst_size_t;

typedef struct {
    ir_inst_t instruction;
    size_t size;
    ir_operand_t op_1;
    ir_operand_t op_2;
    ir_operand_t dst;
} ir_node_t;

void ir_node_dstr(void* _node);

#endif
#endif
