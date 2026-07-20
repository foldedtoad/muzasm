//
//  CodeLine.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 18/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef CodeLine_h
#define CodeLine_h

#include <string>
#include <vector>
#include "Errors.h"
#include "Types.h"
#include "ExpVector.h"

namespace MUZ {
	
	/** Source and assembled content for one line of source code. */
	struct CodeLine
	{
	public:
		// source
		
		/** Source file reference for the Assembler (index in SourceFile array). */
		size_t				file;
		/** Line number of this CodeLine in the file. */
		size_t				line;
		/** Content of the line (may be replaced by file direct access in later release.) */
		std::string 		source;
		// Direct file access for content
		//long				offset;						// position in the source file
		//int 				size;						// number of bytes in this source line

		// parsed tokens
		 
		/** Array of tokens prepared by the Parser::Split() function. */
		ExpVector			tokens;
		/** Index for the instruction token in the tokens array.*/
		size_t				instructiontoken = 0;
		/** current position for Reset/Next functions. */
		size_t				curtoken = 0;
		
		// assembled code
		
		/** Flag to tell if the line has been assembled (errorTypeOK) or not (any other value). non-assembled lines
		 in conditionnal blocks are flagged as errorTypeFALSE */
		ErrorType			assembled = errorTypeFALSE;
		/** Array of code bytes once assembled. May be empty. */
		std::vector<BYTE>	code;
		/** Minimum states  */
		int					statesmin = 0 ;
		/** Maximum states, generally when a jump happens because a condition is met. */
		int					statesmax = 0;
		/** Starting address for this line. */
		DWORD				address = 0;
		/** Pointer to the code or data section where this line is stored iff it contains code. */
		class Section*		section = nullptr;
		/** File reference for the Assembler if this line includes another file. */
		size_t				includefile = 0;
		/** Label reference if this line has a label or is after a line containing only a label. */
		class Label*		label = nullptr;
		/** Assembler reference. */
		class Assembler*	as = nullptr;
		/** Error/Warning message reference, -1 if no message */
		int 				message = -1;
		/** Enable/disable listing. */
		bool				listing = true;
		/** DefSymbol reference for listing. Valid for #define and #requires*/
		std::string			defsymbol;

		/** Resets the token exploration system to the first token after the instruction. */
		void ResetInstruction(size_t start);
		
		/** Set machine cycles. */
		void SetStates(int mintime, int maxtime = -1);
		
		/** Set 1 to 4 code values (bytes). int values given in parameters are masked to DATATYPE */
		void ResetCode();
		void AddCode(int b0);
		void AddCode(int b0, int b1);
		void AddCode(int b0, int b1, int b2) ;
		void AddCode(int b0, int b1, int b2, int b3);
	};

}
#endif /* CodeLine_h */
