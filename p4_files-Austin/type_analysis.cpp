#include "ast.hpp"
#include "err.hpp"
#include "types.hpp"
#include <string>


namespace cRAP{

bool ProgramNode::typeAnalysis(){
	// The programNode does not, itself, have a type.
	// Instead, its return indicates whether or not
	// the program is well typed. (i.e. true means no
	// errors in the subtree).
	return this->myDeclList->typeAnalysis();
}

bool DeclListNode::typeAnalysis(){
	bool result = true;
	for (auto decl : *myDecls){
		result = decl->typeAnalysis() && result;
	}
	return result;
}

bool FnDeclNode::typeAnalysis(){
	bool result = true;

if(this->myDeclaredID == nullptr){
	throw InternalError("Null ID");
}

SemSymbol * callCheck = this->myDeclaredID->getSymbol();
	 if(callCheck->kindToString(this->getDeclType()) != "fn"){
		throw InternalError("FnDeclNode contains non-funciton id symbol");
	 }
//HINT: if you changed the function signature for FnBody
// to pass context to the subtree that it needs for type
// checking, you'd have to change this type as well.
DataType * returnType;

if(myType->getTypeString() == "int")
{
	returnType = PrimType::produce(BaseType::INT);
}

if(myType->getTypeString() == "bool")
{
	returnType = PrimType::produce(BaseType::BOOL);
}

if(myType->getTypeString() == "string")
{
	returnType = PrimType::produce(BaseType::STR);
}

if(myType->getTypeString() == "void")
{
	returnType = PrimType::produce(BaseType::VOID);
}
return myBody->typeAnalysis(returnType);
}

bool FnBodyNode::typeAnalysis(DataType * returnType){
	//HINT: if you changed the function signature for StmtList
	// to pass context to the subtree that it needs for type
	// checking, you'd have to change this type as well.
	return myStmtList->typeAnalysis(returnType);
}

//HINT: you may want to change the function signature here
// to pass context from the parent tree that it needs for type
// checking.
bool StmtListNode::typeAnalysis(DataType * returnType){
	bool result = true;
	for (auto stmt : *myStmts){
		result = stmt->typeAnalysis(returnType) && result;
	}
	return result;
}

bool AssignStmtNode::typeAnalysis(DataType * returnType){
	DataType * subType = myAssign->typeAnalysis();
	return !subType->isError();
}

DataType * AssignNode::typeAnalysis(){
    //TODO: Note that this may be function is incomplete.
    // add if the lhs or rhs is an error
    DataType * lhsType = myExpLHS->typeAnalysis();
    DataType * rhsType = myExpRHS->typeAnalysis();
    if(lhsType->isError() || rhsType->isError())
    {
        return ErrorType::produce();
    }

    if(lhsType->getString().find("->") != std::string::npos &&
        rhsType->getString().find("->") != std::string::npos)
        {
            TypeErr::funcToFunc(myExpLHS->getLine(), myExpLHS->getCol());
            return ErrorType::produce();
        }

		if(lhsType->getString().find("[") != std::string::npos &&
        rhsType->getString().find("[") != std::string::npos)
        {
            TypeErr::arrayToArray(myExpLHS->getLine(), myExpLHS->getCol());
            return ErrorType::produce();
        }

    if(lhsType == rhsType)
    {
        return lhsType;
    }

    TypeErr::mismatch(myExpLHS->getLine(), myExpLHS->getCol());
    return ErrorType::produce();
	}

DataType * IdNode::typeAnalysis(){
	SemSymbol * mySym = getSymbol();
	if (mySym == NULL){
		throw InternalError("Id without a symbol");
	}
	return mySymbol->getType();
}

DataType * StrLitNode::typeAnalysis(){
	return PrimType::produce(BaseType::STR);
}

DataType * IntLitNode::typeAnalysis(){
	return PrimType::produce(BaseType::INT);
}

DataType * TrueNode::typeAnalysis(){
	return PrimType::produce(BaseType::BOOL);
}

DataType * FalseNode::typeAnalysis(){
	return PrimType::produce(BaseType::BOOL);
}

DataType * NotNode::typeAnalysis(){
	DataType* type = myExp->typeAnalysis();
	if(type->getString() == "bool")
	{
	return PrimType::produce(BaseType::BOOL);
	}
	if(type->isError())
	{
		return ErrorType::produce();
	}
	TypeErr::suckyLogic(myExp->getLine(), myExp->getCol());
	return ErrorType::produce();
}

DataType * UnaryMinusNode::typeAnalysis(){
    DataType* type = myExp->typeAnalysis();
    if(type->getString() == "int")
    {
    return PrimType::produce(BaseType::INT);
    }
    if(type->getString() =="ERROR")
    {
        return ErrorType::produce();
    }
    TypeErr::suckyMath(myExp->getLine(), myExp->getCol());
    return ErrorType::produce();
}


bool CallStmtNode::typeAnalysis(DataType * returnType){
	DataType * subType = myCallExp->typeAnalysis();
	return !subType->isError();
}

//----------------------------------------------------------------------------//
DataType * CallExpNode::typeAnalysis(){
	SemSymbol * checkKind = myId->getSymbol();
	if(checkKind->getKind() != SymbolKind::FN){
		TypeErr::callingNonFunc(myId->getLine(), myId->getCol());
		return ErrorType::produce();
	}
	DataType * theFn = checkKind->getType();

	std::list<DataType *> * formalsList = theFn->getFormals();
	std::list<ExpNode *> * ExpList = myExpList->getMyExp();
	int formalLength = formalsList->size();
	int expLength = myExpList->getMyExp()->size();
	if(expLength != formalLength)
	{
		TypeErr::wrongNumParam(myId->getLine(), myId->getCol());
		return ErrorType::produce();
	}

	std::list<DataType *>::iterator it1 = formalsList->begin();
	std::list<ExpNode *>::iterator it2 = ExpList->begin();
	DataType* tempToCheck = nullptr;
	bool check = true;
	for (int i = 0; i < formalLength; i++)
	{
		tempToCheck = (*it2)->typeAnalysis();
		if(tempToCheck != (*it1))
		{
			TypeErr::wrongParamType((*it2)->getLine(), (*it2)->getCol());
			check = false;
		}

		++it1;
		++it2;
	}

		return theFn->getRetType();


}

//----------------------------------------------------------------------------//
bool ReturnStmtNode::typeAnalysis(DataType * returnType){
    std::string myReturnType = returnType->getString();
    if(myReturnType == "ERROR")
    {
        return false;
    }

    if(myExp == NULL)
    {
				if(myReturnType == "void")
				{
					return true;
				}
        TypeErr::missingReturn(0,0);
        return false;
    }

    DataType * myExpType = myExp->typeAnalysis();
    if(myExpType->getString() == "ERROR")
    {
        return false;
    }

		std::string temp = myExpType->getString();
    if(myReturnType != temp)
    {

        if(myReturnType == "void")
        {

					TypeErr::returnFromVoid(myExp->getLine(), myExp->getCol());
					return false;
        }

				TypeErr::badReturnVal(myExp->getLine(), myExp->getCol());
				return false;

    }


    bool good = false;
    if(myReturnType == myExpType->getString())
    {
        good = true;
    }

    return good;
}

//----------------------------------------------------------------------------//
bool WhileStmtNode::typeAnalysis(DataType * returnType){
    DataType* type = myExp->typeAnalysis();
    bool returnBool = true;
		if(myStmts->typeAnalysis(returnType) == false)
		{
			return false;
		}
    if(type->getString() != "bool")
    {
        TypeErr::nonBoolInWhile(myExp->getLine(), myExp->getCol());
        returnBool = false;
    }
    return returnBool && myStmts->typeAnalysis(returnType);
}

//----------------------------------------------------------------------------//
bool IfStmtNode::typeAnalysis(DataType * returnType){
    DataType* type = myExp->typeAnalysis();
	  bool returnBool = true;

		if(myStmts->typeAnalysis(returnType) == false)
		{
			return false;
		}

    if(type->getString() != "bool")
    {
        TypeErr::nonBoolInIf(myExp->getLine(), myExp->getCol());
        returnBool = false;
    }
    return returnBool;
}

//----------------------------------------------------------------------------//
bool IfElseStmtNode::typeAnalysis(DataType * returnType){
    DataType* type = myExp->typeAnalysis();
		bool returnBool = true;
		if(myStmtsT->typeAnalysis(returnType) == false)
		{
			return false;
		}

		if(myStmtsF->typeAnalysis(returnType) == false)
		{
			return false;
		}

    if(type->getString() != "bool")
    {
        TypeErr::nonBoolInIf(myExp->getLine(), myExp->getCol());
        returnBool = false;
    }
    returnBool = returnBool && myStmtsT->typeAnalysis(returnType);
    returnBool = returnBool && myStmtsF->typeAnalysis(returnType);
    return returnBool;
}
//----------------------------------------------------------------------------//
bool WriteStmtNode::typeAnalysis(DataType * returnType){
    DataType * type = myExp->typeAnalysis();
    if(type->getString() == "ERROR")
    {
        return false;
    }

    if(type->getString().find("->") != std::string::npos)
    {
        TypeErr::writeFunc(myExp->getLine(), myExp->getCol());
        return false;
    }

    if(type->getString().find("[") != std::string::npos)
    {
        TypeErr::writeArrayVar(myExp->getLine(), myExp->getCol());
        return false;
    }

    if(type->getString() == "void")
    {
        TypeErr::writeVoid(myExp->getLine(), myExp->getCol());
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------//
bool ReadStmtNode::typeAnalysis(DataType * returnType){
    DataType * type = myExp->typeAnalysis();
    if(type->getString() == "ERROR")
    {
        return false;
    }

    if(type->getString().find("->") != std::string::npos)
    {
        TypeErr::readFunc(myExp->getLine(), myExp->getCol());
        return false;
    }

    if(type->getString().find("[") != std::string::npos)
    {
        TypeErr::readArray(myExp->getLine(), myExp->getCol());
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------//
bool PostIncStmtNode::typeAnalysis(DataType * returnType){
	DataType * rhsType = myExp->typeAnalysis();

	if(rhsType->isError())
	{
		return false;
	}

	if(rhsType->getString( )!= "int"){
		TypeErr::suckyMath(myExp->getLine(), myExp->getCol());
		return false;
	}

	return true;

}

//----------------------------------------------------------------------------//
bool PostDecStmtNode::typeAnalysis(DataType * returnType){
	DataType * rhsType = myExp->typeAnalysis();

	if(rhsType->isError())
	{
		return false;
	}

	if(rhsType->getString() != "int"){
		TypeErr::suckyMath(myExp->getLine(), myExp->getCol());
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------//
bool VarDeclNode::typeAnalysis(){
	// VarDecls always pass type analysis, since they
	// are never used in an expression
	return true;
}

//----------------------------------------------------------------------------//

DataType * PlusNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;


	if(lhsType->getBaseType() != BaseType::INT || rhsType->getBaseType() != BaseType::INT ){
		TypeErr::suckyMath(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::INT);
}

DataType * MinusNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;



	if(lhsType->getBaseType() != BaseType::INT || rhsType->getBaseType() != BaseType::INT ){
		TypeErr::suckyMath(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

		if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::INT);
}


DataType * TimesNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;



	if(lhsType->getBaseType() != BaseType::INT || rhsType->getBaseType() != BaseType::INT ){
		TypeErr::suckyMath(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::INT);
}


DataType * DivideNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;



	if(lhsType->getBaseType() != BaseType::INT || rhsType->getBaseType() != BaseType::INT ){
		TypeErr::suckyMath(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

		if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::BOOL);
}


DataType * AndNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;



	if(lhsType->getBaseType() != BaseType::BOOL || rhsType->getBaseType() != BaseType::BOOL ){
		TypeErr::suckyLogic(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

		if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::BOOL);
}

DataType * OrNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;



	if(lhsType->getBaseType() != BaseType::BOOL || rhsType->getBaseType() != BaseType::BOOL ){
		TypeErr::suckyLogic(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

		if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::BOOL);
}

DataType * EqualsNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != rhsType->getBaseType())
	{

		TypeErr::mismatch(myExp1->getLine(), myExp1->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getString().find("->") != std::string::npos)
	{
			TypeErr:: EqOpToFunc(myExp1->getLine(), myExp1->getCol());
			return ErrorType::produce();
	}

	if(lhsType->getString().find("[") != std::string::npos)
	{
			TypeErr:: EqOpToArray(myExp1->getLine(), myExp1->getCol());
			return ErrorType::produce();
	}

	if(lhsType->getBaseType() == BaseType::VOID || rhsType->getBaseType() == BaseType::VOID)
	{
		TypeErr::logicToVoidFuncs(myExp1->getLine(), myExp1->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != BaseType::BOOL || rhsType->getBaseType() != BaseType::BOOL ){
		TypeErr::suckyLogic(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::BOOL);
}

DataType * NotEqualsNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != rhsType->getBaseType())
	{


		TypeErr::mismatch(myExp1->getLine(), myExp1->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getString().find("->") != std::string::npos)
	{
			TypeErr:: EqOpToFunc(myExp1->getLine(), myExp1->getCol());
			return ErrorType::produce();
	}

	if(lhsType->getString().find("[") != std::string::npos)
	{
			TypeErr:: EqOpToArray(myExp1->getLine(), myExp1->getCol());
			return ErrorType::produce();
	}

	if(lhsType->getBaseType() == BaseType::VOID || rhsType->getBaseType() == BaseType::VOID)
	{
		TypeErr::logicToVoidFuncs(myExp1->getLine(), myExp1->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != BaseType::BOOL || rhsType->getBaseType() != BaseType::BOOL ){
		TypeErr::suckyLogic(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}



	return PrimType::produce(BaseType::BOOL);
}

DataType * LessNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;


	if(lhsType->getBaseType() != BaseType::INT || rhsType->getBaseType() != BaseType::INT ){
		TypeErr::suckyComparisons(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}


	return PrimType::produce(BaseType::INT);
}

DataType * GreaterNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;


	if(lhsType->getBaseType() != BaseType::INT || rhsType->getBaseType() != BaseType::INT ){
		TypeErr::suckyComparisons(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}


	return PrimType::produce(BaseType::INT);
}

DataType * LessEqNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;


	if(lhsType->getBaseType() != BaseType::INT || rhsType->getBaseType() != BaseType::INT ){
		TypeErr::suckyComparisons(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::INT);
}

DataType * GreaterEqNode::typeAnalysis(){
	DataType * lhsType = myExp1->typeAnalysis();
	DataType * rhsType = myExp2->typeAnalysis();

	if(lhsType->isError() || rhsType->isError())
	{
		return ErrorType::produce();
	}

	bool checker = true;


	if(lhsType->getBaseType() != BaseType::INT || rhsType->getBaseType() != BaseType::INT ){
		TypeErr::suckyComparisons(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	if(lhsType->getBaseType() != rhsType->getBaseType())
	{
		TypeErr::mismatch(myExp1->getLine(), myExp2->getCol());
		return ErrorType::produce();
	}

	return PrimType::produce(BaseType::INT);
}
//define rules in the plusnode
//fndecl fnbody type analysis we need symbol in order to acess return type later
}
