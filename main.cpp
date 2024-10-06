/*
 *COMP 360: Programming Assignment #1
 * Date: 10-4-24
 *
 * Jaden Ocampo
 * Jahseim Merritt
 * Nandi Hawkins
 */


#include <iostream>
#include <fstream>
#include <string>
//#include <vector>
#include <cctype>
#include <stdexcept>

enum TokenType {
    ID, NUMBER, ASSIGN, PLUS, MINUS, MULTIPLY, DIVIDE, SEMICOLON,
    INT, FLOAT, LPAREN, RPAREN, LBRACE, RBRACE, COMMA,
    FOR, INCREMENT, LT, GT, LE, GE, EQ, NE,
    INVALID, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

class Lexer {
public:
    Lexer(const std::string &input) : input(input), pos(0), line(1) {}

    Token getNextToken() {
        while (pos < input.size() && isspace(input[pos])) {
            if (input[pos] == '\n') line++;
            pos++;
        }
        if (pos >= input.size()) return {END_OF_FILE, "", line};

        char currentChar = input[pos];

        if (isalpha(currentChar) || currentChar == '_') return identifier();
        if (isdigit(currentChar)) return number();
        if (currentChar == '=') {
            if (pos + 1 < input.size() && input[pos + 1] == '=') {
                pos += 2;
                return {EQ, "==", line};
            }
            pos++;
            return {ASSIGN, "=", line};
        }
        if (currentChar == '+') {
            if (pos + 1 < input.size() && input[pos + 1] == '+') {
                pos += 2;
                return {INCREMENT, "++", line};
            }
            pos++;
            return {PLUS, "+", line};
        }
        if (currentChar == '-') { pos++; return {MINUS, "-", line}; }
        if (currentChar == '*') { pos++; return {MULTIPLY, "*", line}; }
        if (currentChar == '/') { pos++; return {DIVIDE, "/", line}; }
        if (currentChar == ';') { pos++; return {SEMICOLON, ";", line}; }
        if (currentChar == ',') { pos++; return {COMMA, ",", line}; }
        if (currentChar == '(') { pos++; return {LPAREN, "(", line}; }
        if (currentChar == ')') { pos++; return {RPAREN, ")", line}; }
        if (currentChar == '{') { pos++; return {LBRACE, "{", line}; }
        if (currentChar == '}') { pos++; return {RBRACE, "}", line}; }
        if (currentChar == '<') {
            if (pos + 1 < input.size() && input[pos + 1] == '=') {
                pos += 2;
                return {LE, "<=", line};
            }
            pos++;
            return {LT, "<", line};
        }
        if (currentChar == '>') {
            if (pos + 1 < input.size() && input[pos + 1] == '=') {
                pos += 2;
                return {GE, ">=", line};
            }
            pos++;
            return {GT, ">", line};
        }

        pos++;
        return {INVALID, std::string(1, currentChar), line};
    }

private:
    std::string input;
    size_t pos;
    int line;

    Token identifier() {
        size_t start = pos;
        while (pos < input.size() && (isalnum(input[pos]) || input[pos] == '_')) pos++;
        std::string lexeme = input.substr(start, pos - start);
        if (lexeme == "int") return {INT, lexeme, line};
        if (lexeme == "float") return {FLOAT, lexeme, line};
        if (lexeme == "for") return {FOR, lexeme, line};
        return {ID, lexeme, line};
    }

    Token number() {
        size_t start = pos;
        while (pos < input.size() && isdigit(input[pos])) pos++;
        return {NUMBER, input.substr(start, pos - start), line};
    }
};

class Parser {
public:
    Parser(Lexer &lexer) : lexer(lexer), currentToken(lexer.getNextToken()) {}

    void parse() {
        while (currentToken.type != END_OF_FILE) {
            try {
                function();
                std::cout << "The program is generated by the grammar\n";
            } catch (const std::exception& e) {
                std::cerr<<"The program cannot be generated by the EBNF Described Language\n";
                std::cerr << "Error on line " << currentToken.line << ": " << e.what() << std::endl;
                synchronize();
            }
        }

    }

private:
    Lexer &lexer;
    Token currentToken;

