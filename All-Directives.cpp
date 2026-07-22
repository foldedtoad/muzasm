//
//  All-Directives.cpp
//  MUZ-Assembler
//
//  Created by Francis Pierot on 03/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "Section.h"
#include "Parser.h"
#include "All-Directives.h"

namespace MUZ {

	// General utility functions to analyze CodeLine tokens

	/** Returns true if current token is a comma, and go next token. */
	bool GetComma(CodeLine& codeline) {
		if (!EnoughTokensLeft(codeline,1)) return false;
		if (codeline.tokens.at((size_t)codeline.curtoken).type == tokenTypeCOMMA) {
			codeline.curtoken += 1;
			return true;
		}
		return false;
	}
	
	/** Returns true if the tokens array has at least the resquested number of tokens available starting at curtoken. */
	bool EnoughTokensLeft(CodeLine& codeline, int number) {
		// if curtoken is 'i' and we request 3 tokens,
		// then the tokens size must be 'i' + 3 or more
		return (codeline.tokens.size() >= codeline.curtoken + (size_t)number) ;
	}
	

	//MARK: - Preprocessor directives (#)
	
	/** #DEFINE <symbol> <stringexpression>
	 	and
	 	#DEFINE <symbol>
	 	returns true if the new symbol has been defined
	 */
	ErrorType DirectiveDEFINE::Parse( Assembler& as, Parser& parser, CodeLine& codeline, Label* , ErrorList& msg) {

		if (!parser.ExistMoreToken(1)) return msg.Error(errorMissingToken, codeline);
		ParseToken& symbol = parser.NextToken();
		if (symbol.type != tokenTypeLETTERS) {
			return msg.Error(errorInvalidSymbol, codeline);
		}
		// skip directive
		parser.JumpTokens(1);
		// resolve expression parts
		parser.ResolveNextSymbols(false); // skips next symbol before resolving
		
		// undefine the symbol if it exists
		as.DeleteDefSymbol(symbol.source);
		// create the #DEFINE symbol
		std::string value;
		if (parser.ExistMoreToken(1)) {
			parser.JumpNextToken();// skip symbol
			// convert letters to strings
			try { parser.EvaluateString(value); }
			catch (... /*const std::exception & e*/) {
				return msg.Error(errorInvalidExpression, codeline);
			}
		}
		DefSymbol* defsymbol = as.CreateDefSymbol(symbol.source, value);
		if (defsymbol) {
			codeline.defsymbol = symbol.source;
			return errorTypeOK;
		}
		return msg.Error(errorDefine, codeline);
	}
	/** Identifies a source string as self. */
	bool DirectiveDEFINE::Identify( std::string source )
	{
		std::string upper = std::to_upper(source);
		if (upper=="DEFINE") return true;
		return false;
	}

