#include "ast.hpp"
#include "symbol_table.hpp"
#include "err.hpp"
#include "types.hpp"
#include <string>

namespace lake{

void ProgramNode::typeAnalysis(TypeAnalysis * ta){

	//pass the TypeAnalysis down throughout
	// the entire tree, getting the types for
	// each element in turn and adding them
	// to the ta object's hashMap
	this->myDeclList->typeAnalysis(ta);

	//The type of the program node will never
	// be needed. We can just set it to VOID
	ta->nodeType(this, VarType::produce(VOID));

	//Alternatively, we could make our type 
	// be error if the DeclListNode is an error

	//Lookup the type assigned to the declList
	// in the earlier recursive call
	const DataType * childType = 
		ta->nodeType(myDeclList);

	//The asError() function of the DataType 
	// class returns null (false) in every
	// subclass EXCEPT for the ErrorType subclass,
	// where it returns itself (non-null/true).
	if (childType->asError()){
		//The child type is error, so 
		// set the program node to error
		// as well
		ta->nodeType(this, ErrorType::produce());
	}
}

void DeclListNode::typeAnalysis(TypeAnalysis * ta){
	
	ta->nodeType(this, VarType::produce(VOID));

	for (auto decl : *myDecls){
		//Do typeAnalysis on the single decl
		decl->typeAnalysis(ta);

		//Lookup the type that we added
		// to the ta in the recursive call
		// above
		auto eltType = ta->nodeType(decl);

		//If the element type was the special
		// "error" type, set this node to the errorType
		if (eltType->asError()){
			ta->nodeType(this, ErrorType::produce());
		}
	}
	return;
}

void FormalsListNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, getDeclaredType());
}

void VarDeclListNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, VarType::produce(VOID));
	for(auto decl : *myDecls){
		decl->typeAnalysis(ta);
		auto eltType = ta->nodeType(decl);
		if(eltType->asError()){
			ta->nodeType(this, ErrorType::produce());
		}
	}
}

void FnDeclNode::typeAnalysis(TypeAnalysis * ta){

	//HINT: you might want to change the signature for
	// typeAnalysis on FnBodyNode to take a second
	// argument which is the type of the current 
	// function. This will help you to know at a 
	// return statement whether the return type matches
	// the current function
	//Note, this function may need extra code
	myFormals->typeAnalysis(ta);
	myBody->typeAnalysis(ta, myRetAST->getDataType());
	const DataType * formalsType = ta->nodeType(myFormals);
	const DataType * bodyType = ta->nodeType(myBody);
	if(formalsType->asError() || bodyType->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->nodeType(this, this->getDeclaredType());
	}
}

void FnBodyNode::typeAnalysis(TypeAnalysis * ta, const DataType * returnType){
	//HINT: as above, you may want to pass the 
	// fnDecl's type into the statement list as a 
	// second argument to StmtList::typeAnalysis
	ta->nodeType(this, VarType::produce(VOID));
	//Note, this function may need extra code
	 myStmtList->typeAnalysis(ta, returnType);
	 myVarDecls->typeAnalysis(ta);
	 auto stmtType = ta->nodeType(myStmtList);
	 auto declType = ta->nodeType(myVarDecls);
	 if(stmtType->asError() || declType->asError()){
		ta->nodeType(this, ErrorType::produce());
	 }
}

void StmtListNode::typeAnalysis(TypeAnalysis * ta, const DataType * returnType){
	//Note, this function may need extra code
	for (auto stmt : *myStmts){
		stmt->typeAnalysis(ta, returnType);
		const DataType * stmtType = ta->nodeType(stmt);
		if(stmtType->asError()){
			ta->nodeType(this, ErrorType::produce());
		}
	}
	ta->nodeType(this, VarType::produce(VOID));
}

void ExpListNode::typeAnalysis(TypeAnalysis * ta){
	std::list<const DataType *> * expTypes = new std::list<const DataType *>();
	for(auto exp : *myExps){
		exp->typeAnalysis(ta);
		const DataType * expType = ta->nodeType(exp);
		if(expType->asError()){
			ta->nodeType(this, ErrorType::produce());
		} else {
			expTypes->push_back(expType);
		}
	}
	TupleType * expList = new TupleType(expTypes);
	ta->nodeType(this, expList);
}