    void function() {
        if (currentToken.type != INT && currentToken.type != FLOAT) {
            throw std::runtime_error("Expected return type (int or float). Found: " + currentToken.lexeme);
        }

        TokenType returnType = currentToken.type;
        consume(); // Consume return type

        if (currentToken.type != ID) {
            throw std::runtime_error("Expected function name. Found: " + currentToken.lexeme);
        }

        std::string functionName = currentToken.lexeme;
        consume(); // Consume function name

        expect(LPAREN);
        params();
        expect(RPAREN);
        expect(LBRACE);
        stmt_list();
        expect(RBRACE);

        //Testing comment, uncomment for debug
        /*std::cout << "Parsed function: " << functionName << " with return type: "
                  << (returnType == INT ? "int" : "float") << std::endl;
                  */
    }

    void params() {
        while (currentToken.type == INT || currentToken.type == FLOAT) {
            TokenType paramType = currentToken.type;
            consume(); // Consume parameter type

            if (currentToken.type != ID) {
                throw std::runtime_error("Expected parameter name. Found: " + currentToken.lexeme);
            }

            std::string paramName = currentToken.lexeme;
            consume(); // Consume parameter name
            //Testing comment, uncomment for debug
            /*
            std::cout << "Parameter: " << paramName << " of type: "
                      << (paramType == INT ? "int" : "float") << std::endl;
            */
            if (currentToken.type != COMMA) break;
            consume(); // Consume comma
        }
    }

    void stmt_list() {
        while (currentToken.type != RBRACE && currentToken.type != END_OF_FILE) {
            stmt();
        }
    }

    void stmt() {
        switch (currentToken.type) {
            case INT:
            case FLOAT:
                declaration();
                break;
            case ID:
                assignment();
                break;
            case FOR:
                for_loop();
                break;
            default:
                throw std::runtime_error("Unexpected token in statement: " + currentToken.lexeme);
        }
    }

    void declaration(bool expectSemicolon = true) {
        TokenType type = currentToken.type;
        consume(); // Consume type

        do {
            if (currentToken.type != ID) {
                throw std::runtime_error("Expected variable name after type. Found: " + currentToken.lexeme);
            }
            std::string varName = currentToken.lexeme;
            consume(); // Consume variable name
            //Testing comment, uncomment for debug
            /*
            std::cout << "Parsed declaration of " << varName << " with type "
                      << (type == INT ? "int" : "float") << std::endl;
               */
            if (currentToken.type == COMMA) {
                consume(); // Consume comma
            } else {
                break;
            }
        } while (true);

        if (expectSemicolon) {
            expect(SEMICOLON);
        }
    } //end declaration

    void assignment(bool expectSemicolon = true) {
        std::string varName = currentToken.lexeme;
        consume(); // Consume variable name
        expect(ASSIGN);
        expr();
        if (expectSemicolon) {
            expect(SEMICOLON);
        }
        //Testing comment, uncomment for debug
        //std::cout << "Parsed assignment to: " << varName << std::endl;
    }


    void for_loop() { // Takes all forms of for loop, with curly braces and without (one line body)
        expect(FOR);
        expect(LPAREN);

        // Initialization
        if (currentToken.type == INT || currentToken.type == FLOAT) {
            declaration(false); // Don't expect semicolon
        } else {
            assignment(false); // Don't expect semicolon
        }
        expect(SEMICOLON);

        // Condition
        expr();
        expect(SEMICOLON);

        // Increment
        if (currentToken.type == ID) {
            std::string varName = currentToken.lexeme;
            consume(); // Variable
            if (currentToken.type == INCREMENT) {
                consume(); // Increment
                //Testing comment, uncomment for debug
                //std::cout << "Parsed increment of " << varName << std::endl;
            } else if (currentToken.type == ASSIGN) {
                consume(); // Assign
                expr();
                //Testing comment, uncomment for debug
                //std::cout << "Parsed assignment in for loop increment" << std::endl;
            } else {
                throw std::runtime_error("Expected '++' or '=' after variable in for loop increment");
            }
        } else {
            expr(); // Allow any expression in the increment part
        }

        expect(RPAREN);

        // Loop body
        stmt();


        //Testing comment, uncomment for debug
        //std::cout << "Parsed for loop" << std::endl;
    }

