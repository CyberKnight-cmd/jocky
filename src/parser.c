#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

void jky_parser_init(JkyParser *parser, JkyLexer *lexer, JkyArena *arena) {
    parser->lexer = lexer;
    parser->arena = arena;
    parser->had_error = false;
    parser->current_token = jky_lexer_next_token(lexer);
}

JkyAstNode* ast_create_node(JkyArena *arena, JkyAstNodeType type, JkyToken token) {
    JkyAstNode *node = (JkyAstNode*)jky_arena_alloc(arena, sizeof(JkyAstNode));
    node->type = type;
    node->token = token;
    return node;
}

JkyAstNode* ast_create_binary_op(JkyArena *arena, JkyTokenType op_type, JkyAstNode *left, JkyAstNode *right) {
    JkyToken dummy_token;
    dummy_token.type = op_type;
    JkyAstNode *node = ast_create_node(arena, AST_BINARY_EXPR, dummy_token);
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

static void advance(JkyParser *p) {
    p->previous_token = p->current_token;
    p->current_token = jky_lexer_next_token(p->lexer);
}

static void report_error(JkyParser *p, const char *msg) {
    if (p->had_error) return; // Prevent cascading errors
    p->had_error = true;
    fprintf(stderr, "Error at line %d, col %d: %s\n", p->current_token.line, p->current_token.column, msg);
}

static void consume(JkyParser *p, JkyTokenType type, const char *msg) {
    if (p->current_token.type == type) {
        advance(p);
        return;
    }
    report_error(p, msg);
}

static JkyAstNode* parse_expression(JkyParser *p);

static JkyAstNode* parse_primary(JkyParser *p) {
    if (p->current_token.type == TOKEN_INT_LITERAL) {
        JkyAstNode *node = ast_create_node(p->arena, AST_LITERAL_INT, p->current_token);
        advance(p);
        return node;
    }
    if (p->current_token.type == TOKEN_STRING_LITERAL) {
        JkyAstNode *node = ast_create_node(p->arena, AST_LITERAL_STRING, p->current_token);
        advance(p);
        return node;
    }
    if (p->current_token.type == TOKEN_IDENTIFIER) {
        JkyAstNode *node = ast_create_node(p->arena, AST_IDENTIFIER, p->current_token);
        advance(p);
        return node;
    }
    report_error(p, "Expected expression.");
    return NULL;
}

static JkyAstNode* parse_call_or_member(JkyParser *p) {
    JkyAstNode *expr = parse_primary(p);

    while (true) {
        if (p->current_token.type == TOKEN_LPAREN) {
            advance(p); // consume '('
            JkyAstNode *call = ast_create_node(p->arena, AST_CALL_EXPR, p->previous_token);
            call->as.call_expr.callee = expr;
            call->as.call_expr.args = NULL;

            if (p->current_token.type != TOKEN_RPAREN) {
                JkyAstNode *arg_head = parse_expression(p);
                JkyAstNode *arg_tail = arg_head;
                // Since this is v0.1 we can reuse block.next for argument lists
                arg_head->as.block.next = NULL;
                
                while (p->current_token.type == TOKEN_COMMA) {
                    advance(p);
                    JkyAstNode *next_arg = parse_expression(p);
                    next_arg->as.block.next = NULL;
                    arg_tail->as.block.next = next_arg;
                    arg_tail = next_arg;
                }
                call->as.call_expr.args = arg_head;
            }
            consume(p, TOKEN_RPAREN, "Expected ')' after arguments.");
            expr = call;
        } else if (p->current_token.type == TOKEN_DOT) {
            // Member access (e.g., log.info) - treating as binary op for AST simplicity
            JkyToken op = p->current_token;
            advance(p);
            JkyAstNode *right = parse_primary(p); // the 'info' identifier
            expr = ast_create_binary_op(p->arena, op.type, expr, right);
        } else {
            break;
        }
    }

    return expr;
}

static JkyAstNode* parse_unary(JkyParser *p) {
    return parse_call_or_member(p);
}

static int get_token_precedence(JkyTokenType type) {
    switch (type) {
        case TOKEN_EQUAL: return 1;
        case TOKEN_EQUAL_EQUAL: return 2;
        case TOKEN_LESS:
        case TOKEN_GREATER: return 3;
        case TOKEN_PLUS:
        case TOKEN_MINUS: return 4;
        case TOKEN_STAR:
        case TOKEN_SLASH: return 5;
        default: return 0;
    }
}

static JkyAstNode* parse_expression_precedence(JkyParser *p, int min_precedence) {
    JkyAstNode *left = parse_unary(p);

    while (true) {
        if (p->current_token.type == TOKEN_BANG) {
            JkyToken op = p->current_token;
            advance(p);
            JkyAstNode *unary = ast_create_node(p->arena, AST_UNARY_EXPR, op);
            unary->as.unary.operand = left;
            left = unary;
            continue;
        }

        JkyToken op = p->current_token;
        int prec = get_token_precedence(op.type);
        if (prec < min_precedence || prec == 0) break;

        advance(p);
        JkyAstNode *right = parse_expression_precedence(p, prec + 1);
        left = ast_create_binary_op(p->arena, op.type, left, right);
    }
    return left;
}

static JkyAstNode* parse_expression(JkyParser *p) {
    return parse_expression_precedence(p, 1);
}

static JkyAstNode* parse_var_decl(JkyParser *p) {
    JkyToken type_name = p->previous_token;
    
    consume(p, TOKEN_IDENTIFIER, "Expected variable name.");
    JkyAstNode *node = ast_create_node(p->arena, AST_VAR_DECL, p->previous_token);
    node->as.var_decl.type_name = type_name;
    
    if (p->current_token.type == TOKEN_EQUAL) {
        advance(p);
        node->as.var_decl.initializer = parse_expression(p);
    } else {
        node->as.var_decl.initializer = NULL;
    }
    
    consume(p, TOKEN_SEMICOLON, "Expected ';' after variable declaration.");
    return node;
}

static JkyAstNode* parse_block(JkyParser *p);

static JkyAstNode* parse_statement(JkyParser *p) {
    if (p->current_token.type == TOKEN_KEYWORD_WHILE) {
        advance(p); // consume 'while'
        JkyAstNode *while_stmt = ast_create_node(p->arena, AST_WHILE_STMT, p->previous_token);
        
        // No parentheses required around condition in JOCKY
        while_stmt->as.while_stmt.condition = parse_expression(p);
        
        consume(p, TOKEN_LBRACE, "Expected '{' before while loop body.");
        while_stmt->as.while_stmt.body = parse_block(p);
        consume(p, TOKEN_RBRACE, "Expected '}' after while loop body.");
        
        return while_stmt;
    }

    if (p->current_token.type == TOKEN_IDENTIFIER) {
        if (p->current_token.length == 3 && p->current_token.start[0] == 'i' && p->current_token.start[1] == 'n' && p->current_token.start[2] == 't') {
            advance(p);
            return parse_var_decl(p);
        }
    }
    
    JkyAstNode *expr = parse_expression(p);
    consume(p, TOKEN_SEMICOLON, "Expected ';' after expression.");
    JkyAstNode *stmt = ast_create_node(p->arena, AST_EXPR_STMT, expr->token);
    stmt->as.expr_stmt.expr = expr;
    return stmt;
}

static JkyAstNode* parse_block(JkyParser *p) {
    JkyAstNode *head = NULL;
    JkyAstNode *tail = NULL;
    
    while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) {
        JkyAstNode *stmt = parse_statement(p);
        
        JkyAstNode *wrapper = ast_create_node(p->arena, AST_BLOCK, stmt->token);
        wrapper->as.block.head = stmt;
        wrapper->as.block.next = NULL;
        
        if (!head) {
            head = wrapper;
            tail = wrapper;
        } else {
            tail->as.block.next = wrapper;
            tail = wrapper;
        }
    }
    return head;
}

JkyAstNode* jky_parse(JkyParser *p) {
    consume(p, TOKEN_KEYWORD_FN, "Expected 'fn' keyword.");
    consume(p, TOKEN_IDENTIFIER, "Expected function name.");
    JkyAstNode *func = ast_create_node(p->arena, AST_FUNC_DECL, p->previous_token);
    
    consume(p, TOKEN_LPAREN, "Expected '(' after function name.");
    consume(p, TOKEN_RPAREN, "Expected ')' after parameters.");
    
    consume(p, TOKEN_ARROW, "Expected '->' before return type.");
    consume(p, TOKEN_IDENTIFIER, "Expected return type.");
    func->as.func_decl.return_type = p->previous_token;
    
    consume(p, TOKEN_LBRACE, "Expected '{' before function body.");
    func->as.func_decl.body = parse_block(p);
    consume(p, TOKEN_RBRACE, "Expected '}' after function body.");
    
    return func;
}