void StmtNode::typeAnalysis(TypeAnalysis * ta, const DataType * returnType){
	TODO("Implement me in the subclass");
}

void ReturnStmtNode::typeAnalysis(TypeAnalysis * ta, const DataType * returnType){
	const DataType * expType = ta->nodeType(myExp);
	if(returnType->isVoid()){
		ta->extraRetValue(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else if(returnType != expType){
		ta->badRetValue(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

void AssignStmtNode::typeAnalysis(TypeAnalysis * ta, const DataType * returnType)
{
	myAssign->typeAnalysis(ta);
	//It can be a bit of a pain to write 
	// "const DataType *" everywhere, so here
	// the use of auto is used instead to tell the
	// compiler to figure out what the subType variable
	// should be
	auto subType = ta->nodeType(myAssign);

	if (subType->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->nodeType(this, VarType::produce(VOID));
	}
}

void ExpNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Override me in the subclass");
}

void AssignNode::typeAnalysis(TypeAnalysis * ta){
	//TODO: Note that this function is incomplete. 
	// and needs additional code

	//Do typeAnalysis on the subexpressions
	myTgt->typeAnalysis(ta);
	mySrc->typeAnalysis(ta);

	const DataType * tgtType = ta->nodeType(myTgt);
	const DataType * srcType = ta->nodeType(mySrc);
	//While incomplete, this gives you one case for 
	// assignment: if the types are exactly the same
	// it is usually ok to do the assignment. One
	// exception is that if both types are function
	// names, it should fail type analysis
	if(tgtType->asError() || srcType->asError()){
		ta->nodeType(this, ErrorType::produce());
	}else if (tgtType == srcType){
		ta->nodeType(this, VarType::produce(VOID));
	} else if(tgtType != srcType){
		ta->badAssignOpr(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else if(tgtType->asFn() || srcType->asFn()){
		ta->badAssignOpd(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->badAssignOpr(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

void DeclNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Override me in the subclass");
}

void VarDeclNode::typeAnalysis(TypeAnalysis * ta){
	// VarDecls always pass type analysis, since they 
	// are never used in an expression. You may choose
	// to type them void (like this).
	ta->nodeType(this, VarType::produce(VOID));
}

void FormalDeclNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, VarType::produce(VOID));
}

void IdNode::typeAnalysis(TypeAnalysis * ta){
	// IDs never fail type analysis and always
	// yield the type of their symbol (which
	// depends on their definition)
	ta->nodeType(this, this->getSymbol()->getType());
}

void IntLitNode::typeAnalysis(TypeAnalysis * ta){
	// IntLits never fail their type analysis and always
	// yield the type INT
	ta->nodeType(this, VarType::produce(INT));
}

void IntNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, VarType::produce(INT));
}

void BoolNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, VarType::produce(BOOL));
}

void VoidNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, VarType::produce(VOID));
}

void TrueNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, VarType::produce(BOOL));
}

void FalseNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, VarType::produce(BOOL));
}

