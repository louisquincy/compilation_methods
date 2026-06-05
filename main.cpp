#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <iomanip>
#include <variant>
#include <unordered_map>
#include <cstdlib>
using namespace std;

// 1. ЛЕКСИЧЕСКИЙ АНАЛИЗАТОР 

enum TokenCode {
    T_ERR = 0, T_ID = 1, T_NUM_INT = 2, T_NUM_REAL = 3,
    T_READ = 4, T_WRITE = 5, T_IF = 6, T_THEN = 7, T_ELSE = 8,
    T_WHILE = 9, T_DO = 10, T_BEGIN = 11, T_END = 12, T_ASSIGN = 13,
    T_PLUS = 14, T_MINUS = 15, T_MUL = 16, T_DIV = 17, T_LPAREN = 18,
    T_RPAREN = 19, T_LBRACKET = 20, T_RBRACKET = 21, T_SEMI = 22,
    T_LT = 23, T_GT = 24, T_EQ = 25, T_LE = 26, T_GE = 27, T_INT = 28, T_INT1 = 29, T_EOF = 30
};

map<string, TokenCode> keywords = {
    {"read", T_READ}, {"write", T_WRITE}, {"if", T_IF}, {"then", T_THEN},
    {"else", T_ELSE}, {"while", T_WHILE}, {"do", T_DO}, {"begin", T_BEGIN},
    {"end", T_END}, {"int", T_INT}, {"int1", T_INT1}
};

struct Token {
    TokenCode code;
    string value;
    int line, col;
};

// Классы символов
enum CharClass { C_LET = 0, C_DIG = 1, C_DOT = 2, C_COL = 3, C_EQ = 4, C_LT = 5, C_GT = 6, C_SPACE = 7, C_SEP = 8, C_ERR = 9 };

CharClass getCharClass(char c) {
    if (isalpha(c) || c == '_') return C_LET;
    if (isdigit(c)) return C_DIG;
    if (c == '.') return C_DOT;
    if (c == ':') return C_COL;
    if (c == '=') return C_EQ;
    if (c == '<') return C_LT;
    if (c == '>') return C_GT;
    if (isspace(c)) return C_SPACE;
    string seps = "+-*/()[];";
    if (seps.find(c) != string::npos) return C_SEP;
    return C_ERR;
}

// Таблица переходов DFA [State][CharClass] = {NextState, SemanticProg}
// Состояния: 8 - fin_with_consume, 9 - fin_without_consume, -1 - error
pair<int, int> dfa[8][10] = {
    // Let      Dig      Dot       Col      Eq       Lt       Gt       Space    Sep      Err
    {{1,1},   {2,4},   {-1,0},   {5,1},   {8,12},  {6,1},   {7,1},   {0,13},  {8,11},  {-1,0}}, // 0: Start (ИСПРАВЛЕНО: semProg 1 для Col, Lt, Gt)
    {{1,2},   {1,2},   {9,3},    {9,3},   {9,3},   {9,3},   {9,3},   {9,3},   {9,3},   {9,3}},  // 1: ID
    {{9,8},   {2,5},   {3,6},    {9,8},   {9,8},   {9,8},   {9,8},   {9,8},   {9,8},   {9,8}},  // 2: Int
    {{-1,0},  {4,7},   {-1,0},   {-1,0},  {-1,0},  {-1,0},  {-1,0},  {-1,0},  {-1,0},  {-1,0}}, // 3: Dot
    {{9,9},   {4,7},   {9,9},    {9,9},   {9,9},   {9,9},   {9,9},   {9,9},   {9,9},   {9,9}},  // 4: Real
    {{-1,0},  {-1,0},  {-1,0},   {-1,0},  {8,10},  {-1,0},  {-1,0},  {-1,0},  {-1,0},  {-1,0}}, // 5: Colon
    {{9,14},  {9,14},  {9,14},   {9,14},  {8,10},  {9,14},  {9,14},  {9,14},  {9,14},  {9,14}}, // 6: Lt
    {{9,14},  {9,14},  {9,14},   {9,14},  {8,10},  {9,14},  {9,14},  {9,14},  {9,14},  {9,14}}  // 7: Gt
};

