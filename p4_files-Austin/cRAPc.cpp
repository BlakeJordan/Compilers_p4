#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "scanner.hpp"
#include "symbol_table.hpp"

using namespace cRAP;

static void usageAndDie(){
	std::cerr << "Usage: cRAPc <infile>"
	<< " [-t <tokensFile>]"
	<< " [-p <unparseFile>]"
	<< " [-n <nameAnalysisFile>]"
	<< " [-c <typeChecking>]"
	;
	exit(1);
}

static ProgramNode * parse(const char * inFile){
	std::ifstream inStream(inFile);
	if (!inStream.good()){
		std::string msg = "Bad input stream ";
		msg += inFile;
		throw new InternalError(msg.c_str());
	}

	cRAP::Scanner scanner(&inStream);
	ProgramNode * root = NULL;
	cRAP::Parser parser(scanner, &root);
	int errCode = parser.parse();
	if (errCode != 0){ return NULL; }

	return root;
}

static void unparse(ASTNode * astRoot, const char * outFile){
	if (outFile == nullptr){
		throw new InternalError("Null unparse file given");
	}
	if (strcmp(outFile, "--") == 0){
		astRoot->unparse(std::cout, 0);
	} else {
		std::ofstream outStream(outFile);
		astRoot->unparse(outStream, 0);
		outStream.close();
	}
}

int 
main( const int argc, const char **argv )
{
	if (argc == 0){
		usageAndDie();
	}
	const char * inFile = NULL;
	const char * tokensFile = NULL;
	const char * unparseFile = NULL;
	const char * nameAnalysisFile = NULL;
	bool doTypeChecking = false;
	int i = 1;
	for (int i = 1 ; i < argc ; i++){
		if (argv[i][0] == '-'){
			if (argv[i][1] == 't'){
				i++;
				tokensFile = argv[i];
			} else if (argv[i][1] == 'p'){
				i++;
				unparseFile = argv[i];
			} else if (argv[i][1] == 'n'){
				i++;
				nameAnalysisFile = argv[i];
			} else if (argv[i][1] == 'c'){
				doTypeChecking = true;
			} else {
				std::cerr << "Unrecognized argument: ";
				std::cerr << "argv[i]" << std::endl;
				usageAndDie();
			}
		} else {
			if (inFile == NULL){
				inFile = argv[i];
			} else {
				std::cerr << "Only 1 input file allowed";
				std::cerr << argv[i] << std::endl;
				usageAndDie();
			}
		}
	}
	if (inFile == NULL){
		usageAndDie();
	}

	int retCode = 0;

	if (tokensFile != NULL){
		std::ifstream inStream(inFile);
		if (!inStream.good()){
			std::cerr << "Bad input stream: " 
			<< inFile << std::endl;
			exit(1);
		}
		Scanner scanner(&inStream);
		scanner.outputTokens(tokensFile);
	}

	if (unparseFile != NULL){
		try {
			ASTNode * astRoot = parse(inFile);
			if (astRoot == NULL){
				std::cerr << "Parsing Error\n";
				exit(1);
			}
			unparse(astRoot, unparseFile);
		} catch (ToDoError * e){
			std::cerr << "ToDo: " << e->what() << std::endl;
			exit(1);
		}
	}
	
	if (nameAnalysisFile != NULL){
		try {
			ASTNode * astRoot = parse(inFile);
			if (astRoot == NULL){
				std::cerr << "Parsing Error\n";
				exit(1);
			}
			SymbolTable * symTab = new SymbolTable();
			if (astRoot->nameAnalysis(symTab)){
				unparse(astRoot, nameAnalysisFile);
			}
		} catch (ToDoError * e){
			std::cerr << "ToDo: " << e->what() << std::endl;
			exit(1);
		}
	}
	if (doTypeChecking){
		try {
			ProgramNode * astRoot = parse(inFile);
			if (astRoot == NULL){
				std::cerr << "Parsing Error\n";
				exit(1);
			}
			SymbolTable * symTab = new SymbolTable();
			if (!astRoot->nameAnalysis(symTab)){
				std::cerr << "Name Analysis Error\n";
				exit(1);
			}
			if (!astRoot->typeAnalysis()){
				std::cerr << "Type Analysis Error\n";
				exit(1);
			}
		} catch (ToDoError * e){
			std::cerr << "Caught ToDo Error: " 
				<< e->what() << std::endl;
			exit(1);
		}
	}

	return retCode;
}
