#include "ast.hpp"
#include "symbol_table.hpp"
#include "err.hpp"
#include "types.hpp"

namespace lake{

		// A good way to implement your type checker is by writing member functions for 
		// the different subclasses of ASTNode. Your type checker should find all of the
		// type errors described in the table of the project spec. Your type checker must
		// report the specified position of the error, and it must give the specified error message. 

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

	void FormalsListNode::typeAnalysis(TypeAnalysis * ta) {
		ta->nodeType(this, getDeclaredType());
	}

	void FnDeclNode::typeAnalysis(TypeAnalysis * ta){

		//HINT: you might want to change the signature for
		// typeAnalysis on FnBodyNode to take a second
		// argument which is the type of the current 
		// function. This will help you to know at a 
		// return statement whether the return type matches
		// the current function


		myFormals->typeAnalysis(ta);
		myBody->typeAnalysis(ta, myType);

		auto formalsType = ta->nodeType(myFormals);
		auto bodyType = ta->nodeType(myBody);

		if (formalsType->asError() || bodyType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		}
	}

	void FnBodyNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		//HINT: as above, you may want to pass the 
		// fnDecl's type into the statement list as a 
		// second argument to StmtList::typeAnalysis

		//Note, this function may need extra code

		myVarDecls->typeAnalysis(ta);
		myStmtList->typeAnalysis(ta, fnType);

		auto varDeclsType = ta->nodeType(myVarDecls);
		auto myStmtListType = ta->nodeType(myStmtList);

		if(varDeclsType->asError() || myStmtListType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		}

