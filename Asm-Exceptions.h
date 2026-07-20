//
//  Asm-Exceptions.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 13/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef Asm_Exceptions_h
#define Asm_Exceptions_h

#include <exception>

namespace MUZ {
	// wrong type for an operator in an expression
	class ASMOperandTypeException: public std::exception {
		virtual const char* what() const noexcept{ return "ASM: wrong operand type for operator"; }
	};
	
	class PARSERNotInitialized: public std::exception {
		virtual const char *what() const noexcept{ return "PARSER not initialized"; }
	};
	
	class PARSERInvalidTokenIndex: public std::exception {
		virtual const char *what() const noexcept{ return "PARSER invalid token index"; }
	};

	class PARSERDivisionByZero: public std::exception {
		virtual const char *what() const noexcept{ return "PARSER division by zero"; }
	};
	
	class EXPRESSIONOpenParenthesisTooMuch: public std::exception {
		virtual const char *what() const noexcept{ return "too much opening parenthesis in expression"; }
	};
	class EXPRESSIONCloseParenthesisTooMuch: public std::exception {
		virtual const char *what() const noexcept{ return "too much closing parenthesis in expression"; }
	};
	class EXPRESSIONNotAddress: public std::exception {
		virtual const char *what() const noexcept{ return "expression doesn't compute as an address"; }
	};
	class EXPRESSIONNotBoolean: public std::exception {
		virtual const char *what() const noexcept{ return "expression doesn't compute as a boolean"; }
	};
	class EXPRESSIONNotString: public std::exception {
		virtual const char *what() const noexcept{ return "expression doesn't compute as a string"; }
	};
	class EXPRESSIONLeftOperandMissing: public std::exception {
		virtual const char *what() const noexcept{ return "2-operands operator is missing left operand"; }
	};

}
#endif /* Asm_Exceptions_h */
