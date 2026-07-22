//
//  ParseToken.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 02/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "Parser.h"
#include "Expression.h"
#include "CodeLine.h"
#include "Assembler.h"
#include "All-Directives.h"

using std::string;
using std::vector;
using std::map;

namespace MUZ {

	
	//MARK: private functions
	
	/** Checks if a character would act as a token separator. Used when parsing names or numbers. */
	bool Parser::isSeparator(char cc) {
		if (cc == ' ') return true;
		if (cc == '\t') return true;
		if (cc == ':') return true;
		if (cc == ';') return true;
		if (cc == ',') return true;
		if (cc == '+') return true;
		if (cc == '*') return true;
		if (cc == '/') return true;
		if (cc == '=') return true;
		if (cc == '^') return true;
		if (cc == '<') return true;
		if (cc == '>') return true;
		if (cc == '!') return true;
		if (cc == '$') return true;
		if (cc == '(') return true;
		if (cc == ')') return true;
		if (cc == '%') return true;
		if (cc == '&') return true;
		if (cc == '|') return true;
		return false;
	}
	

	/** Checks if current and next characters can combine into the given operator and if so, store it */
	bool Parser::find2CharsOperator(std::string token, TokenType tokentype) {
		if (c == token.at(0) && hasNext && nextc == token.at(1)) {
			StoreToken();
			type = tokentype;
			word = token;
			StoreToken();
			pos += 1; // skip the second character
			return true;
		}
		return false;
	}
	
	/** Checks if current character match the given operator and if so, store it */
	bool Parser::findOperator(char token, TokenType tokentype) {
		if (c == token) {
			StoreToken();
			type = tokentype;
			word = token;
			StoreToken();
			return true;
		}
		return false;
	}
	
	/** Checks if current and next characters are hex digits only until non letter.
	 @param skip number of character to ignore, including the current one
	 @return true if an hex number without 'h' suffix has been detected
	 */
	bool Parser::findHexNumberNoSuffixSkip(size_t skip ) {
		size_t nextpos = (size_t)pos + skip - 1; // incremented at first loop
		bool onlyhex = true;
		char nextchar=0;
		size_t count = 0;
		const size_t sourcesize = source->size();
		while (onlyhex && (nextpos + 1 < sourcesize)) {
			nextpos += 1;
			nextchar = source->at(nextpos);
			char uppernextchar = upperchar(nextchar);
			if (nextchar >= '0' && nextchar<= '9') { count += 1; continue; }
			if (uppernextchar >= 'A' && uppernextchar <= 'F')  { count += 1; continue; }
			if (uppernextchar >= 'G' && uppernextchar <= 'Z') return false; // another letter, don't accept
			// any other character is probably a separator so if we have at least one digit it's a non-suffixed hex number
			return (count >= 1);
		}
		return onlyhex; // all hex digits and no sufffix
	}
	
	/** Checks if current and next characters are hex digits only until 'h' suffix.
	 @param skip number of character to ignore including the current one
	 @return true if a possible hex number has been detected
	 */
	bool Parser::findHexNumberWithSuffixSkip(size_t skip ) {
		size_t nextpos = (size_t)pos + skip - 1; // incremented at first loop
		bool onlyhex = true;
		char nextchar=0;
		size_t count = 0;
		while (onlyhex && (nextpos + 1 < source->size())) {
			nextpos += 1;
			nextchar = source->at(nextpos);
			char uppernextchar = upperchar(nextchar);
			if (nextchar >= '0' && nextchar<= '9') { count += 1; continue; }
			if (uppernextchar >= 'A' && uppernextchar <= 'F')  { count += 1; continue; }
			if (uppernextchar == 'H')  {
				// beware of names starting with 'ch'
				if (nextpos + 1 < source->size()) {
					char nextnextc = source->at(nextpos + 1);
					if (!isSeparator(nextnextc))
						break; // NOT followed by a separator, this is not an hexa number
				}
				return (count >= 1); // hexa suffix
			}
			break; // other character, wrong
		}
		return false; // all hexx digits and no sufffix
	}
	