class Lexer {
    string text;
    int pos = 0, line = 1, col = 1;
public:
    Lexer(string src) : text(src) {}

    Token getNextToken() {
        if (pos >= text.length()) return {T_EOF, "EOF", line, col};
        
        int state = 0;
        string buf = "";
        TokenCode tCode = T_ERR;
        int tLine = line, tCol = col;

        while (pos <= text.length()) {
            char c = (pos < text.length()) ? text[pos] : ' ';
            CharClass cls = (pos < text.length()) ? getCharClass(c) : C_SPACE;
            
            auto [nextState, semProg] = dfa[state][cls];

            if (nextState == -1) {
                cout << "Лексическая ошибка: строка " << line << ", позиция " << col << ", символ '" << c << "'\n";
                exit(1);
            }

            if (semProg == 1 || semProg == 2 || semProg == 4 || semProg == 5 || semProg == 7) {
                buf += c;
            }
            if (semProg == 3) { 
                tCode = keywords.count(buf) ? keywords[buf] : T_ID;
            } else if (semProg == 8) { tCode = T_NUM_INT; }
            else if (semProg == 9) { tCode = T_NUM_REAL; }
            else if (semProg == 10) { 
                buf += c; 
                if (buf == ":=") tCode = T_ASSIGN;
                else if (buf == "<=") tCode = T_LE;
                else if (buf == ">=") tCode = T_GE;
            }
            else if (semProg == 11) {
                buf += c;
                switch (c) {
                    case '+': tCode = T_PLUS; break; case '-': tCode = T_MINUS; break;
                    case '*': tCode = T_MUL; break; case '/': tCode = T_DIV; break;
                    case '(': tCode = T_LPAREN; break; case ')': tCode = T_RPAREN; break;
                    case '[': tCode = T_LBRACKET; break; case ']': tCode = T_RBRACKET; break;
                    case ';': tCode = T_SEMI; break;
                }
            }
            else if (semProg == 12) { buf += c; tCode = T_EQ; }
            else if (semProg == 14) { tCode = (state == 6) ? T_LT : T_GT; }

            if (nextState == 8) { pos++; col++; return {tCode, buf, tLine, tCol}; }
            if (nextState == 9) { return {tCode, buf, tLine, tCol}; }
            
            if (c == '\n') { line++; col = 1; } else { col++; }
            pos++;
            state = nextState;
        }
        return {T_EOF, "EOF", line, col};
    }
};

// 2. СИНТАКСИЧЕСКИЙ АНАЛИЗАТОР И ОПС

enum Symbol {
    S_ID = 1, S_NUM_INT, S_NUM_REAL, S_READ, S_WRITE, S_IF, S_THEN, S_ELSE,
    S_WHILE, S_DO, S_BEGIN, S_END, S_ASSIGN, S_PLUS, S_MINUS, S_MUL, S_DIV,
    S_LPAREN, S_RPAREN, S_LBRACKET, S_RBRACKET, S_SEMI, S_LT, S_GT, S_EQ, S_LE, S_GE, S_INT, S_INT1, S_EOF,
    N_PROG, N_DECL_LIST, N_STMT_LIST, N_STMT, N_A_PRIME, N_READ_PRIME, N_ELSE_PART,
    N_EXPR, N_REL_OPT, N_SIMPLE_EXPR, N_EXPR_PRIME, N_TERM, N_TERM_PRIME, N_FACTOR, N_ID_PRIME,
    A_PUSH_VAR, A_PUSH_CONST, A_ADD, A_SUB, A_MUL, A_DIV, A_NEG, A_IDX, A_ASSIGN, A_READ, A_WRITE,
    A_LT, A_GT, A_EQ, A_LE, A_GE, 
    A_PROG1, A_PROG2, A_PROG3, A_PROG4, A_PROG5, A_ALLOC_VAR, A_ALLOC_ARR
};

struct RPNItem {
    string type;
    string str_val;
    double num_val;
    int addr_val;
};

class Parser {
    Lexer& lexer;
    Token curr_token;
    map<Symbol, map<TokenCode, vector<Symbol>>> table;
    vector<RPNItem> rpn;
    stack<int> sem_stack;
    
    string last_id;
    string last_const;
    int last_arr_size;
    unordered_map<string, int> sym_table; 
    unordered_map<string, int> arr_sizes;
    int mem_offset = 0;

