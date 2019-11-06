#ifndef cRAP_AST_HPP
#define cRAP_AST_HPP

#include <ostream>
#include <list>
#include "err.hpp"
#include "tokens.hpp"
#include "types.hpp"
#include "symbol_table.hpp"

namespace cRAP {

class SymbolTable;
class SemSymbol;

class DeclListNode;
class StmtListNode;
class FormalsListNode;
class DeclNode;
class StmtNode;
class AssignNode;
class FormalDeclNode;
class TypeNode;
class ExpNode;
class IdNode;


class ASTNode{
public:
	ASTNode(size_t lineIn, size_t colIn){
		this->line = lineIn;
		this->col = colIn;
	}
	virtual void unparse(std::ostream& out, int indent) = 0;
	virtual bool nameAnalysis(SymbolTable * symTab) = 0;
	//Note that there is no ASTNode::typeAnalysis. To allow
	// for different type signatures, name analysis is
	// implemented as needed in each of the subclasses
	void doIndent(std::ostream& out, int indent){
		for (int k = 0 ; k < indent; k++){ out << " "; }
	}
	virtual size_t getLine(){ return line; }
	virtual size_t getCol(){ return col; }
	virtual std::string getPosition() {
		std::string res = "";
		res += std::to_string(getLine());
		res += ":";
		res += std::to_string(getCol());
		return res;
	}
protected:
	size_t line;
	size_t col;
};

class ProgramNode : public ASTNode{
public:
	ProgramNode(DeclListNode * declList) : ASTNode(0,0){
		myDeclList = declList;
	}

	void unparse(std::ostream& out, int indent) override;
	virtual bool nameAnalysis(SymbolTable * symTab) override;
	virtual bool typeAnalysis();
	virtual ~ProgramNode(){ }
private:
	DeclListNode * myDeclList;
};

class TypeNode : public ASTNode{
public:
	TypeNode(size_t lineIn, size_t colIn, DataType * dataTypeIn)
	: ASTNode(lineIn, colIn), myType(dataTypeIn) { }
	virtual void unparse(std::ostream& out, int indent)
		override = 0;
	virtual DataType * getType() { return myType; }
	virtual std::string getTypeString(){
		return getType()->getString();
	}
	virtual bool nameAnalysis(SymbolTable * symTab) override {
		throw new InternalError("Name analysis should never"
			" reach type nodes");
	}
	//Note: typeAnalysis is not needed for TypeNodes. They only
	// ever appear beneath declarations, which always pass type
	// analysis
private:
	DataType * myType;
};


class DeclListNode : public ASTNode{
public:
	DeclListNode(std::list<DeclNode *> * decls)
	: ASTNode(0,0){
        	myDecls = decls;
	}
	void unparse(std::ostream& out, int indent);
	virtual bool nameAnalysis(SymbolTable * symTab) override;
	virtual bool typeAnalysis();
private:
	std::list<DeclNode *> * myDecls;
};

class ExpNode : public ASTNode{
public:
	ExpNode(size_t lIn, size_t cIn) : ASTNode(lIn, cIn){ }
	virtual void unparse(std::ostream& out, int indent) override = 0;
	virtual bool nameAnalysis(SymbolTable * symTab) override = 0;
	virtual DataType * typeAnalysis() {
		throw new ToDoError("Every ExpNode must have a type "
			"that explains how it can be used in expressions."
			" As such, subclasses should override this."
		);
	}
protected:
	DataType * myUseType;
};

class IdNode : public ExpNode{
public:
	IdNode(IDToken * token)
	: ExpNode(token->_line, token->_column){
		if (token->_line == 0){
			throw InternalError("bad token pos");
		}
		myStrVal = token->value();
		mySymbol = NULL;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	DataType * typeAnalysis() override;
	virtual std::string getString() { return myStrVal; }

	void attachSymbol(SemSymbol * symbolIn){
		this->mySymbol = symbolIn;
	}

	SemSymbol * getSymbol(){
		return this->mySymbol;
	}

private:
	std::string myStrVal;
	SemSymbol * mySymbol;
};

class ArrayAccessNode : public ExpNode{
public:
	ArrayAccessNode(ExpNode * base, ExpNode * expr)
	: ExpNode(base->getLine(), base->getCol()){
		myBase = base;
		myExpr = expr;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab);
private:
	ExpNode * myBase;
	ExpNode * myExpr;
};

class DeclNode : public ASTNode{
public:
	DeclNode(size_t lIn, size_t cIn, TypeNode * typeIn, IdNode * idIn)
	: ASTNode(lIn, cIn), myUseType(typeIn), myDeclaredID(idIn){ }
	virtual void unparse(std::ostream& out, int indent) = 0;
	virtual bool typeAnalysis() {
		throw new ToDoError("This function should be overridden"
		 " in the subclasses to return if type analysis fails"
		 " in the subtree (varDecls never fail, but fnDecls "
		 " fail if their body is ill-typed.");
	}