	/** Checks if current and next characters are decimal digits only until space. Set skip to ignore current character and some more if needed. */
	bool Parser::findDecimalNumberSkip(size_t skip ) {
		size_t nextpos = (size_t)pos + skip - 1; // incremented at first loop
		char nextchar=0;
		size_t count = 0;
		while (nextpos + 1 < source->size()) {
			nextpos += 1;
			nextchar = source->at(nextpos);
			if (nextchar >= '0' && nextchar <= '9') { count += 1; continue; }
			char uppernextchar = upperchar(nextchar);
			if (uppernextchar >= 'A' && uppernextchar <= 'Z') return false; // letter, don't accept
			// any other separator is ok if we have at least 1 digit
			return (count >= 1);
		}
		return true; // all octal digits
	}

	/** Checks if current and next characters are octal digits only until space. Set skip to ignore current character and some more if needed. */
	bool Parser::findOctalNumberSkip(size_t skip ) {
		size_t nextpos = (size_t)pos + skip - 1; // incremented at first loop
		char nextchar=0;
		while (nextpos + 1 < source->size()) {
			nextpos += 1;
			nextchar = source->at(nextpos);
			if (nextchar < '0' || nextchar > '7') return false;
		}
		return true; // all octal digits
	}
	
	/** Checks if current and next characters are binary digits only until space or 'B' suffix. Set skip to ignore current character and some more if needed. */
	bool Parser::findBinaryNumberSkip(size_t skip ) {
		size_t nextpos = (size_t)pos + skip - 1; // incremented at first loop
		bool onlybin = true;
		char nextchar=0;
		int count = 0;
		while (onlybin && (nextpos + 1 < source->size())) {
			nextpos += 1;
			nextchar = source->at(nextpos);
			if (nextchar == '0' || nextchar == '1') { count += 1; continue; }
			char uppernextchar = upperchar(nextchar);
			if (uppernextchar == 'B') return (count >= 1); // binary suffix
			if (uppernextchar == 'A' || (uppernextchar >= 'C'&& uppernextchar <= 'Z')) return false; // another letter, suspicious
			// any other character is probably a separator so if we have at least one digit it's an hex number
			return (count >= 1);
		}
		return onlybin; // all hexx digits and no sufffix
	}
	