    void expr() {
        term();
        while (currentToken.type == PLUS || currentToken.type == MINUS ||
               currentToken.type == LT || currentToken.type == GT ||
               currentToken.type == LE || currentToken.type == GE ||
               currentToken.type == EQ || currentToken.type == NE) {
            TokenType op = currentToken.type;
            consume(); // Consume operator
            term();
            //Testing comment, uncomment for debug
            //std::cout << "Parsed " << tokenTypeToString(op) << " operation" << std::endl;
        }
    }

    void term() {
        factor();
        while (currentToken.type == MULTIPLY || currentToken.type == DIVIDE) {
            TokenType op = currentToken.type;
            consume(); // Consume operator
            factor();
            //Testing comment, uncomment for debug
            //std::cout << "Parsed " << (op == MULTIPLY ? "multiplication" : "division") << " operation" << std::endl;
        }
    }

    void factor() {
        if (currentToken.type == ID || currentToken.type == NUMBER) {
            //Testing comment, uncomment for debug
            //std::cout << "Parsed " << (currentToken.type == ID ? "identifier" : "number") << ": " << currentToken.lexeme << std::endl;
            consume();
        } else if (currentToken.type == LPAREN) {
            consume(); // Consume '('
            expr();
            expect(RPAREN);
        } else {
            throw std::runtime_error("Expected identifier, number, or '('. Found: " + currentToken.lexeme);
        }
    }

    void expect(TokenType type) {
        if (currentToken.type != type) {
            throw std::runtime_error("Expected " + tokenTypeToString(type) + ". Found: " + currentToken.lexeme);
        }
        consume();
    }

    void consume() {
        currentToken = lexer.getNextToken();
    }

    void synchronize() {
        while (currentToken.type != END_OF_FILE) {
            if (currentToken.type == INT || currentToken.type == FLOAT) {
                return;
            }
            consume();
        }
    }

    std::string tokenTypeToString(TokenType type) { //Every possible token
        switch (type) {
            case ID: return "ID";
            case NUMBER: return "NUMBER";
            case ASSIGN: return "ASSIGN";
            case PLUS: return "PLUS";
            case MINUS: return "MINUS";
            case MULTIPLY: return "MULTIPLY";
            case DIVIDE: return "DIVIDE";
            case SEMICOLON: return "SEMICOLON";
            case INT: return "INT";
            case FLOAT: return "FLOAT";
            case LPAREN: return "LPAREN";
            case RPAREN: return "RPAREN";
            case LBRACE: return "LBRACE";
            case RBRACE: return "RBRACE";
            case COMMA: return "COMMA";
            case FOR: return "FOR";
            case INCREMENT: return "INCREMENT";
            case LT: return "LT"; //for loop
            case GT: return "GT"; //for loop
            case LE: return "LE"; //for loop
            case GE: return "GE"; //for loop
            case EQ: return "EQ"; //for loop
            case NE: return "NE"; //for loop and conditionals
            case INVALID: return "INVALID";
            case END_OF_FILE: return "END_OF_FILE";
            default: return "UNKNOWN";
        }
    }
};

int main() {
    std::ifstream inputFile("../TestProgram2.txt"); //May need to put in absolute filepath for grading
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file\n";
        return 1;
    }

    std::string inputProgram((std::istreambuf_iterator<char>(inputFile)),
                             std::istreambuf_iterator<char>());
    inputFile.close();

    Lexer lexer(inputProgram);
    Parser parser(lexer);
    parser.parse();

    return 0;
}