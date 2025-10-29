#include <iostream>
#include <fstream>
#include <sstream>
#include "scanner.h"
#include "token.h"

using namespace std;

// Function to read entire file into a string
string readFile(const string& filename) {
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Error: Could not open file '" << filename << "'" << endl;
        exit(1);
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

// Display usage information
void printUsage(const string& programName) {
    cout << "NetC Scanner - Lexical Analyzer for NetC Language\n";
    cout << "Usage: " << programName << " <input_file.netc>\n";
    cout << "Example: " << programName << " test.netc\n";
}

int main(int argc, char* argv[]) {
    // Initialize token type names for printing
    initializeTokenTypeNames();
    
    // Check command line arguments
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    string filename = argv[1];
    
    cout << "========================================\n";
    cout << "NetC Scanner - Lexical Analysis Phase\n";
    cout << "========================================\n";
    cout << "Input file: " << filename << "\n";
    cout << "========================================\n";
    
    // Read source file
    string sourceCode = readFile(filename);
    
    // Create scanner and tokenize
    Scanner scanner(sourceCode);
    vector<Token> tokens = scanner.scanTokens();
    
    // Display results
    cout << "\nScanning completed successfully!\n";
    cout << "Total tokens found: " << tokens.size() << "\n";
    
    scanner.printTokens();
    
    // Optional: Save tokens to output file
    string outputFilename = filename.substr(0, filename.find_last_of('.')) + "_tokens.txt";
    ofstream outFile(outputFilename);
    
    if (outFile.is_open()) {
        outFile << "Token Analysis for: " << filename << "\n\n";
        outFile << "Line\tCol\tType\t\t\tLexeme\n";
        outFile << "----\t---\t----\t\t\t------\n";
        
        for (const auto& token : tokens) {
            if (token.type != COMMENT) {
                outFile << token.line << "\t" << token.column << "\t" 
                       << tokenTypeToString(token.type) << "\t\t";
                if (tokenTypeToString(token.type).length() < 16) outFile << "\t";
                if (tokenTypeToString(token.type).length() < 8) outFile << "\t";
                outFile << token.lexeme << "\n";
            }
        }
        
        outFile.close();
        cout << "\nToken list saved to: " << outputFilename << "\n";
    }
    
    return 0;
}