// DiCC - Grammatik
//
//   { X }  = beliebig oft      [ X ] = optional      X | Y = Alternative
//
// PROGRAM     -> { DECLARATION } 'main' '(' ')' BLOCK
//
// DECLARATION -> 'const' NAME '=' ( INTEGER | FLOAT )
//                      { ',' NAME '=' ( INTEGER | FLOAT ) } ';'
//              | 'uint'  NAME { ',' NAME } ';'
//              | 'float' NAME [ '[' ( INTEGER | NAME ) ']' ]
//                      { ',' NAME [ '[' ( INTEGER | NAME ) ']' ] } ';'
//              | 'array' NAME '[' ( INTEGER | NAME ) ']'
//                      { ',' NAME '[' ( INTEGER | NAME ) ']' } ';'
//
// BLOCK       -> '{' { STATEMENT } '}'
//
// STATEMENT   -> ';'
//              | BLOCK
//              | REFERENCE '=' EXPRESSION ';'
//              | 'if' '(' EXPRESSION ')' 'then' BLOCK [ 'else' BLOCK ]
//              | 'while' '(' EXPRESSION ')' BLOCK
//              | 'for' '(' BLOCK ',' EXPRESSION ',' BLOCK ')' BLOCK
//              | 'print' '(' EXPRESSION ')' ';'
//
// EXPRESSION  -> DISJUNCT { '||' DISJUNCT }
// DISJUNCT    -> CONJUNCT { '&&' CONJUNCT }
// CONJUNCT    -> TERM { ( '>' | '>=' | '<' | '<=' | '==' | '!=' ) TERM }
// TERM        -> [ '+' | '-' ] SUMMAND { ( '+' | '-' | '|' ) SUMMAND }
// SUMMAND     -> NEGATION { ( '*' | '/' | '%' | '&' ) NEGATION }
// NEGATION    -> '!' NEGATION | '~' NEGATION | FACTOR
// FACTOR      -> INTEGER | FLOAT | REFERENCE
//              | '(' EXPRESSION ')' | 'input' '(' ')'
// REFERENCE   -> NAME [ '[' EXPRESSION ']' ]
//
// NAME        -> LOWERLETTER { LOWERLETTER | DIGIT }
// INTEGER     -> DIGIT { DIGIT }
// FLOAT       -> DIGIT { DIGIT } '.' { DIGIT }
// DIGIT       -> '0' | ... | '9'
// LOWERLETTER -> 'a' | ... | 'z'
//
// '#' bis Zeilenende ist ein Kommentar.
//
// Typregeln (stehen nicht in der Grammatik, macht der Parser uber etype):
// FLOAT-Format EEEEMMMM, Wert = M * 2^(E-7), ohne Vorzeichen.
// Ein Literal mit '.' ist FLOAT, ausser sein Wert ist ganzzahlig.
// Gemischte Operanden: der UINT-Operand wird nach FLOAT gewandelt (i2f),
// das Ergebnis ist FLOAT.
// Zuweisung und 'print' wandeln in den Zieltyp (i2f bzw. f2i),
// ein Arrayindex wird immer nach UINT gewandelt.
// '|', '&', '%' und unares '-' sind fuer FLOAT nicht erlaubt.
// Vergleiche arbeiten direkt auf dem Byte (normalisierter Wertebereich).
// Ein Element von 'float w[n]' ist FLOAT, eines von 'array u[n]' ist UINT.
// Der Typ kommt vom Namen, im Speicher steht in beiden Fällen ein Byte.

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cstdint>
#include <math.h>

using namespace std;
// Prototypen
struct dref {
  string name;
  int type = 0; // 0 const, 1 variable, 2 array
  dref(string s = "", int n = 0) : name(s), type(n) {}
};
bool def_uint();
bool def_array();
bool def_const();
bool def_main();
bool program();
bool block();
bool stm_assign();
bool stm_if();
bool stm_while();
bool stm_for();
bool stm_print();
bool statement();
bool expression();
bool disjunct();
bool conjunct();
bool term();
bool summand();
bool dnegation();
bool factor();
string name();
int integer();
dref reference();
bool terminal(char c);
bool lowerletter();
map<string, int> consts;
set<string> vars;
set<string> arrays;

set<string> floatvars;
uint8_t encode_float(double val);
double decimal_literal();
int etype = 0; // Typ des zuletzt erzeugten Werts: 0 = uint, 1 = float
bool float_add = false;
bool float_mul = false;
bool float_sub = false;
bool float_div = false;
bool need_f2i = false;
bool need_i2f = false;
bool def_float();

vector<char> input;
vector<char>::iterator cur;
vector<string> out;
int labcnt = 0;
dref refnul("", 0);