void UnaryMinusNode::typeAnalysis(TypeAnalysis * ta){
	myExp->typeAnalysis(ta);
	const DataType * expType = ta->nodeType(myExp);
	if(expType->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else if(expType->isInt()){
		ta->nodeType(this, VarType::produce(INT));
	} else {
		ta->badMathOpd(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

void NotNode::typeAnalysis(TypeAnalysis * ta){
	myExp->typeAnalysis(ta);
	const DataType * expType = ta->nodeType(myExp);
	if(expType->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else if(expType->isBool()){
		ta->nodeType(this, VarType::produce(BOOL));
	} else {
		ta->badLogicOpd(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

void UnaryExpNode::typeAnalysis(TypeAnalysis * ta){}
void BinaryExpNode::typeAnalysis(TypeAnalysis * ta){}

void PlusNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta); 
	const DataType * exp1Type = ta->nodeType(myExp1);
	const DataType * exp2Type = ta->nodeType(myExp2);
	if(exp1Type->asError() || exp2Type->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else if(exp1Type->isInt() && exp2Type->isInt()){
		ta->nodeType(this, VarType::produce(INT));
	} else if(exp1Type->isPtr() && exp2Type->isPtr()){
		ta->badMathOpr(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->badMathOpd(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

void MinusNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType *exp1Type = ta->nodeType(myExp1);
	const DataType *exp2Type = ta->nodeType(myExp2);
	if (exp1Type->asError() || exp2Type->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else if (exp1Type->isInt() && exp2Type->isInt()){
		ta->nodeType(this, VarType::produce(INT));
	} else if (exp1Type->isPtr() && exp2Type->isPtr()){
		ta->badMathOpr(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->badMathOpd(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

void TimesNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType *exp1Type = ta->nodeType(myExp1);
	const DataType *exp2Type = ta->nodeType(myExp2);
	if (exp1Type->asError() || exp2Type->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else if (exp1Type->isInt() && exp2Type->isInt()){
		ta->nodeType(this, VarType::produce(INT));
	} else if (exp1Type->isPtr() && exp2Type->isPtr()){
		ta->badMathOpr(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->badMathOpd(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

void DivideNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType *exp1Type = ta->nodeType(myExp1);
	const DataType *exp2Type = ta->nodeType(myExp2);
	if (exp1Type->asError() || exp2Type->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else if (exp1Type->isInt() && exp2Type->isInt()){
		ta->nodeType(this, VarType::produce(INT));
	} else if (exp1Type->isPtr() && exp2Type->isPtr()){
		ta->badMathOpr(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->badMathOpd(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

void AndNode::typeAnalysis(TypeAnalysis * ta){

}

void OrNode::typeAnalysis(TypeAnalysis * ta){

}

void EqualsNode::typeAnalysis(TypeAnalysis * ta){

}

void NotEqualsNode::typeAnalysis(TypeAnalysis * ta){

}

void LessNode::typeAnalysis(TypeAnalysis * ta){

}

void GreaterNode::typeAnalysis(TypeAnalysis * ta){

}

void LessEqNode::typeAnalysis(TypeAnalysis * ta){

}

void GreaterEqNode::typeAnalysis(TypeAnalysis * ta){

}

void WriteStmtNode::typeAnalysis(TypeAnalysis * ta, const DataType * returnType){
	myExp->typeAnalysis(ta);
	const DataType * expType = ta->nodeType(myExp);
	if(expType->asFn()){
		ta->writeFn(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else if(expType->isVoid()){
		ta->badWriteVoid(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else if(expType->isPtr()){
		ta->writePtr(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else {
		return;
	}
}

void ReadStmtNode::typeAnalysis(TypeAnalysis * ta, const DataType * returnType){
	myExp->typeAnalysis(ta);
	const DataType * expType = ta->nodeType(myExp);
	if(expType->asFn()){
		ta->readFn(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else if(expType->isPtr()){
		ta->badReadPtr(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else if(expType->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->nodeType(this, VarType::produce(VOID));
	}
}

void CallStmtNode::typeAnalysis(TypeAnalysis * ta, const DataType * returnType){
	myCallExp->typeAnalysis(ta);
	const DataType * expType = ta->nodeType(myCallExp);
	if(expType->asError()){
		ta->nodeType(this, ErrorType::produce());
	} else {
		ta->nodeType(this, VarType::produce(VOID));
	}
}

void CallExpNode::typeAnalysis(TypeAnalysis * ta){
	myId->typeAnalysis(ta);
	myExpList->typeAnalysis(ta);
	const DataType * idType = ta->nodeType(myId);
	const FnType * calleeType = myId->getSymbol()->getType()->asFn();
	const DataType * expListType = ta->nodeType(myExpList);
	int actualsSize = expListType->asTuple()->getElts()->size();
	int formalsSize = calleeType->getFormalTypes()->getElts()->size();
	std::string formals = calleeType->getFormalTypes()->getString();
	std::string actuals = expListType->asTuple()->getString();
	if(idType->asError() || expListType->asError()){
		ta->nodeType(this, ErrorType::produce());
	}else if(actualsSize != formalsSize){
		ta->badArgCount(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else if(actuals != formals){
		ta->badArgMatch(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	} else if(idType->asFn()){
		ta->nodeType(this, VarType::produce(VOID));
	} else {
		ta->badCallee(this->getLine(), this->getCol());
		ta->nodeType(this, ErrorType::produce());
	}
}

}