	/** #UNDEF <symbol>
	 	returns true if an existing symbol has been undefined
	 */
	ErrorType DirectiveUNDEFINE::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		ParseToken& symbol = parser.NextToken();
		if (symbol.type != tokenTypeLETTERS) {
			return msg.Error(errorInvalidSymbol, codeline);
			}
		// Delete the symbol : no symbol resolving or the symbol to undefine would be replaced by its value
		as.DeleteDefSymbol(symbol.source);
		parser.JumpTokens(1);
		return errorTypeOK;
	}
	
	/** #IFDEF <symbol>
	 	returns true if the symbol is defined
	 */
	ErrorType DirectiveIFDEF::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		// and now check existence of the symbol
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		std::string symbol;
		parser.JumpNextToken();
		try { parser.EvaluateString(symbol); }
		catch (... /*const std::exception & e*/) {
			return msg.Error(errorInvalidExpression, codeline);
		}
		return as.ExistDefSymbol(symbol) ? errorTypeTRUE : errorTypeFALSE;
	}
	/** Identifies a source string as self. */
	bool DirectiveIFDEF::Identify( std::string source )
	{
		std::string upper = std::to_upper(source);
		if (upper=="IFDEF") return true;
		return false;
	}

	/** #IFNDEF <symbol>
	 	returns true if the symbol is not defined
	 */
	ErrorType DirectiveIFNDEF::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		std::string symbol;
		parser.JumpNextToken();
		try { parser.EvaluateString(symbol); }
		catch (... /*const std::exception & e*/) {
			return msg.Error(errorInvalidExpression, codeline);
		}
		return as.ExistDefSymbol(symbol) ? errorTypeFALSE : errorTypeTRUE;
	}
	/** Identifies a source string as self. */
	bool DirectiveIFNDEF::Identify( std::string source )
	{
		std::string upper = std::to_upper(source);
		if (upper=="IFNDEF") return true;
		return false;
	}

	/** #IF <expression>
	 returns true if the expression is true (or not "" and not 0)
	 */
	ErrorType DirectiveIF::Parse(class Assembler& , Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		if ( ! parser.ExistMoreToken(1) )  return msg.Error(errorMissingToken, codeline);
		parser.ResolveNextSymbols(true); // use '*' joker
		parser.JumpTokens(1);
		bool b = false;
		try { parser.EvaluateBoolean(b); }
		catch (... /*const std::exception & e*/) {
			return msg.Error(errorInvalidExpression, codeline);
		}
		return b ? errorTypeTRUE : errorTypeFALSE;
	}
	/** Identifies a source string as self. */
	bool DirectiveIF::Identify( std::string source )
	{
		std::string upper = std::to_upper(source);
		if (upper=="IF") return true;
		if (upper=="COND") return true;
		return false;
	}

	/** Identifies a source string as self. */
	bool DirectiveELSE::Identify( std::string source )
	{
		std::string upper = std::to_upper(source);
		if (upper=="ELSE") return true;
		return false;
	}

	/** Identifies a source string as self. */
	bool DirectiveENDIF::Identify( std::string source )
	{
		std::string upper = std::to_upper(source);
		if (upper=="ENDIF") return true;
		if (upper=="ENDC") return true;
		return false;
	}

	/** #INCLUDE <file>
	 	returns true if a file must be included
	 */
	ErrorType DirectiveINCLUDE::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		parser.ResolveNextSymbols(false);
		ParseToken& filetoken = parser.JumpNextToken();
		// and include new file
		if ((filetoken.type != tokenTypeSTRING) && (filetoken.type != tokenTypeFILENAME)) {
			return msg.Error(errorInvalidSymbol, codeline);
		}
		// trim spaces at the end of filename
		strtrimright(filetoken.source);
		ErrorType result = errorTypeFALSE;
		if (as.IsFirstPass()) {
			codeline.includefile = as.m_files.size();
			result = as.AssembleIncludedFilePassOne(filetoken.source, codeline, msg);
		} else {
			result = as.AssembleIncludedFilePassTwo(filetoken.source, codeline, msg);
		}
		return result;
	}

	/** #INSERTHEX <file>
	  */
	ErrorType DirectiveINSERTHEX::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		parser.ResolveNextSymbols(false);
		ParseToken& filetoken = parser.NextToken();
		// and include new file
		if ((filetoken.type != tokenTypeSTRING) && (filetoken.type != tokenTypeFILENAME)) {
			return msg.Error(errorFileSyntax, codeline);
		}
		// trim spaces at the end of filename
		strtrimright(filetoken.source);
		if (as.IsFirstPass()) {
			codeline.includefile = as.m_files.size();
		}
		if (as.AssembleHexFile(filetoken.source, codeline, msg)) {
		}
		
		// tells the parser that the file must be included
		parser.JumpTokens(1);
		return errorTypeOK;
	}
	/** #INSERTBIN <file>
	 */
	ErrorType DirectiveINSERTBIN::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		parser.ResolveNextSymbols(false);
		ParseToken& filetoken = parser.NextToken();
		// and include new file
		if ((filetoken.type != tokenTypeSTRING) && (filetoken.type != tokenTypeFILENAME)) {
			return msg.Error(errorFileSyntax, codeline);
		}
		// trim spaces at the end of filename
		strtrimright(filetoken.source);
		if (as.IsFirstPass()) {
			codeline.includefile = as.m_files.size();
		}
		if (as.AssembleBinFile(filetoken.source, codeline, msg)) {
		}
		
		// tells the parser that the file must be included
		parser.JumpTokens(1);
		return errorTypeOK;
	}

	/** #NOLIST
	 */
	ErrorType DirectiveLISTOFF::Parse(class Assembler& as, Parser& , CodeLine& , class Label* , ErrorList& ) {
		as.EnableListing(false);
		return errorTypeOK;
	}
	/** #LIST [ON|OFF]
	 */
	ErrorType DirectiveLIST::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		bool enable = true;
		if (parser.ExistMoreToken(1)) {
			std::string value;
			parser.JumpNextToken();
			try { parser.EvaluateString(value); }
			catch (... /*const std::exception & e*/) {
				return msg.Error(errorInvalidExpression, codeline);
			}
			enable = (std::to_upper(value) == "OFF") ? false : true;
		}
		as.EnableListing(enable);
		// always list this line if it is not #LIST OFF
		if (enable) codeline.listing = true;
		return errorTypeOK;
	}

	//MARK: - Assembler directives (.)
	
	/** .PROC Z80
	 	Sets the instructions set for the Z80 processor.
	 	returns true if Z80 has been specified
	 */
	ErrorType DirectivePROC::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		parser.ResolveNextSymbols(false);// allow string expressions
		ParseToken& proc = parser.NextToken();
		if (! as.isKnownProcessor(proc.source)) {
			return msg.Error(errorProcessor, codeline);
			}
		// sets the instructions set into the assembler
		as.SetInstructions(proc.source);
		parser.JumpTokens(1);
		return errorTypeOK;
	}
	
	/** [label:] .CODE [name]
	 */
	ErrorType DirectiveCODE::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		std::string name;
		if (parser.ExistMoreToken(1)) {
			parser.JumpNextToken();
			try { parser.EvaluateString(name); }
			catch (... /*const std::exception & e*/) {
				return msg.Error(errorInvalidExpression, codeline);
			}
		}
		as.SetCodeSection( name );
		return errorTypeOK;
	}
	
	/** [label:] .DATA [name] [,SAVE]
	 */
	ErrorType DirectiveDATA::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		std::string name;
		bool save = false;
		if (parser.ExistMoreToken(1)) {
			ParseToken& token = parser.JumpNextToken();
			if (token.type != tokenTypeCOMMA) {
				try { parser.EvaluateString(name); }
				catch (... /*const std::exception & e*/) {
					return msg.Error(errorInvalidExpression, codeline);
				}
			}
			if (GetComma(codeline)) {
				std::string param;
				try { parser.EvaluateString(param); }
				catch (... /*const std::exception & e*/) {
					return msg.Error(errorInvalidExpression, codeline);
				}
				param = std::to_upper(param);
				if (param == "SAVE") {
					save = true;
				}
			}
		}
		as.SetDataSection( name, save );
		return errorTypeOK;
	}
	
	/** Sets the current address in the current section.
	 [label:] .ORG <value>
	 	returns true if the assembling address must be changed
	 */
	ErrorType DirectiveORG::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg) {
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		/*std::vector<size_t> unsolved =*/ parser.ResolveNextSymbols(false);
		ADDRESSTYPE address = 0;
		parser.JumpTokens(1); // skip after .EQU
		try { parser.EvaluateAddress(address); }
		catch (... /*const std::exception & e*/) {
			return msg.Error(errorInvalidExpression, codeline);
		}
		if (label == nullptr)
			label = codeline.label;
		if (label && as.IsFirstPass()) {
			label->SetAddress(address);
			codeline.label = label;
		}
		Section* section= as.GetSection();
		if (section) {
			section->SetOrg(address);
		}
		return errorTypeOK;
	}
	
	/** label[:]  [.]EQU <expression>
	 	returns true if the label has been created with the value as a decimal number
	 */
	ErrorType DirectiveEQU::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg)
	{
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		std::vector<size_t> unsolved = parser.ResolveNextSymbols(false);
		DWORD number = 0;
		if (unsolved.size() > 0 && ! as.IsFirstPass()) {
			msg.ForceWarning(warningUnsolvedExpression, codeline);
		}
		// compute address, unsolved symbols have been replaced by "0"
		parser.JumpTokens(1); // skip after .EQU
		try { parser.EvaluateInteger(number); }
		catch (... /*const std::exception & e*/) {
			return msg.Error(errorInvalidExpression, codeline);
		}
		// use previous label if none on this line
		if (label == nullptr)
			label = codeline.label;
		// set label address/value
		if (label) {
			label->Equate(number);
			codeline.label = label;
			return errorTypeOK;
		}
		return msg.Error(errorEquate, codeline);
	}

	/** Identifies a source string as self. */
	bool DirectiveEQU::Identify( std::string source )
	{
		std::string upper = std::to_upper(source);
		if (upper==".EQU") return errorTypeOK;
		if (upper=="EQU") return errorTypeOK;

		return false;
	}

	/** label[:]  [.]SET <expression>
		returns true if the label has been updated with the value as a decimal number
	 */
	ErrorType DirectiveSET::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg)
	{
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		std::vector<size_t> unsolved = parser.ResolveNextSymbols(false);
		DWORD number = 0;
		if (unsolved.size() > 0 && !as.IsFirstPass()) {
			msg.ForceWarning(warningUnsolvedExpression, codeline);
		}
		// compute address, unsolved symbols have been replaced by "0"
		parser.JumpTokens(1); // skip after .EQU
		try { parser.EvaluateInteger(number); }
		catch (... /*const std::exception & e*/) {
			return msg.Error(errorInvalidExpression, codeline);
		}
		// use previous label if none on this line
		if (label == nullptr)
			label = codeline.label;
		// set label address/value
		if (label) {
			label->Equate(number);
			codeline.label = label;
			return errorTypeOK;
		}
		return msg.Error(errorSet, codeline);
	}

	/** Identifies a source string as self. */
	bool DirectiveSET::Identify(std::string source)
	{
		std::string upper = std::to_upper(source);
		// WARNING: In the original code, the .SET directive and SET instruction collide.
		if (upper == ".SET") return errorTypeOK;
		//if (upper == "SET") return errorTypeOK;
		return false;
	}
	
	/** .DB <num8> [, <num8> [...]]
	 	Encode the given bytes into the codeline.
		Accepts one or more :
			- 8-bit number
			- character string
			- HEXCHAR <num8>
	 */
	ErrorType DirectiveBYTE::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg)
	{
		if (!parser.ExistMoreToken(1)) return msg.Error(errorMissingToken, codeline);
		parser.ResolveNextSymbols(false);

		while (parser.ExistMoreToken(1)) {
			ParseToken& token = parser.JumpNextToken();
			if (token.type == tokenTypeSTRING) {
				std::string result;
				try { parser.EvaluateString(result); }
				catch (... /*const std::exception & e*/) {
					return msg.Error(errorInvalidExpression, codeline);
				}
				// store each byte off the string
				for (auto c: result) {
					codeline.AddCode((DATATYPE)c);
				}
			} else if (token.type == tokenTypeOP_HEXCHAR) {
				ADDRESSTYPE address;
				token = parser.JumpNextToken();
				try { parser.EvaluateAddress(address); }
				catch (... /*const std::exception & e*/) {
					return msg.Error(errorInvalidExpression, codeline);
				}
				if ((address > 255)  && ! as.IsFirstPass()) {
					msg.ForceWarning(warningTooBig8, codeline);
				}
				if (address > 15) {
					codeline.AddCode( byte_to_hexchar((address & 0xF0) >> 4));
				}
				codeline.AddCode( byte_to_hexchar(address & 0xF) );

			} else if (token.type == tokenTypeDECNUMBER) {
				ADDRESSTYPE address;
				try { parser.EvaluateAddress(address); }
				catch (... /*const std::exception & e*/) {
					return msg.Error(errorInvalidExpression, codeline);
				}
				if ((address > 255)  && ! as.IsFirstPass()) {
					msg.ForceWarning(warningTooBig8, codeline);
				}
				codeline.AddCode((DATATYPE)(address & DATAMASK));
			} else if (token.type == tokenTypeLETTERS && token.unsolved) {
				codeline.AddCode(0);
				if ( ! as.IsFirstPass()) {
					msg.ForceWarning(warningUnsolvedExpression, codeline);
				}
			}
			if (parser.ExistMoreToken(1)) {
				ParseToken& tokencomma = parser.NextToken(0);// (0) = current token 
				if (tokencomma.type == tokenTypeCOMMA) {
					continue;
				}
			}
			break;			
		}

		
		return errorTypeOK;
	}
	
	ErrorType DirectiveWORD::Parse(class Assembler& , Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg)
	{
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		parser.ResolveNextSymbols(false);
		
		while (parser.ExistMoreToken(1)) {
			ParseToken& token = parser.JumpNextToken();
			if (token.type == tokenTypeSTRING) {
				std::string result;
				try { parser.EvaluateString(result); }
				catch (... /*const std::exception & e*/) {
					return msg.Error(errorInvalidExpression, codeline);
				}
				// store each byte off the string
				for (auto c: result) {
					codeline.AddCode((DATATYPE)c);
				}
			} else if (token.type == tokenTypeDECNUMBER) {
				ADDRESSTYPE address;
				try { parser.EvaluateAddress(address); }
				catch (... /*const std::exception & e*/) {
					return msg.Error(errorInvalidExpression, codeline);
				}
				codeline.AddCode(address & 0xFF, address >> 8);
			} else if (token.type == tokenTypeLETTERS && token.unsolved) {
				codeline.AddCode(0,0);
			}
			if (parser.ExistMoreToken(1)) {
				ParseToken& tokencomma = parser.NextToken(0);// (0) = current token
				if (tokencomma.type == tokenTypeCOMMA) {
					continue;
				}
			}
			break;
		}

		
		return errorTypeOK;
	}
	
	
	ErrorType DirectiveSPACE::Parse(class Assembler& , Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg)
	{
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		parser.ResolveNextSymbols(false);
		ParseToken& token = parser.NextToken();
		DWORD size = token.asInteger();
		for (DWORD i = 0 ; i < size ; i++) {
			codeline.AddCode(0xFF);
		}
		return errorTypeOK;
	}

	/** .END
	 */
	ErrorType DirectiveEND::Parse(class Assembler& as, Parser& , CodeLine& , class Label*, ErrorList& ) {
		as.Terminate();
		return errorTypeOK;
	}
	/** Small Computer Workshop 2019-09-07 and LCD alphanumeric sample compatibility */

	/** #REQUIRES <symbol>
	returns true if the new symbol has been stored in required symbols
 */
	ErrorType DirectiveREQUIRES::Parse(Assembler& as, Parser& parser, CodeLine& codeline, Label*, ErrorList& msg) {

		if (!parser.ExistMoreToken(1)) return msg.Error(errorMissingToken, codeline);
		ParseToken& symbol = parser.NextToken();
		if (symbol.type != tokenTypeLETTERS) {
			return msg.Error(errorInvalidSymbol, codeline);
		}
		// skip directive
		parser.JumpTokens(1);
		// resolve expression parts
		parser.ResolveNextSymbols(false); // skips next symbol before resolving

		// undefine the symbol if it exists
		as.DeleteReqSymbol(symbol.source);
		// create the #REQUIRES symbol
		DefSymbol* reqsymbol = as.CreateReqSymbol(symbol.source);
		if (reqsymbol) {
			codeline.defsymbol = symbol.source;
			return errorTypeOK;
		}
		return msg.Error(errorDefine, codeline);
	}
	/** Identifies a source string as self. */
	bool DirectiveREQUIRES::Identify(std::string source)
	{
		std::string upper = std::to_upper(source);
		if (upper == "REQUIRES") return true;
		return false;
	}

	/** #IFREQUIRED <symbol>
	returns true if the symbol is required
 */
	ErrorType DirectiveIFREQUIRED::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label*, ErrorList& msg) {
		// and now check existence of the symbol
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		std::string symbol;
		parser.JumpNextToken();
		try { parser.EvaluateString(symbol); }
		catch (... /*const std::exception & e*/) {
			return msg.Error(errorInvalidExpression, codeline);
		}
		return as.ExistReqSymbol(symbol) ? errorTypeTRUE : errorTypeFALSE;
	}
	/** Identifies a source string as self. */
	bool DirectiveIFREQUIRED::Identify(std::string source)
	{
		std::string upper = std::to_upper(source);
		if (upper == "IFREQUIRED") return true;
		return false;
	}

	/** .HEXBYTES <n>
	Sets the number of bytes in HEX output lines.
	 */
	ErrorType DirectiveHEXBYTES::Parse(class Assembler& as, Parser& parser, CodeLine& codeline, class Label* , ErrorList& msg) {
		if (!parser.ExistMoreToken(1))  return msg.Error(errorMissingToken, codeline);
		/*std::vector<size_t> unsolved =*/ parser.ResolveNextSymbols(false);
		ADDRESSTYPE address = 0;
		parser.JumpTokens(1); // skip after .HEXBYTES
		try { parser.EvaluateAddress(address); }
		catch (... /*const std::exception & e*/) {
			return msg.Error(errorInvalidExpression, codeline);
		}
		as.SetHexBytes(address & 0xFF);
		return errorTypeOK;
	}

} // namespace MUZ