// Lexer + Input-Filter
#define T_MAIN 0x10
#define T_ARRAY 0x11
#define T_UINT 0x12
#define T_CONST 0x13
#define T_IF 0x14
#define T_THEN 0x15
#define T_ELSE 0x16
#define T_WHILE 0x17
#define T_FOR 0x18
#define T_GT '>'
#define T_GE 0x19
#define T_EQ 0x1a
#define T_NE 0x1b
#define T_LT '<'
#define T_LE 0x1c
#define T_DIS 0x1d
#define T_KON 0x1e
#define T_PRINT 0x1f
#define T_INPUT 0x20
#define T_FLOAT 0x21
const map<string, char> Tokens = {
    {"MAIN", T_MAIN},   {"ARRAY", T_ARRAY}, {"UINT", T_UINT},
    {"CONST", T_CONST}, {"IF", T_IF},       {"THEN", T_THEN},
    {"ELSE", T_ELSE},   {"WHILE", T_WHILE}, {"FOR", T_FOR},
    {">=", T_GE},       {"==", T_EQ},       {"!=", T_NE},
    {"<=", T_LE},       {"<", T_LT},        {">", T_GT},
    {"||", T_DIS},      {"&&", T_KON},      {"PRINT", T_PRINT},
    {"INPUT", T_INPUT}, {"FLOAT", T_FLOAT}, {"main", T_MAIN},
    {"array", T_ARRAY}, {"uint", T_UINT},   {"const", T_CONST},
    {"if", T_IF},       {"then", T_THEN},   {"else", T_ELSE},
    {"while", T_WHILE}, {"for", T_FOR},     {"print", T_PRINT},
    {"input", T_INPUT}, {"float", T_FLOAT}};