	TypeNode * getTypeNode(){ return myUseType; }
	std::string getUseTypeStr(){
		return myUseType->getTypeString();
	}
	std::string getDeclaredName(){
		return myDeclaredID->getString();
	}
	virtual SymbolKind getDeclType() = 0;
protected:
	TypeNode * myUseType;
	IdNode * myDeclaredID;
};


class StmtNode : public ASTNode{
public:
	StmtNode(size_t lIn, size_t cIn) : ASTNode(lIn, cIn){ }
	virtual void unparse(std::ostream& out, int indent) = 0;
	virtual bool typeAnalysis(DataType * returnType) {
		throw new ToDoError("This function should be overridden"
		 " in the subclasses to return if type analysis fails"
		 " in the subtree (stmts fail if they have a subtree "
		 " that is ill-typed.");
	}
};

class FormalDeclNode : public DeclNode{
public:
	FormalDeclNode(TypeNode * type, IdNode * id)
	: DeclNode(type->getLine(), type->getCol(), type, id){ }
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual SymbolKind getDeclType() override {return SymbolKind::FORMAL;}
};


class FormalsListNode : public ASTNode{
public:
	FormalsListNode(std::list<FormalDeclNode *>* formalsIn)
	: ASTNode(0, 0){
		myFormals = formalsIn;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	std::list<FormalDeclNode *> * getDecls(){ return myFormals; }
private:
	std::list<FormalDeclNode *> * myFormals;
};

class ExpListNode : public ASTNode{
public:
	ExpListNode(std::list<ExpNode *> * exps)
	: ASTNode(0,0){
		myExps = *exps;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	std::list<ExpNode *>  * getMyExp(){return &myExps;}
private:
	std::list<ExpNode *> myExps;
};

class StmtListNode : public ASTNode{
public:
	StmtListNode(std::list<StmtNode *> * stmtsIn)
	: ASTNode(0,0){
		myStmts = stmtsIn;
	}
	void unparse(std::ostream& out, int indent) override;
	virtual bool nameAnalysis(SymbolTable * symTab) override;
	virtual bool typeAnalysis(DataType * returnType);
private:
	std::list<StmtNode *> * myStmts;
};

class FnBodyNode : public ASTNode{
public:
	FnBodyNode(size_t lIn, size_t cIn, DeclListNode * decls, StmtListNode * stmts)
	: ASTNode(lIn, cIn){
		myDeclList = decls;
		myStmtList = stmts;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType);
private:
	DeclListNode * myDeclList;
	StmtListNode * myStmtList;
};


class FnDeclNode : public DeclNode{
public:
	FnDeclNode(
		TypeNode * type,
		IdNode * id,
		FormalsListNode * formals,
		FnBodyNode * fnBody)
		: DeclNode(type->getLine(), type->getCol(), type, id)
	{
		myFormals = formals;
		myBody = fnBody;
		myType = type;
	}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis() override;
	virtual SymbolKind getDeclType() override {return SymbolKind::FN;}
private:
	FormalsListNode * myFormals;
	FnBodyNode * myBody;
	TypeNode * myType;
	//Note that FnDeclNode does not have it's own
	// myId field. Instead, it uses it's inherited
	// myDeclaredID field from the DeclNode
};

class ArrayTypeNode : public TypeNode{
public:
	ArrayTypeNode(TypeNode * typeNode, int size)
	: TypeNode(typeNode->getLine(), typeNode->getCol(),
	    new ArrayType(typeNode->getType(), size))
	{
		myPrefixNode = typeNode;
	}
	void unparse(std::ostream& out, int indent);
private:
	TypeNode * myPrefixNode;
};


class IntNode : public TypeNode{
public:
	IntNode(size_t lIn, size_t cIn)
	: TypeNode(lIn, cIn, PrimType::produce(BaseType::INT)) { }
	void unparse(std::ostream& out, int indent);
};

class BoolNode : public TypeNode{
public:
	BoolNode(size_t lIn, size_t cIn)
	: TypeNode(lIn, cIn, PrimType::produce(BaseType::BOOL)) { }
	void unparse(std::ostream& out, int indent);
};

class VoidNode : public TypeNode{
public:
	VoidNode(size_t lIn, size_t cIn)
	: TypeNode(lIn, cIn, PrimType::produce(BaseType::VOID)){ }
	void unparse(std::ostream& out, int indent) override;
};

class IntLitNode : public ExpNode{
public:
	IntLitNode(IntLitToken * token)
	: ExpNode(token->_line, token->_column){
		myInt = token->value();
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override { return true; }
	DataType * typeAnalysis() override;
private:
	int myInt;
};

class StrLitNode : public ExpNode{
public:
	StrLitNode(StringLitToken * token)
	: ExpNode(token->_line, token->_column){
		myString = token->value();
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override { return true; }
	DataType * typeAnalysis() override;
private:
	 std::string myString;
};


class TrueNode : public ExpNode{
public:
	TrueNode(size_t lIn, size_t cIn): ExpNode(lIn, cIn){ }
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override { return true; }
	DataType * typeAnalysis() override;
};

class FalseNode : public ExpNode{
public:
	FalseNode(size_t lIn, size_t cIn): ExpNode(lIn, cIn){ }
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override { return true; }
	DataType * typeAnalysis() override;
};

class AssignNode : public ExpNode{
public:
	AssignNode(
		size_t lIn, size_t cIn,
		ExpNode * expLHS, ExpNode * expRHS)
	: ExpNode(lIn, cIn){
		myExpLHS = expLHS;
		myExpRHS = expRHS;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	DataType * typeAnalysis() override;
private:
	ExpNode * myExpLHS;
	ExpNode * myExpRHS;
};

class CallExpNode : public ExpNode{
public:
	CallExpNode(IdNode * id, ExpListNode * expList)
	: ExpNode(id->getLine(), id->getCol()){
		myId = id;
		myExpList = expList;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	DataType * typeAnalysis() override;

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
	virtual void unparse(std::ostream& out, int indent) = 0;
	bool nameAnalysis(SymbolTable * symTab) override = 0;
protected:
	ExpNode * myExp;
};

class UnaryMinusNode : public UnaryExpNode{
public:
	UnaryMinusNode(ExpNode * exp)
	: UnaryExpNode(line, col, exp){ }
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual DataType * typeAnalysis() override;
};

class NotNode : public UnaryExpNode{
public:
	NotNode(size_t lIn, size_t cIn, ExpNode * exp)
	: UnaryExpNode(lIn, cIn, exp){ }
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual DataType * typeAnalysis() override;

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
	virtual DataType * typeAnalysis() {return nullptr;}

protected:
	ExpNode * myExp1;
	ExpNode * myExp2;
};

class PlusNode : public BinaryExpNode{
public:
	PlusNode(size_t lIn, size_t cIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2) { }
	virtual std::string myOp(){ return "+"; }

