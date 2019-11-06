#ifndef CRAP_DATA_TYPES
#define CRAP_DATA_TYPES

#include <list>
#include "err.hpp"

using Dims = std::list<int>;

namespace cRAP{

class PrimType;
class FnType;
class ArrayType;
class ErrorType;

enum BaseType{
	INT, VOID, BOOL, STR
};

class DataType{
public:
	virtual bool validVarType() = 0;
	virtual bool validRetType() = 0;
	virtual DataType * getRetType() = 0;
	virtual std::list<DataType *> * getFormals() = 0;

	static std::string baseTypeToString(BaseType baseType){
		switch(baseType){
			case BaseType::INT: return "int";
			case BaseType::VOID: return "void";
			case BaseType::BOOL: return "bool";
			case BaseType::STR: return "string";
		}
		throw new InternalError("Unknown Base Type");
	}
	virtual std::string getString() = 0;
	virtual BaseType getBaseType() = 0;
	virtual bool isError() { return false; }
	virtual DataType * indexType();
	virtual ArrayType * asArray(){ return NULL; }

protected:
};

class ErrorType : public DataType{
public:
	static ErrorType * produce(){
		static ErrorType * err = new ErrorType();
		return err;
	}
	virtual bool isError() override { return true; }
	virtual BaseType getBaseType() override {
		throw InternalError("No base type");
	}
	virtual bool validVarType() override { return false; }
	virtual bool validRetType() override { return false; }
	virtual std::string getString() { return "ERROR"; }
	DataType * getRetType() {return NULL;}
	std::list<DataType *> * getFormals() { return NULL;}

};

class PrimType : public DataType{
public:
	static PrimType * produce(BaseType base);
	virtual BaseType getBaseType() override { return myBaseType; }
	virtual bool validVarType() override;
	virtual bool validRetType() { return true; }
	virtual std::string getString();
	DataType * getRetType() {return NULL;}
	std::list<DataType *> * getFormals() { return NULL;}
private:
	PrimType(BaseType baseTypeIn) : myBaseType(baseTypeIn) {}
	BaseType myBaseType;
};

class FnType : public DataType{
public:
	FnType(std::list<DataType *> * formalsIn, DataType * retTypeIn)
	: DataType(),
	  myFormalTypes(formalsIn),
	  myRetType(retTypeIn)
	{
	}
	std::string getString() override{
		std::string result = "";
		bool first = true;
		for (DataType * arg : *myFormalTypes){
			if (first){ first = false; }
			else { result += ","; }
			result += arg->getString();
		}
		result += "->";
		result += myRetType->getString();
		return result;
	}
	BaseType getBaseType() override {
		throw InternalError("No base type");
	}
	bool validVarType() { return false; }
	bool validRetType() { return false; }
	DataType * getRetType() {return myRetType;}
	std::list<DataType *> * getFormals() { return myFormalTypes;}
	bool isFunc() {return true;}

private:
	std::list<DataType *> * myFormalTypes;
	DataType * myRetType;
};

class ArrayType : public DataType{
public:
	ArrayType(DataType * prefixType, int dim);
	std::string getString() override;
	bool validVarType() override;
	bool validRetType() override;
	BaseType getBaseType() override { return myBaseType; }
	DataType * indexType() { throw new ToDoError(); }
	virtual ArrayType * asArray() override { return this; }
	DataType * getRetType() {return NULL;}
	std::list<DataType *> * getFormals() { return NULL;}
private:
	BaseType myBaseType;
	Dims * myDims;
};

}

#endif
