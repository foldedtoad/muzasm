//
//  AllOperators.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 13/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "All-Operators.h"
#include "Asm-Exceptions.h"
#include "Expression.h"

namespace MUZ {

	/** helper for boolean conversion */
	inline bool to_bool(ParseToken token) {
		if (token.type == tokenTypeDECNUMBER) {
			return token.source != "0";
		}
		// below works for BOOL as well as STRING
		return !token.source.empty();
	}
	
	/** helper to convert a boolean into a token */
	inline void to_booltoken(bool b, ParseToken& token) {
		token.type = tokenTypeBOOL;
		token.source = b ? "t" : "";
	}
	/** helper to convert a string into a token */
	inline void to_stringtoken(std::string s, ParseToken& token) {
		token.type = tokenTypeSTRING;
		token.source = s;
	}
	/** helper to convert a number into a token */
	inline void to_numtoken(unsigned int n, ParseToken& token) {
		token.type = tokenTypeDECNUMBER;
		token.source = std::to_string(n);
	}

	
	// No need to define the operator classes in the header, we only need one instance for each operator and the instance address
	// stored in the allOps array.
	
	// NOT number
	// NOT boolean
	struct OperatorNOT : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER && arg1.type != tokenTypeBOOL) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER && arg2.type != tokenTypeBOOL) return nop;
			if (arg1.type == tokenTypeDECNUMBER || arg1.type == tokenTypeSTRING) {
				DWORD n = ~arg1.asInteger();
				ParseToken result;
				to_numtoken(n, result);
				return result;
			}else if (arg1.type == tokenTypeBOOL) {
				ParseToken result;
				to_booltoken(arg1.source.empty(), result);
				return result;
			}
			throw ASMOperandTypeException();
		}
	} & opNOT = *new OperatorNOT; // singleton
	
	// MUL number number
	struct OperatorMUL : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			DWORD n = arg1.asInteger() * arg2.asInteger() ;
			ParseToken result ;
			to_numtoken(n, result);
			return result;
		}
	} & opMUL = *new OperatorMUL; // singleton
	
	// DIV number number
	struct OperatorDIV : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			DWORD n = arg1.asInteger() / arg2.asInteger();
			ParseToken result;
			to_numtoken(n, result);
			return result;
		}
	} &  opDIV = *new OperatorDIV; // singleton
	
	// MOD number number
	struct OperatorMOD : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			DWORD n = arg1.asInteger() % arg2.asInteger() ;
			ParseToken result;
			to_numtoken(n, result);
			return result;
		}
	} & opMOD = *new OperatorMOD; // singleton
	
	// + number number
	// + bool bool (or)
	// + string string
	/** Add two numbers or concatenate strings.
	 If at least one argument is a boolean, addition is considered as OR between both booleans
	 If at least one argument is a string, the addition is a concatenation of both strings.
	 If not, they're considered as decimal numbers and added.
	 */
	struct OperatorADD : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// Boolean + boolean?
			if ((arg1.type == tokenTypeBOOL) || (arg2.type == tokenTypeBOOL)) {
				to_booltoken(to_bool(arg1) || to_bool(arg2), result);
				return result;
			}
			// string + number?
			if (arg1.type == tokenTypeSTRING && arg2.type == tokenTypeDECNUMBER) {
				std::string s;
				unsigned int num = arg2.asAddress();
				for (auto c: arg1.source) {
					s = s + (char)(((int)c + (int)num) & 0xff);
				}
				to_stringtoken( s, result);
				return result;
			}
			// one or both arguments as string?
			if (arg1.type == tokenTypeSTRING || arg2.type == tokenTypeSTRING) {
				to_stringtoken( arg1.source + arg2.source, result);
				return result;
			}
			// consider both are decimal numbers
			DWORD n = arg1.asInteger() + arg2.asInteger();
			to_numtoken(n, result);
			return result;
		}
	} & opADD = *new OperatorADD; // singleton
	
	// SUB number number
	struct OperatorSUB : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			ParseToken result;
			// string - number?
			if (arg1.type == tokenTypeSTRING && arg2.type == tokenTypeDECNUMBER) {
				std::string s;
				ADDRESSTYPE num = arg2.asAddress();
				for (auto c: arg1.source) {
					s = s + (char)(((int)c - (int)num) & DATAMASK);
				}
				to_stringtoken( s, result);
				return result;
			}
			DWORD n = arg1.asInteger() - arg2.asInteger();
			to_numtoken(n, result);
			return result;
		}
	} & opSUB = *new OperatorSUB; // singleton
	
	// LSHIFT number number
	struct OperatorLSHIFT : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			ParseToken result;
			DWORD n = arg1.asInteger() << arg2.asInteger();
			to_numtoken(n, result);
			return result;
		}
	} & opLSHIFT = *new OperatorLSHIFT; // singleton
	
	// RSHIFT number number
	struct OperatorRSHIFT : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			ParseToken result;
			DWORD n = arg1.asInteger() >> arg2.asInteger();
			to_numtoken(n, result);
			return result;
		}
	} & opRSHIFT = *new OperatorRSHIFT; // singleton
	
	// AND number number
	// AND boolean boolean
	struct OperatorAND : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				to_booltoken( to_bool(arg1) && to_bool(arg2), result);
				return result;
			}
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			DWORD n = arg1.asInteger() & arg2.asInteger() ;
			to_numtoken(n, result);
			return result;
		}
	} & opAND = *new OperatorAND; // singleton
	
	// OR number number
	// OR boolean boolean
	struct OperatorOR : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				to_booltoken( to_bool(arg1) || to_bool(arg2), result);
				return result;
			}
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			DWORD n = arg1.asInteger() | arg2.asInteger();
			to_numtoken(n, result);
			return result;
		}
	} & opOR = *new OperatorOR; // singleton
	
	// XOR number number
	// XOR boolean boolean
	struct OperatorXOR : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				to_booltoken( to_bool(arg1) ^ to_bool(arg2), result);
				return result;
			}
			if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER) return nop;
			if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER) return nop;
			DWORD n = arg1.asInteger() ^ arg2.asInteger();
			to_numtoken(n, result);
			return result;
		}
	} & opXOR = *new OperatorXOR; // singleton
	
	// LT number number
	// LT string string
	/** Compare two strings, numbers or booleans for inferiority.
	 - string1 < string2:
	 . all characters of string1 are inferior to string2 characters one by one
	 . and length of string1 is less or equal to length of string2
	 . character '*' is not a joker
	 - number1 < number2: arithmetic
	 - false < true, all other cases return false
	 
	 if one argument is a boolean, the comparison is a boolean comparison
	 else, if one argument is a string, the comparison is a string comparison
	 else, both arguments are considered as having the numeric type.
	 
	 a string is converted to boolean true is it is not empty
	 
	 */
	struct OperatorLT : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				to_booltoken( ! to_bool(arg1) && to_bool(arg2), result);
				return result;
			}
			// string comparison?
			if (arg1.type == tokenTypeSTRING || arg2.type == tokenTypeSTRING) {
				size_t len1 = arg1.source.length();
				size_t len2 = arg2.source.length();
				size_t len = std::min<size_t>(len1, len2);
				bool inferior = true;
				for (size_t i = 0 ; inferior && (i < len) ; i++) {
					const unsigned char c1 = (const unsigned char)arg1.source[i];
					const unsigned char c2 = (const unsigned char)arg2.source[i];
					if (c1 >= c2)
						inferior = false;
				}
				to_booltoken(inferior && (len1 <= len2), result);
				return result;
			}
			
			// consider both decimal numbers
			bool inferior = arg1.asInteger() < arg2.asInteger();
			to_booltoken(inferior, result);
			return result;
		}
	} & opLT = *new OperatorLT; // singleton

	// GT number number
	// GT string string
	/** Compare two strings, numbers or booleans for superiority.
	 - string1 >string2:
	 . all characters of string1 are superior to string2 characters one by one
	 . and length of string1 is greater or equal to length of string2
	 . character '*' is not a joker
	 - number1 > number2: arithmetic
	 - true > false, all other cases return false
	 
	 if one argument is a boolean, the comparison is a boolean comparison
	 else, if one argument is a string, the comparison is a string comparison
	 else, both arguments are considered as having the numeric type.
	 
	 a string is converted to boolean true is it is not empty
	 
	 */
	struct OperatorGT : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				to_booltoken( to_bool(arg1) && ! to_bool(arg2), result);
				return result;
			}
			// string comparison?
			if (arg1.type == tokenTypeSTRING || arg2.type == tokenTypeSTRING) {
				size_t len1 = arg1.source.length();
				size_t len2 = arg2.source.length();
				size_t len = std::min<size_t>(len1, len2);
				bool superior = true;
				bool equal = true;
				for (size_t i = 0 ; superior && (i < len) ; i++) {
					const unsigned char c1 = (const unsigned char)arg1.source[i];
					const unsigned char c2 = (const unsigned char)arg2.source[i];
					if (c1 < c2)
						superior = false;
					if (equal && (c1 != c2))
						equal = false;
				}
				to_booltoken(superior && !equal && (len1 > len2), result);
				return result;
			}
			
			// consider both decimal numbers
			bool inferior = arg1.asInteger() < arg2.asInteger() ;
			to_booltoken(inferior, result);
			return result;
		}
	} & opGT = *new OperatorGT;
	
	// LTE number number
	// LTE string string
	/** Compare two strings, numbers or booleans for inferiority or equality.
	 - string1 <= string2:
	 . all characters of string1 are inferior or equal to string2 characters one by one
	 . and length of string1 is less or equal to length of string2
	 . character '*' is not a joker
	 - number1 <= number2: arithmetic
	 - true <= false returns false, all other cases return true
	 
	 if one argument is a boolean, the comparison is a boolean comparison
	 else, if one argument is a string, the comparison is a string comparison
	 else, both arguments are considered as having the numeric type.
	 
	 a string is converted to boolean true is it is not empty
	 
	 */
	struct OperatorLTE : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				to_booltoken( ! to_bool(arg1) || to_bool(arg2), result);
				return result;
			}
			// string comparison?
			if (arg1.type == tokenTypeSTRING || arg2.type == tokenTypeSTRING) {
				size_t len1 = arg1.source.length();
				size_t len2 = arg2.source.length();
				size_t len = std::min<size_t>(len1, len2);
				bool inferior = true;
				for (size_t i = 0 ; inferior && (i < len) ; i++) {
					const unsigned char c1 = (const unsigned char)arg1.source[i];
					const unsigned char c2 = (const unsigned char)arg2.source[i];
					if (c1 >= c2)
						inferior = false;
				}
				to_booltoken(inferior && (len1 <= len2), result);
				return result;
			}
			// consider both decimal numbers
			bool inferior = arg1.asInteger() <= arg2.asInteger() ;
			to_booltoken(inferior, result);
			return result;
		}
	} & opLTE = *new OperatorLTE;
	
	// GTE number number
	// GTE string string
	/** Compare two strings, numbers or booleans for superiority or equality.
	 - string1 >= string2:
	 . all characters of string1 are superior or equal to string2 characters one by one
	 . and length of string1 is greater or equal to length of string2
	 . character '*' is not a joker
	 - number1 	= number2: arithmetic
	 - false >= true returns false, all other cases return true
	 
	 if one argument is a boolean, the comparison is a boolean comparison
	 else, if one argument is a string, the comparison is a string comparison
	 else, both arguments are considered as having the numeric type.
	 
	 a string is converted to boolean true is it is not empty
	 
	 */
	struct OperatorGTE : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2)  {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				to_booltoken( to_bool(arg1) || ! to_bool(arg2), result);
				return result;
			}
			// string comparison?
			if (arg1.type == tokenTypeSTRING || arg2.type == tokenTypeSTRING) {
				size_t len1 = arg1.source.length();
				size_t len2 = arg2.source.length();
				size_t len = std::min<size_t>(len1, len2);
				bool superior = true;
				for (size_t i = 0 ; superior && (i < len) ; i++) {
					const unsigned char c1 = (const unsigned char)arg1.source[i];
					const unsigned char c2 = (const unsigned char)arg2.source[i];
					if (c1 < c2)
						superior = false;
				}
				to_booltoken(superior && (len1 >= len2), result);
				return result;
			}
			// consider both decimal numbers
			bool superior = arg1.asInteger() >= arg2.asInteger();
			to_booltoken(superior, result);
			return result;
		}
	} & opGTE = *new OperatorGTE; // singleton
	
	// DIFF number number
	// DIFF string string
	/** Compare two strings, numbers or booleans for difference.
	 - strings are considered as different if any character doesn't match before either the end of both strings, either a "*" is met
	 - numbers are different if their numeric values are different
	 - booleans are different if they are not both true or both false
	 
	 if one argument is a boolean, the comparison is a boolean comparison
	 else, if one argument is a string, the comparison is a string comparison
	 else, both arguments are considered as having the same type.
	 
	 a string is converted to boolean true is it is not empty
	 
	 */
	struct OperatorDIFF : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				// convert numbers to TRUE if they're not 0
				to_booltoken(to_bool(arg1) != to_bool(arg2), result);
				return result;
			}
			// string comparison?
			if (arg1.type == tokenTypeSTRING || arg2.type == tokenTypeSTRING) {
				size_t len1 = arg1.source.length();
				size_t len2 = arg2.source.length();
				if (len1 != len2) {
					to_booltoken(false, result);
				} else {
					bool diff = false;
					for (size_t i = 0 ; !diff && (i < len1) ; i++) {
						const char c1 = arg1.source[i];
						const char c2 = arg2.source[i];
						if ((c1 == '\x1A') || (c2 == '\x1A'))
							break;
						if (c1 != c2)
							diff = true;
					}
					to_booltoken(diff, result);
				}
				return result;
			}
			
			// consider both decimal numbers
			bool diff = arg1.asInteger() != arg2.asInteger() ;
			to_booltoken(diff, result);
			return result;
		}
	} & opDIFF = *new OperatorDIFF; // singleton
	
	// EQUAL number number
	// EQUAL string string
	/** Compare two strings, numbers or booleans for equality.
	 - strings are considered as equal if their characters match until either the end of both strings, either a "*" is met
	 - numbers are equals if their numeric value is the same
	 - booleans are equal if they are both true or both false
	 
	 if one argument is a boolean, the comparison is a boolean comparison
	 else, if one argument is a string, the comparison is a string comparison
	 else, both arguments are considered as having the same type.
	 
	 a string is converted to boolean true is it is not empty
	 
	 */
	struct OperatorEQUAL : public Operator
	{
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
			ParseToken result;
			// boolean comparison?
			if (arg1.type == tokenTypeBOOL || arg2.type == tokenTypeBOOL) {
				// convert numbers to TRUE if they're not 0
				to_booltoken(to_bool(arg1) == to_bool(arg2), result);
				return result;
			}
			// string comparison?
			if (arg1.type == tokenTypeSTRING || arg2.type == tokenTypeSTRING) {
				size_t len1 = arg1.source.length();
				size_t len2 = arg2.source.length();
				size_t len = std::min<size_t>(len1, len2);
				bool equal = true;
				for (size_t i = 0 ; equal && (i < len) ; i++) {
					const char c1 = arg1.source[i];
					const char c2 = arg2.source[i];
					if ((c1 == '\x1A') || (c2 == '\x1A'))
						break;
					if (c1 != c2)
						equal = false;
				}
				to_booltoken(equal, result);
				return result;
			}
			
			// consider both decimal numbers
			bool equal = arg1.asInteger() == arg2.asInteger() ;
			to_booltoken(equal, result);
			return result;
		}
	} & opEQUAL = *new OperatorEQUAL; // singleton

