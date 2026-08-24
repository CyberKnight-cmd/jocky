#ifndef JOCKY_AST_H
#define JOCKY_AST_H

#include "lexer.h"
#include "arena.h"

typedef enum {
    AST_FUNC_DECL,
    AST_VAR_DECL,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_LITERAL_INT,
    AST_IDENTIFIER,
    AST_EXPR_STMT,
    AST_WHILE_STMT,
    AST_CALL_EXPR,
    AST_LITERAL_STRING,
    AST_BLOCK,
} JkyAstNodeType;

typedef struct JkyAstNode {
    JkyAstNodeType type;
    JkyToken token; 
    
    union {
        struct {
            struct JkyAstNode *left;
            struct JkyAstNode *right;
        } binary;
        struct {
            struct JkyAstNode *operand;
        } unary;
        struct {
            JkyToken type_name;
            struct JkyAstNode *initializer;
        } var_decl;
        struct {
            JkyToken return_type;
            struct JkyAstNode *body;
        } func_decl;
        struct {
            struct JkyAstNode *head;
            struct JkyAstNode *next; 
        } block;
        struct {
            struct JkyAstNode *expr;
        } expr_stmt;
        struct {
            struct JkyAstNode *condition;
            struct JkyAstNode *body;
        } while_stmt;
        struct {
            struct JkyAstNode *callee;
            struct JkyAstNode *args; // Linked list via block.next pattern for simplicity
        } call_expr;
    } as;
} JkyAstNode;

JkyAstNode* ast_create_node(JkyArena *arena, JkyAstNodeType type, JkyToken token);
JkyAstNode* ast_create_binary_op(JkyArena *arena, JkyTokenType op_type, JkyAstNode *left, JkyAstNode *right);

#endif // JOCKY_AST_H