void lexer() {
  // no whitespaces and comments
  while (!cin.eof()) {
    char c;
    cin.get(c);
    if (c == '#')
      while (c != '\n')
        cin.get(c);
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
      input.push_back(c);
  }
  // replace all lexems by Tokens
  for (auto &token : Tokens) {
    unsigned short len = token.first.length();
    for (auto input_it = input.begin(); input_it < input.end() - len + 1;
         ++input_it) {
      int hit = 1;
      for (int i = 0; i < len; ++i) // substring match?
        if (*(input_it + i) != token.first[i])
          hit = 0;
      if (hit == 1) { // substring replace
        for (int j = 0; j < len - 1; ++j)
          input.erase(input_it);
        *input_it = token.second;
      }
    }
  }
  cur = input.begin();
}
// Input-Band
void next() { ++cur; }
// Assembler Codes
typedef enum {
  ASM_ADD,
  ASM_SUB,
  ASM_MUL,
  ASM_DIV,
  ASM_MOD,
  ASM_ASGN,
  ASM_AND,
  ASM_OR,
  ASM_INV,
  ASM_OUT,
  ASM_IN,
  ASM_NEG,
  ASM_INT,
  ASM_REF,
  ASM_IF1,
  ASM_IF2,
  ASM_IF3,
  ASM_WHL1,
  ASM_WHL2,
  ASM_WHL3,
  ASM_DIS,
  ASM_KON,
  ASM_NOT,
  ASM_FOR1,
  ASM_FOR2,
  ASM_FOR3,
  ASM_FOR4,
  ASM_GT,
  ASM_GE,
  ASM_LT,
  ASM_LE,
  ASM_EQ,
  ASM_NE,
  ASM_FADD,
  ASM_FSUB,
  ASM_FMUL,
  ASM_FDIV
} optype;
bool CodeGen(optype op, dref p) {
  // op = Code-Selektor
  // p = optionaler Parameter
  static int cnt = 0; // counter für Sprunglabels
  string cntstr = to_string(++cnt);
  vector<string> str;
  // CoCC-Assembler-Codes for DICC commands
  switch (op) {
  case ASM_ADD:
    str = {"pop A", "pop B", "add", "push A"};
    break;
  case ASM_SUB:
    str = {"pop B", "pop A", "sub", "push A"};
    break;
  case ASM_MUL:
    str = {"pop A", "pop B", "mlo", "push A"};
    break;
  case ASM_DIV:
    str = {"pop B",
           "pop A",
           "ldi C 0",
           "divlop" + cntstr + ":",
           "cmp",
           "jc %divend" + cntstr,
           "sub",
           "push A",
           "mov A C",
           "inc",
           "mov C A",
           "pop A",
           "jmp %divlop" + cntstr,
           "divend" + cntstr + ":",
           "push C"};
    break;
  case ASM_MOD:
    str = {"pop B",
           "pop A",
           "ldi C 0",
           "modlop" + cntstr + ":",
           "cmp",
           "jc %modend" + cntstr,
           "sub",
           "push A",
           "mov A C",
           "inc",
           "mov C A",
           "pop A",
           "jmp %modlop" + cntstr,
           "modend" + cntstr + ":",
           "push A"};
    break;
  case ASM_NEG:
    str = {"ldi A 0", "pop B", "sub", "push A"};
    break;
  case ASM_OR:
    str = {"pop A", "pop B", "or", "push A"};
    break;
  case ASM_AND:
    str = {"pop A", "pop B", "and", "push A"};
    break;
  case ASM_NOT:
    str = {"pop A",
           "ldi B 0",
           "cmp",
           "ldi C 1",
           "jz %not" + cntstr,
           "ldi C 0",
           "not" + cntstr + ":",
           "push C"};
    break;
  case ASM_INV:
    str = {"pop A", "not", "push A"};
    break;
  case ASM_ASGN:
    if (p.type == 1)
      str = {"pop A", "stx A %" + p.name};
    else if (p.type == 2)
      str = {"pop C", "pop A", "ldi B %" + p.name + "000", "add", "sta C"};
    break;
  case ASM_REF:
    if (p.type == 1)
      str = {"ldx A %" + p.name, "push A"};
    else if (p.type == 2)
      str = {"pop A", "ldi B %" + p.name + "000", "add", "lda C", "push C"};
    break;
  case ASM_INT:
    str = {"ldi A " + p.name, "push A"};
    break;
  case ASM_IF1:
    str = {"pop A", "ldi B 0", "cmp", "jz %else" + p.name};
    break;
  case ASM_IF2:
    str = {"jmp %cont" + p.name, "else" + p.name + ":"};
    break;
  case ASM_IF3:
    str = {"cont" + p.name + ":"};
    break;
  case ASM_WHL1:
    str = {"wlp" + p.name + ":"};
    break;
  case ASM_WHL2:
    str = {"pop A", "ldi B 0", "cmp", "jz %wlpe" + p.name};
    break;
  case ASM_WHL3:
    str = {"jmp %wlp" + p.name, "wlpe" + p.name + ":"};
    break;
  case ASM_FOR1:
    str = {"flp1" + p.name + ":"};
    break;
  case ASM_FOR2:
    str = {"pop A",
           "ldi B 0",
           "cmp",
           "jz %flpe" + p.name,
           "jmp %flp3" + p.name,
           "flp2" + p.name + ":"};
    break;
  case ASM_FOR3:
    str = {"jmp %flp1" + p.name, "flp3" + p.name + ":"};
    break;
  case ASM_FOR4:
    str = {"jmp %flp2" + p.name, "flpe" + p.name + ":"};
    break;
  case ASM_DIS:
    str = {"pop A",
           "pop B",
           "or",
           "ldi B 0",
           "cmp",
           "ldi C 1",
           "jnz %dis" + cntstr,
           "ldi C 0",
           "dis" + cntstr + ":",
           "push C"};
    break;
  case ASM_KON:
    str = {"ldi B 0",
           "ldi C 1",
           "pop A",
           "cmp",
           "jnz %kon1" + cntstr,
           "ldi C 0",
           "kon1" + cntstr + ":",
           "pop A",
           "cmp",
           "jnz %kon2" + cntstr,
           "ldi C 0",
           "kon2" + cntstr + ":",
           "push C"};
    break;
  case ASM_GT:
    str = {"pop B",
           "pop A",
           "cmp",
           "ldi C 1",
           "jnz %bo1" + cntstr,
           "ldi C 0",
           "bo1" + cntstr + ":",
           "jnc %bo2" + cntstr,
           "ldi C 0",
           "bo2" + cntstr + ":",
           "push C"};
    break;
  case ASM_LT:
    str = {"pop A",
           "pop B",
           "cmp",
           "ldi C 1",
           "jnz %bo1" + cntstr,
           "ldi C 0",
           "bo1" + cntstr + ":",
           "jnc %bo2" + cntstr,
           "ldi C 0",
           "bo2" + cntstr + ":",
           "push C"};
    break;
  case ASM_GE:
    str = {"pop B",
           "pop A",
           "cmp",
           "ldi C 1",
           "jnc %bo1" + cntstr,
           "ldi C 0",
           "bo1" + cntstr + ":",
           "push C"};
    break;
  case ASM_LE:
    str = {"pop A",
           "pop B",
           "cmp",
           "ldi C 1",
           "jnc %bo1" + cntstr,
           "ldi C 0",
           "bo1" + cntstr + ":",
           "push C"};
    break;
  case ASM_EQ:
    str = {"pop A",
           "pop B",
           "cmp",
           "ldi C 1",
           "jz %bo1" + cntstr,
           "ldi C 0",
           "bo1" + cntstr + ":",
           "push C"};
    break;
  case ASM_NE:
    str = {"pop A",
           "pop B",
           "cmp",
           "ldi C 1",
           "jnz %bo1" + cntstr,
           "ldi C 0",
           "bo1" + cntstr + ":",
           "push C"};
    break;
  case ASM_OUT:
    str = {"pop A", "rout A"};
    break;
  case ASM_IN:
    str = {"rin A", "push A"};
    break;

    /*
          Since working with floats requires some functionalities that will be
       needed for every operation we decided to write the ASM code using call
       function that will allow us to reuse the space in the ram more efficient.
       Alignmengt, normalization, packing and splitting are used in all
       operations, which only differ by minimal code.
    */

  case ASM_FADD:
    str = {"call %splitter", "call %align", "call %fadd", "call %pack"};
    float_add = true;
    break;
  case ASM_FSUB:
    str = {"call %splitter", "call %align", "call %fsub", "call %pack"};
    float_sub = true;
    break;
  case ASM_FMUL:
    str = {"call %splitter", "call %fmul", "call %pack"};
    float_mul = true;
    break;
  case ASM_FDIV:
    str = {"call %splitter", "call %fdiv", "call %pack"};
    float_div = true;
    break;
  }

  // append code to asm
  out.insert(out.end(), str.begin(), str.end());
  return true;
}