	/** Stores a new token given a string and a type.
	 In some cases no token will be added, e.g. a number with empty content.
	 In any cases, the parsing status is prepared for next token by reseting
	 status and clearing the given string.
	 */
	void Parser::StoreToken() {
		if (status != inSpace) {
			// some types don't go with empty content
			if (word.empty()) {
				if (type==tokenTypeUNKNOWN) return;
				if (type==tokenTypeLETTERS) return;
				if (type==tokenTypeCOMMENT) return;
				if (type==tokenTypeDIRECTIVE) return;
				if (type==tokenTypeSTRING) return;
				if (type==tokenTypeCHAR) return;
				if (type==tokenTypeHEXNUMBER) return;
				if (type==tokenTypeBINNUMBER) return;
				if (type==tokenTypeDECNUMBER) return;
			}
			
			// normalize file name for #INCLUDE
			if (type == tokenTypeFILENAME) {
				string filepath, filename;
				splitpath(word, filepath, filename);
				word = (!filepath.empty() ? filepath + NORMAL_DIR_SEPARATOR : "") + filename;
			}
			
			// Store the token, use upper case for directives
			if (type == tokenTypeDIRECTIVE) {
				if (word == ".") {
					type = tokenTypeSTRING;
				}
				word = to_upper(word);
			}
			
			// Test as a directive, only for letters
			Directive* directive = nullptr;

			// "SET" collides with the Z80/Z180 SET (bit) instruction: the leading dot is
			// stripped before we get here, so a bare "SET" and a dot-prefixed ".SET" are
			// indistinguishable by word alone. Only allow the SET *directive* match when
			// the token was actually dot/hash-prefixed (type is already tokenTypeDIRECTIVE);
			// a bare tokenTypeLETTERS "SET" must fall through to instruction lookup.
			bool skipDirectiveMatch = (type == tokenTypeLETTERS) && (to_upper(word) == "SET");

			if (!skipDirectiveMatch && (type == tokenTypeLETTERS || type == tokenTypeDIRECTIVE)) {
				directive = as->GetDirective(word);
				if (directive) {
					type = tokenTypeDIRECTIVE;
				}
			}

			// Store token
			ParseToken token;
			token.source = word;
			token.type = type;
			tokens->push_back(token);// store current value
			
			// set result flags for some directives and handle special #INCLUDE case
			if (token.type == tokenTypeDIRECTIVE) {
				resultFlag = hasNOTHING;
				lastDirective = directive;
				if (DirectiveIF::Identify(token.source) || DirectiveIFREQUIRED::Identify(token.source) || DirectiveIFDEF::Identify(token.source) || DirectiveIFNDEF::Identify(token.source)) {
					resultFlag = hasIF;
				}
				else if (DirectiveELSE::Identify(token.source)) resultFlag = hasELSE;
				else if (DirectiveENDIF::Identify(token.source)) resultFlag = hasENDIF;
				else if (DirectiveDEFINE::Identify(token.source)) resultFlag = hasDEFINE;
				else if (token.isIncludingDirective()) {
					// special case: the rest of source is a filename even if no quotes surrounds it
					// first skip white space
					size_t len = source->length();
					while ((size_t)pos < len && (source->at((size_t)pos) == ' ' || source->at((size_t)pos) == '\t')) {
						pos++;
					}
					// handle quotes
					char ch =source->at((size_t)pos);
					doubleQuoted = (ch == '"');
					if (doubleQuoted) {
						pos += 1; // skip double quote
					} else {
						word = ch; // retain first character in filename
					}
					type = tokenTypeFILENAME;
					status = inFilename;
					return;
				}
			}
		}
		word.clear();
		type = tokenTypeUNKNOWN;
		status = inNothing;
	}
	
	
	
	// publics
	
	Parser::Parser(class Assembler& assembler) {
		// references to (CodeLine) elements
		tokens = nullptr;
		curtoken = nullptr;
		as = &assembler;
		
		//internal allocated objects
		evalString = new ExpressionEvaluator;
		evalString->SetDefaultConversion(tokenTypeSTRING);
//		evalString->SetConversion(tokenTypeLETTERS, tokenTypeSTRING);
//		evalString->SetConversion(tokenTypeDECNUMBER, tokenTypeSTRING);
//		evalString->SetConversion(tokenTypeBINNUMBER, tokenTypeSTRING);
//		evalString->SetConversion(tokenTypeOCTNUMBER, tokenTypeSTRING);
//		evalString->SetConversion(tokenTypeHEXNUMBER, tokenTypeSTRING);

		evalBool = new ExpressionEvaluator;
//		evalBool->SetDefaultConversion(tokenTypeBOOL);
//		evalBool->SetConversion(tokenTypeLETTERS, tokenTypeBOOL);
//		evalBool->SetConversion(tokenTypeDECNUMBER, tokenTypeBOOL);
//		evalBool->SetConversion(tokenTypeBINNUMBER, tokenTypeBOOL);
//		evalBool->SetConversion(tokenTypeOCTNUMBER, tokenTypeBOOL);
//		evalBool->SetConversion(tokenTypeHEXNUMBER, tokenTypeBOOL);

		evalNumber = new ExpressionEvaluator;
		evalNumber->SetDefaultConversion(tokenTypeDECNUMBER);
//		evalNumber->SetConversion(tokenTypeLETTERS, tokenTypeDECNUMBER);
//		evalNumber->SetConversion(tokenTypeDECNUMBER, tokenTypeDECNUMBER);
//		evalNumber->SetConversion(tokenTypeBINNUMBER, tokenTypeDECNUMBER);
//		evalNumber->SetConversion(tokenTypeOCTNUMBER, tokenTypeDECNUMBER);
//		evalNumber->SetConversion(tokenTypeHEXNUMBER, tokenTypeDECNUMBER);

	}

