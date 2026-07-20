//
//  ParsingMode.h
//  MUZ-Workshop
//
// ParsingMode defines how the parser will handle the conditionnal directives it will encounter.
// Conditionnal directives are #IF #IFDEF #IFNDEF #ELSE #ENDIF. Before parsing a line, the assembler
// will check for the presence of these directives and modify its mode level and stack accordingly.
//
//  Created by Francis Pierot on 05/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef ParsingMode_h
#define ParsingMode_h

#include <vector>
#include <stack>

namespace MUZ {
	
	/** Defines the possible parsing modes. If not #IF, #IFDEF or #IFNDEF has been met yet, the mode is ROOT.
	 When a condition is met, the mode will be set to DOTOELSE if it was true, or SKIPTOELSE if false.
	 When an ELSE is met, the mode will be set to DOTOEND if the mode was SKIPTOELSE, or SKIPTOEND if it was DOTOELSE.
	 Lines will be ignored when in a SKIP mode, but presence of conditionnal directives will still be checked to maintain
	 a level of imbrication, so an imbricated ELSE or ENDIF is not mistaken as the one matching the current condition.
	 */
	enum ParsingMode {
		parsingModeROOT,			// no conditionnal parsing mode, always at the bottom of stack
		parsingModeDOTOEND,			// Accept lines until the next #ENDIF of same level
		parsingModeDOTOELSE,		// Accept lines until the next #ELSE or #ENDIF of same level
		parsingModeSKIPTOEND,		// Skip lines until the next #ENDIF of same level
		parsingModeSKIPTOELSE,		// Skip lines until the next #ELSE of same level
	};
	
	/** Defines a stack of conditionnal parsing levels. The first level will always be parsingModeROOT and will never be popped. */
	class ParsingModeStack : public std::stack<ParsingMode>
	{
	public:
		ParsingModeStack() {
			push(parsingModeROOT);
		}
		
		/** Tells if the current mode is root, in which all lines can be unconditionnaly parsed. */
		bool  isRoot() {
			return (top() == parsingModeROOT); // could be size() == 1
		}
		
		
	};
} // namespace
#endif /* ParsingMode_h */
