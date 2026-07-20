//
//  Instruction.hpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 02/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef Instruction_h
#define Instruction_h

#include <vector>
#include <string>
#include <unordered_map>

#include "Errors.h"

namespace MUZ {
	
	/** Base class for each possible CPU instruction name.
	 Each derived instruction class will implement its operands detection and use.
	 There may be a lot of different functioning modes for an instruction, but there is only
	 one derived class for each instruction name, so the instruction class will have to manage
	 any possible sub-deriving to delegate its work to different internal derived classes.
	*/
	class Instruction
	{
	public:
		virtual ~Instruction() {}
		/** Assemble the instruction at current token, returns false if error
		 @param codeline the code line in which assembled codes will be stored
		 @param msg the message list which will receive any warning or error information
		 */
		virtual bool Assemble(struct CodeLine& codeline, ErrorList& msg) {
			msg.Fatal(ErrorKind::errorNonDerivedInstruction, codeline);
			return true;
		};
		
	};
	
	// Type for the map of all possible instructions
	typedef std::unordered_map<std::string, Instruction*> InstructionsMap;
	
} // namespace MUZ
#endif /* Instruction_h */
