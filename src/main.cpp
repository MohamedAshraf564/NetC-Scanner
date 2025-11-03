//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include "scanner.h"
//#include "token.h"
//
//using namespace std;
//
//// Function to read entire file into a string
//string readFile(const string& filename) {
//    ifstream file(filename);
//    
//    if (!file.is_open()) {
//        cerr << "Error: Could not open file '" << filename << "'" << endl;
//        exit(1);
//    }
//    
//    stringstream buffer;
//    buffer << file.rdbuf();
//    file.close();
//    
//    return buffer.str();
//}
//
//// Display usage information
//void printUsage(const string& programName) {
//    cout << "NetC Scanner - Lexical Analyzer for NetC Language\n";
//    cout << "Usage: " << programName << " <input_file.netc>\n";
//    cout << "Example: " << programName << " test.netc\n";
//}
//
//int main(int argc, char* argv[]) {
//    // Initialize token type names for printing
//    initializeTokenTypeNames();
//    
//    // Check command line arguments
//    if (argc != 2) {
//        printUsage(argv[0]);
//        return 1;
//    }
//    
//    string filename = argv[1];
//    
//    cout << "========================================\n";
//    cout << "NetC Scanner - Lexical Analysis Phase\n";
//    cout << "========================================\n";
//    cout << "Input file: " << filename << "\n";
//    cout << "========================================\n";
//    
//    // Read source file
//    string sourceCode = readFile(filename);
//    
//    // Create scanner and tokenize
//    Scanner scanner(sourceCode);
//    vector<Token> tokens = scanner.scanTokens();
//    
//    // Display results
//    cout << "\nScanning completed successfully!\n";
//    cout << "Total tokens found: " << tokens.size() << "\n";
//    
//    scanner.printTokens();
//    
//    // Optional: Save tokens to output file
//    string outputFilename = filename.substr(0, filename.find_last_of('.')) + "_tokens.txt";
//    ofstream outFile(outputFilename);
//    
//    if (outFile.is_open()) {
//        outFile << "Token Analysis for: " << filename << "\n\n";
//        outFile << "Line\tCol\tType\t\t\tLexeme\n";
//        outFile << "----\t---\t----\t\t\t------\n";
//        
//        for (const auto& token : tokens) {
//            if (token.type != COMMENT) {
//                outFile << token.line << "\t" << token.column << "\t" 
//                       << tokenTypeToString(token.type) << "\t\t";
//                if (tokenTypeToString(token.type).length() < 16) outFile << "\t";
//                if (tokenTypeToString(token.type).length() < 8) outFile << "\t";
//                outFile << token.lexeme << "\n";
//            }
//        }
//        
//        outFile.close();
//        cout << "\nToken list saved to: " << outputFilename << "\n";
//    }
//    
//    return 0;
//}

#include <iostream>
#include <fstream>
#include <sstream>
#include "scanner.h"
#include "parser.h"
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
    cout << "NetC Compiler - Scanner and Parser\n";
    cout << "Usage: " << programName << " <input_file.netc> [options]\n";
    cout << "Options:\n";
    cout << "  -s, --scan-only    Run scanner only (skip parsing)\n";
    cout << "  -p, --parse-only   Run parser only (skip token display)\n";
    cout << "Example: " << programName << " test.netc\n";
}

int main(int argc, char* argv[]) {
    // Initialize token type names for printing
    initializeTokenTypeNames();

    // Parse command line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    string filename = argv[1];
    bool scanOnly = false;
    bool parseOnly = false;

    // Check for options
    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-s" || arg == "--scan-only") {
            scanOnly = true;
        }
        else if (arg == "-p" || arg == "--parse-only") {
            parseOnly = true;
        }
    }

    cout << "============================================\n";
    cout << "NetC Compiler - Lexical and Syntax Analysis\n";
    cout << "============================================\n";
    cout << "Input file: " << filename << "\n";
    cout << "============================================\n\n";

    // ==================== SCANNER PHASE ====================
    cout << "PHASE 1: LEXICAL ANALYSIS (SCANNER)\n";
    cout << "--------------------------------------------\n";

    // Read source file
    string sourceCode = readFile(filename);

    // Create scanner and tokenize
    Scanner scanner(sourceCode);
    vector<Token> tokens = scanner.scanTokens();

    cout << "Scanning completed!\n";
    cout << "Total tokens found: " << tokens.size() << "\n";

    // Display tokens if not parse-only mode
    if (!parseOnly) {
        scanner.printTokens();

        // Save tokens to file
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
    }

    // If scan-only mode, exit here
    if (scanOnly) {
        cout << "\n============================================\n";
        cout << "Scan-only mode: Parsing skipped\n";
        cout << "============================================\n";
        return 0;
    }

    // ==================== PARSER PHASE ====================
    cout << "\n\n";
    cout << "PHASE 2: SYNTAX ANALYSIS (PARSER)\n";
    cout << "--------------------------------------------\n";

    // Create parser and parse
    Parser parser(tokens);
    parser.parse();

    // Check for errors
    if (parser.hasError()) {
        cout << "\n============================================\n";
        cout << "Parsing failed with errors!\n";
        cout << "============================================\n";
        return 1;
    }

    cout << "\n============================================\n";
    cout << "Compilation completed successfully!\n";
    cout << "============================================\n";

    return 0;
}