	Parser::~Parser()
	{
		delete evalNumber;
		delete evalBool;
		delete evalString;
	}
	
	/** Resolves symbol, equates and labels starting in a given token.
	 	If a symbol cannot be replaced by a value, the function returns false.
	 */
	bool Parser::ResolveSymbolAt(int index, bool joker)
	{
#ifdef DEBUG
		if (!curtoken || !tokens) throw PARSERNotInitialized();
		if (index < 0 || index > (int)tokens->size()) throw PARSERInvalidTokenIndex();
#endif
		auto &token = tokens->at((size_t)index);
		if (token.type == tokenTypeCOMMENT)
			return true;
		if (token.type == tokenTypeDECNUMBER)
			return true;
		// This only occurs on the LETTER type tokens
		if (token.type == tokenTypeLETTERS) {
			if (as->ReplaceDefSymbol(token))
				return true;
			if (as->ReplaceLabel(token.source)) {
				token.type = tokenTypeDECNUMBER;
				return true;
			}
			token.unsolved = true;
			return false;// unknown symbol, not resolved
		}
		// replace "$" current address if found
		if (token.type == tokenTypeDOLLAR) {
			token.source = std::to_string(as->GetAddress());
			token.type = tokenTypeDECNUMBER;
			return true;
		}
		// translate hex numbers
		if (token.type == tokenTypeHEXNUMBER) {
			unsigned int uint = hex_to_unsigned(token.source);
			token.source = std::to_string(uint);
			token.type = tokenTypeDECNUMBER;
			return true;
		}
		// translate binary numbers
		if (token.type == tokenTypeBINNUMBER) {
			unsigned int uint = bin_to_unsigned(token.source);
			token.source = std::to_string(uint);
			token.type = tokenTypeDECNUMBER;
			return true;
		}
		// translate octal numbers
		if (token.type == tokenTypeOCTNUMBER) {
			unsigned int uint = oct_to_unsigned(token.source);
			token.source = std::to_string(uint);
			token.type = tokenTypeDECNUMBER;
			return true;
		}
		// translate characters in bytes
		if (token.type == tokenTypeCHAR) {
			token.source = unescape(token.source, joker); // take care of escaped characters
			unsigned int uint = token.source.size() > 0 ? (unsigned int)token.source.at(0) : 0; // '' will be 00
			token.source = std::to_string(uint);
			token.type = tokenTypeDECNUMBER;
			return true;
		}
		// translate escape sequences in strings
		if (token.type == tokenTypeSTRING) {
			token.source = unescape(token.source, joker);
			return true;
		}
		
		// ok by default, meaning any other operator like tokenTypeOP_PLUS
		return true;
	}
	bool Parser::ResolveSymbolAt(size_t index, bool joker)
	{
		return ResolveSymbolAt((int)index, joker);
	}

	/** Resolve symbols, equates and labels starting at given token index. If no start index is given, will start at current token.
	 Returns a list of indexes of unsolved label tokens. Unsolved labels are replaced by a decimal "0" value.
	 */
	std::vector<size_t> Parser::ResolveSymbols(int start /* = -1 */, bool joker)
	{
		// next token if no start
		if (start == (int)-1) start = (int)*curtoken;
		// replace existing symbols, EQU values, labels by their value		
		std::vector<size_t> unsolved;
		for (size_t i = (size_t)start ; i < tokens->size() ; i++) {
			if (!ResolveSymbolAt((int)i, joker)) {
				unsolved.push_back(i);
			}
		}
		return unsolved;
	}
	/** Resolve symbols, equates and labels starting at given token index. If no start index is given, will start at current token.
	 Returns a list of indexes of unsolved label tokens. Unsolved labels are replaced by a decimal "0" value.
	 */
	std::vector<size_t> Parser::ResolveSymbols(size_t start /* = -1 */, bool joker)
	{
		return ResolveSymbols((int)start, joker);
	}

