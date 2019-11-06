%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines
%define api.namespace {cRAP}
%define parser_class_name {Parser}
%define parse.error verbose
%output "parser.cc"
%token-table

%code requires{
   #include <list>
   #include "tokens.hpp"
   #include "ast.hpp"
   namespace cRAP {
      class Scanner;
   }

// The following definitions is missing when %locations isn't used
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

}

%parse-param { cRAP::Scanner  &scanner  }
%parse-param { cRAP::ProgramNode** root }

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   /* include for interoperation between scanner/parser */
   #include "scanner.hpp"

#undef yylex
#define yylex scanner.yylex
}

/*%define api.value.type variant*/
%union {
	cRAP::IntLitToken * intTokenValue;
	cRAP::StringLitToken * strTokenValue;
	cRAP::IDToken * idTokenValue;
	cRAP::Token * tokenValue;
	cRAP::ASTNode * astNode;
	cRAP::ProgramNode * programNode;
	std::list<DeclNode *> * declList;
	//std::list<FormalDeclNode *> * declList;
	cRAP::VarDeclNode * varDeclNode;
	cRAP::DeclNode * declNode;
	cRAP::FnDeclNode * fnDecl;
	cRAP::FormalDeclNode * formalDecl;
	std::list<FormalDeclNode *> * formalsList;
	cRAP::FormalsListNode * formalsType;
	cRAP::FnBodyNode * fnBody;
	std::list<StmtNode *> * stmtList;
	std::list<ExpNode *> * expList;
	cRAP::TypeNode * typeNode;
	cRAP::StmtNode * stmtNode;
	cRAP::ExpNode * exp;
	cRAP::IdNode * idNode;
	cRAP::AssignNode * assignNode;
	cRAP::CallExpNode * callNode;
}

%define parse.assert

%token                  END    0     "end of file"
%token                  NEWLINE "newline"
%token <tokenValue>     CHAR
%token <tokenValue>     BOOL
%token <tokenValue>     INT
%token <tokenValue>     VOID
%token <tokenValue>     TRUE
%token <tokenValue>     FALSE
%token <tokenValue>     IF
%token <tokenValue>     ELSE
%token <tokenValue>     WHILE
%token <tokenValue>     RETURN
%token <idTokenValue>   ID
%token <intTokenValue>  INTLITERAL
%token <strTokenValue>  STRINGLITERAL
%token <tokenValue>     LCURLY
%token <tokenValue>     RCURLY
%token <tokenValue>     LPAREN
%token <tokenValue>     RPAREN
%token <tokenValue>     LBRACE
%token <tokenValue>     RBRACE
%token <tokenValue>     SEMICOLON
%token <tokenValue>     COMMA
%token <tokenValue>     WRITE
%token <tokenValue>     READ
%token <tokenValue>     CROSSCROSS
%token <tokenValue>     DASHDASH
%token <tokenValue>     CROSS
%token <tokenValue>     DASH
%token <tokenValue>     STAR
%token <tokenValue>     SLASH
%token <tokenValue>     NOT
%token <tokenValue>     AND
%token <tokenValue>     OR
%token <tokenValue>     EQUALS
%token <tokenValue>     NOTEQUALS
%token <tokenValue>     LESS
%token <tokenValue>     GREATER
%token <tokenValue>     LESSEQ
%token <tokenValue>     GREATEREQ
%token <tokenValue>     ASSIGN

/* Nonterminals
*  NOTE: You will need to add more nonterminals
*  to this list as you add productions to the grammar
*  below.
*/
%type <programNode> program
%type <declList> declList
%type <declNode> decl
%type <varDeclNode> varDecl
%type <typeNode> type
%type <idNode> id
%type <declList> varDeclList
%type <fnDecl> fnDecl
%type <fnBody> fnBody
%type <stmtList> stmtList
%type <formalsType> formals
%type <formalsList> formalsList
%type <formalDecl> formalDecl
%type <stmtNode> stmt
%type <exp> exp
%type <callNode> fncall
%type <assignNode> assignExp
%type <exp> term
%type <exp> loc
%type <expList> actualList

/* NOTE: Make sure to add precedence and associativity
 * declarations
*/
%right ASSIGN
%left OR
%left AND
%nonassoc LESS GREATER LESSEQ GREATEREQ EQUALS NOTEQUALS
%left CROSS DASH
%left STAR SLASH
%left NOT
%%

program : declList 
          {
          $$ = new ProgramNode(new DeclListNode($1));
          *root = $$;
          }

declList : declList decl 
           {
           $1->push_back($2);
           $$ = $1;
           }
         | /* epsilon */ 
           {
           $$ = new std::list<DeclNode *>();
           }

decl : varDecl { $$ = $1; }
     | fnDecl { $$ = $1; }


varDecl : type id SEMICOLON 
          {
          $$ = new VarDeclNode($1, $2);
          }

varDeclList : /* epsilon */ 
              {
              $$ = new std::list<DeclNode *>();
              }
            | varDeclList varDecl 
              {
              $1->push_back($2);
              $$ = $1;
              }

fnDecl : type id formals fnBody 
         {
         $$ = new FnDeclNode($1, $2, $3, $4);
         }

formals : LPAREN RPAREN 
          {
          $$ = new FormalsListNode(new std::list<FormalDeclNode *>()); 
          }
	| LPAREN formalsList RPAREN 
          {
          $$ = new FormalsListNode($2); 
          }

formalsList : formalDecl 
              {
              std::list<FormalDeclNode *> * list = new std::list<FormalDeclNode *>();
              list->push_back($1);
              $$ = list;
              }
            | formalDecl COMMA formalsList 
              {
              $3->push_front($1);
              $$ = $3;
              }

