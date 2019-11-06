#include "ast.hpp"
#include "symbol_table.hpp"
#include "err.hpp"
#include "types.hpp"

namespace cRAP{

bool ProgramNode::nameAnalysis(SymbolTable * symTab){
	//Enter the global scope
	symTab->enterScope();
	bool res = this->myDeclList->nameAnalysis(symTab);
	//Leave the global scope
	symTab->leaveScope();
	return res;
}

bool AssignStmtNode::nameAnalysis(SymbolTable * symTab){
	return myAssign->nameAnalysis(symTab);
}

bool PostIncStmtNode::nameAnalysis(SymbolTable * symTab){
	return myExp->nameAnalysis(symTab);
}

bool PostDecStmtNode::nameAnalysis(SymbolTable * symTab){
	return myExp->nameAnalysis(symTab);
}

bool ReadStmtNode::nameAnalysis(SymbolTable * symTab){
	return myExp->nameAnalysis(symTab);
}

bool WriteStmtNode::nameAnalysis(SymbolTable * symTab){
	return myExp->nameAnalysis(symTab);
}

bool IfStmtNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	result = myExp->nameAnalysis(symTab) && result;
	symTab->enterScope();
	result = myDecls->nameAnalysis(symTab) && result;
	result = myStmts->nameAnalysis(symTab) && result;
	symTab->leaveScope();
	return result;
}

bool IfElseStmtNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	result = myExp->nameAnalysis(symTab) && result;
	symTab->enterScope();
	result = myDeclsT->nameAnalysis(symTab) && result;
	result = myStmtsT->nameAnalysis(symTab) && result;
	symTab->leaveScope();
	symTab->enterScope();
	result = myDeclsF->nameAnalysis(symTab) && result;
	result = myStmtsF->nameAnalysis(symTab) && result;
	symTab->leaveScope();
	return result;
}

bool WhileStmtNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	symTab->enterScope();
	result = myExp->nameAnalysis(symTab) && result;
	result = myDecls->nameAnalysis(symTab) && result;
	result = myStmts->nameAnalysis(symTab) && result;
	symTab->leaveScope();
	return result;
	
}

bool DeclListNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	for (auto decl : *myDecls){
		result = decl->nameAnalysis(symTab) && result;
	}
	return result;
}

bool StmtListNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	for (auto elt : *myStmts){
		result = elt->nameAnalysis(symTab) && result;
	}
	return result;
}

static bool dataDecl(SymbolTable * symTab, DeclNode * decl){
	DataType * dataType = decl->getTypeNode()->getType();
	std::string varName = decl->getDeclaredName();

	bool validType = dataType->validVarType();
	if (!validType){
		TypeErr::badVoid(decl->getLine(), decl->getCol()); 
	}

	bool validName = !symTab->clash(varName);
	if (!validName){ 
		TypeErr::multiDecl(decl->getLine(), decl->getCol()); 
	}

	if (!validType || !validName){ return false; }

	symTab->insert(new SemSymbol(VAR, dataType, varName));
	return true;
}

bool VarDeclNode::nameAnalysis(SymbolTable * symTab){
	return dataDecl(symTab, this);
}

bool FormalDeclNode::nameAnalysis(SymbolTable * symTab){
	return dataDecl(symTab, this);
}

bool FnDeclNode::nameAnalysis(SymbolTable * symTab){

	std::string fnName = this->getDeclaredName();
	
	DataType * retType = myUseType->getType();
	bool validRet = retType->validRetType();
	if (!validRet){
		size_t line = myUseType->getLine();
		size_t col = myUseType->getCol();
		TypeErr::badVoid(line, col);
	}

	// hold onto the scope where the function itself is
	ScopeTable * atFnScope = symTab->getCurrentScope();
	//Enter a new scope for this function.
	ScopeTable * inFnScope = symTab->enterScope();

	std::list<DataType *> * formalTypes = nullptr;
	bool validFormals = myFormals->nameAnalysis(symTab);
	if (validFormals) { 
		formalTypes = new std::list<DataType *>();
		for (FormalDeclNode * formal : *myFormals->getDecls()){
			std::string formalName = formal->getDeclaredName();
			SemSymbol * sym = inFnScope->lookup(formalName);
			formalTypes->push_back(sym->getType());
		}
	}

	//Note that we check for a clash of the function name in
	// the scope at which it exists (i.e. the function scope)
	bool validName = !atFnScope->clash(fnName);
	if (validName == false){
		TypeErr::multiDecl(
			myDeclaredID->getLine(), 
			myDeclaredID->getCol()); 
	}

	//Make sure the fnSymbol is in the symbol table before 
	// analyzing the body, to allow for recursive calls
	if (validName && validRet && validFormals){
		FnType * fnType = new FnType(formalTypes, retType);
		atFnScope->insert(new SemSymbol(FN, fnType, fnName));
	}

	bool validBody = myBody->nameAnalysis(symTab);

	symTab->leaveScope();
	return (validRet && validName && validFormals && validBody);
}

bool FormalsListNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	for (auto elt : *myFormals){
		result = elt->nameAnalysis(symTab) && result;
	}
	return result;
}

bool FnBodyNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	result = myDeclList->nameAnalysis(symTab);
	result = myStmtList->nameAnalysis(symTab) && result;
	return result;
}

bool BinaryExpNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	result = myExp1->nameAnalysis(symTab) && result;
	myExp2->nameAnalysis(symTab) && result;
	return result;
}

bool ArrayAccessNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	result = myBase->nameAnalysis(symTab) && result;
	result = myExpr->nameAnalysis(symTab) && result;
	return result;
}

bool ExpListNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	for (auto elt : myExps){
		result = elt->nameAnalysis(symTab) && result;
	}
	return result;
}

bool CallExpNode::nameAnalysis(SymbolTable* symTab){
	bool result = true;
	result = myId->nameAnalysis(symTab) && result;
	result = myExpList->nameAnalysis(symTab) && result;
	return result;
}

bool UnaryMinusNode::nameAnalysis(SymbolTable* symTab){
	return myExp->nameAnalysis(symTab);
}

bool NotNode::nameAnalysis(SymbolTable* symTab){
	return myExp->nameAnalysis(symTab);
}

bool AssignNode::nameAnalysis(SymbolTable* symTab){
	bool result = true;
	result = myExpLHS->nameAnalysis(symTab) && result;
	result = myExpRHS->nameAnalysis(symTab) && result;
	return result;
}

bool ReturnStmtNode::nameAnalysis(SymbolTable * symTab){
	if (myExp == nullptr){
		return true;
	}
	return myExp->nameAnalysis(symTab);
}

bool CallStmtNode::nameAnalysis(SymbolTable* symTab){
	return myCallExp->nameAnalysis(symTab);
}

bool IdNode::nameAnalysis(SymbolTable* symTab){
	std::string myName = this->getString();
	SemSymbol * sym = symTab->find(myName);
	if (sym == nullptr){
		return TypeErr::undecl(this->getLine(), getCol());
	}
	this->attachSymbol(sym);
	return true;
}

}
