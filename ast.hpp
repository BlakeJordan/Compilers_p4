#ifndef TEENC_AST_HPP
#define TEENC_AST_HPP

#include <ostream>
#include <sstream>
#include <string.h>
#include <list>
#include "err.hpp"
#include "tokens.hpp"
#include "types.hpp"

namespace lake {

class TypeAnalysis;

class SymbolTable;
class SemSymbol;

class DerefNode;
class RefNode;
class DeclListNode;
class StmtListNode;
class FormalsListNode;
class DeclNode;
class VarDeclNode;
class StmtNode;
class AssignNode;
class FormalDeclNode;
class TypeNode;
class ExpNode;
class IdNode;

class ASTNode{
public:
	ASTNode(size_t lineIn, size_t colIn);
	virtual void unparse(std::ostream&, int) = 0;
	virtual bool nameAnalysis(SymbolTable *) = 0;
	//Note that there is no ASTNode::typeAnalysis. To allow
	// for different function signatures, you may want to 
	// implement type analysis per-subclass without overrides
	void doIndent(std::ostream&, int);
	virtual size_t getLine();
	virtual size_t getCol();
	virtual std::string getPosition();
private:
	size_t line;
	size_t col;
};

class ProgramNode : public ASTNode{
public:
	ProgramNode(DeclListNode *);
	void unparse(std::ostream&, int) override;
	virtual bool nameAnalysis(SymbolTable *) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
	virtual ~ProgramNode(){ }
private:
	DeclListNode * myDeclList;
};

class TypeNode : public ASTNode{
public:
	TypeNode(size_t lineIn, size_t colIn);
	void unparse(std::ostream&, int) override = 0;
	virtual const DataType * getDataType() = 0;
	virtual std::string getTypeString();
	virtual bool nameAnalysis(SymbolTable *) override;
	virtual void setPtrDepth(size_t depth); 
	virtual size_t getPtrDepth(){ return myPtrDepth; }
	virtual void printIndirection(std::ostream& out);
private:
	size_t myPtrDepth;
};


class DeclListNode : public ASTNode{
public:
	DeclListNode(std::list<DeclNode *> * decls) 
	: ASTNode(0,0){
        	myDecls = decls;
	}
	void unparse(std::ostream& out, int indent) override;
	virtual bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	std::list<DeclNode *> * myDecls;
};

class VarDeclListNode : public ASTNode{
public: 
	VarDeclListNode(std::list<VarDeclNode *> * decls) 
	: ASTNode(0, 0), myDecls(decls){ }
	virtual void unparse(std::ostream&, int);
	virtual bool nameAnalysis(SymbolTable *);
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	std::list<VarDeclNode *> * myDecls;
};

class ExpNode : public ASTNode{
public:
	ExpNode(size_t lIn, size_t cIn) : ASTNode(lIn, cIn){ }
	virtual void unparse(std::ostream& out, int indent) override = 0;
	virtual bool nameAnalysis(SymbolTable * symTab) override = 0;
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class DerefNode : public ExpNode {
public:
	DerefNode(size_t line, size_t column, ExpNode *);
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable *) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	ExpNode * myTgt;
};

class IdNode : public ExpNode{
public:
	IdNode(IDToken * token);
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual std::string getString();
	void attachSymbol(SemSymbol * symbolIn);
	SemSymbol * getSymbol();
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	std::string myStrVal;
	SemSymbol * mySymbol;
};



class StmtNode : public ASTNode{
public:
	StmtNode(size_t lIn, size_t cIn) : ASTNode(lIn, cIn){ }
	virtual void unparse(std::ostream& out, int indent) override = 0;
	virtual void typeAnalysis(TypeAnalysis * ta, FnType * fnType);
};

class DeclNode : public ASTNode{
public:
	DeclNode(size_t l, size_t c, IdNode *); 
	void unparse(std::ostream& out, int indent) override =0;
	virtual const DataType * getDeclaredType() const = 0;
	std::string getDeclaredName();
	IdNode * getDeclaredID();
	virtual void typeAnalysis(TypeAnalysis * ta);
protected:
	IdNode * myID;
};

class FormalDeclNode : public DeclNode{
public:
	FormalDeclNode(TypeNode * type, IdNode * id) 
	: DeclNode(id->getLine(), id->getCol(), id), myType(type){ }
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual TypeNode * getTypeNode() { return myType; }
	virtual const DataType * getDeclaredType() const { 
		return myType->getDataType(); }
private:
	TypeNode * myType;
};


class FormalsListNode : public ASTNode{
public:
	FormalsListNode(std::list<FormalDeclNode *>* formalsIn)
	: ASTNode(0, 0){
		myFormals = formalsIn;
		auto eltTypeList = new std::list<const DataType *>();
		for (auto elt : *formalsIn){
			eltTypeList->push_back(elt->getDeclaredType());
		}
		myDataType = new TupleType(eltTypeList);
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	std::list<FormalDeclNode *> * getDecls(){ return myFormals; }
	TupleType * getDeclaredType(){ return myDataType; }
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	std::list<FormalDeclNode *> * myFormals;
	TupleType * myDataType;
};

class ExpListNode : public ASTNode{
public:
	ExpListNode(std::list<ExpNode *> * exps) 
	: ASTNode(0,0){
		myExps = exps;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	void typeAnalysis(TypeAnalysis * ta);
	size_t size(){ return myExps->size(); }
private:
	std::list<ExpNode *> * myExps;
};

class StmtListNode : public ASTNode{
public:
	StmtListNode(std::list<StmtNode *> * stmtsIn) 
	: ASTNode(0,0){
		myStmts = stmtsIn;
	}
	void unparse(std::ostream& out, int indent) override;
	virtual bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta, FnType * fnType);
private:
	std::list<StmtNode *> * myStmts;
};

class FnBodyNode : public ASTNode{
public:
	FnBodyNode(size_t lIn, size_t cIn, VarDeclListNode * decls, StmtListNode * stmts) 
	: ASTNode(lIn, cIn){
		myStmtList = stmts;
		myVarDecls = decls;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta, FnType * fnDeclType);
private:
	StmtListNode * myStmtList;
	VarDeclListNode * myVarDecls;
};


class FnDeclNode : public DeclNode{
public:
	FnDeclNode(
		TypeNode * retASTNode, 
		IdNode * id, 
		FormalsListNode * formals, 
		FnBodyNode * fnBody) 
		: DeclNode(retASTNode->getLine(),retASTNode->getCol(), id)
	{
		myFormals = formals;
		myBody = fnBody;
		myRetAST = retASTNode;
		myType = new FnType(
			formals->getDeclaredType(),
			myRetAST->getDataType());
	}
	TypeNode * getReturnTypeNode(){ return myRetAST; }
	virtual const DataType * getDeclaredType() const override {
		return myType;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	FormalsListNode * myFormals;
	FnBodyNode * myBody;
	TypeNode * myRetAST;
	FnType * myType;
	//Note that FnDeclNode does not have it's own 
	// myId field. Instead, it uses it's inherited
	// myDeclaredID field from the DeclNode
};

class IntNode : public TypeNode{
public:
	IntNode(size_t lIn, size_t cIn) 
	: TypeNode(lIn, cIn){}
	void unparse(std::ostream& out, int indent) override;
	virtual const DataType * getDataType() override;
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class BoolNode : public TypeNode{
public:
	BoolNode(size_t lIn, size_t cIn) 
	: TypeNode(lIn, cIn) { }
	void unparse(std::ostream& out, int indent) override;
	virtual const DataType * getDataType() override;
	void typeAnalysis(TypeAnalysis * ta);
};

class VoidNode : public TypeNode{
public:
	VoidNode(size_t lIn, size_t cIn) 
	: TypeNode(lIn, cIn){}
	virtual const DataType * getDataType() override;
	void unparse(std::ostream& out, int indent) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class IntLitNode : public ExpNode{
public:
	IntLitNode(IntLitToken * token)
	: ExpNode(token->_line, token->_column){
		myInt = token->value();
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override { 
		if (symTab == nullptr) { 
			throw InternalError("null symtab");
		}
		return true; 
	}
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	int myInt;
};

class StrLitNode : public ExpNode{
public:
	StrLitNode(StringLitToken * token)
	: ExpNode(token->_line, token->_column){
		myString = token->value();
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable *) override { 
		return true; 
	}
	void typeAnalysis(TypeAnalysis * ta);
private:
	 std::string myString;
};


class TrueNode : public ExpNode{
public:
	TrueNode(size_t lIn, size_t cIn): ExpNode(lIn, cIn){ }
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable *) override { 
		return true; 
	}
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class FalseNode : public ExpNode{
public:
	FalseNode(size_t lIn, size_t cIn): ExpNode(lIn, cIn){ }
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override { 
		if (symTab == nullptr) { 
			throw InternalError("null symTab"); 
		}
		return true; 
	}
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class AssignNode : public ExpNode{
public:
	AssignNode(size_t lIn, size_t cIn, ExpNode * tgt, ExpNode * src)
	: ExpNode(lIn, cIn){
		myTgt = tgt;
		mySrc = src;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	ExpNode * myTgt;
	ExpNode * mySrc;
};

class CallExpNode : public ExpNode{
public:
	CallExpNode(IdNode * id, ExpListNode * expList)
	: ExpNode(id->getLine(), id->getCol()){
		myId = id;
		myExpList = expList;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	void typeAnalysis(TypeAnalysis * ta);
private:
	IdNode * myId;
	ExpListNode * myExpList;
};

class UnaryExpNode : public ExpNode {
public:
	UnaryExpNode(size_t lIn, size_t cIn, ExpNode * expIn) 
	: ExpNode(lIn, cIn){
		this->myExp = expIn;
	}
	virtual void unparse(std::ostream& out, int indent) override = 0;
	virtual bool nameAnalysis(SymbolTable * symTab) override = 0;
protected:
	ExpNode * myExp;
};

class UnaryMinusNode : public UnaryExpNode{
public:
	UnaryMinusNode(ExpNode * exp)
	: UnaryExpNode(exp->getLine(), exp->getCol(), exp){ }
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
};

class NotNode : public UnaryExpNode{
public:
	NotNode(size_t lIn, size_t cIn, ExpNode * exp)
	: UnaryExpNode(lIn, cIn, exp){ }
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	void typeAnalysis(TypeAnalysis * ta);
};

class BinaryExpNode : public ExpNode{
public:
	BinaryExpNode(
		size_t lIn, size_t cIn, 
		ExpNode * exp1, ExpNode * exp2)
	: ExpNode(lIn, cIn) {
		this->myExp1 = exp1;
		this->myExp2 = exp2;
	}
	virtual void unparse(std::ostream& out, int indent)
		override;
	virtual std::string myOp() = 0;
	bool nameAnalysis(SymbolTable * symTab) override;
protected:
	ExpNode * myExp1;
	ExpNode * myExp2;
};

class PlusNode : public BinaryExpNode{
public:
	PlusNode(size_t lIn, size_t cIn, 
		ExpNode * exp1, ExpNode * exp2) 
	: BinaryExpNode(lIn, cIn, exp1, exp2) { }
	virtual std::string myOp() override { return "+"; }
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class MinusNode : public BinaryExpNode{
public:
	MinusNode(size_t lIn, size_t cIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2){ }
	virtual std::string myOp() override { return "-"; }
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class TimesNode : public BinaryExpNode{
public:
	TimesNode(size_t lIn, size_t cIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2){ }
	virtual std::string myOp() override { return "*"; }
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class DivideNode : public BinaryExpNode{
public:
	DivideNode(size_t lIn, size_t cIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2){ }
	virtual std::string myOp() override { return "/"; }
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class AndNode : public BinaryExpNode{
public:
	AndNode(size_t lIn, size_t cIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2){ }
	virtual std::string myOp() override { return " and "; }
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class OrNode : public BinaryExpNode{
public:
	OrNode(size_t lIn, size_t cIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2){ }
	virtual std::string myOp() override { return " or "; }
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class EqualsNode : public BinaryExpNode{
public:
	EqualsNode(size_t lineIn, size_t colIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp() override { return "=="; }
	virtual void typeAnalysis(TypeAnalysis * ta); 
};

class NotEqualsNode : public BinaryExpNode{
public:
	NotEqualsNode(size_t lineIn, size_t colIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp() override { return "!="; } 
	virtual void typeAnalysis(TypeAnalysis * ta);
	
};

class LessNode : public BinaryExpNode{
public:
	LessNode(size_t lineIn, size_t colIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp() override { return "<"; }
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class GreaterNode : public BinaryExpNode{
public:
	GreaterNode(size_t lineIn, size_t colIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp() override { return ">"; }
	virtual void typeAnalysis(TypeAnalysis * ta); 
};

class LessEqNode : public BinaryExpNode{
public:
	LessEqNode(size_t lineIn, size_t colIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp() override { return "<="; } 
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class GreaterEqNode : public BinaryExpNode{
public:
	GreaterEqNode(size_t lineIn, size_t colIn, 
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp() override { return ">="; } 
	virtual void typeAnalysis(TypeAnalysis * ta);
};

class AssignStmtNode : public StmtNode{
public:
	AssignStmtNode(AssignNode * assignment)
	: StmtNode(assignment->getLine(), assignment->getCol()){
		myAssign = assignment;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	AssignNode * myAssign;
};

class PostIncStmtNode : public StmtNode{
public:
	PostIncStmtNode(ExpNode * exp)
	: StmtNode(exp->getLine(), exp->getCol()){
		if (exp->getLine() == 0){
			throw InternalError("0 pos");
		}	
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	ExpNode * myExp;
};

class PostDecStmtNode : public StmtNode{
public:
	PostDecStmtNode(ExpNode * exp)
	: StmtNode(exp->getLine(), exp->getCol()){
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	ExpNode * myExp;
};

class ReadStmtNode : public StmtNode{
public:
	ReadStmtNode(ExpNode * exp)
	: StmtNode(exp->getLine(), exp->getCol()){
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	ExpNode * myExp;
};

class WriteStmtNode : public StmtNode{
public:
	WriteStmtNode(ExpNode * exp)
	: StmtNode(exp->getLine(), exp->getCol()){
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
private:
	ExpNode * myExp;
};

class IfStmtNode : public StmtNode{
public:
	IfStmtNode(size_t lineIn, size_t colIn, ExpNode * exp, VarDeclListNode * decls, StmtListNode * stmts)
	: StmtNode(lineIn, colIn){
		myExp = exp;
		myStmts = stmts;
		myDecls = decls;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta, FnType * fnType);
private:
	ExpNode * myExp;
	VarDeclListNode * myDecls;
	StmtListNode * myStmts;
};

class IfElseStmtNode : public StmtNode{
public:
	IfElseStmtNode(ExpNode * exp, VarDeclListNode * declsT, StmtListNode * stmtsT, VarDeclListNode * declsF, StmtListNode * stmtsF)
	: StmtNode(exp->getLine(), exp->getCol()){
		myExp = exp;
		myDeclsT = declsT;
		myStmtsT = stmtsT;
		myDeclsF = declsF;
		myStmtsF = stmtsF;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta, FnType * fnType);
private:
	ExpNode * myExp;
	VarDeclListNode * myDeclsT;
	StmtListNode * myStmtsT;
	VarDeclListNode * myDeclsF;
	StmtListNode * myStmtsF;
};

class WhileStmtNode : public StmtNode{
public:
	WhileStmtNode(size_t lineIn, size_t colIn, ExpNode * exp, VarDeclListNode * decls, StmtListNode * stmts)
	: StmtNode(lineIn, colIn){
		myExp = exp;
		myDecls = decls;
		myStmts = stmts;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta, FnType * fnType);
private:
	ExpNode * myExp;
	VarDeclListNode * myDecls;
	StmtListNode * myStmts;
};

class CallStmtNode : public StmtNode{
public:
	CallStmtNode(CallExpNode * callExp)
	: StmtNode(callExp->getLine(), callExp->getCol()){
		myCallExp = callExp;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta, FnType * fnType);
private:
	CallExpNode * myCallExp;
};

class ReturnStmtNode : public StmtNode{
public:
	ReturnStmtNode(size_t lineIn, size_t colIn, ExpNode * exp)
	: StmtNode(lineIn, colIn){
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta, FnType * fnType);
private:
	ExpNode * myExp;
};

class VarDeclNode : public DeclNode{
public:
	VarDeclNode(TypeNode * type, IdNode * id) 
	: DeclNode(id->getLine(), id->getCol(), id), myType(type){ }
	void unparse(std::ostream& out, int indent) override;
	virtual const DataType * getDeclaredType() const { 
		return myType->getDataType(); }
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis * ta);
	virtual TypeNode * getTypeNode() { return myType; } 
private:
	TypeNode * myType;
	//Note that VarDeclNode does not have it's own 
	// id field, it is inherited from the superclass
	
};

} //End namespace TEENC

#endif