// Optimierungsroutinen
void optimize() {
  for (vector<string>::iterator it = out.begin(); it < out.end() - 1; ++it) {
    if (*it == "push A" && *(it + 1) == "pop A") {
      out.erase(it, it + 2);
    }
    if (*it == "push B" && *(it + 1) == "pop B") {
      out.erase(it, it + 2);
    }
    if (*it == "push A" && *(it + 1) == "pop B") {
      out.erase(it);
      *it = "mov B A";
    }
    if (*it == "push B" && *(it + 1) == "pop A") {
      out.erase(it);
      *it = "mov A B";
    }
    if (*it == "push C" && *(it + 1) == "pop A") {
      out.erase(it);
      *it = "mov A C";
    }
    if (*it == "push A" && *(it + 1) == "pop C") {
      out.erase(it);
      *it = "mov C A";
    }
    if (*it == "push B" && *(it + 1) == "pop A" && *(it + 2) == "pop B") {
      out.erase(it);
      *it = "mov A B";
    }
    if (*it == "push A" && *(it + 1) == "pop B" && *(it + 2) == "pop A") {
      out.erase(it);
      *it = "mov B A";
    }
  }
}
// Syntax-Error Output
void err(string s) {
  cout << s << endl;
  exit(0);
}
// Assembler-Code Output
void output() {
  printf(".text\n");
  for (auto it : out)
    if (it.back() == ':')
      cout << it << endl;
    else
      cout << "\t" << it << endl;

  if (need_f2i) {
    cout << "f2i:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop A" << endl;
    cout << "\tmov D A" << endl;
    cout << "\tldi B 15" << endl;
    cout << "\tand" << endl;
    cout << "\tmov F A" << endl;
    cout << "\tmov A D" << endl;
    cout << "\tldi B 240" << endl;
    cout << "\tand" << endl;
    cout << "\tmov E A" << endl;
    cout << "f2i_loop:" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 112" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjz %f2i_done" << endl;
    cout << "\tjc %f2i_down" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tmov B A" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov F A" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tsub" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tjmp %f2i_loop" << endl;
    cout << "f2i_down:" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tldi B 1" << endl;
    cout << "\tshr" << endl;
    cout << "\tmov F A" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tjmp %f2i_loop" << endl;
    cout << "f2i_done:" << endl;
    cout << "\tpush F" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }
  if (need_i2f) {
    cout << "i2f:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop A" << endl;
    cout << "\tldi B 0" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjz %i2f_zero" << endl;
    cout << "\tmov F A" << endl;
    cout << "\tldi A 112" << endl;
    cout << "\tmov E A" << endl;
    cout << "i2f_dn:" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjc %i2f_up" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tldi B 1" << endl;
    cout << "\tshr" << endl;
    cout << "\tmov F A" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tjmp %i2f_dn" << endl;
    cout << "i2f_up:" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tldi B 8" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjnc %i2f_done" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 0" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjz %i2f_done" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tmov B A" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov F A" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tsub" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tjmp %i2f_up" << endl;
    cout << "i2f_done:" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tmov B F" << endl;
    cout << "\tor" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
    cout << "i2f_zero:" << endl;
    cout << "\tldi A 0" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }

  if (float_add || float_sub || float_mul || float_div) {
    cout << "splitter:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop A" << endl;
    cout << "\tmov D A" << endl;
    cout << "\tpop A" << endl;
    cout << "\tmov F A" << endl;
    cout << "\tldi B 240" << endl;
    cout << "\tand" << endl;
    cout << "\tmov B A" << endl;
    cout << "\tpush A" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tsub" << endl;
    cout << "\tpush A" << endl;
    cout << "\tmov A D" << endl;
    cout << "\tldi B 240" << endl;
    cout << "\tand" << endl;
    cout << "\tmov B A" << endl;
    cout << "\tpush A" << endl;
    cout << "\tmov A D" << endl;
    cout << "\tsub" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }

  if (float_add || float_sub) {
    cout << "align:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop F" << endl;
    cout << "\tpop E" << endl;
    cout << "\tpop D" << endl;
    cout << "\tpop C" << endl;
    cout << "compare:" << endl;
    cout << "\tmov A C" << endl;
    cout << "\tmov B E" << endl;
    cout << "\tcmp" << endl;
    cout << "\tldi B 1" << endl;
    cout << "\tjz %cmp_retcall" << endl;
    cout << "\tjc %a_smaller_b" << endl;
    cout << "\tjmp %b_smaller_a" << endl;
    cout << "a_smaller_b:" << endl;
    cout << "\tmov A D" << endl;
    cout << "\tldi B 1" << endl;
    cout << "\tshr" << endl;
    cout << "\tmov D A" << endl;
    cout << "\tmov A C" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tjmp %compare" << endl;
    cout << "b_smaller_a:" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tldi B 1" << endl;
    cout << "\tshr" << endl;
    cout << "\tmov F A" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tjmp %compare" << endl;
    cout << "cmp_retcall:" << endl;
    cout << "\tpush E" << endl;
    cout << "\tpush D" << endl;
    cout << "\tpush F" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }
  if (float_add) {
    cout << "fadd:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop B" << endl;
    cout << "\tpop A" << endl;
    cout << "\tadd" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjc %retcall_add" << endl;
    cout << "\tldi B 1" << endl;
    cout << "\tshr" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tpop A" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tadd" << endl;
    cout << "\tpush A" << endl;
    cout << "\tmov A C" << endl;
    cout << "retcall_add:" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }
  if (float_sub) {
    cout << "fsub:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop B" << endl;
    cout << "\tpop A" << endl;
    cout << "\tsub" << endl;
    cout << "\tpop E" << endl;
    cout << "\tldi B 0" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjz %sub_zero" << endl;
    cout << "sub_up:" << endl;
    cout << "\tldi B 8" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjnc %sub_done" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 0" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjz %sub_denorm" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tsub" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tmov A C" << endl;
    cout << "\tmov B A" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tjmp %sub_up" << endl;
    cout << "sub_denorm:" << endl;
    cout << "\tmov A C" << endl;
    cout << "sub_done:" << endl;
    cout << "\tpush E" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
    cout << "sub_zero:" << endl;
    cout << "\tldi A 0" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }
  if (float_mul) {
    cout << "fmul:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop F" << endl;
    cout << "\tpop E" << endl;
    cout << "\tpop D" << endl;
    cout << "\tpop A" << endl;
    cout << "\tldi B 112" << endl;
    cout << "\tsub" << endl;
    cout << "\tmov B E" << endl;
    cout << "\tadd" << endl;
    cout << "\tpush A" << endl;
    cout << "\tmov A F" << endl;
    cout << "\tmov B D" << endl;
    cout << "\tmlo" << endl;
    cout << "\tldi B 0" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjz %mul_zero" << endl;
    cout << "mul_norm:" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjc %retcall_mul" << endl;
    cout << "\tldi B 1" << endl;
    cout << "\tshr" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tpop A" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tadd" << endl;
    cout << "\tpush A" << endl;
    cout << "\tmov A C" << endl;
    cout << "\tjmp %mul_norm" << endl;
    cout << "retcall_mul:" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
    cout << "mul_zero:" << endl;
    cout << "\tpop A" << endl;
    cout << "\tldi A 0" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }
  if (float_div) {
    cout << "fdiv:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop F" << endl;
    cout << "\tpop B" << endl;
    cout << "\tpop D" << endl;
    cout << "\tpop A" << endl;
    cout << "\tsub" << endl;
    cout << "\tldi B 112" << endl;
    cout << "\tadd" << endl;
    cout << "\tldi B 64" << endl;
    cout << "\tsub" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tmov A D" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tmlo" << endl;
    cout << "\tmov B F" << endl;
    cout << "\tldi C 0" << endl;
    cout << "float_divloop:" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjc %div_ready" << endl;
    cout << "\tsub" << endl;
    cout << "\tpush A" << endl;
    cout << "\tmov A C" << endl;
    cout << "\tinc" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tpop A" << endl;
    cout << "\tjmp %float_divloop" << endl;
    cout << "div_ready:" << endl;
    cout << "\tmov A C" << endl;
    cout << "\tldi B 0" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjz %div_zero" << endl;
    cout << "div_dn:" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjc %div_up" << endl;
    cout << "\tldi B 1" << endl;
    cout << "\tshr" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tmov A C" << endl;
    cout << "\tjmp %div_dn" << endl;
    cout << "div_up:" << endl;
    cout << "\tldi B 8" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjnc %div_done" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tmov A E" << endl;
    cout << "\tldi B 0" << endl;
    cout << "\tcmp" << endl;
    cout << "\tjz %div_denorm" << endl;
    cout << "\tldi B 16" << endl;
    cout << "\tsub" << endl;
    cout << "\tmov E A" << endl;
    cout << "\tmov A C" << endl;
    cout << "\tmov B A" << endl;
    cout << "\tadd" << endl;
    cout << "\tmov C A" << endl;
    cout << "\tjmp %div_up" << endl;
    cout << "div_denorm:" << endl;
    cout << "\tmov A C" << endl;
    cout << "div_done:" << endl;
    cout << "\tpush E" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
    cout << "div_zero:" << endl;
    cout << "\tldi A 0" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }
  if (float_add || float_sub || float_mul || float_div) {
    cout << "pack:" << endl;
    cout << "\tpop H" << endl;
    cout << "\tpop A" << endl;
    cout << "\tpop B" << endl;
    cout << "\tor" << endl;
    cout << "\tpush A" << endl;
    cout << "\tpush H" << endl;
    cout << "\tret" << endl;
  }

  printf(".data\n");
  for (auto it : vars)
    cout << "\t" << it << " = 0\n";
}