fnBody : LCURLY varDeclList stmtList RCURLY {
         $$ = new FnBodyNode($1->_line, $1->_column, 
                  new DeclListNode($2), new StmtListNode($3));
       }

formalDecl : type id 
             {
             $$ = new FormalDeclNode($1, $2);
             }

stmtList : /* epsilon */ 
           { 
           $$ = new std::list<StmtNode *>();}
         | stmtList stmt 
           { 
           $1->push_back($2);
           $$ = $1;
           }

stmt : assignExp SEMICOLON { $$ = new AssignStmtNode($1); }
     | loc CROSSCROSS SEMICOLON { $$ = new PostIncStmtNode($1); }
     | loc DASHDASH SEMICOLON { $$ = new PostDecStmtNode($1); }
     | READ loc SEMICOLON { $$ = new ReadStmtNode($2); }
     | WRITE exp SEMICOLON { $$ = new WriteStmtNode($2); }
     | IF LPAREN exp RPAREN LCURLY varDeclList stmtList RCURLY 
        { 
        $$ = new IfStmtNode($1->_line, $1->_column, $3, 
                     new DeclListNode($6), new StmtListNode($7));
        }
     | IF LPAREN exp RPAREN LCURLY varDeclList stmtList RCURLY ELSE LCURLY varDeclList stmtList RCURLY
        { 
        $$ = new IfElseStmtNode(
                $3, 
                new DeclListNode($6), 
                new StmtListNode($7), 
                new DeclListNode($11), 
                new StmtListNode($12)); 
        }
     | WHILE LPAREN exp RPAREN LCURLY varDeclList stmtList RCURLY
       { 
        $$ = new WhileStmtNode($1->_line, $1->_column, 
                 $3, new DeclListNode($6), new StmtListNode($7)); 
       }
     | RETURN exp SEMICOLON 
	{ $$ = new ReturnStmtNode($1->_line, $1->_column, $2); }
     | RETURN SEMICOLON 
       { $$ = new ReturnStmtNode($1->_line, $1->_column, nullptr); }
     | fncall SEMICOLON { $$ = new CallStmtNode($1); }


assignExp : loc ASSIGN exp 
    { $$ = new AssignNode($2->_line, $2->_column, $1, $3); }

exp : assignExp { $$ = $1;}
    | exp CROSS exp 
      { $$ = new PlusNode($2->_line, $2->_column, $1, $3); }
    | exp DASH exp 
      { $$ = new MinusNode($2->_line, $2->_column, $1, $3); }
    | exp STAR exp 
      { $$ = new TimesNode($2->_line, $2->_column, $1, $3); }
    | exp SLASH exp 
      { $$ = new DivideNode($2->_line, $2->_column, $1, $3); }
    | NOT exp 
      { $$ = new NotNode($1->_line, $1->_column, $2); }
    | exp AND exp 
      { $$ = new AndNode($2->_line, $2->_column, $1, $3); }
    | exp OR exp 
      { $$ = new OrNode($2->_line, $2->_column, $1, $3); }
    | exp EQUALS exp 
      { $$ = new EqualsNode($2->_line, $2->_column, $1, $3); }
    | exp NOTEQUALS exp 
      { $$ = new NotEqualsNode($2->_line, $2->_column, $1, $3); }
    | exp LESS exp 
      { $$ = new LessNode($2->_line, $2->_column, $1, $3); }
    | exp GREATER exp 
      { $$ = new GreaterNode($2->_line, $2->_column, $1, $3); }
    | exp LESSEQ exp 
      { $$ = new LessEqNode($2->_line, $2->_column, $1, $3); }
    | exp GREATEREQ exp 
      { $$ = new GreaterEqNode($2->_line, $2->_column, $1, $3); }
    | DASH term { $$ = new UnaryMinusNode($2); }
    | term { $$ = $1; }

term : loc { $$ = $1; }
     | INTLITERAL { $$ = new IntLitNode($1); }
     | STRINGLITERAL { $$ = new StrLitNode($1); }
     | TRUE { $$ = new TrueNode($1->_line, $1->_column); }
     | FALSE { $$ = new FalseNode($1->_line, $1->_column); }
     | LPAREN exp RPAREN { $$ = $2; }
     | fncall { $$ = $1; }

fncall : id LPAREN RPAREN 
        { 
        $$ = new CallExpNode($1, new ExpListNode(new std::list<ExpNode *>()));
        }
        | id LPAREN actualList RPAREN 
        { 
        $$ = new CallExpNode($1, new ExpListNode($3)); 
        }

actualList : exp 
        { 
        std::list<ExpNode *> * list = new std::list<ExpNode *>();
        list->push_back($1);
        $$ = list;
        }
        | actualList COMMA exp 
        {
        $1->push_back($3);
        $$ = $1;
        }

type : INT { $$ = new IntNode($1->_line, $1->_column); }
     | BOOL { $$ = new BoolNode($1->_line, $1->_column); }
     | VOID { $$ = new VoidNode($1->_line, $1->_column); }
     | type LBRACE INTLITERAL RBRACE 
	{ 
	TypeNode * type = $1;
	int arraySize = $3->value();
	$$ = new ArrayTypeNode($1, arraySize); 
	}


loc : id { $$ = $1; }
    | loc LBRACE exp RBRACE
      { $$ = new ArrayAccessNode($1, $3); }

id : ID { $$ = new IdNode($1); }

%%
void
cRAP::Parser::error(const std::string &err_msg )
{
   cRAP::Err::syntaxReport(err_msg);
}
