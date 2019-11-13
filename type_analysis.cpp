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
		ta->nodeType(this, VarType::produce(VOID));

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
		ta->nodeType(this, VarType::produce(VOID));
		ta->nodeType(this, getDeclaredType());
	}

	void TypeNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		ta->nodeType(this, getDataType());
	}

	void FnDeclNode::typeAnalysis(TypeAnalysis * ta){
		ta->nodeType(this, VarType::produce(VOID));

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
		} else {
			ta->nodeType(this, getDeclaredType());	
		}
	}

	void FnBodyNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

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
	}

	void VarDeclListNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		for (auto varDecl : *myDecls) {
			varDecl->typeAnalysis(ta);
		}
	}

	void StmtListNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType){

		//Note, this function may need extra code

		ta->nodeType(this, VarType::produce(VOID));
		for (auto stmt : *myStmts) {
			stmt->typeAnalysis(ta, fnType);

			auto stmtType = ta->nodeType(stmt);

			if(stmtType->asError()) {
				ta->nodeType(this, ErrorType::produce());
			}
		}
	}

	void StmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		TODO("Implement me in the subclass");
	}

	void AssignStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));


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
		ta->nodeType(this, VarType::produce(VOID));

		//TODO: Note that this function is incomplete. 
		// and needs additional code

		//Do typeAnalysis on the subexpressions
		myTgt->typeAnalysis(ta);
		mySrc->typeAnalysis(ta);

		const DataType * tgtType = ta->nodeType(myTgt);
		const DataType * srcType = ta->nodeType(mySrc);

		if(tgtType->asError() || srcType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(srcType->asFn() && tgtType->asFn()) {
			ta->badAssignOpd(myTgt->getLine(), myTgt->getCol());
			ta->badAssignOpd(mySrc->getLine(), mySrc->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(tgtType->asFn()) {
			ta->badAssignOpd(myTgt->getLine(), myTgt->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(srcType->asFn()) {
			ta->badAssignOpd(mySrc->getLine(), mySrc->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(tgtType != srcType) {
			ta->badAssignOpr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, tgtType);
			return;
		}
	}

	void DeclNode::typeAnalysis(TypeAnalysis * ta){
		ta->nodeType(this, VarType::produce(VOID));

		TODO("Override me in the subclass");
	}

	void VarDeclNode::typeAnalysis(TypeAnalysis * ta){
		ta->nodeType(this, VarType::produce(VOID));

		// VarDecls always pass type analysis, since they 
		// are never used in an expression. You may choose
		// to type them void (like this).


		//Alternatively, you could give the VarDecl
		// the type of the symbol it declares (this works
		// because it's type was attached during 
		// nameAnalysis)
		// ta->nodeType(this, myID->getSymbol()->getType());
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
		ta->nodeType(this, VarType::produce(STR));
	}

	void PlusNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!rType->isInt()) {
			ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		}else {
			ta->nodeType(this, VarType::produce(INT));
		}
	}

	void MinusNode::typeAnalysis(TypeAnalysis * ta) {
		ta->nodeType(this, VarType::produce(VOID));

		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!rType->isInt()) {
			ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		}else {
			ta->nodeType(this, VarType::produce(INT));
		}
	}

	void TimesNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!rType->isInt()) {
			ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		}else {
			ta->nodeType(this, VarType::produce(INT));
		}
	}

	void DivideNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt()) {
			ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!rType->isInt()) {
			ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		}else {
			ta->nodeType(this, VarType::produce(INT));
		}
	}

	void AndNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isBool() || !rType->isBool()) {
			ta->badLogicOpd(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void OrNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isBool() || !rType->isBool()) {
			ta->badLogicOpd(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void EqualsNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(lType->asFn() || rType->asFn()) {
			ta->badEqOpd(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(lType != rType) {
			ta->badEqOpr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void NotEqualsNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(lType->asFn() || rType->asFn()) {
			ta->badEqOpd(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(lType != rType) {
			ta->badEqOpr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void LessNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!rType->isInt()) {
			ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void GreaterNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!rType->isInt()) {
			ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void LessEqNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!rType->isInt()) {
			ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void GreaterEqNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp1->typeAnalysis(ta);
		myExp2->typeAnalysis(ta);

		const DataType * lType = ta->nodeType(myExp1);
		const DataType * rType = ta->nodeType(myExp2);

		if(lType->asError() || rType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt() && !rType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!lType->isInt()) {
			ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!rType->isInt()) {
			ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void NotNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!type->isBool()) {
			ta->badLogicOpd(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void PostDecStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!type->isInt()) {
			ta->badRelOpd(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void PostIncStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!type->isInt()) {
			ta->badRelOpd(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(BOOL));
		}
	}

	void ReadStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(type->isPtr()) {
			ta->badReadPtr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(type->asFn()) {
			ta->readFn(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void WriteStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		myExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myExp);

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(type->isPtr()) {
			ta->writePtr(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(type->isVoid()) {
			ta->badWriteVoid(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(type->asFn()) {
			ta->writeFn(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void IfStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		myExp->typeAnalysis(ta);
		myStmts->typeAnalysis(ta, fnType);
		myDecls->typeAnalysis(ta);

		const DataType * condType = ta->nodeType(myExp);
		const DataType * myStmtsType = ta->nodeType(myExp);
		const DataType * myDeclsType = ta->nodeType(myExp);

		if(condType->asError() || myStmtsType->asError() || myDeclsType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(!condType->isBool()) {
			ta->badIfCond(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void IfElseStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

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
		} else if(condType->isBool()) {
			ta->badIfCond(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void WhileStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		myExp->typeAnalysis(ta);
		myStmts->typeAnalysis(ta, fnType);
		myDecls->typeAnalysis(ta);

		const DataType * condType = ta->nodeType(myExp);
		const DataType * myStmtsType = ta->nodeType(myExp);
		const DataType * myDeclsType = ta->nodeType(myExp);

		if(condType->asError() || myStmtsType->asError() || myDeclsType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(condType->isBool()) {
			ta->badWhileCond(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, VarType::produce(VOID));
		}
	}

	void CallStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		myCallExp->typeAnalysis(ta);

		const DataType * type = ta->nodeType(myCallExp);

		ta->nodeType(this, type);
	}

	void CallExpNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));

		myId->typeAnalysis(ta);
		myExpList->typeAnalysis(ta);

		auto idType = ta->nodeType(myId);
		auto fnType = myId->getSymbol()->getType()->asFn();
		auto expListType = ta->nodeType(myExpList);
		auto type = ta->nodeType(myExpList);

		if(fnType == nullptr) {
			ta->badCallee(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
			return;
		}

		int actualsSize = expListType->asTuple()->getElts()->size();
		int formalsSize = fnType->getFormalTypes()->getElts()->size();
		std::string actuals = expListType->asTuple()->getString();
		std::string formals = fnType->getFormalTypes()->getString();

		if(type->asError() || idType->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(actualsSize != formalsSize) {
			ta->badArgCount(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(actuals != formals) {
			ta->badArgMatch(myExpList->getExps()->front()->getLine(), myExpList->getExps()->front()->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(idType->asFn()) {
			ta->nodeType(this, VarType::produce(VOID));
		} else {
			ta->badCallee(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		}
	}

	void ExpListNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
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

	void ReturnStmtNode::typeAnalysis(TypeAnalysis * ta, FnType * fnType) {
		ta->nodeType(this, VarType::produce(VOID));

		if(myExp == nullptr) {
			if(!fnType->getReturnType()->isVoid()) {
				ta->badNoRet(this->getLine(), this->getCol());
				ta->nodeType(this, ErrorType::produce());
				return;
			} else {
				ta->nodeType(this, VarType::produce(VOID));
				return;
			}
		}
		myExp->typeAnalysis(ta);
		const DataType * type = ta->nodeType(myExp);
		const DataType * retType = fnType->getReturnType();

		if(type->asError()) {
			ta->nodeType(this, ErrorType::produce());
		} else if(retType->isVoid() && !type->isVoid()) {
			ta->extraRetValue(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(!retType->isVoid() && type->isVoid()) {
			ta->badNoRet(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else if(type != retType) {
			ta->badRetValue(myExp->getLine(), myExp->getCol());
			ta->nodeType(this, ErrorType::produce());
		} else {
			ta->nodeType(this, type);
		}
	}

	void DerefNode::typeAnalysis(TypeAnalysis * ta) { 
		ta->nodeType(this, VarType::produce(VOID));
		myTgt->typeAnalysis(ta);
		auto tgtType = ta->nodeType(myTgt);
		if(!tgtType->isPtr())
		{
			ta->badDeref(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		}
	}
}