int main() {
  lexer();    // Lexer
  program();  // Parser
  optimize(); // Optimizer
  output();   // Output
}
bool def_uint() {
  if (terminal(T_UINT)) {
    do {
      string s = name();
      if (s == "")
        err("Syntax error UINT missing name");
      if (consts.find(s) != consts.end())
        err("Error: CONST name '" + s + "' already defined");
      if (arrays.find(s) != arrays.end())
        err("Error: ARRAY name '" + s + "' already defined");
      if (vars.find(s) != vars.end())
        err("Error: UINT  name '" + s + "' already defined");
      if (floatvars.find(s) != floatvars.end())
        err("Error: FLOAT name '" + s + "' already defined");
      vars.insert(s);
    } while (terminal(','));
    if (!terminal(';'))
      err("Syntax error UINT missing ';'");
    return true;
  }
  return false;
}

bool def_float() {
  if (terminal(T_FLOAT)) {
    do {
      string s = name();
      if (s == "")
        err("Syntax error FLOAT missing name");
      if (consts.find(s) != consts.end())
        err("Error: CONST name '" + s + "' already defined");
      if (arrays.find(s) != arrays.end())
        err("Error: ARRAY name '" + s + "' already defined");
      if (vars.find(s) != vars.end())
        err("Error: UINT name '" + s + "' already defined");
      if (floatvars.find(s) != floatvars.end())
        err("Error: FLOAT name '" + s + "' already defined");
      floatvars.insert(s);
      if (*cur == '[') { // Float-Array
        next();
        arrays.insert(s);
        int n;
        string k = name();
        if (k == "") {
          n = integer();
        } else {
          if (floatvars.find(k) != floatvars.end())
            err("Error: Array size must be UINT.");
          if (consts.find(k) == consts.end())
            err("Error: Array size must be const.");
          n = consts[k];
        }
        if (n <= 0)
          err("Syntax error FLOAT size must be > 0");
        if (!terminal(']'))
          err("Syntax error FLOAT missing ']'");
        char sn[20];
        for (int i = 0; i < n; ++i) {
          snprintf(sn, sizeof sn, "%s%03d", s.c_str(), i);
          vars.insert(string(sn));
        }
      } else {
        vars.insert(s);
      }
    } while (terminal(','));
    if (!terminal(';'))
      err("Syntax error FLOAT missing ';'");
    return true;
  }
  return false;
}

