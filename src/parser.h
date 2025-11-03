#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "token.h"

using namespace std;

// Parser class - performs syntax analysis on NetC token stream
class Parser {
private:
    vector<Token> tokens;       // List of tokens from scanner
    int current;                // Current token position
    bool hadError;              // Track if any errors occurred
    
    // Utility methods
    Token peek();               // Look at current token
    Token previous();           // Look at previous token
    Token advance();            // Consume and return current token
    bool isAtEnd();            // Check if at end of tokens
    bool check(TokenType type); // Check if current token is of type
    bool match(TokenType type); // Check and consume if matches
    bool match(vector<TokenType> types); // Check multiple types
    
    // Error handling
    void error(string message);
    void synchronize();         // Recover from errors
    
    // Grammar rules - one function per non-terminal
    void program();
    void statementList();
    void statement();
    void linkStmt();
    void declaration();
    void assignment();
    void ifStmt();
    void untilStmt();
    void iterateStmt();
    void networkStmt();
    void initStmt();
    void returnStmt();
    void feedStmt();
    void forwardStmt();
    void parameterList();
    void argumentList();
    void condition();
    void expr();
    void term();
    void factor();
    
    // Helper methods
    bool isDataType(TokenType type);
    bool isAddOp(TokenType type);
    bool isMulOp(TokenType type);
    bool isRelOp(TokenType type);
    bool isLogicalOp(TokenType type);
    bool isUnaryOp(TokenType type);
    
public:
    Parser(vector<Token> tokens);
    void parse();              // Main parsing method
    bool hasError();           // Check if parsing had errors
};

#endif // PARSER_H
