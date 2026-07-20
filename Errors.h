//
//  Errors.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 04/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef Errors_h
#define Errors_h

#include <vector>
#include <map>
#include <string>

namespace MUZ {

	/** Errors for the OperandTools functions. The tools are processor specific (e.g. MUZ::Z80::OperandTools) but
	 the error codes are generic and accessible to non-Z80 sources (parser, assembler...)
	 */
	enum OperandError {
		
		operrOK,
		operrMISSINGPAROPEN,		// missing opened parenthesis
		operrMISSINGPARCLOSE,		// missing closed parenthesis
		operrMISSINGCOMMA,			// missing comma between arguments
		operrTOKENNUMBER,			// wrong token number
		operrREGISTERNAME,			// unknown register name
		operrWRONGREGISTER,			// wrong register used, like LD H,L
		operrWRONGOP,				// wrong operator, like (IX-2)
		operrNOTNUMBER,				// a number was needed
		operrNOTBIT,				// value too big for a bit number
		operrUNSOLVED,				// unsolved symbol in expression
		operrNOTSTRING,				// token is not a string or letters
		operrNOTCONDITION,			// token is not a condition name
		operrTOOBIG,				// too big for 8 or 16 bits
		operrNOTREGISTER,			// register name not found
	};
	
	/** Error type for the errors/warnings list. */
	enum ErrorType {
		errorTypeOK,		// no error
		errorTypeINFO,		// warmless informations
		errorTypeWARNING,	// doesn't break assembly
		errorTypeABOUTFILE,	// Warning or error about a filename (#include, #insertbin, #inserthex)
		errorTypeERROR,		// doesn't generate code but can continue assembly with possibly wrong addresses
		errorTypeFATAL,		// breaks assembly
		// Parser boolean equivalents for IF family Parse() returns
		errorTypeTRUE,
		errorTypeFALSE,
	};
	
	/** Error codes for the errors/warnings list. */
	enum ErrorKind {
		errorOK,						// no error
		errorUnknown,					// unknown error
		errorNonDerivedInstruction,		// SHOULD NOT OCCUR: Non derived Instruction class used (fatal)
		errorNonDerivedDirective,		// SHOULD NOT OCCUR: Non derived Directive class used (fatal)
		errorWritingListing,			// Cannot write listing file (about file)
		errorOpeningSource,				// "cannot open source file" asm, hex or binary file not found
		errorElseNoIf,					// #ELSE without corresponding #IF/#IFDEF/#IFNDEF
		errorEndifNoIf,					// #ENDIF without #ELSE or #IF
		errorLabelExists,				// A label already exists with the same name
		errorUnknownSyntax,				// line does not start with a label, a directive or an instruction
		errorUnknownDirective,			// directive starting with '.' or '#' is unknown
		errorUknownInstruction,			// an instruction should have been found, probable wrong syntax
		errorMUZNoSection,				// SHOULD NOT OCCUR: assembled code has no section
		warningMisplacedChar,			// a '.' or '#' was found in an unsusual place
		
		// general syntax
		errorMissingComma,				// a ',' is missing in instruction operands
		errorWrongOperand1,				// first operand is wrong type
		errorWrongOperand2,				// second operand is wrong type
		errorWrongOperand3,				// third operand is wrong type
		errorWrongRegister,				// register name is not valid
		errorMissingParenthesisClose,	// a ')' is missing
		errorWrongCondition,			// A condition is invalid (e.g. JR PO,nn)
		errorNotRegister,				// expected register name was not found
		errorWrongComma,				// unexpected comma
		errorLeftOperandMissing,		// left operand missing in expression
		errorMissingToken,				// token missing

		// errors detected by directives
		errorDefine,					// #DEFINE could not define a symbol
		errorInvalidSymbol,				// invalid symbol name after DEFINE
		errorInvalidExpression,			// invalid expression after symbol
		errorFileSyntax,				// invalid syntax for file name
		errorProcessor,					// unsupported processor in .PROC
		warningUnsolvedExpression,		// a symbol was unsolved in an expression
		errorEquate,					// .EQU could not create label or assign value
		errorSet,						// .SET could not create label or assign value

		// errors on numbers
		errorTooBigValue,				// number too big for accepted values
		errorTooBigBit,					// number too big for a bit number (0-7)
		warningTooBig8,					// number too big for 8 bits
		warningTooBig16,				// number too big for 16 bits
		warningTooFar,					// DJNZ or JR target is too far

	};
	
	struct ErrorMessage {
		ErrorType type=errorTypeINFO;			// (see enum above): info, warning, about a file, serious error or fatal error
		ErrorKind kind=errorUnknown;			// (see enum above): what kind of error it is
		size_t file;								// code file where it occured
		size_t line;								// code line in code file
		std::string filename="";				// relevant file name, or empty()
		size_t token = (size_t)(-1);						// relevant token index, or (size_t)(-1)
	};
	
	class ErrorList : public std::vector<ErrorMessage>
	{
		static std::map<ErrorKind,const char*> * messageText ;
		static int arc;
	public:
		ErrorList();
		~ErrorList();
		/** Clears the message list. */
		void Clear();
		/** Returns a message text for an error code (kind). */
		static std::string GetMessage( ErrorKind kind ) ;
		/** Stores an information message if the Assembler is doing Pass 1. */
		void Info( ErrorKind kind, struct CodeLine& codeline, int pass = 1) ;
		/** Stores a warning message if the Assembler is doing Pass 1. */
		void Warning( ErrorKind kind, struct CodeLine& codeline, int pass = 1);
		/** Stores a warning message. Ignore the assembler pass. */
		void ForceWarning( ErrorKind kind, struct CodeLine& codeline);
		/** Stores a message about a file name if the Assembler is doing Pass 1. */
		void AboutFile( ErrorKind kind, struct CodeLine& codeline, std::string filename, int pass = 1);
		/** Stores an error message if the Assembler is doing Pass 1. */
		ErrorType Error( ErrorKind kind, struct CodeLine& codeline, int pass = 1);
		/** Stores an error message if the Assembler is doing Pass 1. */
		ErrorType Error( ErrorKind kind, struct CodeLine& codeline, std::string filename, int pass = 1) ;
		/** Stores a fatal error message if the Assembler is doing Pass 1. */
		ErrorType Fatal( ErrorKind kind, struct CodeLine& codeline, int pass = 1);
		/** Stores a fatal error message if the Assembler is doing Pass 1. */
		ErrorType Fatal( ErrorKind kind, struct CodeLine& codeline, std::string filename, int pass = 1) ;


		/** Close the list by sorting it and setting message references into codelines. */
		void Close(class Assembler& as);
	};
	
} // namespace MUZ

#endif /* Errors_h */
