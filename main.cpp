#include <iostream>
#include <algorithm>
#include <cmath>
#include <regex>
#include <stack>
#include <queue>
#include <vector>

// We use enum to easily identify type of token 
enum TokenType {NUMBER, FUNCTION, OPERATOR, LEFTPAREN, RIGHTPAREN, UNARY};


// We use struct to keep track information of token type and its value
struct Token {
    std::string value;
    TokenType valueType;
    Token(std::string value, TokenType valueType){
        this->value = value;
        this->valueType = valueType;
    }
};


class Expression {
    private:
        /// @brief Check identity of operator and apply it accordingly
        /// @param a number a (left operand)
        /// @param b number b (right operand)
        /// @param op operand to be applied
        /// @return Result of operator applied
        long double applyOperator(long double a, long double b, std::string op){
            if(op == "^") return powl(a, b);
            else if(op == "*") return a * b;
            else if(op == "/") return a / b;
            else if(op == "-") return a - b;
            else if(op == "+") return a + b;
            else throw std::runtime_error("Unknown operator");
        }

        /// @brief Calculate the result from postfix (also known as Reverse Polish Notation or RPN) queue processed before
        /// @return Result of postfix expression
        long double calculate(){
            std::stack<long double> operands;
            if(output.empty()) throw std::runtime_error("Function calculate must be called after output queue processed");
            while(!output.empty()){
                Token cur = output.front();
                output.pop();
                if(cur.valueType == NUMBER){
                    operands.push(std::stold(cur.value));
                } else {
                    // Assume output queue other than number would be operators/unary/function so we need at least 1 operands
                    if(operands.empty()) throw std::runtime_error("Not enough operands");
                    long double res;
                    long double a = operands.top();
                    long double b;
                    operands.pop();
                    // After we process them, we push the result back to operands stack
                    switch(cur.valueType){
                        case UNARY:
                            res = -a;
                            operands.push(res);
                            break;
                        case OPERATOR:
                            if(operands.empty()) throw std::runtime_error("Not enough operands");
                            b = operands.top();
                            operands.pop();
                            res = this->applyOperator(b, a, cur.value);
                            operands.push(res);
                            break;
                        case FUNCTION:
                            if(cur.value == "log_"){
                                // 2 argument function so we seek the next output queue as second operand
                                if(output.empty()) throw std::runtime_error("Not enough operands");
                                // If the next next queue is not number then the expression is invalid
                                if(output.front().valueType != NUMBER) throw std::runtime_error("Invalid syntax");
                                b = std::stold(output.front().value);
                                output.pop();
                                res = log2(b)/log2(a);
                                operands.push(res);
                            } else {
                                // Registered 1 argument function processed here
                                if(cur.value == "cos") res = cosl(a);
                                else if(cur.value == "sin") res = sinl(a);
                                else if(cur.value == "tan") res = tanl(a);
                                else if(cur.value == "log") res = logl(a);
                                else if(cur.value == "sqrt") res = sqrtl(a);
                                else throw std::runtime_error("Unknown function");
                                operands.push(res);
                            }
                            break;
                        default:
                            throw std::runtime_error("Unknown token type to process");
                    }
                }
            }
            /*
            The result of RPN expression would be top of the stack
            If there are 2 or more values left on the stack after processing a postfix expression (also known as Reverse Polish Notation or RPN), 
            it means that the expression is invalid. This can happen if the expression contains too many operands or missing an operator. 
            In this case, function will keep return top of the stack even if there are 2 or more values left 
             */
            return operands.top();
        }
    protected:
        std::string rawInput;
        std::queue<Token> output;
        std::stack<Token> operators;

        /// @brief Determine precedence of operator / unary
        /// @param s Operator string
        /// @return Precedence of operator in integer
        int precedence(std::string s){
            if(s == "NEGATE") return INT32_MAX;
            if(s == "^") return 3;
            if(s == "/" || s == "*") return 2;
            if(s == "+" || s == "-") return 1;
            return -1;
        }

        // Using regex to check if it matches character in range 'a' to 'z' (char code 97 to 122) OR symbol '_' (special case for log_).
        bool isLetter(char c){ return std::regex_match(std::string(1, c) ,std::regex ("[a-z]|_", std::regex::icase)); }

        // Using regex to check if it matches one of the character '+' OR '*' OR '/' OR '^'
        bool isOperator(char c){ return std::regex_match(std::string(1, c) ,std::regex ("\\+|-|\\*|\\/|\\^")); }

        // Using regex to check if it matches any digit character (0-9)
        bool isDigit(char c){ return std::regex_match(std::string(1, c) ,std::regex ("\\d")); }

