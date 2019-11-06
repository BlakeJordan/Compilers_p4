#include "types.hpp"
#include <list>
#include <sstream>

namespace cRAP{

ArrayType::ArrayType(DataType * parent, int dim){
	myBaseType = parent->getBaseType();
	myDims = new Dims();

	ArrayType * parentArray = parent->asArray();
	if (parentArray != NULL){
		Dims * pDims = parentArray->myDims;
		if (pDims != nullptr){
			myDims->insert(myDims->end(), 
				pDims->begin(), pDims->end());
		}
	}
	myDims->push_back(dim);
}

bool ArrayType::validRetType(){
	if (getBaseType() != BaseType::VOID){ return true; }
	return false;
}

bool ArrayType::validVarType(){
	if (getBaseType() != BaseType::VOID){ return true; }
	return false;
	
}

std::string ArrayType::getString(){
	std::string res = DataType::baseTypeToString(getBaseType());
	for (int dim : *myDims){
		res += "[";
		res += std::to_string(dim);
		res += "]";
	}
	return res;
}

PrimType * PrimType::produce(BaseType base){
	static PrimType * intFly = new PrimType(BaseType::INT);
	static PrimType * voidFly = new PrimType(BaseType::VOID);
	static PrimType * boolFly = new PrimType(BaseType::BOOL);
	static PrimType * strFly = new PrimType(BaseType::STR);

	switch (base){
		case BaseType::INT: return intFly;
		case BaseType::VOID: return voidFly;
		case BaseType::BOOL: return boolFly;
		case BaseType::STR: return strFly;
	}
	throw new InternalError("Unknown type " + base);
}

std::string PrimType::getString(){
	return baseTypeToString(myBaseType);
}

DataType * DataType::indexType(){
	return ErrorType::produce();
}

bool PrimType::validVarType(){
	if (myBaseType == BaseType::VOID) { return false; }
	return true;
}

} //End namespace