	/** Cuts a string into a vector of typed tokens. Each token has a type but is not interpreted, so
	 symbols, labels or equates are not replaced by their values. Numbers can be decimal, octal, binary:
	 they don't have any prefix or suffix, only a type.
	 Spaces and tabs are not stored in tokens.
	 A vector of warnings and errors is returned.
	 */
	void Parser::Split(CodeLine& codeline, ErrorList& msg)
	{
		// Init target on codeline
		tokens = &codeline.tokens;
		curtoken = &codeline.curtoken;
		source = &codeline.source;
		
		// split initialisation
		*curtoken = 0;
		tokens->clear();
		resultFlag = hasNOTHING;
		pos = -1;								// current parsing position in string
		const int len = (int)source->length();		// explicitely signed because unsigned would fail the test (pos < len)
		if (word.empty()) {						// current parsed word
			// keep compiler happy
		}
		type = tokenTypeUNKNOWN;				// current token type
				
		tokens->clear();						// clear results
		*curtoken = 0;
		
		// Parse the string, concatenating characters in 'word' until a token can be stored.
		while (pos + 1 < len) {
			
			// Get next character and keep an upper-case equivalent for letters.
			pos += 1;
			c = (*source)[(size_t)pos];
			upperc = upperchar(c);
			
			// NOTICE: each of the following tests ends either with a break if the string parsing is finished,
			// either with a continue to check the next character. Each test can be thought as either a try to detect
			// a new token, either as a way to continue to store its string content character by character.
			// Spaces and tabulations are parsed too, but they will not be stored.
			// Each test which starts a new token first does a StoreToken(word, type) to store
			// the previous token if there was one being parsed
			// some ambiguous specific cases are handle either in StoreToken(), either in the parsing loop:
			//		- #INCLUDE accepts a non quoted string
			//      - hexa numbers can use letter digits followed by a 'h' suffix
			
			// include filenames have absolute priority over anything else
			if (status == inFilename) {
				// take care of possible double quoting
				if (doubleQuoted) {
					if (c == '"') {
						StoreToken(); // filename is finished
						pos += 1 ; // skip double quote
						continue;
					}
					// running within double quoted filename
					word += c;
					continue;
				}
				// end of filename by space or tab
				if (c == ' ' || c == '\t') {
					StoreToken();
					status = inSpace;
					continue;
				}
				word += c;
				continue;
			}

			// double quote running?
			if (status == inDoubleQuotes) {
				// ignore escaped double quote
				if (c == '\\' && hasNext && nextc == '"') {
					word += c;
					word += nextc;
					pos += 1; // skip backslash, and will skip double quote
					continue;
				}
				if (c == '"') {
					StoreToken();
					doubleQuoted = false;
					continue; // finished the double quoted string
				}
				// else continue to add
				word += c;
				continue;
			}
			// single quote running?
			if (status == inSingleQuotes) {
				// ignore escaped single quote
				if (c == '\\' && hasNext && nextc == '\'') {
					word += c;
					word += nextc;
					pos += 1; // skip backslash, and will skip double quote
					continue;
				}
				if (c == '\'') {
					StoreToken();
					continue; // finished the single quoted string
				}
				// else continue to add
				word += c;
				continue;
			}

			// End a running number?
			if (status == inDigits) {
				
				// continuing digit sequence?
				if (isHexDigit(upperc) && (type == tokenTypeHEXNUMBER)) {
					word += c;
					continue;
				}
				if (isDecDigit(c) && (type == tokenTypeDECNUMBER)) {
					word += c;
					continue;
				}
				if (isBinDigit(c) && (type == tokenTypeBINNUMBER)) {
					word += c;
					continue;
				}
				
				// H suffix after hex digits?
				if (upperc == 'H') {
					type = tokenTypeHEXNUMBER;
					StoreToken();
					continue;
				}
				// B suffix after digits?
				if (upperc == 'B' && (type != tokenTypeHEXNUMBER)) {
					type = tokenTypeBINNUMBER;
					StoreToken();
					continue;
				}
				
				// end of number
				StoreToken();
				// parse again from this position
				pos -= 1;
				status = inNothing;
				continue;
			}
			
			// check characters which interrupt a letters sequence
			
			// comment?
			if (c == ';') {
				StoreToken();
				word = source->substr((size_t)pos); // add the rest of line as a comment
				type = tokenTypeCOMMENT;
				StoreToken();
				break; // finished!
			}

			// end of label?
			if (c == ':') {
				StoreToken();
				if (*curtoken > 1) {
					type = tokenTypeCHAR;
					word = c;
				} else {
					type = tokenTypeCOLON;
				}
				StoreToken();
				continue;
			}
			
			// Directive prefix?
			if (c == '.' || c == '#') {
				if (word.empty()) {
					// nothing stored yet, start a directive
					status = inDirective;
					//$$ word = c;
					word = ""; // don't store prefix
					type = tokenTypeDIRECTIVE;
					continue;
				}
				// we're not supposed to reach here: would mean a '.' or '#' contained in a word
				// so just keep going and emit a warning
				word += c;
				msg.Warning(warningMisplacedChar, codeline); // pass 1 only
				continue;
			}
			
			// start double quotes?
			if (c == '"' && status != inLetters) {
				StoreToken();
				status = inDoubleQuotes;
				doubleQuoted = true;
				type = tokenTypeSTRING;
				continue;
			}
			
			// start single quotes?
			if (c == '\'' && status != inLetters) {
				status = inSingleQuotes;
				doubleQuoted = false; // useless?
				type = tokenTypeCHAR;
				continue;
			}

			// check next character availability
			hasNext = ((size_t)pos + 1 < source->length());
			nextc = hasNext ? (*source)[(size_t)pos + 1] : '\0';
			uppernextc = upperchar(nextc);
			// "//" comments?
			if ((c == '/') && hasNext && (nextc == '/')) {
				StoreToken();
				word = source->substr((size_t)pos); // add the rest of line as a comment
				type = tokenTypeCOMMENT;
				StoreToken();
				break; // finished!
			}

			// check if it is starting a new binary, octal, decimal or hexa number
			//bool bindigit = (c >='0') && (c <= '1');
			//bool octdigit = (c >='0') && (c <= '7');
			//bool decdigit = (c >='0') && (c <= '9');
			//bool hexdigit = (upperc >= 'A') && (upperc <= 'F');
			if (status != inDigits) {
			
				// prefixed?
				if ((c == '0') && hasNext) {
					// "0x" prefix of hex numbers?
					if ((uppernextc == 'X') && findHexNumberNoSuffixSkip(2)) { // request NO suffix
						// start an hex number
						StoreToken();
						status = inDigits;
						type = tokenTypeHEXNUMBER;
						pos += 1; // skip "x"
						continue;
					}
					// "0b" prefix of binary numbers?
					if ((uppernextc == 'B') && findBinaryNumberSkip(2)) {
						// start a binary number
						StoreToken();
						status = inDigits;
						type = tokenTypeBINNUMBER;
						pos += 1; // skip "B"
						continue;
					}
					// "0" prefix on octal numbers?
					if (findOctalNumberSkip(1)) {
						StoreToken();
						status = inDigits ;
						type = tokenTypeOCTNUMBER;
						continue;
					}
				}

				// no prefix: starting hex number suffixed by 'h' ?
				if (word.empty()) {
					if (findHexNumberWithSuffixSkip(0)) {
						StoreToken();
						word = c;
						status = inDigits;
						type = tokenTypeHEXNUMBER;
						continue;
					}
					// no prefix: starting a decimal number?
					if (findDecimalNumberSkip(0)) {
						StoreToken();
						word = c;
						status = inDigits;
						type = tokenTypeDECNUMBER;
						continue;
					}
					// no prefix: starting a binary number?
					if (findBinaryNumberSkip(0)) {
						StoreToken();
						word = c;
						status = inDigits;
						type = tokenTypeBINNUMBER;
						continue;
					}
				}
			}

			// HEXCHAR operator?
			if (word == "HEXCHAR") {
				type = tokenTypeOP_HEXCHAR;
				StoreToken();
				continue;
			}

			// "$" prefix of hex numbers?
			if (word == "$") {
				if (findHexNumberNoSuffixSkip(0)) {
					// forget the "$" and start an hex number
					word = c;
					status = inDigits;
					type = tokenTypeHEXNUMBER;
					continue;
				}
				// $ followed by non-hex = store a current assembling address  token
				word.clear();
				type = tokenTypeDOLLAR;
				StoreToken();
				
				// and let parsing start again on current character
				pos = pos - 1; // back
				continue;
			}
			
			// space?
			if (c == ' ' || c == '\t') {
				if (status == inSpace) continue;
				StoreToken();
				if (status != inFilename)
					status = inSpace;
				continue;
			}
			
			// parenthesis?
			if (c == '(') {
				StoreToken();
 				word = c;
				type = tokenTypePAROPEN;
				StoreToken();
				continue;
			}
			if (c == ')') {
				StoreToken();
				word = c;
				type = tokenTypePARCLOSE;
				StoreToken();
				continue;
			}
			// comma?
			if (c == ',') {
				StoreToken();
				word = c;
				type = tokenTypeCOMMA;
				StoreToken();
				continue;
			}
			
			if (status == inDirective) {
				word += c;
				continue;
			}
			
			// 2 characters operators?
			if (find2CharsOperator("<<", tokenTypeOP_LSHIFT))
				continue;
			if (find2CharsOperator(">>", tokenTypeOP_RSHIFT))
				continue;
			if (find2CharsOperator("<>", tokenTypeOP_DIFF))
				continue;
			if (find2CharsOperator("!=", tokenTypeOP_DIFF))
				continue;
			if (find2CharsOperator("==", tokenTypeOP_EQUAL))
				continue;
			if (find2CharsOperator("<=", tokenTypeOP_LTE))
				continue;
			if (find2CharsOperator(">=", tokenTypeOP_GTE))
				continue;
			if (find2CharsOperator("&&", tokenTypeOP_AND))
				continue;
			if (find2CharsOperator("||", tokenTypeOP_OR))
				continue;

			// single character operators?
			if (findOperator('<', tokenTypeOP_LT))
				continue;
			if (findOperator('>', tokenTypeOP_GT))
				continue;
			if (findOperator('=', tokenTypeOP_EQUAL))
				continue;
			if (findOperator('+', tokenTypeOP_PLUS))
				continue;
			if (findOperator('-', tokenTypeOP_MINUS))
				continue;
			if (findOperator('*', tokenTypeOP_MUL))
				continue;
			if (findOperator('/', tokenTypeOP_DIV))
				continue;
			if (findOperator('\\', tokenTypeOP_DIV))
				continue;
			if (findOperator('%', tokenTypeOP_MOD))
				continue;
			if (findOperator('!', tokenTypeOP_NOT))
				continue;
			if (findOperator('&', tokenTypeOP_BINAND))
				continue;
			if (findOperator('|', tokenTypeOP_BINOR))
				continue;
			if (findOperator('^', tokenTypeOP_BINXOR))
				continue;

			// continue running text sequence?
			if (status == inLetters) {
				word += c;
				continue;
			}
			
			// nothing special found, start a new text sequence
			StoreToken();
			type = tokenTypeLETTERS;
			word = c;
			status = inLetters;
		}
		
		// store last current token if any
		StoreToken();
	}