// HEXCHAR number / label
struct OperatorHEXCHAR : public Operator
{
	virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2) {
		if (arg1.type != tokenTypeSTRING && arg1.type != tokenTypeDECNUMBER && arg1.type != tokenTypeBOOL) return nop;
		if (arg2.type != tokenTypeSTRING && arg2.type != tokenTypeDECNUMBER && arg2.type != tokenTypeBOOL) return nop;
		if (arg1.type == tokenTypeDECNUMBER || arg1.type == tokenTypeSTRING) {
			unsigned int n = (~arg1.asAddress()) & ADDRESSMASK;
			ParseToken result;
			to_numtoken(n, result);
			return result;
		}else if (arg1.type == tokenTypeBOOL) {
			ParseToken result;
			to_booltoken(arg1.source.empty(), result);
			return result;
		}
		throw ASMOperandTypeException();
	}
} & opHEXCHAR = *new OperatorHEXCHAR; // singleton


	
	/** Externally accessible array for the operators coresponding to each token type. This array must have exactly one member for each
	 possible token type, and it must be in the same order as them in the TokenType enum. In debug compile, a check is done by
	 the Expression Evaluator constructor to make sure this array is correctly set.
	 Some token type are used by the parser but not linked to actual operators: they're assigned  the "nop" operator which does nothing. */
	OperatorDef allOps[tokenTypeLAST] = {
		{9999, &nop, tokenTypeUNKNOWN },		// No token of this type can be stored
		
		{9999, &nop, tokenTypeFIRSTCONVERTIBLE},// the following tokens (-> tokenTypeLASTCONVERTIBLE) can be converted by ExpressionEvaluator
		  {9999, &nop, tokenTypeFIRSTNUMERIC},	// the following tokens (-> tokenTypeLASTNUMERIC) contain numeric values
		    {9999, &nop, tokenTypeHEXNUMBER},	// a "0x" prefixed, "$" prefixed or "h" suffixed number (- )before decimal translation)
		    {9999, &nop, tokenTypeBINNUMBER},	// a "b" suffixed binary number (- )before decimal translation)
		    {9999, &nop, tokenTypeOCTNUMBER},	// a "0" prefixed octal number - followed by at least one 0-7 digit (before decimal translation)
		    {9999, &nop, tokenTypeDECNUMBER},	// a non prefixed and non suffixed decimal number
		  {9999, &nop, tokenTypeLASTNUMERIC},	// the following tokens (-> tokenTypeLASTNUMERIC) contain numeric values
		  {9999, &nop, tokenTypeLETTERS},		// during parsing of a characters string but not knowing what king of token it will be
		  {9999, &nop, tokenTypeSTRING},        // a " delimited string
		  {9999, &nop, tokenTypeBOOL},			// for boolean values - this can only be written by expression evaluator
		  {9999, &nop, tokenTypeCHAR},			// a ' delimited character
		{9999, &nop, tokenTypeLASTCONVERTIBLE},

		// final stored tokens
		{9999, &nop, tokenTypeFILENAME},		// a filename
		{9999, &nop, tokenTypeCOMMENT},			// ';' and all that follows on line
		{9999, &nop, tokenTypeDIRECTIVE},		// a '.' or '#' directive
		{9999, &nop, tokenTypeCOMMA},			// a ','
		{9999, &nop, tokenTypePAROPEN},			// (
		{9999, &nop, tokenTypePARCLOSE},		// )
		{9999, &nop, tokenTypeDOLLAR},			// $ alone (not an hex number prefix)
		{9999, &nop, tokenTypeCOLON},			// ':' ending a label definition
		
		// set individual priorities			// char		args		args type			result type
		{3, &opLSHIFT, tokenTypeOP_LSHIFT},		// <<			2			number				- as arg -
		{3, &opRSHIFT, tokenTypeOP_RSHIFT},		// >>			2			number				- as arg -
		{5, &opDIFF, tokenTypeOP_DIFF},			// <> and !=	2			number, string		boolean
		{5, &opEQUAL, tokenTypeOP_EQUAL},		// 	= and ==	2			number, string		boolean
		{4, &opLT, tokenTypeOP_LT},				// <			2			number, string		boolean
		{4, &opGT, tokenTypeOP_GT},				// >			2			number, string		boolean
		{4, &opLTE, tokenTypeOP_LTE},			// <=			2			number, string		boolean
		{4, &opGTE, tokenTypeOP_GTE},			// >=			2			number string		boolean
		{10, &opOR, tokenTypeOP_OR},			// ||			2			boolean				- as arg -
		{9, &opAND, tokenTypeOP_AND},			// &&			2			boolean				- as arg -
		{8, &opOR, tokenTypeOP_BINOR},			// |			2			number				- as arg -
		{6, &opAND, tokenTypeOP_BINAND},		// &			2			number				- as arg -
		{7, &opXOR, tokenTypeOP_BINXOR},		// ^			2			number				- as arg -
		{0, &opNOT, tokenTypeOP_NOT}, 			// !			1			number, boolean		- as arg -
		{2, &opADD, tokenTypeOP_PLUS},			// 	+			2			number, string		- as arg -
		{2, &opSUB, tokenTypeOP_MINUS},			// -			1 or 2		number				- as arg -
		{1, &opMUL, tokenTypeOP_MUL},			// 	*			2			number				- as arg -
		{1, &opDIV, tokenTypeOP_DIV},			// 	/ \\		2			number				- as arg -
		{1, &opMOD, tokenTypeOP_MOD},			// %			2			number				- as arg -
		{0, &opHEXCHAR, tokenTypeOP_HEXCHAR}, 	// HEXCHAR		1			number				string (2 hex digits)

		{9999, &nop, tokenTypeIGNORE},	
	};

}