bool def_array() {
  if (terminal(T_ARRAY)) {
    do {
      string s = name();
      if (s == "")
        err("Syntax error ARRAY missing name");
      if (consts.find(s) != consts.end())
        err("Error: CONST name '" + s + "' already defined");
      if (arrays.find(s) != arrays.end())
        err("Error: ARRAY name '" + s + "' already defined");
      if (vars.find(s) != vars.end())
        err("Error: UINT  name '" + s + "' already defined");
      if (floatvars.find(s) != floatvars.end())
        err("Error: FLOAT name '" + s + "' already defined");
      arrays.insert(s);
      if (!terminal('['))
        err("Syntax error ARRAY missing '['");
      int n;
      string k = name();
      if (k == "") {
        n = integer();
      } else {
        if (floatvars.find(k) != floatvars.end())
          err("Error: Array size must be UINT.");
        if (consts.find(k) == consts.end())
          err("Error: Array size must be const.");
        n = consts[k];
      }
      if (n <= 0)
        err("Syntax error ARRAY size must be > 0");
      if (!terminal(']'))
        err("Syntax error ARRAY missing ']'");
      // generate variables for array block
      char sn[20];
      for (int i = 0; i < n; ++i) {
        sprintf(sn, "%s%03d", s.c_str(), i);
        vars.insert(string(sn));
      }
    } while (terminal(','));
    if (!terminal(';'))
      err("Syntax error ARRAY missing ';'");
    return true;
  }
  return false;
}
bool def_const() {
  if (terminal(T_CONST)) {
    do {
      string s = name();
      if (s == "")
        err("Syntax error CONST missing name");
      if (consts.find(s) != consts.end())
        err("Error: CONST name '" + s + "' already defined");
      if (arrays.find(s) != arrays.end())
        err("Error: ARRAY name '" + s + "' already defined");
      if (vars.find(s) != vars.end())
        err("Error: UINT  name '" + s + "' already defined");
      if (floatvars.find(s) != floatvars.end())
        err("Error: FLOAT name '" + s + "' already defined");
      if (!terminal('='))
        err("Syntax error CONST missing '='");
      double val = decimal_literal();
      if (val != (int)val) { // FLOAT-Konstante
        consts[s] = encode_float(val);
        floatvars.insert(s);
      } else {
        consts[s] = (int)val;
      };
    } while (terminal(','));
    if (!terminal(';'))
      err("Syntax error CONST missing ';'");
    return true;
  }
  return false;
}
bool def_main() {
  if (terminal(T_MAIN)) {
    if (!terminal('('))
      err("Syntax error MAIN missing '('");
    if (!terminal(')'))
      err("Syntax error MAIN missing ')'");
    if (!block())
      err("Syntax error MAIN missing block");
    return true;
  }
  return false;
}
// Syntax-Analyse und Code-Generierung
bool program() {
  while (def_const() || def_uint() || def_float() || def_array()) {
  }
  if (!def_main())
    err("Syntax error PROTRAM missing MAIN");
  out.insert(out.end(), "hlt");
  return true;
}
bool block() {
  if (terminal('{')) {
    while (statement()) {
    }
    if (!terminal('}'))
      err("Syntax error BLOCK missing '}'");
    return true;
  }
  return false;
}
bool statement() {
  return terminal(';') || block() || stm_assign() || stm_if() || stm_while() ||
         stm_for() || stm_print();
}
bool expression() {
  if (!disjunct())
    return false;
  while (terminal(T_DIS)) {
    disjunct();
    CodeGen(ASM_DIS, refnul);
    etype = 0;
  }
  return true;
}
bool stm_assign() {
  if (lowerletter()) {
    dref lref = reference();
    int lt = etype;
    if (lref.type == 0)
      err("Error ASSIGN: CONST cannot be lvalue");
    if (!terminal('='))
      err("Syntax error ASSIGN missing '='");
    if (!expression())
      err("Syntax error ASSIGN expression");
    if (!terminal(';'))
      err("Syntax error ASSIGN missing ';'");
    if (lt == 1 && etype == 0) {
      out.insert(out.end(), "call %i2f");
      need_i2f = true;
    }
    if (lt == 0 && etype == 1) {
      out.insert(out.end(), "call %f2i");
      need_f2i = true;
    }
    CodeGen(ASM_ASGN, lref);
    return true;
  }
  return false;
}
bool stm_if() {
  if (terminal(T_IF)) {
    string labcntstr = to_string(++labcnt);
    if (!terminal('('))
      err("Syntax error IF missing '('");
    if (!expression())
      err("Syntax error IF expression");
    if (!terminal(')'))
      err("Syntax error IF missing ')'");
    CodeGen(ASM_IF1, dref(labcntstr, 0));
    if (!terminal(T_THEN))
      err("Syntax error IF missing 'THEN'");
    if (!block())
      err("Syntax error IF block");
    CodeGen(ASM_IF2, dref(labcntstr, 0));
    if (terminal(T_ELSE))
      if (!block())
        err("Syntax error ELSE block");
    CodeGen(ASM_IF3, dref(labcntstr, 0));
    return true;
  }
  return false;
}
bool stm_while() {
  if (terminal(T_WHILE)) {
    string labcntstr = to_string(++labcnt);
    CodeGen(ASM_WHL1, dref(labcntstr, 0));
    if (!terminal('('))
      err("Syntax error WHILE missing '('");
    if (!expression())
      err("Syntax error WHILE expression");
    CodeGen(ASM_WHL2, dref(labcntstr, 0));
    if (!terminal(')'))
      err("Syntax error WHILE missing ')'");
    if (!block())
      err("Syntax error WHILE block");
    CodeGen(ASM_WHL3, dref(labcntstr, 0));
    return true;
  }
  return false;
}
bool stm_for() {
  if (terminal(T_FOR)) {
    string labcntstr = to_string(++labcnt);
    if (!terminal('('))
      err("Syntax error FOR missing '('");
    if (!block())
      err("Syntax error FOR in block 1");
    if (!terminal(','))
      err("Syntax error FOR missing ','");
    CodeGen(ASM_FOR1, dref(labcntstr, 0));
    if (!expression())
      err("Syntax error FOR expression");
    CodeGen(ASM_FOR2, dref(labcntstr, 0));
    if (!terminal(','))
      err("Syntax error FOR missing ','");
    if (!block())
      err("Syntax error FOR in block 2");
    if (!terminal(')'))
      err("Syntax error FOR missing ')'");
    CodeGen(ASM_FOR3, dref(labcntstr, 0));
    if (!block())
      err("Syntax error FOR in block 3");
    CodeGen(ASM_FOR4, dref(labcntstr, 0));
    return true;
  }
  return false;
}
bool stm_print() {
  if (terminal(T_PRINT)) {
    if (!terminal('('))
      err("Syntax error PRINT missing '('");
    if (!expression())
      err("Syntax error PRINT expression");
    if (!terminal(')'))
      err("Syntax error PRINT missing ')'");
    if (!terminal(';'))
      err("Syntax error PRINT missing ';'");
    if (etype == 1) {
      out.insert(out.end(), "call %f2i");
      need_f2i = true;
    }
    CodeGen(ASM_OUT, refnul);
    return true;
  }
  return false;
}
// Typen zweier Operanden angleichen: der uint-Operand wird nach Float
// aufgewertet, das Ergebnis ist Float.
// pos = Index in out, an dem der Code des RECHTEN Operanden beginnt.