	/** Execute the last directive and returns its result: this is used by IF directives called from CodeLine.Assemble() to choose
	 the parsing mode, or INCLUDE to asssemble a child source file. */
	ErrorType Parser::LastDirective(CodeLine& codeline, ErrorList& msg)
	{
		if (!lastDirective) return errorTypeFALSE;
		// set curtoken on the directive token so the directive have access to its arguments
		*curtoken = 0;
		while (*curtoken < tokens->size()) {
			if ((*tokens)[*curtoken].type == tokenTypeDIRECTIVE)
				break;
			*curtoken += 1;
		};
		// and call the directive
		return lastDirective->Parse(*as, *this, codeline, nullptr, msg);
	}
	
	/** Evaluate next tokens to produce a boolean result.
	 @throw EXPRESSIONLeftOperandMissing
	 */
	bool Parser::EvaluateBoolean(bool & result)
	{
		int lasttoken = -1;
		ParseToken evaluated = evalBool->Evaluate(*tokens, (int)*curtoken, lasttoken);
		if (evaluated.type == tokenTypeDECNUMBER) {
			*curtoken = (size_t)lasttoken;
			result = evaluated.asInteger() != 0;
			return true;
		}
		if ((evaluated.type == tokenTypeBOOL) || (evaluated.type == tokenTypeSTRING)) {
			*curtoken = (size_t)lasttoken;
			result = !evaluated.source.empty();
			return true;
		}
		return false;
	}
	
