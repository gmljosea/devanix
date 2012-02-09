%defines
%output "parser.cc"
%define api.pure
%locations

%code requires {
#include <cstdio>
#include <iostream>
#include <string>
#include <list>

#include "expression.hh"
#include "statement.hh"
#include "program.hh"
#include "type.hh"

}

%code {

int yylex(YYSTYPE*,YYLTYPE*);
void yyerror (char const *);
extern FILE *yyin;

Program program;

int current_scope() {
  return 0;
}

void setLocation(Statement* stmt, YYLTYPE* yylloc) {
  stmt->setLocation(yylloc->first_line, yylloc->first_column, yylloc->last_line,
		    yylloc->last_column);
}

}

%union {
  int ival;
  double fval;
  std::string *str;
  Statement *stmt;
  Block *blk;
  Expression *exp;
  Type *type;
  std::list<Expression*> *exps;
  std::list<Lvalue*> *lvalues;
  Lvalue *lvalue;
};

// Tokens de palabras reservadas
%token TK_IF          "if"
%token TK_ELSE        "else"
%token TK_FOR         "for"
%token TK_IN          "in"
%token TK_STEP        "step"
%token TK_WHILE       "while"
%token TK_INT         "int"
%token TK_CHAR        "char"
%token TK_BOOL        "bool"
%token TK_FLOAT       "float"
%token TK_ARRAY       "array"
%token TK_STRING      "string"
%token TK_BOX         "box"
%token TK_VOID        "void"
%token TK_VARIANT     "variant"
%token TK_TRUE        "true"
%token TK_FALSE       "false"
%token TK_RETURN      "return"
%token TK_BREAK       "break"
%token TK_NEXT        "next"

// Tokens de símbolos especiales

%token TK_PLUS        "+"
%token TK_MINUS       "-"
%token TK_TIMES       "*"
%token TK_DIV         "/"
%token TK_MOD         "%"
%token TK_EQU         "="
%token TK_LT          "<"
%token TK_GT          ">"
%token TK_GTE         ">="
%token TK_LTE         "<="
%token TK_AND         "and"
%token TK_OR          "or"
%token TK_NOT         "not"

%token TK_LBRACE      "{"
%token TK_RBRACE      "}"
%token TK_LBRACKET    "["
%token TK_RBRACKET    "]"
%token TK_LPARENT     "("
%token TK_RPARENT     ")"
%token TK_COMMA       ","
%token TK_SCOLON      ";"

%token TK_DPERIOD     ".."

%token TK_DOLLAR      "$"
%token TK_DDOLLAR     "$$"
%token TK_COLON       ":"

// Token identificador (de variable, función o box)
%token <str> TK_ID

// Token de un string cualquiera encerrado entre comillas
 // No confundirse con TK_STRING que se refiere a la palabra reservada 'string'
%token <str> TK_CONSTSTRING

// Tokens de constantes numéricas
%token <ival> TK_CONSTINT
%token <fval> TK_CONSTFLOAT

%type <stmt> statement if while for variabledec asignment
%type <blk> block stmts funblock
%type <exp> expr funcallexp
%type <str> label
%type <type> type
%type <lvalues> lvalues
%type <lvalue> lvalue
%type <exps> explist nonempty_explist

%% /* Gramática */

/*
Nota: para engranar el sistema de leblanc-cook, se modifica block para que maneje
apropiadamente los alcances.
Update: puse las reglas vacías 'enterscope' para hacer esto.
*/

/*
Nota: chequear que no se declare una variable de tipo void, ni que se declaren
funciones de tipos arreglo y box se convirtió en un chequeo de contexto. Resulta
que separándolos a nivel de gramática daba conflicto reduce/reduce
*/

/*
Nota: la grmática para la 1º entrega debería estar lista. Falta probarla bien.
Ahora intentar parsear algo probablemente va a producir un segfault. Eso lo
voy arreglar pronto (la razón es la pila de { } vacíos que no instancian los
AST que les toca y más arriba en el parseo se dereferencia una cosa que no es
un objeto)
*/

/**
 * Un programa es una secuencia de declaraciones globales. Mínimo una de ellas
 * debe ser una función llamada 'main'.
 */
globals:
 /* En estas producciones no hay que hacer nada, la regla 'global' produce los
    efectos de borde deaseados */
   global
 | globals global

global:
   /* Declaración de una o más variables globales, posiblemente con asignación */
   variabledec
   { program.globalinits.push_back(dynamic_cast<VariableDec*> $1); }
 | type TK_ID enterscope "(" params ")" funblock
   { program.symtable.leave_scope();
     /*SymFunction* sym = new SymFunction(...);
     program.symtable.insert(sym); */
     // Imprimo la función solo para debugging
     std::cout << "Función '" << *$2 << "':" << std::endl;
     $7->print(0);
 }

 // ** Inicio (de la mayor parte de) gramática de la declaración de funciones
params: // Debería devolver una lista de tuplas como dentro de SymFunction
   /* empty */
 | paramlist

paramlist:
   passby type TK_ID
   { /* Meter símbolo en la tabla y crear la lista de argumentos */ }
 | paramlist "," passby type TK_ID
   { /* Meter símbolo en la tabla y agregarlo a la lista de argumentos */ }