int balance(int lt, int rt, size_t pos) {
  if (lt == rt)
    return lt;
  need_i2f = true;
  if (lt == 0)
    out.insert(out.begin() + pos, "call %i2f"); // links ist uint
  else
    out.insert(out.end(), "call %i2f"); // rechts ist uint
  return 1;
}
bool disjunct() {
  if (!conjunct())
    return false;
  while (*cur == T_KON) {
    next();
    conjunct();
    CodeGen(ASM_KON, refnul);
    etype = 0;
  }
  return true;
}
bool conjunct() {
  if (!term())
    return false;
  for (;;) {
    optype op;
    switch (*cur) {
    case T_GT:
      op = ASM_GT;
      break;
    case T_GE:
      op = ASM_GE;
      break;
    case T_LT:
      op = ASM_LT;
      break;
    case T_LE:
      op = ASM_LE;
      break;
    case T_EQ:
      op = ASM_EQ;
      break;
    case T_NE:
      op = ASM_NE;
      break;
    default:
      return true;
    }
    int lt = etype;
    next();
    size_t pos = out.size();
    term();
    balance(lt, etype, pos); // Float/Float: kein Cast, cmp stimmt
    CodeGen(op, refnul);
    etype = 0; // Ergebnis ist ein Wahrheitswert
  }
}
bool term() {
  switch (*cur) {
  case '+':
    next();
    summand();
    break;
  case '-':
    next();
    summand();
    if (etype == 1)
      err("Error TERM: unäres '-' geht nicht für FLOAT");
    CodeGen(ASM_NEG, refnul);
    break;
  default:
    summand();
    break;
  }
  int lt = etype;
  while (*cur == '+' || *cur == '-' || *cur == '|') {
    char o = *cur;
    next();
    size_t pos = out.size();
    summand();
    int t = balance(lt, etype, pos);
    if (t == 1) {
      switch (o) {
      case '+':
        CodeGen(ASM_FADD, refnul);
        break;
      case '-':
        CodeGen(ASM_FSUB, refnul);
        break;
      case '|':
        err("Error TERM: '|' geht nicht für FLOAT");
      }
    } else {
      switch (o) {
      case '+':
        CodeGen(ASM_ADD, refnul);
        break;
      case '-':
        CodeGen(ASM_SUB, refnul);
        break;
      case '|':
        CodeGen(ASM_OR, refnul);
        break;
      }
    }
    lt = etype = t;
  }
  return true;
}