	/** Evaluate next tokens to produce a string result. Operands must be strings, but automatic conversion will happen on
	 decimal numbers and booleans.
	 @throw EXPRESSIONLeftOperandMissing
	 */
	bool Parser::EvaluateString(string & result)
	{
		// convert tokens
		int lasttoken = -1;
		ParseToken evaluated = evalString->Evaluate(*tokens, (int)*curtoken, lasttoken);
		if ((evaluated.type == tokenTypeSTRING) || (evaluated.type == tokenTypeDECNUMBER) || (evaluated.type == tokenTypeLETTERS)) {
			*curtoken = (size_t)lasttoken;
			result = evaluated.source;
			return true;
		}
		if (evaluated.type == tokenTypeBOOL) {
			*curtoken = (size_t)lasttoken;
			result = evaluated.source.empty() ? "" : "t";
			return true;
		}
		return false;
	}
	
	/** Evaluate next tokens to produce an integer number masked by the address size.
	 @throw EXPRESSIONLeftOperandMissing
	 */
	bool Parser::EvaluateAddress(ADDRESSTYPE & result)
	{
		int lasttoken = -1;
		ParseToken evaluated;
		evaluated = evalNumber->Evaluate(*tokens, (int)*curtoken, lasttoken);
		if ((evaluated.type == tokenTypeSTRING) || (evaluated.type == tokenTypeDECNUMBER)) {
			*curtoken = (size_t)lasttoken;
			// special case with one character: return character code
			if (evaluated.type == tokenTypeSTRING) {
				if (evaluated.source.length() == 1) {
					result = (ADDRESSTYPE)evaluated.source.at(0);
					return true;
				}
			}
			// else, interpret as a number or return 0
			result = evaluated.asAddress();
			return true;
		}
		return false;
	}

/** Evaluate next tokens to produce a 32-bits integer number.
	 @throw EXPRESSIONLeftOperandMissing
	 */
	bool Parser::EvaluateInteger(DWORD & result)
	{
		int lasttoken = -1;
		ParseToken evaluated;
		evaluated = evalNumber->Evaluate(*tokens, (int)*curtoken, lasttoken);
		if ((evaluated.type == tokenTypeSTRING) || (evaluated.type == tokenTypeDECNUMBER)) {
			*curtoken = (size_t)lasttoken;
			// special case with one character: return character code
			if (evaluated.type == tokenTypeSTRING) {
				if (evaluated.source.length() == 1) {
					result = (ADDRESSTYPE)evaluated.source.at(0);
					return true;
				}
			}
			// else, interpret as a number or return 0
			result = evaluated.asInteger();
			return true;
		}
		return false;
	}
} // namespace MUZ