    Symbol tokenToSymbol(TokenCode c) { return (Symbol)c; }

    void initTable() {
        table[N_PROG][T_INT] = table[N_PROG][T_INT1] = table[N_PROG][T_ID] = table[N_PROG][T_IF] = table[N_PROG][T_WHILE] = table[N_PROG][T_READ] = table[N_PROG][T_WRITE] = table[N_PROG][T_BEGIN] = table[N_PROG][T_EOF] = 
            {N_DECL_LIST, N_STMT_LIST};
        
        table[N_DECL_LIST][T_INT] = {S_INT, S_ID, A_ALLOC_VAR, S_SEMI, N_DECL_LIST};
        table[N_DECL_LIST][T_INT1] = {S_INT1, S_ID, S_LBRACKET, S_NUM_INT, A_ALLOC_ARR, S_RBRACKET, S_SEMI, N_DECL_LIST};
        table[N_DECL_LIST][T_ID] = table[N_DECL_LIST][T_IF] = table[N_DECL_LIST][T_WHILE] = table[N_DECL_LIST][T_READ] = table[N_DECL_LIST][T_WRITE] = table[N_DECL_LIST][T_BEGIN] = table[N_DECL_LIST][T_EOF] = {};

        table[N_STMT_LIST][T_ID] = table[N_STMT_LIST][T_IF] = table[N_STMT_LIST][T_WHILE] = table[N_STMT_LIST][T_READ] = table[N_STMT_LIST][T_WRITE] = table[N_STMT_LIST][T_BEGIN] = 
            {N_STMT, N_STMT_LIST};
        table[N_STMT_LIST][T_END] = table[N_STMT_LIST][T_EOF] = {};

        table[N_STMT][T_ID] = {S_ID, A_PUSH_VAR, N_A_PRIME};
        table[N_STMT][T_IF] = {S_IF, N_EXPR, A_PROG1, S_THEN, N_STMT, A_PROG2, N_ELSE_PART, A_PROG3};
        table[N_STMT][T_WHILE] = {S_WHILE, A_PROG4, N_EXPR, A_PROG1, S_DO, N_STMT, A_PROG5};
        table[N_STMT][T_READ] = {S_READ, S_ID, A_PUSH_VAR, N_READ_PRIME};
        table[N_STMT][T_WRITE] = {S_WRITE, N_EXPR, S_SEMI, A_WRITE};
        table[N_STMT][T_BEGIN] = {S_BEGIN, N_STMT_LIST, S_END};

        table[N_A_PRIME][T_ASSIGN] = {S_ASSIGN, N_EXPR, S_SEMI, A_ASSIGN};
        table[N_A_PRIME][T_LBRACKET] = {S_LBRACKET, N_EXPR, S_RBRACKET, A_IDX, S_ASSIGN, N_EXPR, S_SEMI, A_ASSIGN};

        table[N_READ_PRIME][T_SEMI] = {S_SEMI, A_READ};
        table[N_READ_PRIME][T_LBRACKET] = {S_LBRACKET, N_EXPR, S_RBRACKET, A_IDX, S_SEMI, A_READ};

        table[N_ELSE_PART][T_ELSE] = {S_ELSE, N_STMT};
        table[N_ELSE_PART][T_ID] = table[N_ELSE_PART][T_IF] = table[N_ELSE_PART][T_WHILE] = table[N_ELSE_PART][T_READ] = table[N_ELSE_PART][T_WRITE] = table[N_ELSE_PART][T_BEGIN] = table[N_ELSE_PART][T_END] = table[N_ELSE_PART][T_EOF] = {};

        table[N_EXPR][T_ID] = table[N_EXPR][T_NUM_INT] = table[N_EXPR][T_NUM_REAL] = table[N_EXPR][T_LPAREN] = table[N_EXPR][T_MINUS] = {N_SIMPLE_EXPR, N_REL_OPT};

        table[N_REL_OPT][T_LT] = {S_LT, N_SIMPLE_EXPR, A_LT};
        table[N_REL_OPT][T_GT] = {S_GT, N_SIMPLE_EXPR, A_GT};
        table[N_REL_OPT][T_EQ] = {S_EQ, N_SIMPLE_EXPR, A_EQ};
        table[N_REL_OPT][T_LE] = {S_LE, N_SIMPLE_EXPR, A_LE};
        table[N_REL_OPT][T_GE] = {S_GE, N_SIMPLE_EXPR, A_GE};
        table[N_REL_OPT][T_SEMI] = table[N_REL_OPT][T_THEN] = table[N_REL_OPT][T_DO] = table[N_REL_OPT][T_RPAREN] = table[N_REL_OPT][T_RBRACKET] = {};

        table[N_SIMPLE_EXPR][T_ID] = table[N_SIMPLE_EXPR][T_NUM_INT] = table[N_SIMPLE_EXPR][T_NUM_REAL] = table[N_SIMPLE_EXPR][T_LPAREN] = table[N_SIMPLE_EXPR][T_MINUS] = {N_TERM, N_EXPR_PRIME};

        table[N_EXPR_PRIME][T_PLUS] = {S_PLUS, N_TERM, A_ADD, N_EXPR_PRIME};
        table[N_EXPR_PRIME][T_MINUS] = {S_MINUS, N_TERM, A_SUB, N_EXPR_PRIME};
        table[N_EXPR_PRIME][T_LT] = table[N_EXPR_PRIME][T_GT] = table[N_EXPR_PRIME][T_EQ] = table[N_EXPR_PRIME][T_LE] = table[N_EXPR_PRIME][T_GE] = table[N_EXPR_PRIME][T_SEMI] = table[N_EXPR_PRIME][T_THEN] = table[N_EXPR_PRIME][T_DO] = table[N_EXPR_PRIME][T_RPAREN] = table[N_EXPR_PRIME][T_RBRACKET] = {};

        table[N_TERM][T_ID] = table[N_TERM][T_NUM_INT] = table[N_TERM][T_NUM_REAL] = table[N_TERM][T_LPAREN] = table[N_TERM][T_MINUS] = {N_FACTOR, N_TERM_PRIME};

        table[N_TERM_PRIME][T_MUL] = {S_MUL, N_FACTOR, A_MUL, N_TERM_PRIME};
        table[N_TERM_PRIME][T_DIV] = {S_DIV, N_FACTOR, A_DIV, N_TERM_PRIME};
        table[N_TERM_PRIME][T_PLUS] = table[N_TERM_PRIME][T_MINUS] = table[N_TERM_PRIME][T_LT] = table[N_TERM_PRIME][T_GT] = table[N_TERM_PRIME][T_EQ] = table[N_TERM_PRIME][T_LE] = table[N_TERM_PRIME][T_GE] = table[N_TERM_PRIME][T_SEMI] = table[N_TERM_PRIME][T_THEN] = table[N_TERM_PRIME][T_DO] = table[N_TERM_PRIME][T_RPAREN] = table[N_TERM_PRIME][T_RBRACKET] = {};

        table[N_FACTOR][T_ID] = {S_ID, A_PUSH_VAR, N_ID_PRIME};
        table[N_FACTOR][T_NUM_INT] = {S_NUM_INT, A_PUSH_CONST};
        table[N_FACTOR][T_NUM_REAL] = {S_NUM_REAL, A_PUSH_CONST};
        table[N_FACTOR][T_LPAREN] = {S_LPAREN, N_EXPR, S_RPAREN};
        table[N_FACTOR][T_MINUS] = {S_MINUS, N_FACTOR, A_NEG};

        table[N_ID_PRIME][T_LBRACKET] = {S_LBRACKET, N_EXPR, S_RBRACKET, A_IDX};
        table[N_ID_PRIME][T_MUL] = table[N_ID_PRIME][T_DIV] = table[N_ID_PRIME][T_PLUS] = table[N_ID_PRIME][T_MINUS] = table[N_ID_PRIME][T_LT] = table[N_ID_PRIME][T_GT] = table[N_ID_PRIME][T_EQ] = table[N_ID_PRIME][T_LE] = table[N_ID_PRIME][T_GE] = table[N_ID_PRIME][T_SEMI] = table[N_ID_PRIME][T_THEN] = table[N_ID_PRIME][T_DO] = table[N_ID_PRIME][T_RPAREN] = table[N_ID_PRIME][T_RBRACKET] = {};
    }