		ta->nodeType(this, myStmtListType);
	}

	void VarDeclListNode::typeAnalysis(TypeAnalysis * ta) {
		for (auto varDecl : *myDecls) {
			varDecl->typeAnalysis(ta);
		}
	}

	void StmtListNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType){
		//Note, this function may need extra code
		for (auto stmt : *myStmts) {
			stmt->typeAnalysis(ta, fnType);
			auto stmtType = ta->nodeType(stmt);
			if(stmtType->asError()) {
				ta->nodeType(this, ErrorType::produce());
			}
		}
		ta->nodeType(this, VarType::produce(VOID));
	}

	void StmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		TODO("Implement me in the subclass");
	}

	void AssignStmtNode::typeAnalysis(TypeAnalysis * ta) {

		myAssign->typeAnalysis(ta);

		//It can be a bit of a pain to write 
		// "const DataType *" everywhere, so here
		// the use of auto is used instead to tell the
		// compiler to figure out what the subType variable
		// should be
		auto subType = ta->nodeType(myAssign);

		if (subType->asError()){
			ta->nodeType(this, subType);
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
		if(tgtType != srcType) {
			ta->badAssignOpr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(srcType->asFn()) {
			//Some functions are already defined for you to
			// report type errors. Note that these functions
			// also tell the typeAnalysis object that the
			// analysis has failed, meaning that main.cpp
			// will print "Type check failed" at the end
			ta->badAssignOpd(this->getLine(), this->getCol());
			//Note that reporting an error does not set the
			// type of the current node, so setting the node
			// type must be done
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(tgtType->getString));
			return;
		}
	}

	void DeclNode::typeAnalysis(TypeAnalysis * ta){
		TODO("Override me in the subclass");
	}

	void VarDeclNode::typeAnalysis(TypeAnalysis * ta){
		// VarDecls always pass type analysis, since they 
		// are never used in an expression. You may choose
		// to type them void (like this).

		// ta->nodeType(this, VarType::produce(VOID));

		//Alternatively, you could give the VarDecl
		// the type of the symbol it declares (this works
		// because it's type was attached during 
		// nameAnalysis)
		ta->nodeType(this, myID->getSymbol()->getType());
	}

	void IdNode::typeAnalysis(TypeAnalysis * ta){
		// IDs never fail type analysis and always
		// yield the type of their symbol (which
		// depends on their definition)
		ta->nodeType(this, this->getSymbol()->getType());
	}

	void IntNode::typeAnalysis(TypeAnalysis * ta) {
		ta->nodeType(this, VarType::produce(INT));
	}

	void IntLitNode::typeAnalysis(TypeAnalysis * ta){
		// IntLits never fail their type analysis and always
		// yield the type INT
		ta->nodeType(this, VarType::produce(INT));
	}

	void BoolNode::typeAnalysis(TypeAnalysis * ta) {
		ta->nodeType(this, VarType::produce(BOOL));
	}

	void VoidNode::typeAnalysis(TypeAnalysis * ta) {
		ta->nodeType(this, VarType::produce(VOID));
	}

	void FalseNode::typeAnalysis(TypeAnalysis * ta) {
		ta->nodeType(this, VarType::produce(BOOL));
	}

	void TrueNode::typeAnalysis(TypeAnalysis * ta) {
		ta->nodeType(this, VarType::produce(BOOL));
	}

	void StrLitNode::typeAnalysis(TypeAnalysis * ta) {
		ta->nodeType(this, VarType::produce(VOID));
	}

	void PlusNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badMathOpr(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() || !rType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(INT));
		}
	}

	void MinusNode::typeAnalysis(TypeAnalysis * ta)
	{
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badMathOpr(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() || !rType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(INT));
		}
	}

	void TimesNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badMathOpr(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() || !rType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(INT));
		}
	}

	void DivideNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badMathOpr(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() || !rType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(INT));
		}
	}

	void AndNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isBool() || !rType->isBool()) {
			ta->badLogicOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void OrNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isBool() || !rType->isBool()) {
			ta->badLogicOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void EqualsNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(lType->asFn() || rType->asFn()) {
			ta->badEqOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(lType != rType) {
			ta->badEqOpr(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void NotEqualsNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(lType->asFn() || rType->asFn()) {
			ta->badEqOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(lType != rType) {
			ta->badEqOpr(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void LessNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() || !rType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void GreaterNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() || !rType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void LessEqNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() || !rType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void GreaterEqNode::typeAnalysis(TypeAnalysis * ta) {
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() || !rType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void NotNode::typeAnalysis(TypeAnalysis * ta) {
		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!type->isBool) {
			ta->badLogicOpd(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void PostDecStmtNode::typeAnalysis(TypeAnalysis * ta) {
		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!type->isInt()) {
			ta->badRelOpd(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void PostIncStmtNode::typeAnalysis(TypeAnalysis * ta) {
		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!type->isInt()) {
			ta->badRelOpd(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void ReadStmtNode::typeAnalysis(TypeAnalysis * ta) {
		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(type->isPtr) {
			ta->badReadPtr(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(type->asFn()) {
			ta->readFn(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void WriteStmtNode::typeAnalysis(TypeAnalysis * ta) {
		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(type->isPtr) {
			ta->writePtr(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(type->isVoid) {
			ta->badWriteVoid(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(type->asFn()) {
			ta->writeFn(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		}else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void IfStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		myExp->typeAnalysis(ta);
		myStmts->typeAnalysis(ta, fnType);
		myDecls->typeAnalysis(ta);

		const DataType * condType = ta->nodeType(myExp);
		const DataType * myStmtsType = ta->nodeType(myExp);
		const DataType * myDeclsType = ta->nodeType(myExp);

		if(condType->asError() || myStmtsType->asError() || myDeclsType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(condType->isBool) {
			ta->badIfCond(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void IfElseStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		myExp->typeAnalysis(ta);
		myStmtsT->typeAnalysis(ta, fnType);
		myDeclsT->typeAnalysis(ta);
		myStmtsF->typeAnalysis(ta, fnType);
		myDeclsF->typeAnalysis(ta);

		const DataType * condType = ta->nodeType(myExp);
		const DataType * myStmtsTypeT = ta->nodeType(myExp);
		const DataType * myDeclsTypeT = ta->nodeType(myExp);
		const DataType * myStmtsTypeF = ta->nodeType(myExp);
		const DataType * myDeclsTypeF = ta->nodeType(myExp);

		if(condType->asError() || myStmtsTypeT->asError() ||
			myStmtsTypeF->asError() || myDeclsTypeF->asError()
			|| myDeclsTypeT->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(condType->isBool) {
			ta->badIfCond(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void WhileStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		myExp->typeAnalysis(ta);
		myStmts->typeAnalysis(ta, fnType);
		myDecls->typeAnalysis(ta);

		const DataType * condType = ta->nodeType(myExp);
		const DataType * myStmtsType = ta->nodeType(myExp);
		const DataType * myDeclsType = ta->nodeType(myExp);

		if(condType->asError() || myStmtsType->asError() || myDeclsType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(condType->isBool) {
			ta->badWhileCond(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void CallStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		myCallExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myCallExp);

		ta->nodeType(this, type);
	}

	void ReturnStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(type != fnType->getReturnType()) {
			ta->badRetValue(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, type);
		}

	}

	void DerefNode::typeAnalysis(TypeAnalysis * ta) {
		myTgt->typeAnalysis(ta);
	}
}
