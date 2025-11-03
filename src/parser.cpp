#include "parser.h"
#include <iostream>

using namespace std;

// Constructor
Parser::Parser(vector<Token> tokens) : tokens(tokens), current(0), hadError(false) {}

// ==================== Utility Methods ====================

// Return current token without consuming it
Token Parser::peek() {
    return tokens[current];
}

// Return previous token
Token Parser::previous() {
    return tokens[current - 1];
}

// Consume and return current token
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

// Check if we've reached end of tokens
bool Parser::isAtEnd() {
    return peek().type == END_OF_FILE;
}

// Check if current token matches type
bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

// If current token matches, consume it and return true
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

// Check if current token matches any of the given types
bool Parser::match(vector<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

// ==================== Error Handling ====================

void Parser::error(string message) {
    Token token = peek();
    cerr << "Parse Error at line " << token.line << ", column " << token.column 
         << ": " << message << endl;
    cerr << "  Found: " << tokenTypeToString(token.type) 
         << " ('" << token.lexeme << "')" << endl;
    hadError = true;
}

// Synchronize after error - skip tokens until we find a statement boundary
void Parser::synchronize() {
    advance();
    
    while (!isAtEnd()) {
        // Stop at semicolon (end of statement)
        if (previous().type == SEMICOLON) return;
        
        // Stop at keywords that start new statements
        switch (peek().type) {
            case LINK:
            case TEXT:
            case DNUM:
            case CNUM:
            case FLAG:
            case IF:
            case UNTIL:
            case ITERATE:
            case NETWORK:
            case INIT:
            case FEED:
            case FORWARD:
            case YIELD:
                return;
            default:
                break;
        }
        advance();
    }
}

// ==================== Helper Methods ====================

bool Parser::isDataType(TokenType type) {
    return type == TEXT || type == DNUM || type == CNUM || type == FLAG;
}

bool Parser::isAddOp(TokenType type) {
    return type == PLUS || type == MINUS || type == BITWISE_OR || 
           type == BITWISE_AND || type == BITWISE_XOR;
}

bool Parser::isMulOp(TokenType type) {
    return type == MULTIPLY || type == DIVIDE || type == MODULO || 
           type == LEFT_SHIFT || type == RIGHT_SHIFT;
}

bool Parser::isRelOp(TokenType type) {
    return type == EQ || type == NEQ || type == LT || 
           type == GT || type == LTE || type == GTE;
}

bool Parser::isLogicalOp(TokenType type) {
    return type == AND || type == OR;
}

bool Parser::isUnaryOp(TokenType type) {
    return type == MINUS || type == NOT || type == BITWISE_NOT || 
           type == INCREMENT || type == DECREMENT;
}

// ==================== Grammar Rules ====================

// Program → StatementList EOF
void Parser::program() {
    cout << "Parsing program..." << endl;
    statementList();
    
    if (!isAtEnd()) {
        error("Expected end of file");
    }
    
    if (!hadError) {
        cout << "\n=== Parsing completed successfully! ===" << endl;
    } else {
        cout << "\n=== Parsing completed with errors ===" << endl;
    }
}

// StatementList → Statement StatementList | ε
void Parser::statementList() {
    // Keep parsing statements until we hit a stopping point
    while (!isAtEnd() && !check(RBRACE)) {
        statement();
    }
}

// Statement → Declaration | Assignment | IfStmt | UntilStmt | ...
void Parser::statement() {
    // Skip comments
    if (match(COMMENT)) {
        return;
    }
    
    // Link statement
    if (check(LINK)) {
        linkStmt();
        return;
    }
    
    // Declaration (starts with data type)
    if (isDataType(peek().type)) {
        declaration();
        return;
    }
    
    // Control structures
    if (check(IF)) {
        ifStmt();
        return;
    }
    
    if (check(UNTIL)) {
        untilStmt();
        return;
    }
    
    if (check(ITERATE)) {
        iterateStmt();
        return;
    }
    
    // Function definitions
    if (check(NETWORK)) {
        networkStmt();
        return;
    }
    
    if (check(INIT)) {
        initStmt();
        return;
    }
    
    // I/O statements
    if (check(FEED)) {
        feedStmt();
        return;
    }
    
    if (check(FORWARD)) {
        forwardStmt();
        return;
    }
    
    // Return statement
    if (check(YIELD)) {
        returnStmt();
        return;
    }
    
    // Assignment (starts with identifier)
    if (check(IDENTIFIER)) {
        assignment();
        return;
    }
    
    // Empty statement (just semicolon)
    if (match(SEMICOLON)) {
        return;
    }
    
    // If none match, it's an error
    if (!isAtEnd() && !check(RBRACE)) {
        error("Unexpected token in statement");
        synchronize();
    }
}

// LinkStmt → link StringLiteral ;
void Parser::linkStmt() {
    cout << "Parsing link statement..." << endl;
    
    if (!match(LINK)) {
        error("Expected 'link'");
        return;
    }
    
    if (!match(STRING_LITERAL)) {
        error("Expected string literal after 'link'");
        return;
    }
    
    if (!match(SEMICOLON)) {
        error("Expected ';' after link statement");
        synchronize();
    }
}

// Declaration → DataType Identifier = Expr ; | DataType Identifier ;
void Parser::declaration() {
    cout << "Parsing declaration..." << endl;
    
    // Consume data type
    if (!match({TEXT, DNUM, CNUM, FLAG})) {
        error("Expected data type");
        return;
    }
    TokenType dataType = previous().type;
    
    // Consume identifier
    if (!match(IDENTIFIER)) {
        error("Expected identifier in declaration");
        return;
    }
    
    // Optional initialization
    if (match(ASSIGN)) {
        expr();
    }
    
    if (!match(SEMICOLON)) {
        error("Expected ';' after declaration");
        synchronize();
    }
}

// Assignment → Identifier = Expr ;
void Parser::assignment() {
    cout << "Parsing assignment..." << endl;
    
    if (!match(IDENTIFIER)) {
        error("Expected identifier in assignment");
        return;
    }
    
    if (!match(ASSIGN)) {
        error("Expected '=' in assignment");
        return;
    }
    
    expr();
    
    if (!match(SEMICOLON)) {
        error("Expected ';' after assignment");
        synchronize();
    }
}

// IfStmt → if ( Condition ) { StatementList } [ else { StatementList } ]
void Parser::ifStmt() {
    cout << "Parsing if statement..." << endl;
    
    if (!match(IF)) {
        error("Expected 'if'");
        return;
    }
    
    if (!match(LPAREN)) {
        error("Expected '(' after 'if'");
        return;
    }
    
    condition();
    
    if (!match(RPAREN)) {
        error("Expected ')' after condition");
        return;
    }
    
    if (!match(LBRACE)) {
        error("Expected '{' after if condition");
        return;
    }
    
    statementList();
    
    if (!match(RBRACE)) {
        error("Expected '}' after if body");
        return;
    }
    
    // Optional else
    if (match(ELSE)) {
        if (!match(LBRACE)) {
            error("Expected '{' after 'else'");
            return;
        }
        
        statementList();
        
        if (!match(RBRACE)) {
            error("Expected '}' after else body");
        }
    }
}

// UntilStmt → until ( Condition ) { StatementList }
void Parser::untilStmt() {
    cout << "Parsing until loop..." << endl;
    
    if (!match(UNTIL)) {
        error("Expected 'until'");
        return;
    }
    
    if (!match(LPAREN)) {
        error("Expected '(' after 'until'");
        return;
    }
    
    condition();
    
    if (!match(RPAREN)) {
        error("Expected ')' after condition");
        return;
    }
    
    if (!match(LBRACE)) {
        error("Expected '{' after until condition");
        return;
    }
    
    statementList();
    
    if (!match(RBRACE)) {
        error("Expected '}' after until body");
    }
}

// IterateStmt → iterate ( Declaration ; Condition ; Assignment ) { StatementList }
void Parser::iterateStmt() {
    cout << "Parsing iterate loop..." << endl;
    
    if (!match(ITERATE)) {
        error("Expected 'iterate'");
        return;
    }
    
    if (!match(LPAREN)) {
        error("Expected '(' after 'iterate'");
        return;
    }
    
    // Initialization (declaration without semicolon check in declaration())
    if (isDataType(peek().type)) {
        match({TEXT, DNUM, CNUM, FLAG});
        if (!match(IDENTIFIER)) {
            error("Expected identifier in iterate initialization");
        }
        if (match(ASSIGN)) {
            expr();
        }
    }
    
    if (!match(SEMICOLON)) {
        error("Expected ';' after iterate initialization");
        return;
    }
    
    // Condition
    condition();
    
    if (!match(SEMICOLON)) {
        error("Expected ';' after iterate condition");
        return;
    }
    
    // Update (assignment without semicolon)
    if (match(IDENTIFIER)) {
        if (!match(ASSIGN)) {
            error("Expected '=' in iterate update");
        }
        expr();
    }
    
    if (!match(RPAREN)) {
        error("Expected ')' after iterate clauses");
        return;
    }
    
    if (!match(LBRACE)) {
        error("Expected '{' after iterate header");
        return;
    }
    
    statementList();
    
    if (!match(RBRACE)) {
        error("Expected '}' after iterate body");
    }
}

// NetworkStmt → network Identifier ( ParameterList ) { StatementList }
void Parser::networkStmt() {
    cout << "Parsing network function..." << endl;
    
    if (!match(NETWORK)) {
        error("Expected 'network'");
        return;
    }
    
    if (!match(IDENTIFIER)) {
        error("Expected function name after 'network'");
        return;
    }
    
    if (!match(LPAREN)) {
        error("Expected '(' after function name");
        return;
    }
    
    // Parameters (if any)
    if (!check(RPAREN)) {
        parameterList();
    }
    
    if (!match(RPAREN)) {
        error("Expected ')' after parameters");
        return;
    }
    
    if (!match(LBRACE)) {
        error("Expected '{' after function header");
        return;
    }
    
    statementList();
    
    if (!match(RBRACE)) {
        error("Expected '}' after function body");
    }
}

// InitStmt → init ( ) { StatementList }
void Parser::initStmt() {
    cout << "Parsing init function..." << endl;
    
    if (!match(INIT)) {
        error("Expected 'init'");
        return;
    }
    
    if (!match(LPAREN)) {
        error("Expected '(' after 'init'");
        return;
    }
    
    if (!match(RPAREN)) {
        error("Expected ')' after 'init'");
        return;
    }
    
    if (!match(LBRACE)) {
        error("Expected '{' after init header");
        return;
    }
    
    statementList();
    
    if (!match(RBRACE)) {
        error("Expected '}' after init body");
    }
}

// ParameterList → DataType Identifier [ , DataType Identifier ]*
void Parser::parameterList() {
    do {
        if (!match({TEXT, DNUM, CNUM, FLAG})) {
            error("Expected data type in parameter list");
            return;
        }
        
        if (!match(IDENTIFIER)) {
            error("Expected parameter name");
            return;
        }
    } while (match(COMMA));
}

// ArgumentList → Expr [ , Expr ]*
void Parser::argumentList() {
    do {
        expr();
    } while (match(COMMA));
}

// ReturnStmt → yield Expr ;
void Parser::returnStmt() {
    cout << "Parsing return statement..." << endl;
    
    if (!match(YIELD)) {
        error("Expected 'yield'");
        return;
    }
    
    expr();
    
    if (!match(SEMICOLON)) {
        error("Expected ';' after return statement");
        synchronize();
    }
}

// FeedStmt → feed Identifier ;
void Parser::feedStmt() {
    cout << "Parsing feed statement..." << endl;
    
    if (!match(FEED)) {
        error("Expected 'feed'");
        return;
    }
    
    if (!match(IDENTIFIER)) {
        error("Expected identifier after 'feed'");
        return;
    }
    
    if (!match(SEMICOLON)) {
        error("Expected ';' after feed statement");
        synchronize();
    }
}

// ForwardStmt → forward ( Expr ) ;
void Parser::forwardStmt() {
    cout << "Parsing forward statement..." << endl;
    
    if (!match(FORWARD)) {
        error("Expected 'forward'");
        return;
    }
    
    if (!match(LPAREN)) {
        error("Expected '(' after 'forward'");
        return;
    }
    
    expr();
    
    if (!match(RPAREN)) {
        error("Expected ')' after expression");
        return;
    }
    
    if (!match(SEMICOLON)) {
        error("Expected ';' after forward statement");
        synchronize();
    }
}

// Condition → Expr RelOp Expr | Expr LogicalOp Expr | ! Condition | ( Condition )
void Parser::condition() {
    // Handle NOT operator
    if (match(NOT)) {
        condition();
        return;
    }
    
    // Handle parenthesized condition
    if (match(LPAREN)) {
        condition();
        if (!match(RPAREN)) {
            error("Expected ')' after condition");
        }
        return;
    }
    
    // Standard: Expr RelOp Expr or Expr LogicalOp Expr
    expr();
    
    if (isRelOp(peek().type) || isLogicalOp(peek().type)) {
        advance();
        expr();
    }
}

// Expr → Term [ AddOp Term ]*
void Parser::expr() {
    term();
    
    while (isAddOp(peek().type)) {
        advance();
        term();
    }
}

// Term → Factor [ MulOp Factor ]*
void Parser::term() {
    factor();
    
    while (isMulOp(peek().type)) {
        advance();
        factor();
    }
}

// Factor → Number | Identifier | String | ( Expr ) | UnaryOp Factor | FunctionCall
void Parser::factor() {
    // Unary operators
    if (isUnaryOp(peek().type)) {
        advance();
        factor();
        return;
    }
    
    // Literals
    if (match({INTEGER_LITERAL, FLOAT_LITERAL, STRING_LITERAL, BOOLEAN_LITERAL})) {
        return;
    }
    
    // Identifier or function call
    if (match(IDENTIFIER)) {
        // Check for function call
        if (match(LPAREN)) {
            if (!check(RPAREN)) {
                argumentList();
            }
            if (!match(RPAREN)) {
                error("Expected ')' after arguments");
            }
        }
        return;
    }
    
    // Parenthesized expression
    if (match(LPAREN)) {
        expr();
        if (!match(RPAREN)) {
            error("Expected ')' after expression");
        }
        return;
    }
    
    error("Expected expression");
}

// ==================== Public Methods ====================

void Parser::parse() {
    program();
}

bool Parser::hasError() {
    return hadError;
}