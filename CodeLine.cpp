//
//  CodeLine.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 28/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "CodeLine.h"
#include "Assembler.h"

namespace MUZ {
	
	/** Resets the token exploration system to the first token after the instruction. */
	void CodeLine::ResetInstruction(size_t start) {
		instructiontoken = start;
		curtoken = (size_t)instructiontoken + 1;
	}
	
	/** Set machine cycles. */
	void CodeLine::SetStates(int mintime, int maxtime) {
		statesmin = mintime;
		statesmax = maxtime > 0 ? maxtime : mintime;
	}
	
	/** Pushes codes. */
	void CodeLine::ResetCode() {
		code.clear();
	}
	void CodeLine::AddCode(int b0) {
		code.push_back(b0 & DATAMASK);
	}
	void CodeLine::AddCode(int b0, int b1) {
		AddCode(b0);
		AddCode(b1);
	}
	void CodeLine::AddCode(int b0, int b1, int b2) {
		AddCode(b0);
		AddCode(b1);
		AddCode(b2);
	}
	void CodeLine::AddCode(int b0, int b1, int b2, int b3) {
		AddCode(b0);
		AddCode(b1);
		AddCode(b2);
		AddCode(b3);
	}
} // namespace