bool summand() {
  dnegation();
  int lt = etype;
  while (*cur == '*' || *cur == '/' || *cur == '%' || *cur == '&') {
    char o = *cur;
    next();
    size_t pos = out.size();
    dnegation();
    int t = balance(lt, etype, pos);
    if (t == 1) {
      switch (o) {
      case '*':
        CodeGen(ASM_FMUL, refnul);
        break;
      case '/':
        CodeGen(ASM_FDIV, refnul);
        break;
      case '%':
        err("Error SUMMAND: '%' geht nicht für FLOAT");
      case '&':
        err("Error SUMMAND: '&' geht nicht für FLOAT");
      }
    } else {
      switch (o) {
      case '*':
        CodeGen(ASM_MUL, refnul);
        break;
      case '/':
        CodeGen(ASM_DIV, refnul);
        break;
      case '%':
        CodeGen(ASM_MOD, refnul);
        break;
      case '&':
        CodeGen(ASM_AND, refnul);
        break;
      }
    }
    lt = etype = t;
  }
  return true;
}

bool dnegation() {
  switch (*cur) {
  case '!':
    next();
    dnegation();
    CodeGen(ASM_NOT, refnul);
    etype = 0;
    break;
  case '~':
    next();
    dnegation();
    if (etype == 1)
      err("Error NEGATION: '~' geht nicht für FLOAT");
    CodeGen(ASM_INV, refnul);
    break;
  default:
    factor();
  }
  return true;
}
bool factor() {
  dref rref;
  switch (*cur) {
  case T_INPUT:
    next();
    if (*cur == '(')
      next();
    else
      err("Syntax INPUT error missing '('");
    if (*cur == ')')
      next();
    else
      err("Syntax INPUT error missing ')'");
    CodeGen(ASM_IN, refnul);
    etype = 0;
    break;
  case '0' ... '9': {
    double val = decimal_literal();
    if (val != (int)val) {
      CodeGen(ASM_INT, dref(to_string(encode_float(val)), 0));
      etype = 1;
    } else {
      CodeGen(ASM_INT, dref(to_string((int)val), 0));
      etype = 0;
    }
    break;
  }
    /*
    Since we store floats the same way as integers we dont need a new function.
    We just encode the float as an integer and then call the storage fct with
    this encoded float
    */
  case 'a' ... 'z':
    rref = reference();
    if (rref.type == 0) {
      CodeGen(ASM_INT, rref);
    } else {
      CodeGen(ASM_REF, rref);
    }
    break;
  case '(':
    next();
    expression();
    if (*cur == ')')
      next();
    else
      err("Syntax erro missing ')'");
    break;
  default:
    err("Syntax error expecting factor");
    break;
  }
  return true;
}
// Rückgabe des Variablenmens und Aufnahme in die Variablenliste
dref reference() {
  int t = 1; // default type = UINT
  string s = name();
  if (arrays.find(s) == arrays.end() && vars.find(s) == vars.end() &&
      consts.find(s) == consts.end())
    err("Error: name '" + s + "' not defined");
  int dt = (floatvars.find(s) != floatvars.end());
  if (*cur == '[') { // found ARRAY
    next();
    expression();
    if (etype == 1) { // Index muss uint sein
      out.insert(out.end(), "call %f2i");
      need_f2i = true;
    }
    if (*cur == ']')
      next();
    else
      err("Syntax error missing ]");
    t = 2;
  } else if (consts.find(s) != consts.end()) {
    t = 0;
    s = to_string(consts[s]);
  }
  etype = dt;
  return dref(s, t);
}
int integer() {
  int z = 0;
  while (isdigit(*cur)) {
    z = 10 * z + int(*cur - '0');
    next();
  }
  return z;
}
string name() {
  string s = "";
  if (lowerletter()) {
    s = s + *cur;
    next();
    while (islower(*cur) || isdigit(*cur)) {
      s = s + *cur;
      next();
    }
  }
  return s;
}
bool terminal(char c) {
  if (*cur == c) {
    next();
    return true;
  } else
    return false;
}
bool lowerletter() { return islower(*cur); }

uint8_t encode_float(double val) {
  uint8_t best = 0;
  double best_err = 1e18;
  for (int b = 0; b < 256; b++) {
    int E = (b >> 4) & 0xF;
    int M = b & 0xF;
    double decoded = M * pow(2.0, E - 7);
    double err = fabs(decoded - val);
    if (err < best_err) {
      best_err = err;
      best = b;
    }
  }
  return best;
}

double decimal_literal() {
  double z = 0;
  while (isdigit(*cur)) {
    z = 10 * z + (*cur - '0');
    next();
  }
  if (*cur == '.') {
    next();
    double frac = 0.1;
    while (isdigit(*cur)) {
      z += (*cur - '0') * frac;
      frac *= 0.1;
      next();
    }
  }
  return z;
}