	virtual DataType * typeAnalysis() override;
};

class MinusNode : public BinaryExpNode{
public:
	MinusNode(size_t lIn, size_t cIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2){ }
	virtual std::string myOp(){ return "-"; }

	virtual DataType * typeAnalysis() override;
};

class TimesNode : public BinaryExpNode{
public:
	TimesNode(size_t lIn, size_t cIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(line, col, exp1, exp2){ }
	virtual std::string myOp(){ return "*"; }

	virtual DataType * typeAnalysis() override;
};

class DivideNode : public BinaryExpNode{
public:
	DivideNode(size_t lIn, size_t cIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(line, col, exp1, exp2){ }
	virtual std::string myOp(){ return "/"; }

	virtual DataType * typeAnalysis() override;
};

class AndNode : public BinaryExpNode{
public:
	AndNode(size_t lIn, size_t cIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2){ }
	virtual std::string myOp(){ return " and "; }
		virtual DataType * typeAnalysis() override;
};

class OrNode : public BinaryExpNode{
public:
	OrNode(size_t lIn, size_t cIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lIn, cIn, exp1, exp2){ }
	virtual std::string myOp(){ return " or "; }
	virtual DataType * typeAnalysis() override;

};

class EqualsNode : public BinaryExpNode{
public:
	EqualsNode(size_t lineIn, size_t colIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp(){ return "=="; }
	virtual DataType * typeAnalysis() override;

};

class NotEqualsNode : public BinaryExpNode{
public:
	NotEqualsNode(size_t lineIn, size_t colIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp(){ return "!="; }
	virtual DataType * typeAnalysis() override;

};

class LessNode : public BinaryExpNode{
public:
	LessNode(size_t lineIn, size_t colIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp(){ return "<"; }
	virtual DataType * typeAnalysis() override;

};

class GreaterNode : public BinaryExpNode{
public:
	GreaterNode(size_t lineIn, size_t colIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp(){ return ">"; }
	virtual DataType * typeAnalysis() override;

};

class LessEqNode : public BinaryExpNode{
public:
	LessEqNode(size_t lineIn, size_t colIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(lineIn, colIn, exp1, exp2){ }
	virtual std::string myOp(){ return "<="; }
	virtual DataType * typeAnalysis() override;

};

class GreaterEqNode : public BinaryExpNode{
public:
	GreaterEqNode(size_t lineIn, size_t colIn,
		ExpNode * exp1, ExpNode * exp2)
	: BinaryExpNode(line, col, exp1, exp2){ }
	virtual std::string myOp(){ return ">="; }
	virtual DataType * typeAnalysis() override;

};

class AssignStmtNode : public StmtNode{
public:
	AssignStmtNode(AssignNode * assignment)
	: StmtNode(assignment->getLine(), assignment->getCol()){
		myAssign = assignment;
	}
	void unparse(std::ostream& out, int indent);
	virtual std::string myOp(){ return "="; }
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
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
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
private:
	ExpNode * myExp;
};

class PostDecStmtNode : public StmtNode{
public:
	PostDecStmtNode(ExpNode * exp)
	: StmtNode(exp->getLine(), exp->getCol()){
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
		bool typeAnalysis(DataType * returnType) override;
private:
	ExpNode * myExp;
};

class ReadStmtNode : public StmtNode{
public:
	ReadStmtNode(ExpNode * exp)
	: StmtNode(exp->getLine(), exp->getCol()){
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
private:
	ExpNode * myExp;
};

class WriteStmtNode : public StmtNode{
public:
	WriteStmtNode(ExpNode * exp)
	: StmtNode(exp->getLine(), exp->getCol()){
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
private:
	ExpNode * myExp;
};

class IfStmtNode : public StmtNode{
public:
	IfStmtNode(size_t lineIn, size_t colIn, ExpNode * exp,
	  DeclListNode * decls, StmtListNode * stmts)
	: StmtNode(lineIn, colIn){
		myExp = exp;
		myDecls = decls;
		myStmts = stmts;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
private:
	ExpNode * myExp;
	DeclListNode * myDecls;
	StmtListNode * myStmts;
};

class IfElseStmtNode : public StmtNode{
public:
	IfElseStmtNode(ExpNode * exp,
	  DeclListNode * declsT, StmtListNode * stmtsT,
	  DeclListNode * declsF, StmtListNode * stmtsF)
	: StmtNode(exp->getLine(), exp->getCol()){
		myExp = exp;
		myDeclsT = declsT;
		myStmtsT = stmtsT;
		myDeclsF = declsF;
		myStmtsF = stmtsF;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
private:
	ExpNode * myExp;
	DeclListNode * myDeclsT;
	StmtListNode * myStmtsT;
	DeclListNode * myDeclsF;
	StmtListNode * myStmtsF;
};

class WhileStmtNode : public StmtNode{
public:
	WhileStmtNode(size_t lineIn, size_t colIn,
	ExpNode * exp, DeclListNode * decls, StmtListNode * stmts)
	: StmtNode(lineIn, colIn){
		myExp = exp;
		myDecls = decls;
		myStmts = stmts;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
private:
	ExpNode * myExp;
	DeclListNode * myDecls;
	StmtListNode * myStmts;
};

class CallStmtNode : public StmtNode{
public:
	CallStmtNode(CallExpNode * callExp)
	: StmtNode(callExp->getLine(), callExp->getCol()){
		myCallExp = callExp;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
private:
	CallExpNode * myCallExp;
};

class ReturnStmtNode : public StmtNode{
public:
	ReturnStmtNode(size_t lineIn, size_t colIn, ExpNode * exp)
	: StmtNode(lineIn, colIn){
		myExp = exp;
	}
	void unparse(std::ostream& out, int indent);
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis(DataType * returnType) override;
private:
	ExpNode * myExp;
};

class VarDeclNode : public DeclNode{
public:
	VarDeclNode(TypeNode * type, IdNode * id)
	: DeclNode(id->getLine(), id->getCol(), type, id){ }
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable * symTab) override;
	bool typeAnalysis() override;
	virtual SymbolKind getDeclType() override {return SymbolKind::VAR;}
private:
	//Note that VarDeclNode does not have it's own
	// id or type field. Instead, it uses it's inherited
	// fields from the DeclNode
};

} //End namespace cRAP

#endif