passby:
   /* empty */
 | "$" /* Implementar esto con un enum por favor */
 | "$$"

funblock:
 /* Igual que la regla 'block', pero no abre un alcance.
    Esto porque como es un bloque de función, el alcance que le corresponde
    fue abierto antes de llegar a esta regla (para meter los símbolos de los
    parámetros) */
   "{" stmts "}" { setLocation($2,&@$); $$ = $2; }

 // ** Fin gramática de la declaración de funciones

/**
 * Un bloque es un entorno de referencia único junto a una secuencia de
 * instrucciones.
 */
block:
   "{" enterscope stmts "}"
   { setLocation($3,&@$); $$ = $3; program.symtable.leave_scope(); }

enterscope:
   /* empty */ { program.symtable.enter_scope(0); }

 // ** Produce una secuencia de instrucciones
stmts:
   statement
   { $$ = new Block(current_scope(), $1);
     $1->setEnclosing($$);
     setLocation($1, &@$);}
 | stmts statement
   { $1->push_back($2);
     $2->setEnclosing($1);
     setLocation($2, &@$);
     $$ = $1; }

statement:
  ";"       { $$ = new Null(); }
 | if
 | while
 | for
 | variabledec { $$ = new VariableDec(); } /* Temporal */
 | asignment   { $$ = new Asignment(); } /* Temporal */
 | funcallexp ";" { $$ = new FunctionCall($1); }
 | "break" TK_ID ";" { $$ = new Break($2); }
 | "break" ";" { $$ = new Break(NULL); }
 | "next" TK_ID ";" { $$ = new Next($2); }
 | "next" ";" { $$ = new Next(NULL); }
 | "return" expr ";" { $$ = new Return($2); }
 | "return" ";"     { $$ = new Return(); }

if:
   "if" expr block
   { std::cout << "Encontré un if sin else" << std::endl;
     $$ = new If($2, $3);
     $3->setEnclosing($$);
     setLocation($$,&@$);}

 | "if" expr block "else" block
   { std::cout << "Encontré un if con else" << std::endl;
     $$ = new If($2, $3, $5);
     $3->setEnclosing($$);
     $5->setEnclosing($$);
     setLocation($$, &@$);}

while:
   label "while" expr block
   { std::cout << "Encontré un while" << std::endl;
     $$ = new While($1, $3, $4);}

for:
   label "for" TK_ID "in" expr ".." expr block
   { std::cout << "Encontré un for sin paso" << std::endl;
     $$ = new BoundedFor($1, $3, $5, $7, NULL, $8); }
 | label "for" TK_ID "in" expr ".." expr "step" expr block
   { std::cout << "Encontré un for con paso" << std::endl;
     $$ = new BoundedFor($1, $3, $5, $7, $9, $10); }
 /*| "for" TK_ID "in" TK_ID block  //foreach de arreglos*/

label:
   /* empty */  { $$ = NULL; }
 | TK_ID ":"    { $$ = $1; }

 // ** Inicio gramática de la asignación
asignment: // Modificar clase Asignment para que reciba listas de rvalues y lvalues
   lvalues "=" explist ";" {   }

lvalues: // Devuelve list<Lvalue*>
   lvalue { $$ = new std::list<Lvalue*>(); $$->push_back($1); }
 | lvalues "," lvalue { $1->push_back($3); $$ = $1; }

lvalue: // Instanciar lvalue (falta hacer la clase)
   TK_ID  { $$ = new Lvalue(); }

 // ** Fin gramática de la asignación

 // ** Inicio gramática de la declaración de variables
variabledec:
   type vardec_items ";"

vardec_items: // Devuelve una lista de pair<string,expr>
   vardec_item
 | vardec_items "," vardec_item

vardec_item: // Devuelve un pair<string,expr>
   TK_ID
 | TK_ID "=" expr

type:
   "int"   { $$ = new Type(); }
 | "char"  { $$ = new Type(); }
 | "bool"  { $$ = new Type(); }
 | "float" { $$ = new Type(); }
 | "void"  { $$ = new Type(); }
 // ** Fin gramática de la declaración de variables

 // ** Inicio gramática de las expresiones
expr:
   TK_ID          { $$ = new Expression(); }
 | TK_CONSTINT    { $$ = new Expression(); }
 | TK_CONSTFLOAT  { $$ = new Expression(); }
 | TK_TRUE        { $$ = new Expression(); }
 | TK_FALSE       { $$ = new Expression(); }

funcallexp:
   TK_ID "(" explist ")" { $$ = new Expression(); }

explist:
   /* empty */
 | nonempty_explist

nonempty_explist:
   expr    { $$ = new std::list<Expression*>(); $$->push_back($1); }
 | explist "," expr { $1->push_back($3); $$ = $1; }

 // ** Fin gramática de las expresiones

%%

void yyerror (char const *s) {
  std::cerr << "Error: " << s << std::endl;
}

// Por ahora el main está aquí, pero luego hay que moverlo
int main (int argc, char **argv) {
  if (argc == 2) {
    yyin = fopen(argv[1], "r");
  }
  yyparse();
  // Pedirle cosas a Program
  return 0;
}
