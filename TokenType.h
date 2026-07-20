//
//  TokenType.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 11/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef TokenType_h
#define TokenType_h

namespace MUZ {

	/** ParseToken type for parser. */
	enum TokenType {

		// transitionnal parsing token types

		/** Token type for unparsed token. No token of this type is stored. */
		tokenTypeUNKNOWN,
		
		/** Not an actual type: this value preceeds all the convertible token types. */
		tokenTypeFIRSTCONVERTIBLE,
		  /** Not an actual type: value preceeding all the numeric token types. */
		  tokenTypeFIRSTNUMERIC,
		    /** Token type for an hexadecimal number prefixed with 0x or $, or suffixed with H. */
		    tokenTypeHEXNUMBER,
		    /** Token type for a b sufffixxed binary number. */
		    tokenTypeBINNUMBER,
		    /** Token type for a 0 prefixed octal number. */
		    tokenTypeOCTNUMBER,
		    /** Token type for all number after conversion. */
		    tokenTypeDECNUMBER,
		  /** Value following the last numeric token type. */
		  tokenTypeLASTNUMERIC,
		  /** Token type for a sequence of letters during parsing. */
		  tokenTypeLETTERS,
		  /** Token type for a " delimited character string. */
		  tokenTypeSTRING,
		  /** Token type for a boolean value. This is internally used by expression evaluator. */
		  tokenTypeBOOL,
		  /** Token type for a ' delimited character. */
		  tokenTypeCHAR,
		/** Value affter the last convertible token type. */
		tokenTypeLASTCONVERTIBLE,

		// final stored tokens

		/** Type for a filename in an including directive. */
		tokenTypeFILENAME,
		/** Type for a comment starting by ';'. */
		tokenTypeCOMMENT,
		/** Type for a directive. */
		tokenTypeDIRECTIVE,
		/** Type for the comma. */
		tokenTypeCOMMA,
		/** Type for an opening parenthesis. */
		tokenTypePAROPEN,
		/** Type for a closing parenthesis. */
		tokenTypePARCLOSE,
		/** Type for a '$' alone, not used as hexadecimal prefix. */
		tokenTypeDOLLAR,
		/** Type ffor a ':' ending a label name. */
		tokenTypeCOLON,
		
		// stored operators
		tokenTypeOP_LSHIFT,	/// <<
		tokenTypeOP_RSHIFT,	/// >>
		tokenTypeOP_DIFF,	/// <> and !=
		tokenTypeOP_EQUAL,	/// =
		tokenTypeOP_LT,		/// <
		tokenTypeOP_GT,		/// >
		tokenTypeOP_LTE,	/// <=
		tokenTypeOP_GTE,	/// >=
		tokenTypeOP_OR,		/// ||
		tokenTypeOP_AND,	/// &&
		tokenTypeOP_BINOR,	/// |
		tokenTypeOP_BINAND,	/// &
		tokenTypeOP_BINXOR,	/// ^
		tokenTypeOP_NOT,	/// !
		tokenTypeOP_PLUS,	/// +
		tokenTypeOP_MINUS,	/// -
		tokenTypeOP_MUL,	/// *
		tokenTypeOP_DIV,	/// \ /  
		tokenTypeOP_MOD,	/// %
		tokenTypeOP_HEXCHAR, /// hexchar
		
		tokenTypeIGNORE,	/// used by parser to neutralize colons in label definitions
		
		tokenTypeLAST		/// used for priority array size
	};

}
#endif /* TokenType_h */
