//
//  Errors.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 14/01/2019.
//  Copyright © 2019 Francis Pierot. All rights reserved.
//
//#include "pch.h"

#include <algorithm> // Required for std::sort

#include "Errors.h"
#include "CodeLine.h"
#include "Assembler.h"

namespace MUZ {
	using MUZ::ErrorKind;
	using MUZ::ErrorList;

	bool TestPass(CodeLine& codeline, int pass);


	// This construction avoids the "requires exit-time destructor" warning
	//MessageTextMap & messageText = *new MessageTextMap;
	std::map<ErrorKind,const char*> * MUZ::ErrorList::messageText = nullptr;
	int MUZ::ErrorList::arc = 0;

	ErrorList::ErrorList() {
		if (messageText == nullptr) {
			messageText = new std::map<ErrorKind,const char*>;
			(*messageText)[errorOK] = "no error";
			(*messageText)[errorUnknown] = "unknown error";
			(*messageText)[errorNonDerivedInstruction] = "SHOULD NOT OCCUR: Non derived Instruction class used (fatal)";
			(*messageText)[errorNonDerivedDirective] = "SHOULD NOT OCCUR: Non derived Directive class used (fatal)";
			(*messageText)[errorWritingListing] = "Cannot write listing file (about file) ";
			(*messageText)[errorOpeningSource] = "Cannot open source file: asm, hex or binary file not found";
			(*messageText)[errorElseNoIf] = "#ELSE without corresponding #IF/#IFDEF/#IFNDEF";
			(*messageText)[errorEndifNoIf] = "#ENDIF without #ELSE or #IF";
			(*messageText)[errorLabelExists] = "label re-defined later";
			(*messageText)[errorUnknownSyntax] = "line does not start with a label, a directive or an instruction";
			(*messageText)[errorUnknownDirective] = "directive starting with '.' or '#' is unknown";
			(*messageText)[errorUknownInstruction] = "an instruction should have been found, probable wrong syntax";
			(*messageText)[errorMUZNoSection] = "SHOULD NOT OCCUR: assembled code has no section";
			(*messageText)[warningMisplacedChar] = "a '.' or '#' was found in an unsusual place";
			(*messageText)[errorMissingComma] = "a ',' is missing in instruction operands";
			(*messageText)[errorWrongOperand1] = "first operand is wrong type";
			(*messageText)[errorWrongOperand2] = "second operand is wrong type";
			(*messageText)[errorWrongOperand3] = "third operand is wrong type";
			(*messageText)[errorWrongRegister] = "register name is not valid";
			(*messageText)[errorMissingParenthesisClose] = "a ')' is missing";
			(*messageText)[errorWrongCondition] = "A condition is invalid (e.g. JR PO,nn)";
			(*messageText)[errorNotRegister] = "expected register name was not found";
			(*messageText)[errorWrongComma] = "unexpected comma";
			(*messageText)[errorLeftOperandMissing] = "left operand missing in expression";
			(*messageText)[errorMissingToken] = "missing operands or punctuation";
			(*messageText)[errorDefine] = "#DEFINE could not define a symbol";
			(*messageText)[errorInvalidSymbol] = "invalid symbol name after DEFINE";
			(*messageText)[errorInvalidExpression] = "invalid expression after symbol";
			(*messageText)[errorFileSyntax] = "invalid syntax for file name";
			(*messageText)[errorProcessor] = "unsupported processor in .PROC";
			(*messageText)[warningUnsolvedExpression] = "a symbol was unsolved in an expression";
			(*messageText)[errorEquate] = ".EQU could not create label or assign value";
			(*messageText)[errorSet] = ".SET could not create label or assign value";
			(*messageText)[errorTooBigValue] = "number too big for accepted values";
			(*messageText)[errorTooBigBit] = "number too big for a bit number (0-7)";
			(*messageText)[warningTooBig8] = "number too big for 8 bits";
			(*messageText)[warningTooBig16] = "number too big for 16 bits";
			(*messageText)[warningTooFar] = "DJNZ or JR target is too far";
		}
		arc += 1;
	}

	ErrorList::~ErrorList() {
		arc -= 1;
		if (arc <= 0) {
			delete messageText;
			messageText = nullptr;
		}
	}

	void ErrorList::Clear() {
	}
	std::string ErrorList::GetMessage( ErrorKind kind ) {
		return (*messageText)[kind];
	}
	bool TestPass(CodeLine& codeline, int pass)
	{
		if (codeline.as == nullptr) return false;
		return pass == 1 ? codeline.as->IsFirstPass() : ! codeline.as->IsFirstPass();

	}
	void ErrorList::Info( ErrorKind kind, struct CodeLine& codeline, int pass) {
		if (TestPass(codeline,pass))
			push_back({ errorTypeINFO, kind, codeline.file, codeline.line, "", codeline.curtoken});
	}
	void ErrorList::Warning( ErrorKind kind, struct CodeLine& codeline, int pass) {
		if (TestPass(codeline,pass))
			push_back({ errorTypeWARNING, kind, codeline.file, codeline.line, "", codeline.curtoken});
	}
	void ErrorList::ForceWarning( ErrorKind kind, struct CodeLine& codeline) {
		push_back({ errorTypeWARNING, kind, codeline.file, codeline.line, "", codeline.curtoken});
	}
	void ErrorList::AboutFile( ErrorKind kind, struct CodeLine& codeline, std::string file, int pass) {
		if (TestPass(codeline,pass))
			push_back({ errorTypeABOUTFILE, kind, codeline.file, codeline.line, file, codeline.curtoken});
	}
	ErrorType ErrorList::Error( ErrorKind kind, struct CodeLine& codeline, int pass) {
		if (TestPass(codeline,pass))
			push_back({ errorTypeERROR, kind, codeline.file, codeline.line, "", codeline.curtoken});
		return errorTypeERROR;
	}
	ErrorType ErrorList::Error( ErrorKind kind, struct CodeLine& codeline, std::string file, int pass) {
		if (TestPass(codeline,pass))
			push_back({ errorTypeERROR, kind, codeline.file, codeline.line, file, codeline.curtoken});
		return errorTypeERROR;
	}
	ErrorType ErrorList::Fatal( ErrorKind kind, struct CodeLine& codeline, int pass) {
		if (TestPass(codeline,pass))
			push_back({ errorTypeFATAL, kind, codeline.file, codeline.line, "", codeline.curtoken});
		return errorTypeFATAL;
	}
	ErrorType ErrorList::Fatal( ErrorKind kind, struct CodeLine& codeline, std::string file, int pass) {
		if (TestPass(codeline,pass))
			push_back({ errorTypeFATAL, kind, codeline.file, codeline.line, file, codeline.curtoken});
		return errorTypeFATAL;
	}

	/** Close the list by sorting it and setting message references into codelines. */
	void ErrorList::Close(Assembler& as)
	{
		std::sort(begin(), end(), []( ErrorMessage& m1, ErrorMessage& m2) {
			if (m1.file < m2.file) return true;
			if (m1.file > m2.file) return false;
			return m1.line < m2.line;
		});

		for (size_t m = 0 ; m < size() ; m++) {
			ErrorMessage& msg = at(m);
			CodeLine* codeline = as.GetCodeLine(msg.file, msg.line);
			if (codeline) {
				codeline->message = (int)m;
			}
		}
	}
} // namespace