        /// @brief Convert string to token. NOTE that this function assume user give valid input so it will skip any form of validaton
        /// @param s String input to convert
        /// @return Vector of Token object
        std::vector<Token> tokenize(std::string s){
            std::vector<Token> tokens;
            std::string numberBuffer = "";
            std::string letterBuffer = "";
            // Keep track of previous token pushed (special case to handle unary)
            Token prev = Token("ANY", OPERATOR);
            for(char c: s){ 
                if(isDigit(c)) numberBuffer.push_back(c);
                else if(c == '.') numberBuffer.push_back(c);
                else {
                    // Assume we got symbol, then we got one whole number to push in result
                    if(numberBuffer.size()){
                        tokens.push_back(Token(numberBuffer, NUMBER));
                        prev = Token(numberBuffer, NUMBER);
                        numberBuffer = "";
                    } 
                    if(isLetter(c)) letterBuffer.push_back(c);
                    else if(isOperator(c)) {
                        // - recognized as unary if previous token is paren/op/func
                        if((prev.valueType == RIGHTPAREN || prev.valueType == OPERATOR || prev.valueType == FUNCTION) && c == '-') 
                            tokens.push_back(Token("NEGATE", UNARY));
                        else 
                            tokens.push_back(Token(std::string(1, c), OPERATOR));
                    }
                    else {
                        // Assume we got parenthesis/curly bracket then we got one whole letter to push in result
                        if(letterBuffer.size()){
                            // Assume the strict input constraint, then this letter should be a function
                            tokens.push_back(Token(letterBuffer, FUNCTION));
                            prev = Token(letterBuffer, FUNCTION);
                            letterBuffer = "";
                        }
                        // Assume {} and () is the same as it doesn't effect the calculation
                        if(c == '{' || c == '('){ 
                            tokens.push_back(Token("(", LEFTPAREN));
                            prev = Token("(", LEFTPAREN);
                        }
                        if(c == '}' || c == ')'){
                            tokens.push_back(Token(")", RIGHTPAREN));
                            prev = Token(")", RIGHTPAREN);
                        }
                    }
                }
            }
            // Push remaining number after we read all character
            if(numberBuffer.size()) tokens.push_back(Token(numberBuffer, NUMBER));
            // Using same strict input constraint, we shouldn't get any letterBuffer left. This can happen if you try to use variable
            if(letterBuffer.size()) throw std::runtime_error("Letter buffer exist");
            numberBuffer = "";
            letterBuffer = "";
            return tokens;
        }

    public:
        void input(){
            // Self explanatory
            std::getline(std::cin, this->rawInput);
        }
        
        void eval(){
            // We remove space from input because it doen't matter
            std::string cleanInput = this->rawInput;
            cleanInput.erase(std::remove(cleanInput.begin(), cleanInput.end(), ' '), cleanInput.end());

            this->output = std::queue<Token> ();
            this->operators = std::stack<Token> ();

            auto tokens = tokenize(cleanInput);
            for(Token token: tokens){
                // Shunting Yard algorithm
                switch(token.valueType){
                    case NUMBER:
                        this->output.push(token);
                        break;
                    case FUNCTION:
                        this->operators.push(token);
                        break;
                    case OPERATOR:
                        /*
                        While top operators is not left parentheses and it has higher or equal precedence than current token, Pop it to output queue
                        Then push current token to operators
                        FUNCTION here treated as highest precedence.
                         */
                        while(!operators.empty() && operators.top().valueType != LEFTPAREN && 
                        (precedence(operators.top().value) >= precedence(token.value) && operators.top().valueType != FUNCTION)){
                            output.push(operators.top());
                            operators.pop();
                        }
                        operators.push(token);
                        break;
                    case UNARY:
                        // Basically unary has highest precedence so we skip precedence check
                        operators.push(token);
                        break;
                    case LEFTPAREN:
                        operators.push(token);
                        break;
                    case RIGHTPAREN:
                        // Pop the operators stack to queue until we found left parentheses
                        while(!operators.empty() && operators.top().valueType != LEFTPAREN){
                            output.push(operators.top());
                            operators.pop();
                            // Stack empty before we found it then there are mismatched parentheses
                            if(operators.empty()) throw std::runtime_error("Mismatched parentheses");
                        }
                        // With while loop ended we assume top stack is left parentheses so we pop it
                        operators.pop();
                        // Parenteses may belong to function
                        if(!operators.empty() && operators.top().valueType == FUNCTION){
                            output.push(operators.top());
                            operators.pop();
                        }
                        // We could add implicit multiplication with logic same as above but it isn't said by input constraint so by default it's not allowed
                        break;
                    default:
                        throw std::runtime_error("Unhandled token type");
                }
            }
            // After we read token, pop remaining stack to output queue
            while(!operators.empty()){
                // If top of the stack is a parentheses then there are mismatched parentheses
                if(operators.top().valueType == LEFTPAREN) throw std::runtime_error("Mismatched parentheses");
                output.push(operators.top());
                operators.pop();
            }
            std::cout << this->calculate();
        }
};

int main () { // DO NOT CHANGE MAIN!!
    Expression exp1;
    exp1.input();
    exp1.eval();
    return 0;
}