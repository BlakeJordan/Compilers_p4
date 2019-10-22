#include "ast.hpp"
#include "symbol_table.hpp"
#include "err.hpp"
#include "types.hpp"

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

void FnDeclNode::typeAnalysis(TypeAnalysis * ta){

	//HINT: you might want to change the signature for
	// typeAnalysis on FnBodyNode to take a second
	// argument which is the type of the current 
	// function. This will help you to know at a 
	// return statement whether the return type matches
	// the current function

	//Note, this function may need extra code

	return myBody->typeAnalysis(ta);
}

void FnBodyNode::typeAnalysis(TypeAnalysis * ta){
	//HINT: as above, you may want to pass the 
	// fnDecl's type into the statement list as a 
	// second argument to StmtList::typeAnalysis

	//Note, this function may need extra code

	return myStmtList->typeAnalysis(ta);
}

void StmtListNode::typeAnalysis(TypeAnalysis * ta){
	//Note, this function may need extra code
	for (auto stmt : *myStmts){
		stmt->typeAnalysis(ta);
	}
}

void StmtNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Implement me in the subclass");
}

void AssignStmtNode::typeAnalysis(TypeAnalysis * ta){

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
	if (tgtType == srcType){
		return;
	}
	
	//Some functions are already defined for you to
	// report type errors. Note that these functions
	// also tell the typeAnalysis object that the
	// analysis has failed, meaning that main.cpp
	// will print "Type check failed" at the end
	ta->badAssignOpr(this->getLine(), this->getCol());


	//Note that reporting an error does not set the
	// type of the current node, so setting the node
	// type must be done
	ta->nodeType(this, ErrorType::produce());
}

void DeclNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Override me in the subclass");
}

void VarDeclNode::typeAnalysis(TypeAnalysis * ta){
	// VarDecls always pass type analysis, since they 
	// are never used in an expression. You may choose
	// to type them void (like this).
	ta->nodeType(this, VarType::produce(VOID));

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

void IntLitNode::typeAnalysis(TypeAnalysis * ta){
	// IntLits never fail their type analysis and always
	// yield the type INT
	ta->nodeType(this, VarType::produce(INT));
}

}
