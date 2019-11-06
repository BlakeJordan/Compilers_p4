#ifndef cRAP_ERROR_REPORTING_HH
#define cRAP_ERROR_REPORTING_HH

#include <iostream>
#include <string>

namespace cRAP{

class Err{
	public:
	static void report(const std::string msg){
		std::cerr << msg << std::endl;
	}
	static void semanticReport(
		size_t line,
		size_t col,
		const std::string msg
	){
		std::cerr << line << "," << col
			<< ": " << msg << std::endl;
	}

	static void syntaxReport(const std::string msg){
		cRAP::Err::report(" ***ERROR*** " + msg);
	}


};

class InternalError{
public:
	InternalError(const char * msgIn){
		msg = msgIn;
	}
	std::string what(){
		return msg;
	}
private:
	std::string msg;
};

class ToDoError{
public:
	ToDoError(){
		msg = "ToDo!";
	}
	ToDoError(std::string msgIn){
		msg = msgIn;
	}
	std::string what(){
		return msg;
	}
private:
	std::string msg;
};

//-----------------Program Errors-----------------/

class TypeErr{
public:
static bool undecl(size_t line, size_t col){
	Err::semanticReport(line, col, "Undeclared identifier");
	return false;
}
static bool multiDecl(size_t line, size_t col){
	Err::semanticReport(line, col, "Multiply declared identifier");
	return false;
}
static bool badArray(size_t line, size_t col){
	Err::semanticReport(line, col, "Invalid array index");
	return false;
}
static bool badVoid(size_t line, size_t col){
	Err::semanticReport(line, col, "Non-function declared void");
	return false;
}
static void mismatch(size_t line, size_t col){
	Err::semanticReport(line, col, "Type mismatch");
}
//TODO: Add the other type error reports

static void writeFunc(size_t line, size_t col){
	Err::semanticReport(line, col, "Attempt to write a function");
}

static void writeArrayVar(size_t line, size_t col){
	Err::semanticReport(line, col, "Attempt to write an array variable");
}

static void writeVoid(size_t line, size_t col){
	Err::semanticReport(line, col, "Attempt to write void");
}

static void readFunc(size_t line, size_t col){
	Err::semanticReport(line, col, "Attempt to read a function");
}

static void readArray(size_t line, size_t col){
	Err::semanticReport(line, col, "Attempt to read an array variable");
}

static void callingNonFunc(size_t line, size_t col){
	Err::semanticReport(line, col, "Attempt to call a non-function");
}

static void wrongNumParam(size_t line, size_t col){
	Err::semanticReport(line, col, "Function call with wrong number of args");
}

static void wrongParamType(size_t line, size_t col){
	Err::semanticReport(line, col, "Type of actual does not match type of formal");
}

static void missingReturn(size_t line, size_t col){
	Err::semanticReport(line, col, "Missing return value");
}

static void returnFromVoid(size_t line, size_t col){
	Err::semanticReport(line, col, "Return with a value in a void function");
}

static void badReturnVal(size_t line, size_t col){
	Err::semanticReport(line, col, "Bad return value");
}

static void suckyMath(size_t line, size_t col){
	Err::semanticReport(line, col, "Arithmetic operator applied to non-numeric operand");
}

static void suckyComparisons(size_t line, size_t col){
	Err::semanticReport(line, col, "Relational operator applied to non-numeric operand");
}

static void suckyLogic(size_t line, size_t col){
	Err::semanticReport(line, col, "Logical operator applied to non-bool operand");
}

static void nonBoolInIf(size_t line, size_t col){
	Err::semanticReport(line, col, "Non-bool expression used as an if condition");
}

static void nonBoolInWhile(size_t line, size_t col){
	Err::semanticReport(line, col, "Non-bool expression used as a while condition");
}

static void logicToVoidFuncs(size_t line, size_t col){
	Err::semanticReport(line, col, "Equality operator applied to void functions");
}

static void EqOpToFunc(size_t line, size_t col){
	Err::semanticReport(line, col, "Equality operator applied to functions");
}

static void EqOpToArray(size_t line, size_t col){
	Err::semanticReport(line, col, "Equality operator applied to array variables");
}

static void funcToFunc(size_t line, size_t col){
	Err::semanticReport(line, col, "Function assignment");
}

static void arrayToArray(size_t line, size_t col){
	Err::semanticReport(line, col, "Array variable assignment");
}

};

} //End namespace cRAP

#endif