    void addCMD(string cmd) { rpn.push_back({"CMD", cmd, 0, 0}); }
    
public:
    Parser(Lexer& l) : lexer(l) { initTable(); }

    void parse() {
        curr_token = lexer.getNextToken();
        stack<Symbol> st;
        st.push(S_EOF);
        st.push(N_PROG);

        while (!st.empty()) {
            Symbol top = st.top();
            st.pop();

            if (top >= A_PUSH_VAR && top <= A_ALLOC_ARR) {
                if (top == A_PUSH_VAR) rpn.push_back({"VAR", last_id, 0, sym_table[last_id]});
                else if (top == A_PUSH_CONST) rpn.push_back({"CONST", "", stod(last_const), 0});
                else if (top == A_ADD) addCMD("+");
                else if (top == A_SUB) addCMD("-");
                else if (top == A_MUL) addCMD("*");
                else if (top == A_DIV) addCMD("/");
                else if (top == A_NEG) addCMD("NEG");
                else if (top == A_IDX) addCMD("i");
                else if (top == A_ASSIGN) addCMD(":=");
                else if (top == A_READ) addCMD("r");
                else if (top == A_WRITE) addCMD("w");
                else if (top == A_LT) addCMD("<");
                else if (top == A_GT) addCMD(">");
                else if (top == A_EQ) addCMD("=");
                else if (top == A_LE) addCMD("<=");
                else if (top == A_GE) addCMD(">=");
                else if (top == A_PROG1) { addCMD("!f"); sem_stack.push(rpn.size() - 1); }
                else if (top == A_PROG2) { 
                    addCMD("!"); int p1 = sem_stack.top(); sem_stack.pop();
                    rpn[p1].addr_val = rpn.size(); sem_stack.push(rpn.size() - 1);
                }
                else if (top == A_PROG3) { 
                    if(!sem_stack.empty()) {
                        int p = sem_stack.top(); sem_stack.pop(); rpn[p].addr_val = rpn.size();
                    }
                }
                else if (top == A_PROG4) { sem_stack.push(rpn.size()); }
                else if (top == A_PROG5) { 
                    int p1 = sem_stack.top(); sem_stack.pop();
                    int p4 = sem_stack.top(); sem_stack.pop();
                    addCMD("!"); rpn.back().addr_val = p4;
                    rpn[p1].addr_val = rpn.size();
                }
                else if (top == A_ALLOC_VAR) { sym_table[last_id] = mem_offset; arr_sizes[last_id] = 1; mem_offset += 1; }
                else if (top == A_ALLOC_ARR) { sym_table[last_id] = mem_offset; arr_sizes[last_id] = last_arr_size; mem_offset += last_arr_size; }
                continue;
            }

            if (top < N_PROG) { 
                if (top == tokenToSymbol(curr_token.code)) {
                    if (curr_token.code == T_ID) last_id = curr_token.value;
                    if (curr_token.code == T_NUM_INT || curr_token.code == T_NUM_REAL) {
                        last_const = curr_token.value;
                        if (curr_token.code == T_NUM_INT) last_arr_size = stoi(curr_token.value);
                    }
                    curr_token = lexer.getNextToken();
                } else {
                    cout << "Синтаксическая ошибка: ожидался другой символ, строка " << curr_token.line << ", поз " << curr_token.col << "\n";
                    exit(1);
                }
            } else { 
                if (table[top].count(curr_token.code)) {
                    vector<Symbol> rule = table[top][curr_token.code];
                    for (int i = rule.size() - 1; i >= 0; i--) st.push(rule[i]);
                } else {
                    cout << "Синтаксическая ошибка: непредвиденная лексема '" << curr_token.value << "', строка " << curr_token.line << ", поз " << curr_token.col << "\n";
                    exit(1);
                }
            }
        }
        cout << "Синтаксический анализ пройден. ОПС сгенерирована.\n";
    }

    void printRPN() {
        cout << "--- ОПС ---\n";
        for (int i = 0; i < rpn.size(); i++) {
            cout << i << ": ";
            if (rpn[i].type == "CMD") {
                if (rpn[i].str_val == "!" || rpn[i].str_val == "!f") cout << rpn[i].str_val << " " << rpn[i].addr_val;
                else cout << rpn[i].str_val;
            } else if (rpn[i].type == "VAR") cout << "V(" << rpn[i].str_val << ":" << rpn[i].addr_val << ")";
            else if (rpn[i].type == "CONST") cout << "C(" << rpn[i].num_val << ")";
            cout << "\n";
        }
        cout << "-----------\n";
    }

    vector<RPNItem> getRPN() { return rpn; }
    int getMemSize() { return mem_offset; }
};

// 3. ИНТЕРПРЕТАТОР ОПС

class Interpreter {
    vector<RPNItem> rpn;
    vector<double> memory;
public:
    Interpreter(vector<RPNItem> ops, int mem_size) : rpn(ops) {
        memory.resize(mem_size, 0);
    }

    void execute() {
        struct StackItem { bool is_addr; double val; };
        stack<StackItem> st;
        int pc = 0;
        
        while (pc < rpn.size()) {
            RPNItem item = rpn[pc];
            if (item.type == "CONST") { st.push({false, item.num_val}); pc++; }
            else if (item.type == "VAR") { st.push({true, (double)item.addr_val}); pc++; }
            else if (item.type == "CMD") {
                if (item.str_val == "!") { pc = item.addr_val; }
                else if (item.str_val == "!f") {
                    StackItem cond = st.top(); st.pop();
                    double val = cond.is_addr ? memory[(int)cond.val] : cond.val;
                    if (val == 0) pc = item.addr_val; else pc++;
                }
                else if (item.str_val == "i") {
                    StackItem offset = st.top(); st.pop();
                    StackItem base = st.top(); st.pop();
                    double off_v = offset.is_addr ? memory[(int)offset.val] : offset.val;
                    st.push({true, base.val + off_v});
                    pc++;
                }
                else if (item.str_val == ":=") {
                    StackItem val = st.top(); st.pop();
                    StackItem addr = st.top(); st.pop();
                    double v = val.is_addr ? memory[(int)val.val] : val.val;
                    memory[(int)addr.val] = v;
                    pc++;
                }
                else if (item.str_val == "r") {
                    StackItem addr = st.top(); st.pop();
                    cout << "Ввод: "; cin >> memory[(int)addr.val];
                    pc++;
                }
                else if (item.str_val == "w") {
                    StackItem val = st.top(); st.pop();
                    double v = val.is_addr ? memory[(int)val.val] : val.val;
                    cout << "Вывод: " << v << "\n";
                    pc++;
                }
                else if (item.str_val == "NEG") {
                    StackItem val = st.top(); st.pop();
                    double v = val.is_addr ? memory[(int)val.val] : val.val;
                    st.push({false, -v});
                    pc++;
                }
                else {
                    StackItem right = st.top(); st.pop();
                    StackItem left = st.top(); st.pop();
                    double v2 = right.is_addr ? memory[(int)right.val] : right.val;
                    double v1 = left.is_addr ? memory[(int)left.val] : left.val;
                    double res = 0;
                    if (item.str_val == "+") res = v1 + v2;
                    else if (item.str_val == "-") res = v1 - v2;
                    else if (item.str_val == "*") res = v1 * v2;
                    else if (item.str_val == "/") res = v1 / v2;
                    else if (item.str_val == "<") res = (v1 < v2);
                    else if (item.str_val == ">") res = (v1 > v2);
                    else if (item.str_val == "=") res = (v1 == v2);
                    else if (item.str_val == "<=") res = (v1 <= v2);
                    else if (item.str_val == ">=") res = (v1 >= v2);
                    st.push({false, res});
                    pc++;
                }
            }
        }
    }
};

int main() {
    system("chcp 65001 > nul");

    ifstream file("program.txt");
    if (!file) {
        cout << "Файл program.txt не найден!\n";
        return 1;
    }
    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    
    Lexer lexer(code);
    Parser parser(lexer);
    
    parser.parse();
    parser.printRPN();
    
    cout << "\n--- Запуск интерпретатора ---\n";
    Interpreter interp(parser.getRPN(), parser.getMemSize());
    interp.execute();
    cout << "-----------------------------\n";
    
    return 0;
}