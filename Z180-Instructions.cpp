//
//  Z180-Instructions.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 02/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "Assembler.h"
#include "All-Directives.h"
#include "Z80-Operands.h"
#include "Z180-Instructions.h"

using namespace MUZ::Z80;

namespace MUZ {
namespace Z180 {

//MARK: Z-80 instructions with modiied timings and disabled  undocumented Z-80
OperandTools & optools = *new OperandTools;

/** Assemble instruction at current token, returns false if error.
 tok index 0   1   2   3
 ADC A   ,   A/B/C/D/E/H/L
 ADC A   ,   n
 ADC A   ,   (HL)/(IX+d)/(IY+d)
 ADC HL  ,   BC/DE/HL/SP
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionADC::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest, src;
	int d;
	// save current position
	size_t curtoken = codeline.curtoken;
	// ADC A, 8 bit
	OperandError operr = optools.GetReg8(codeline, dest,REGFLAGS::A);
	if (operr == operrOK) {
		if (!GetComma(codeline)) {
			// ADC A
			codeline.AddCode(0x88 + optools.GetSubCode(regA));
			codeline.SetStates(4);
			return true;
		}
		// ADC A,r
		operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
		if (operr == operrOK) {
			codeline.AddCode(0x88 + optools.GetSubCode(src));
			codeline.SetStates(4);
			return true;
		}
//		operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//		if (operr == operrOK) {
//			codeline.AddCode(optools.GetPrefix(src), 0x88 + optools.GetSubCode(src));
//			codeline.SetStates(6);
//			return true;
//		}
		// ADC  A,(HL)
		operr = optools.GetIndHL(codeline);
		if (operr == operrOK) {
			codeline.AddCode(0x8E);
			codeline.SetStates(6);
			return true;
		}
		// ADC A,(X+d)
		operr = optools.GetIndX(codeline, src, d);
		if (operr == operrOK) {
			codeline.AddCode(optools.GetPrefix(src), 0x8E, d);
			codeline.SetStates(14);
			return true;
		}
		// ADC A,n
		operr = optools.GetNum8(codeline, d);
		if (operr == operrOK) {
			codeline.AddCode(0xCE, d);
			codeline.SetStates(6);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);
	}
	// ADC 16 bits
	operr = optools.GetReg16(codeline, dest,REGFLAGS::HL);
	if (operr == operrOK) {
		if (!GetComma(codeline)) return msg.Error(errorMissingComma, codeline);
		// ADC HL, BC DE HL SP
		operr = optools.GetReg16(codeline, src,REGFLAGS::BCDESP | REGFLAGS::HL);
		if (operr == operrOK) {
			codeline.AddCode(0xED, 0x4A + optools.GetSubCode(src));
			codeline.SetStates(10);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);
	}
	// restore initial position, <ADC operand> is equivalent to <ADC A,operand>
	codeline.curtoken = curtoken; //not needed?
	// ADC r
	operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		if (GetComma(codeline)) return msg.Error(errorWrongComma, codeline);
		codeline.AddCode(0x88 + optools.GetSubCode(src));
		codeline.SetStates(4);
		return true;
	}
//	operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(src), 0x88 + optools.GetSubCode(src));
//		codeline.SetStates(8);
//		return true;
//	}
	// ADC  (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		if (GetComma(codeline)) return msg.Error(errorWrongComma, codeline);
		codeline.AddCode(0x8E);
		codeline.SetStates(6);
		return true;
	}
	// ADC (X+d)
	operr = optools.GetIndX(codeline, src, d);
	if (operr == operrOK) {
		if (GetComma(codeline)) return msg.Error(errorWrongComma, codeline);
		codeline.AddCode(optools.GetPrefix(src), 0x8E, d);
		codeline.SetStates(14);
		return true;
	}
	// ADC n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		if (GetComma(codeline)) return msg.Error(errorWrongComma, codeline);
		codeline.AddCode(0xCE, d);
		codeline.SetStates(6);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 tok index 0   1   2   3
 ADD A   ,   A/B/C/D/E/H/L
 ADD A   ,   n
 ADD A   ,   (HL)/(IX+d)/(IY+d)
 ADD HL  ,   BC/DE/HL/SP
 ADD IX  ,   BC/DE/IX/SP
 ADD IY  ,   BC/DE/IY/SP
 allowed:
 ADD A/B/C/D/E/H/L
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionADD::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest, src;
	int d;
	// save current position
	size_t curtoken = codeline.curtoken;
	// ADD A,8bit
	OperandError operr = optools.GetReg8(codeline, dest,REGFLAGS::A);
	if (operr == operrOK) {
		if (!GetComma(codeline)) {
			// ADD A
			codeline.AddCode(0x87);
			codeline.SetStates(4);
			return true;
		}
		// ADD A,r
		operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
		if (operr == operrOK) {
			codeline.AddCode(0x80 + optools.GetSubCode(src));
			codeline.SetStates(4);
			return true;
		}
//		// ADD r,undoc
//		operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//		if (operr == operrOK) {
//			codeline.AddCode(optools.GetPrefix(src), 0x80 + optools.GetSubCode(src));
//			codeline.SetStates(8);
//			return true;
//		}
		// ADD A,(HL)
		operr = optools.GetIndHL(codeline);
		if (operr == operrOK) {
			codeline.AddCode(0x86);
			codeline.SetStates(6);
			return true;
		}
		// ADD A,(X+d)
		operr = optools.GetIndX(codeline, src, d);
		if (operr == operrOK) {
			codeline.AddCode(optools.GetPrefix(src), 0x86, d);
			codeline.SetStates(14);
			return true;
		}
		// ADD A,n
		operr = optools.GetNum8(codeline, d);
		if (operr == operrOK) {
			codeline.AddCode(0xC6, d);
			codeline.SetStates(6);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);;
	}
	// ADD 16 bits
	operr = optools.GetReg16(codeline, dest,REGFLAGS::HLXY);
	if (operr == operrOK) {
		if (!GetComma(codeline))return msg.Error(errorMissingComma, codeline);
		// ADD HL, BC DE HL SP
		// ADD IX, BC DE IX SP
		// ADD IY, BC DE IY SP
		operr = optools.GetReg16(codeline, src,REGFLAGS::BCDESP | REGFLAGS::HLXY);
		if (operr == operrOK) {
			int cycles = 7;
			// HL/IX/IY as source: only accept HL,HL - IX,IX - IY,IY
			if (optools.RegAccept(REGFLAGS::HLXY, src)) {
				if (src != dest) {
					return msg.Error(errorWrongRegister, codeline);
				}
				src = regHL; // mimic ADD HL opcode
			}
			// indexed prefix?
			if ( optools.RegAccept(REGFLAGS::XY, dest)) {
				codeline.AddCode(optools.GetPrefix(dest));
				cycles = 10;
			}
			// ADD HL opcode
			codeline.AddCode(0x09 + optools.GetSubCode(src));
			codeline.SetStates(cycles);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);
	}
	// all checked, maybe shortcut form ADD A / ADD B etc
	// error if there is a comma after next operand
	// restore initial position
	codeline.curtoken = curtoken;// normaly not needed?

	// ADD r
	operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0x80 + optools.GetSubCode(src));
		codeline.SetStates(4);
		return true;
	}
//	// ADD undoc
//	operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(src), 0x80 + optools.GetSubCode(src));
//		codeline.SetStates(8);
//		return true;
//	}
	// ADD (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0x86);
		codeline.SetStates(6);
		return true;
	}
	// ADD (X+d)
	operr = optools.GetIndX(codeline, src, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(src), 0x86, d);
		codeline.SetStates(14);
		return true;
	}
	// ADD n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		codeline.AddCode(0xC6, d);
		codeline.SetStates(6);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 AND A/B/C/D/E/H/L
 AND n
 AND (HL)/(IX+d)/(IY+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionAND::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src;
	int d;
	// save current position
	size_t curtoken = codeline.curtoken;
	OperandError operr = optools.GetReg8(codeline, src, REGFLAGS::A);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// AND A,r
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0xA0 + optools.GetSubCode(src));
				codeline.SetStates(4);
				return true;
			}
//			operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(src), 0xA0 + optools.GetSubCode(src));
//				codeline.SetStates(8);
//				return true;
//			}
			// AND A,(HL)
			operr = optools.GetIndHL(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0xA6);
				codeline.SetStates(6);
				return true;
			}
			// AND A,(X+d)
			operr = optools.GetIndX(codeline, src, d);
			if (operr == operrOK) {
				codeline.AddCode(optools.GetPrefix(src), 0xA6, d);
				codeline.SetStates(14);
				return true;
			}
			// AND A,n
			operr = optools.GetNum8(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0xE6, d);
				codeline.SetStates(6);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		// restore initial position
		codeline.curtoken = curtoken;
	}
	// AND r
	operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xA0 + optools.GetSubCode(src));
		codeline.SetStates(4);
		return true;
	}
//	operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(src), 0xA0 + optools.GetSubCode(src));
//		codeline.SetStates(8);
//		return true;
//	}
	// AND (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xA6);
		codeline.SetStates(6);
		return true;
	}
	// AND (X+d)
	operr = optools.GetIndX(codeline, src, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(src), 0xA6, d);
		codeline.SetStates(14);
		return true;
	}
	// AND n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		codeline.AddCode(0xE6, d);
		codeline.SetStates(6);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 BIT b,r
 b = 0 to 7
 r = A B C D E H L (HL) (IX+d) (IY+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionBIT::Assemble(CodeLine& codeline, ErrorList& msg)
{
	int d=0;
	OperandType src, bit;
	OperandError operr = optools.GetBitNumber(codeline, bit);
	if (operr != operrOK) {
		if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
		return msg.Error(errorTooBigBit, codeline, 2);
	}
	if (!GetComma(codeline)) return msg.Error(errorMissingComma, codeline);
	operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.SetStates(6);
	} else if (operrOK == optools.GetIndHL(codeline)) {
		codeline.SetStates(9);
		src = indHL;
	} else if (operrOK == optools.GetIndX(codeline, src, d)) {
		codeline.SetStates(15);
	} else {
		return msg.Error(errorWrongOperand2, codeline);
	}
	// (IX+d) and (IY+d) are prefixed then use (HL) encoding
	if (optools.RegAccept(REGFLAGS::XY, src)) {
		codeline.AddCode(optools.GetPrefix(src), 0xCB, d, 0x40 + optools.GetSubCode(bit) + optools.GetSubCode(indHL));
	} else {
		codeline.AddCode(0xCB, 0x40 + optools.GetSubCode(bit) + optools.GetSubCode(src));
	}
	return true;
}

/** Assemble instruction at current token, returns false if error

 CALL cond, num16
 CALL num16

 num16 stored low byte first

 condNZ, condZ, condNC, condC, condPO, condPE, condP, condM,
 c4      cc     d4      dc     e4      ec      f4     fc

 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionCALL::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType cond;
	int addr;
	OperandError operr = optools.GetCond(codeline, cond);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetNum16(codeline, addr);
			if (operr == operrOK) {
				codeline.AddCode(0xC4 + optools.GetSubCode(cond), (addr & 0xFF), (addr >> 8));
				codeline.SetStates(6,16);
				return true;
			}
			if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	operr = optools.GetNum16(codeline, addr);
	if (operr == operrOK) {
		codeline.AddCode(0xCD, (addr & 0xFF), (addr >> 8));
		codeline.SetStates(16);
		return true;
	}
	if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 CCF
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionCCF::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x3F);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 CP A/B/C/D/E/H/L
 CP n
 CP (HL)/(IX+d)/(IY+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionCP::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src;
	int d ;
	OperandError operr = optools.GetReg8(codeline, src,REGFLAGS::A);
	if (operr == operrOK) {
		if (!GetComma(codeline)) {
			// CP A
			codeline.AddCode(0xB8 + optools.GetSubCode(src));
			codeline.SetStates(4);
			return true;
		}

		// CP A,r
		operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
		if (operr == operrOK) {
			codeline.AddCode(0xB8 + optools.GetSubCode(src));
			codeline.SetStates(4);
			return true;
		}
//		operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//		if (operr == operrOK) {
//			codeline.AddCode(optools.GetPrefix(src), 0xB8 + optools.GetSubCode(src));
//			codeline.SetStates(8);
//			return true;
//		}
		// CP A,(HL)
		operr = optools.GetIndHL(codeline);
		if (operr == operrOK) {
			codeline.AddCode(0xBE);
			codeline.SetStates(6);
			return true;
		}
		// CP A,(X+d)
		operr = optools.GetIndX(codeline, src, d);
		if (operr == operrOK) {
			codeline.AddCode(optools.GetPrefix(src), 0xBE, d);
			codeline.SetStates(14);
			return true;
		}
		// CP A,n
		operr = optools.GetNum8(codeline, d);
		if (operr == operrOK) {
			codeline.AddCode(0xFE, d);
			codeline.SetStates(6);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);
	}
	// CP r
	operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xB8 + optools.GetSubCode(src));
		codeline.SetStates(4);
		return true;
	}
//	operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(src), 0xB8 + optools.GetSubCode(src));
//		codeline.SetStates(8);
//		return true;
//	}
	// CP (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xBE);
		codeline.SetStates(6);
		return true;
	}
	// CP (X+d)
	operr = optools.GetIndX(codeline, src, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(src), 0xBE, d);
		codeline.SetStates(14);
		return true;
	}
	// CP n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		codeline.AddCode(0xFE, d);
		codeline.SetStates(6);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionCPD::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xA9);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionCPDR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xB9);
	codeline.SetStates(12,14);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionCPI::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xA1);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionCPIR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xB1);
	codeline.SetStates(12,14); // 5 if repeat, 4 when finished
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionCPL::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x2F);
	codeline.SetStates(3); // 5 if repeat, 4 when finished
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionDAA::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x27);
	codeline.SetStates(4); // 5 if repeat, 4 when finished
	return true;
}

/** Assemble instruction at current token, returns false if error
 DEC B  05     5 + 8 * 0
 DEC C  0D     5 + 8 * 1
 DEC D  15     5 + 8 * 2
 DEC E  1D     5 + 8 * 3
 DEC H  25     5 + 8 * 4
 DEC L  2D     5 + 8 * 5
 DEC (HL) 35   5 + 8 * 6
 DEC A  3D     5 + 8 * 7
 DEC (IX+0xAA) dd 35 d
 DEC (IY+0xAA) fd 35 d
 DEC BC        0B
 DEC DE        1B
 DEC HL        2B
 DEC SP        3B
 DEC IX        dd 2B
 DEC IY        fd 2B
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionDEC::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	// DEC r
	OperandError operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0x05 + 8 * optools.GetSubCode(dest));
		codeline.SetStates(4);
		return true;
	}
//	// DEC undoc
//	operr = optools.GetReg8(codeline, dest, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(dest), 0x05 + 8 * optools.GetSubCode(dest));
//		codeline.SetStates(8);
//		return true;
//	}
	// DEC rp
	operr = optools.GetReg16(codeline, dest);
	if (operr == operrOK) {
		// DEC BC DE HL SP
		if (optools.RegAccept(REGFLAGS::BCDEHLSP, dest)) {
			codeline.AddCode(0x0B + optools.GetSubCode(dest));
			codeline.SetStates(4);
			return true;
		}
		// DEC IX IY
		if (optools.RegAccept(REGFLAGS::XY, dest)) {
			codeline.AddCode(optools.GetPrefix(dest), 0x0B + optools.GetSubCode(regHL));
			codeline.SetStates(7);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);
	}
	// DEC (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0x35);
		codeline.SetStates(10);
		return true;
	}
	// DEC (X+d)
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(dest), 0x35, d);
		codeline.SetStates(18);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionDI::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xF3);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionDJNZ::Assemble(CodeLine& codeline, ErrorList& msg)
{
	int d;
	OperandError operr = optools.GetNum16(codeline, d);
	if (operr == operrOK) {
		int depl = (int)codeline.as->GetAddress() + 2 - d;
		if (depl < -126 || depl > +129) {
			msg.Warning(warningTooFar, codeline, 2);
		}
		depl = 0x100 - depl;
		codeline.AddCode(0x10, depl);
		codeline.SetStates(7,9);
		return true;
	}
	if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionEI::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xFB);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 EX AF,AF'  08
 EX DE,HL   EB
 EX (SP),HL E3
 EX (SP),IX dd E3
 EX (SP),IY fd E3

 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionEX::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest, src;
	OperandError operr = optools.GetReg16(codeline, dest,REGFLAGS::AF | REGFLAGS::DE);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetReg16(codeline, src, REGFLAGS::AFp | REGFLAGS::HL);
			if (operr == operrOK) {
				// EX AF,AF'
				if (src == regAFp && dest == regAF) {
					codeline.AddCode(0x08);
					codeline.SetStates(4);
					return true;
				}
				// EX DE,HL
				if (src == regHL && dest == regDE) {
					codeline.AddCode(0xEB);
					codeline.SetStates(3);
					return true;
				}
				// wrong syntax,  EX AF,HL or EX DE,AF'
				return msg.Error(errorWrongRegister, codeline); // useless
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	operr = optools.GetIndSP(codeline);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// EX (SP), HL/IX/IY
			operr = optools.GetReg16(codeline, src, REGFLAGS::HLXY);
			if (operr == operrOK) {
				if (src == regHL) {
					codeline.AddCode(0xE3);
					codeline.SetStates(16);
					return true;
				}
				codeline.AddCode(optools.GetPrefix(src), 0xE3);
				codeline.SetStates(19);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
			}
		return msg.Error(errorMissingComma, codeline);
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionEXX::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xD9);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionHALT::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x76);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 IM 0 ED46
 IM 1 ED56
 IM 2 ED5E

 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionIM::Assemble(CodeLine& codeline, ErrorList& msg)
{
	int m;
	OperandError operr = optools.GetNum8(codeline, m);
	if (operr == operrOK) {
		if (m >= 0 && m <= 2) {
			const int code[3] = { 0x46, 0x56, 0x5E };
			codeline.AddCode(0xED, code[m]);
			codeline.SetStates(6);
			return true;
		}
		return msg.Error(errorTooBigValue, codeline, 2);
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 IN A,(num8)
 IN r,(C)  ->  A B C D E F H L

 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionIN::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int n;
	// IN r,(C) or IN A,(num8)
	OperandError operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL|REGFLAGS::F);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// IN r,(C)
			operr = optools.GetIndC(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0xED, 0x40 + optools.GetSubCode(dest) * 8);
				codeline.SetStates(9);
				return true;
			}
			// IN A,(num8)
			if (dest == regA && optools.GetInd16(codeline, n) == operrOK) {
				if (n > 255) {
					if (! codeline.as->IsFirstPass()) msg.ForceWarning(warningTooBig8, codeline);
				}
				codeline.AddCode(0xDB, n);
				codeline.SetStates(9);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// IN (C)
	operr = optools.GetIndC(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xED, 0x40 + optools.GetSubCode(regA) * 8);
		codeline.SetStates(9);
		return true;
	}
	// IN (num8)
	operr = optools.GetInd16(codeline, n);
	if (operr == operrOK) {
		if (n > 255) {
			msg.Warning(warningTooBig8, codeline); // pass 1 only
		}
		codeline.AddCode(0xDB, n);
		codeline.SetStates(9);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 INC B  05     4 + 8 * 0
 INC C  0D     4 + 8 * 1
 INC D  15     4 + 8 * 2
 INC E  1D     4 + 8 * 3
 INC H  25     4 + 8 * 4
 INC L  2D     4 + 8 * 5
 INC (HL) 35   4 + 8 * 6
 INC A  3D     4 + 8 * 7
 INC (IX+0xAA) dd 34 d
 INC (IY+0xAA) fd 34 d
 INC BC        03
 INC DE        13
 INC HL        23
 INC SP        33
 INC IX        dd 23
 INC IY        fd 23
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionINC::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	// INC r
	OperandError operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0x04 + 8 * optools.GetSubCode(dest));
		codeline.SetStates(4);
		return true;
	}
//	// INC undoc
//	operr = optools.GetReg8(codeline, dest, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(dest), 0x04 + 8 * optools.GetSubCode(dest));
//		codeline.SetStates(8);
//		return true;
//	}
	// INC rp
	operr = optools.GetReg16(codeline, dest,REGFLAGS::BCDEHLSP | REGFLAGS::XY);
	if (operr == operrOK) {
		// INC IX/IY
		if (optools.RegAccept(REGFLAGS::XY, dest)) {
			codeline.AddCode(optools.GetPrefix(dest), 0x03 + optools.GetSubCode(regHL));
			codeline.SetStates(7);
			return true;
		}
		// INC HL/SP
		codeline.AddCode(0x03 + optools.GetSubCode(dest));
		codeline.SetStates(4);
		return true;
	}
	// INC (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0x34);
		codeline.SetStates(10);
		return true;
	}
	// INC (X+d)
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(dest), 0x34, d);
		codeline.SetStates(18);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

//MARK: TODO: terminer les cycles et renommer cycle en state

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionIND::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xAA);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionINDR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xBA);
	codeline.SetStates(12,14); // 5 when repeat, 4 when finished
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionINI::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xA2);
	codeline.SetStates(12); // 5 when repeat, 4 when finished
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionINIR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xB2);
	codeline.SetStates(12,14); // 5 when repeat, 4 when finished
	return true;
}

/** Assemble instruction at current token, returns false if error
 JP cond, num16
 JP num16
 JP (HL) (IX) (IY)

 num16 is stored low byte first

 condNZ, condZ, condNC, condC, condPO, condPE, condP, condM,
 c2      ca     d2      da     e2      ea      f2     fa
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionJP::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType cond, reg;
	int addr;
	// JP cond,nn
	OperandError operr = optools.GetCond(codeline, cond);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetNum16(codeline, addr);
			if (operr == operrOK) {
				codeline.AddCode(0xC2 + optools.GetSubCode(cond), (addr & 0xFF), (addr >> 8));
				codeline.SetStates(6,9);
				return true;
			}
			if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// JP (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xE9);
		codeline.SetStates(3);
		return true;
	}
//	// JP (X+d)
//	operr = optools.GetIndX(codeline, reg, addr);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(reg), 0xE9);
//		codeline.SetStates(8);
//		return true;
//	}
	// JP (IX) or (IY)
	if (codeline.tokens[codeline.curtoken].type == tokenTypePAROPEN) {
		codeline.curtoken += 1;
		operr = optools.GetReg16(codeline, reg, REGFLAGS::XY);
		if (operr == operrOK) {
			if (codeline.curtoken < codeline.tokens.size()) {
				if (codeline.tokens[codeline.curtoken].type == tokenTypePARCLOSE) {
					codeline.AddCode(optools.GetPrefix(reg), 0xE9);
					codeline.SetStates(6);
					return true;
				}
			}
			return msg.Error(errorMissingParenthesisClose, codeline);
		}
		return msg.Error(errorWrongOperand1, codeline);
	}
	// JP nn
	operr = optools.GetNum16(codeline, addr);
	if (operr == operrOK) {
		codeline.AddCode(0xC3, (addr & 0xFF), (addr >> 8));
		codeline.SetStates(9);
		return true;
	}
	if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error

 JR cond,d
 JR d

 d: 2 complement of $ + 2 - dest address

 condNZ, condZ, condNC, condC,
 20      28     30      38

 invalid: condPO, condPE, condP, condM

 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionJR::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType cond;
	int d;
	// JR cond,nn
	OperandError operr = optools.GetCond(codeline, cond);
	if (operr == operrOK) {
		if (cond == condZ || cond == condNZ || cond == condC || cond == condNC) {
			if (GetComma(codeline)) {
				operr = optools.GetNum16(codeline, d);
				if (operr == operrOK) {
					int depl = (int)codeline.as->GetAddress() + 2 - d;
					if ( depl < -126 || depl > +129) {
						msg.Warning(warningTooFar, codeline, 2);
					}
					depl = 0x100 - depl;
					codeline.AddCode(0x20 + optools.GetSubCode(cond), depl);
					codeline.SetStates(6,8);
					return true;
				}
				if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
				return msg.Error(errorWrongOperand2, codeline);
			}
			return msg.Error(errorMissingComma, codeline);
		}
		return msg.Error(errorWrongCondition, codeline);
	}
	// JR nn
	operr = optools.GetNum16(codeline, d);
	if (operr == operrOK) {
		int depl = (int)codeline.as->GetAddress() + 2 - d;
		if (depl < -126 || depl > +129) {
			msg.Warning(warningTooFar, codeline, 2);
		}
		depl = 0x100 - depl; // 2-complement
		codeline.AddCode(0x18, depl);
		codeline.SetStates(8);
		return true;
	}
	if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble LD instruction at current token, returns false if error

 LD rd,rs           B C D E H L (HL) A    $40 + rd << 3 + rs
					A,I                   $ED $57
					A,R                   $ED $5F
					I,A                   $ED $47
					R,A                   $ED $4F

 LD rd,(ix)         (HL) (IX+d) (IY+d)	  ($DD,$FD) $46 + rd << 3
 LD (ix),rs         (HL) (IX+d) (IY+d)	  ($DD,$FD) $70 + rs

 LD rd,num8         B C D E H L (HL) A    $70 + rd << 3

 LD A,(num8)                              $3E n

 LD A,(BC)                                $0A
 LD A,(DE)                                $1A
 LD (BC),A                                $02
 LD (DE),A                                $12

 LD rp,num16
 LD IX,num16
 LD IY,num16

 LD rp,(addr)       BC DE HL SP
 LD IX,(addr)
 LD IY,(addr)
 LD HL,(addr)

 LD (HL),n                                $36 n

 LD (addr),A
 LD (addr),BC
 LD (addr),DE
 LD (addr),HL
 LD (addr),IX
 LD (addr),IY
 LD (addr),SP

 LD SP,HL
 LD SP,IX
 LD SP,IY

 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionLD::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src, dest;
	int d;
	// LD I/R,A
	OperandError operr = optools.GetReg8(codeline, dest, REGFLAGS::I | REGFLAGS::R);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetReg8(codeline, src, REGFLAGS::A);
			if (operr == operrOK) {
				codeline.AddCode(0xED, 0x40 + optools.GetSubCode(src));
				codeline.SetStates(6);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}

	// LD SP,(nn) HL IX IY nn
	operr = optools.GetReg16(codeline, dest, REGFLAGS::SP);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// LD SP, HL IX IY
			operr = optools.GetReg16(codeline, src, REGFLAGS::HLXY);
			if (operr == operrOK) {
				int cycles = 4;
				if (src != regHL) {
					codeline.AddCode(optools.GetPrefix(src));
					cycles = 7;
				}
				codeline.AddCode(0xF9);
				codeline.SetStates(cycles);
				return true;
			}
			// LD SP,(nn)
			operr = optools.GetInd16(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0xED, 0x7B, d & 0xFF, d >> 8);
				codeline.SetStates(18);
				return true;
			}
			// LD SP,nn
			operr = optools.GetNum16(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0x31, d & 0xFF, d >> 8);
				codeline.SetStates(9);
				return true;
			}
			if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// LD BC DE HL IX IY, (nn) nn
	operr = optools.GetReg16(codeline, dest, REGFLAGS::BC | REGFLAGS::DE | REGFLAGS::HLXY);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetInd16(codeline, d);
			if (operr == operrOK) {
				// LD BC,(nn)  ED 4B nn nn
				// LD DE,(nn)  ED 5B nn nn
				// LD HL,(nn)  2A nn nn
				// LD IX,(nn)  DD 2A nn nn
				// LD IY,(nn)  FD 2A nn nn
				if (dest == regBC) {
					codeline.AddCode(0xED, 0x4B, d & 0xFF, d >> 8);
					codeline.SetStates(18);
					return true;
				} else if (dest == regDE) {
					codeline.AddCode(0xED, 0x5B, d & 0xFF, d >> 8);
					codeline.SetStates(18);
					return true;
				}
				int cycles  = 15;
				if (dest != regHL) {
					codeline.AddCode(optools.GetPrefix(dest));
					cycles = 18;
				}
				codeline.AddCode(0x2A, d & 0xFF, d >> 8);
				codeline.SetStates(cycles);
				return true;
			}
			operr = optools.GetNum16(codeline, d);
			if (operr == operrOK) {
				// LD BC,nn  01 nn nn
				// LD DE,nn  11 nn nn
				// LD HL,nn  21 nn nn
				// LD IX,nn  DD 21 nn nn
				// LD IY,nn  FD 21 nn nn
				if (dest == regBC) {
					codeline.AddCode(0x01, d & 0xFF, d >> 8);
					codeline.SetStates(9);
					return true;
				} else if (dest == regDE) {
					codeline.AddCode(0x11, d & 0xFF, d >> 8);
					codeline.SetStates(9);
					return true;
				}
				int cycles  = 9;
				if (dest != regHL) {
					codeline.AddCode(optools.GetPrefix(dest));
					cycles = 12;
				}
				codeline.AddCode(0x21, d & 0xFF, d >> 8);
				codeline.SetStates(cycles);
				return true;
			}
			if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
		}
	// LD (HL) , n r
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// LD (HL), r
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0x70 + optools.GetSubCode(src));
				codeline.SetStates(7);
				return true;
			}
			// LD (HL), n
			operr = optools.GetNum8(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0x36, d);
				codeline.SetStates(9);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// LD (IX+d) (IY+d), n r
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(optools.GetPrefix(dest), 0x70 + optools.GetSubCode(src), d);
				codeline.SetStates(15);
				return true;
			}
			operr = optools.GetNum8(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(optools.GetPrefix(dest), 0x36, d);
				codeline.SetStates(15);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// LD (BC),A
	operr = optools.GetIndBC(codeline);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetReg8(codeline, src, REGFLAGS::A);
			if (operr == operrOK) {
				codeline.AddCode(0x02);
				codeline.SetStates(7);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// LD (DE),A
	operr = optools.GetIndDE(codeline);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetReg8(codeline, src, REGFLAGS::A);
			if (operr == operrOK) {
				codeline.AddCode(0x12);
				codeline.SetStates(7);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// LD (nn), A BC DE HL IX IY SP
	operr = optools.GetInd16(codeline, d);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetReg8(codeline, src, REGFLAGS::A);
			if (operr == operrOK) {
				codeline.AddCode(0x32, d & 0xFF, d >> 8);
				codeline.SetStates(13);
				return true;
			}
			operr = optools.GetReg16(codeline, src, REGFLAGS::BCDEHLSP | REGFLAGS::XY);
			if (operr == operrOK) {
				if (src == regBC) {
					codeline.AddCode(0xED, 0x43, d & 0xFF, d >> 8);
					codeline.SetStates(19);
					return true;
				}
				if (src == regDE) {
					codeline.AddCode(0xED, 0x53, d & 0xFF, d >> 8);
					codeline.SetStates(19);
					return true;
				}
				if (src == regSP) {
					codeline.AddCode(0xED, 0x73, d & 0xFF, d >> 8);
					codeline.SetStates(19);
					return true;
				}
				// HL, IX, IY
				int cycles = 16;
				if (src == regIX || src == regIY) {
					codeline.AddCode(optools.GetPrefix(src));
					cycles = 19;
				}
				codeline.AddCode(0x22, d & 0xFF, d >> 8);
				codeline.SetStates(cycles);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// LD A, (BC) (DE) (HL) (IX+d) (IY+d) n A B C D E H L I R
	operr = optools.GetReg8(codeline, dest, REGFLAGS::A);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// LD A,I R
			operr = optools.GetReg8(codeline, src, REGFLAGS::I | REGFLAGS::R);
			if (operr == operrOK) {
				codeline.AddCode(0xED, 0x50 + optools.GetSubCode(src));
				codeline.SetStates(6);
				return true;
			}
			// LD A, r
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0x78 + optools.GetSubCode(src));
				codeline.SetStates(4);
				return true;
			}
//			// LD A,undoc
//			operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(src), 0x78 + optools.GetSubCode(src));
//				codeline.SetStates(8);
//				return true;
//			}
			// LD A,(HL)
			operr = optools.GetIndHL(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0x7E);
				codeline.SetStates(6);
				return true;
			}
			// LD A,(BC)
			operr = optools.GetIndBC(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0x0A);
				codeline.SetStates(6);
				return true;
			}
			// LD A,(DE)
			operr = optools.GetIndDE(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0x1A);
				codeline.SetStates(6);
				return true;
			}
			// LD A,(X+d)
			operr = optools.GetIndX(codeline, src, d);
			if (operr == operrOK) {
				codeline.AddCode(optools.GetPrefix(src), 0x7E, d);
				codeline.SetStates(14);
				return true;
			}
			// LD A,(nn)
			operr = optools.GetInd16(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0x3A, d & 0xFF, d >> 8);
				codeline.SetStates(12);
				return true;
			}
			// LD A,n
			operr = optools.GetNum8(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0x3E, d);
				codeline.SetStates(6);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	// LD r,r (HL) (X+d) n
	operr = optools.GetReg8(codeline, dest, REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// LD r,r
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0x40 + 8 * optools.GetSubCode(dest) + optools.GetSubCode(src));
				codeline.SetStates(4);
				return true;
			}
//			// LD r,undoc
//			operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(src), 0x40 + 8 * optools.GetSubCode(dest) + optools.GetSubCode(src));
//				codeline.SetStates(8);
//				return true;
//			}
			// LD r,(HL)
			operr = optools.GetIndHL(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0x46 + 8 * optools.GetSubCode(dest));
				codeline.SetStates(6);
				return true;
			}
			// LD r,(x+d)
			operr = optools.GetIndX(codeline, src, d);
			if (operr == operrOK) {
				codeline.AddCode(optools.GetPrefix(src), 0x46 + 8 * optools.GetSubCode(dest), d );
				codeline.SetStates(14);
				return true;
			}
			// LD r,n
			operr = optools.GetNum8(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0x06 + 8 * optools.GetSubCode(dest), d);
				codeline.SetStates(6);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
//	// LD undoc,r n
//	operr = optools.GetReg8(codeline, dest, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// LD undoc,r
//			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0x40 + 8 * optools.GetSubCode(dest) + optools.GetSubCode(src));
//				codeline.SetStates(8);
//				return true;
//			}
//			// LD undoc,undoc
//			operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0x40 + 8 * optools.GetSubCode(dest) + optools.GetSubCode(src));
//				codeline.SetStates(8);
//				return true;
//			}
//			// LD undoc,n
//			operr = optools.GetNum8(codeline, d);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0x06 + 8 * optools.GetSubCode(dest), d);
//				codeline.SetStates(11);
//				return true;
//			}
//			return msg.Error(errorWrongOperand2, codeline);
//		}
//		return msg.Error(errorMissingComma, codeline);
//	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionLDD::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xA8);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionLDDR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xB8);
	codeline.SetStates(12,14);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionLDI::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xA0);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionLDIR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xB0);
	codeline.SetStates(12,14);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionNEG::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x44);
	codeline.SetStates(6);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionNOP::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x00);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 AND A/B/C/D/E/H/L
 AND n
 AND (HL)/(IX+d)/(IY+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionOR::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src;
	int d;
	// save current position
	size_t curtoken = codeline.curtoken;
	OperandError operr = optools.GetReg8(codeline, src, REGFLAGS::A);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// OR A,r
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0xB0 + optools.GetSubCode(src));
				codeline.SetStates(4);
				return true;
			}
//			// OR A,undoc
//			operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(src), 0xB0 + optools.GetSubCode(src));
//				codeline.SetStates(8);
//				return true;
//			}
			// OR A,(HL)
			operr = optools.GetIndHL(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0xB6);
				codeline.SetStates(6);
				return true;
			}
			// OR A,(X+d)
			operr = optools.GetIndX(codeline, src, d);
			if (operr == operrOK) {
				codeline.AddCode(optools.GetPrefix(src), 0xB6, d);
				codeline.SetStates(14);
				return true;
			}
			// OR A,n
			operr = optools.GetNum8(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0xF6, d);
				codeline.SetStates(6);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		// restore initial position
		codeline.curtoken = curtoken;
	}
	// OR r
	operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xB0 + optools.GetSubCode(src));
		codeline.SetStates(4);
		return true;
	}
//	// OR undoc
//	operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(src), 0xB0 + optools.GetSubCode(src));
//		codeline.SetStates(8);
//		return true;
//	}
	// OR (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xB6);
		codeline.SetStates(6);
		return true;
	}
	// OR (X+d)
	operr = optools.GetIndX(codeline, src, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(src), 0xB6, d);
		codeline.SetStates(14);
		return true;
	}
	// OR n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		codeline.AddCode(0xF6, d);
		codeline.SetStates(6);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionOTDR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xBB);
	codeline.SetStates(12,14);
	return true;
}


/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionOTIR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xB3);
	codeline.SetStates(12,14);
	return true;
}

/** Assemble instruction at current token, returns false if error
 OUT (num8),A
 OUT (C),r  ->  A B C D E   H L
 OUT (C), 0 (undoc)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionOUT::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src;
	int n;
	OperandError operr = optools.GetIndC(codeline);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// OUT (C),r
			operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0xED, 0x41 + optools.GetSubCode(src) * 8);
				codeline.SetStates(10);
				return true;
			}
//			// OUT (C),0 (undoc)
//			operr = optools.GetNum8(codeline, n);
//			if (operr == operrOK)  {
//				if (n == 0) {
//					codeline.AddCode(0xED, 0x71);
//					codeline.SetStates(12);
//					return true;
//				}
//			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	operr = optools.GetInd16(codeline, n);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetReg8(codeline, src,REGFLAGS::A);
			if (operr == operrOK) {
				codeline.AddCode(0xD3, n);
				codeline.SetStates(10);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionOUTD::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xAB);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionOUTI::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0xA3);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionPOP::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	OperandError operr = optools.GetReg16(codeline, dest,REGFLAGS::AF|REGFLAGS::BC|REGFLAGS::DE|REGFLAGS::HLXY);
	if (operr == operrOK) {
		codeline.SetStates(9);
		if (dest == regIX || dest == regIY) {
			codeline.AddCode(optools.GetPrefix(dest));
			dest = regHL;
			codeline.SetStates(12);
		}
		codeline.AddCode(0xC1 + optools.GetSubCode(dest));
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionPUSH::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	OperandError operr = optools.GetReg16(codeline, dest,REGFLAGS::AF|REGFLAGS::BC|REGFLAGS::DE|REGFLAGS::HLXY);
	if (operr == operrOK) {
		codeline.SetStates(11);
		if (dest == regIX || dest == regIY) {
			codeline.AddCode(optools.GetPrefix(dest));
			dest = regHL;
			codeline.SetStates(14);
		}
		codeline.AddCode(0xC5 + optools.GetSubCode(dest));
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionRES::Assemble(CodeLine& codeline, ErrorList& msg)
{
	int d=0;
	OperandType src, bit;
	OperandError operr = optools.GetBitNumber(codeline, bit);
	if (operr != operrOK) {
		if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
		return msg.Error(errorTooBigBit, codeline, 2);
	}
	if (!GetComma(codeline))return msg.Error(errorMissingComma, codeline);
	operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		/// RES b,r
		codeline.SetStates(7);
	} else if (operrOK == optools.GetIndHL(codeline)) {
		/// RES b,(HL)
		codeline.SetStates(13);
		src = indHL;
	} else if (operrOK == optools.GetIndX(codeline, src, d)) {
		/// RES b,(IX+d) or (IY+d)
		codeline.SetStates(19);
//		// 3 arguments? (undoc)
//		if (GetComma(codeline)) {
//			// RES bit,(X+d),reg  (undoc)
//			operr = optools.GetReg8(codeline, reg, REGFLAGS::ABCDEHL);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(src), 0xCB, d, 0x80 + optools.GetSubCode(bit) + optools.GetSubCode(reg));
//				return true;
//			}
//			return msg.Error(errorWrongOperand3, codeline);
//		}
		// 2 arguments form, keep going
	} else {
		return msg.Error(errorWrongOperand2, codeline);
	}
	// (IX+d) and (IY+d) are prefixed then use (HL) encoding
	if (optools.RegAccept(REGFLAGS::XY, src)) {
		codeline.AddCode(optools.GetPrefix(src), 0xCB, d, 0x80 + optools.GetSubCode(bit) + optools.GetSubCode(indHL));
	} else {
		codeline.AddCode(0xCB, 0x80 + optools.GetSubCode(bit) + optools.GetSubCode(src));
	}
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRET::Assemble(CodeLine& codeline, ErrorList& )
{
	OperandType cond;
	OperandError operr = optools.GetCond(codeline, cond);
	if (operr == operrOK) {
		codeline.AddCode(0xC0 + optools.GetSubCode(cond));
		codeline.SetStates(5,10);
		return true;
	}
	codeline.AddCode(0xC9);
	codeline.SetStates(9);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRETI::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x4D);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRETN::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x45);
	codeline.SetStates(12);
	return true;
}

/** Assemble instruction at current token, returns false if error
 RL r
 RL (HL) (X+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionRL::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		// RL (HL)
		codeline.AddCode(0xCB, 0x16);
		codeline.SetStates(13);
		return true;
	}
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// RL (X+d),r (undoc)
//			operr = optools.GetReg8(codeline, src);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x10 + optools.GetSubCode(src));
//				codeline.SetStates(23);
//				return true;
//			}
//			return msg.Error(errorWrongOperand2, codeline);
//		}
		codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x16);
		codeline.SetStates(19);
		return true;
	}
	operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x10 + optools.GetSubCode(dest));
		codeline.SetStates(7);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRLA::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x17);
	codeline.SetStates(3);
	return true;
}
/** Assemble instruction at current token, returns false if error
 RLC r
 RLC (HL) (X+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionRLC::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		// RLC (HL)
		codeline.AddCode(0xCB, 0x06);
		codeline.SetStates(13);
		return true;
	}
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// RLC (X+d),r (undoc)
//			operr = optools.GetReg8(codeline, src);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x00 + optools.GetSubCode(src));
//				codeline.SetStates(19);
//				return true;
//			}
//			return msg.Error(errorWrongOperand2, codeline);
//		}
		codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x06);
		codeline.SetStates(19);
		return true;
	}
	operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x00 + optools.GetSubCode(dest));
		codeline.SetStates(7);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRLCA::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x07);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRLD::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x6F);
	codeline.SetStates(16);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionRR::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		/// RR (HL)
		codeline.AddCode(0xCB, 0x1E);
		codeline.SetStates(13);
		return true;
	}
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// RR (X+d),r (undoc)
//			operr = optools.GetReg8(codeline, src);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x18 + optools.GetSubCode(src));
//				codeline.SetStates(19);
//				return true;
//			}
//			return msg.Error(errorWrongOperand2, codeline);
//		}
		codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x1E);
		codeline.SetStates(19);
		return true;
	}
	operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x18 + optools.GetSubCode(dest));
		codeline.SetStates(7);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRRA::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x1F);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionRRC::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		// RRC (HL)
		codeline.AddCode(0xCB, 0x0E);
		codeline.SetStates(13);
		return true;
	}
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// RRC (X+d),r (undoc)
//			operr = optools.GetReg8(codeline, src);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x08 + optools.GetSubCode(src));
//				codeline.SetStates(19);
//				return true;
//			}
//		}
		codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x0E);
		codeline.SetStates(19);
		return true;
	}
	operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x08 + optools.GetSubCode(dest));
		codeline.SetStates(7);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRRCA::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x0F);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionRRD::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x67);
	codeline.SetStates(16);
	return true;
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionRST::Assemble(CodeLine& codeline, ErrorList& msg)
{
	int addr;
	OperandError operr = optools.GetNum8(codeline, addr);
	if (operr == operrOK) {
		if (addr <= 0x38 && addr % 8 == 0) {
			codeline.AddCode(0xC7 + addr);
			codeline.SetStates(11);
			return true;
		}
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 SBC A   ,   A/B/C/D/E/H/L
 SBC A   ,   n
 SBC A   ,   (HL)/(IX+d)/(IY+d)
 SBC HL  ,   BC/DE/HL/SP
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionSBC::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest, src;
	int d;
	// save current position
	size_t curtoken = codeline.curtoken;
	// SBC A, 8 bit
	OperandError operr = optools.GetReg8(codeline, dest,REGFLAGS::A);
	if (operr == operrOK) {
		if (!GetComma(codeline)) {
			// SBC A
			codeline.AddCode(0x98 + optools.GetSubCode(regA));
			codeline.SetStates(4);
			return true;
		}
		// SBC A,r
		operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
		if (operr == operrOK) {
			codeline.AddCode(0x98 + optools.GetSubCode(src));
			codeline.SetStates(4);
			return true;
		}
//		// SBC A,undoc
//		operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//		if (operr == operrOK) {
//			codeline.AddCode(optools.GetPrefix(src), 0x98 + optools.GetSubCode(src));
//			codeline.SetStates(8);
//			return true;
//		}
		// SBC A,(HL)
		operr = optools.GetIndHL(codeline);
		if (operr == operrOK) {
			codeline.AddCode(0x9E);
			codeline.SetStates(6);
			return true;
		}
		// SBC A,(X+d)
		operr = optools.GetIndX(codeline, src, d);
		if (operr == operrOK) {
			codeline.AddCode(optools.GetPrefix(src), 0x9E, d);
			codeline.SetStates(14);
			return true;
		}
		// SBC A,n
		operr = optools.GetNum8(codeline, d);
		if (operr == operrOK) {
			codeline.AddCode(0xDE, d);
			codeline.SetStates(6);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);
	}
	// SBC 16 bits
	operr = optools.GetReg16(codeline, dest,REGFLAGS::HL);
	if (operr == operrOK) {
		if (!GetComma(codeline)) return msg.Error(errorMissingComma, codeline);
		// SBC HL, BC DE HL SP
		operr = optools.GetReg16(codeline, src,REGFLAGS::BCDESP | REGFLAGS::HL);
		if (operr == operrOK) {
			codeline.AddCode(0xED, 0x42 + optools.GetSubCode(src));
			codeline.SetStates(10);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);
	}
	// restore initial position
	codeline.curtoken = curtoken;// not needed?
	// SBC r
	operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0x98 + optools.GetSubCode(src));
		codeline.SetStates(4);
		return true;
	}
//	// SBC undoc
//	operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(src), 0x98 + optools.GetSubCode(src));
//		codeline.SetStates(8);
//		return true;
//	}
	// SBC (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0x9E);
		codeline.SetStates(6);
		return true;
	}
	// SBC (X+d)
	operr = optools.GetIndX(codeline, src, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(src), 0x9E, d);
		codeline.SetStates(14);
		return true;
	}
	// SBC n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		codeline.AddCode(0xDE, d);
		codeline.SetStates(6);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionSCF::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0x37);
	codeline.SetStates(3);
	return true;
}

/** Assemble instruction at current token, returns false if error
 SET b,r
 b = 0 to 7
 r = A B C D E H L (HL) (IX+d) (IY+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionSET::Assemble(CodeLine& codeline, ErrorList& msg)
{
	int d=0;
	OperandType src, bit;
	OperandError operr = optools.GetBitNumber(codeline, bit);
	if (operr != operrOK) {
		if (operr == operrWRONGREGISTER) return msg.Error(errorWrongRegister, codeline);
		return msg.Error(errorTooBigBit, codeline, 2);
	}
	if (!GetComma(codeline))return msg.Error(errorMissingComma, codeline);
	if (operrOK == optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL)) {
		codeline.SetStates(7);
	} else if (operrOK == optools.GetIndHL(codeline)) {
		codeline.SetStates(13);
		src = indHL;
	} else if (operrOK == optools.GetIndX(codeline, src, d)) {
		codeline.SetStates(19);
//		// 3 arguments? (undoc)
//		if (GetComma(codeline)) {
//			OperandType reg;
//			// SET bit,(X+d),reg  (undoc)
//			operr = optools.GetReg8(codeline, reg, REGFLAGS::ABCDEHL);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(src), 0xCB, d, 0xC0 + optools.GetSubCode(bit) + optools.GetSubCode(reg));
//				return true;
//			}
//			return msg.Error(errorWrongOperand3, codeline);
//		}
		// 2 arguments form, keep going
	} else {
		return msg.Error(errorWrongOperand2, codeline);
	}
	// (IX+d) and (IY+d) are prefixed then use (HL) encoding
	if (optools.RegAccept(REGFLAGS::XY, src)) {
		codeline.AddCode(optools.GetPrefix(src), 0xCB, d, 0xC0 + optools.GetSubCode(bit) + optools.GetSubCode(indHL));
	} else {
		codeline.AddCode(0xCB, 0xC0 + optools.GetSubCode(bit) + optools.GetSubCode(src));
	}
	return true;
}

/** Assemble instruction at current token, returns false if error
 SLA r
 SLA (HL) (X+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionSLA::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	// SLA (HL)
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x26);
		codeline.SetStates(13);
		return true;
	}
	// SLA (X+d)
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// SLA (X+d),r (undoc)
//			operr = optools.GetReg8(codeline, src);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x20 + optools.GetSubCode(src));
//				codeline.SetStates(23);
//				return true;
//			}
//			return msg.Error(errorWrongOperand2, codeline);
//		}
		codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x26);
		codeline.SetStates(19);
		return true;
	}
	operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x20 + optools.GetSubCode(dest));
		codeline.SetStates(7);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 SLL r
 SLL (HL) (X+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionSLL::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	// SLL(HL)
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x36);
		codeline.SetStates(13);
		return true;
	}
	// SLL (X+d)
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// SLL (X+d),r (undoc)
//			operr = optools.GetReg8(codeline, src);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x30 + optools.GetSubCode(src));
//				codeline.SetStates(23);
//				return true;
//			}
//			return msg.Error(errorWrongOperand2, codeline);
//		}
		codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x36);
		codeline.SetStates(19);
		return true;
	}
	operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x30 + optools.GetSubCode(dest));
		codeline.SetStates(7);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}
/** Assemble instruction at current token, returns false if error
 SRA r
 SRA (HL) (X+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionSRA::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x2E);
		codeline.SetStates(13);
		return true;
	}
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// SRA (X+d),r (undoc)
//			operr = optools.GetReg8(codeline, src);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x28 + optools.GetSubCode(src));
//				codeline.SetStates(19);
//				return true;
//			}
//			return msg.Error(errorWrongOperand2, codeline);
//		}
		codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x2E);
		codeline.SetStates(19);
		return true;
	}
	operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x28 + optools.GetSubCode(dest));
		codeline.SetStates(7);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 SRL r
 SRL (HL) (X+d)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionSRL::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	int d;
	// SRL (HL)
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x3E);
		codeline.SetStates(13);
		return true;
	}
	// SRL (X+d)
	operr = optools.GetIndX(codeline, dest, d);
	if (operr == operrOK) {
//		if (GetComma(codeline)) {
//			// SRL (X+d),r (undoc)
//			operr = optools.GetReg8(codeline, src);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x38 + optools.GetSubCode(src));
//				codeline.SetStates(19);
//				return true;
//			}
//			return msg.Error(errorWrongOperand2, codeline);
//		}
		codeline.AddCode(optools.GetPrefix(dest), 0xCB, d, 0x3E);
		codeline.SetStates(19);
		return true;
	}
	// SRL r
	operr = optools.GetReg8(codeline, dest,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xCB, 0x38 + optools.GetSubCode(dest));
		codeline.SetStates(7);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionSUB::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src;
	int d;
	// SUB (HL)
	OperandError operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0x96);
		codeline.SetStates(6);
		return true;
	}
	// SUB (X+d)
	operr = optools.GetIndX(codeline, src, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(src), 0x96, d);
		codeline.SetStates(14);
		return true;
	}
	// save current position
	size_t curtoken = codeline.curtoken;
	// SUB A, ...
	operr = optools.GetReg8(codeline, src,REGFLAGS::A);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// SUB A,r
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0x90 + optools.GetSubCode(src));
				codeline.SetStates(4);
				return true;
			}
//			// SUB A,undoc
//			operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(src), 0x90 + optools.GetSubCode(src));
//				codeline.SetStates(8);
//				return true;
//			}
			// SUB A,(HL)
			operr = optools.GetIndHL(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0x96);
				codeline.SetStates(6);
				return true;
			}
			// SUB A, (X+d)
			operr = optools.GetIndX(codeline, src, d);
			if (operr == operrOK) {
				codeline.AddCode(optools.GetPrefix(src), 0x96, d);
				codeline.SetStates(14);
				return true;
			}
			// SUB A, n
			operr = optools.GetNum8(codeline, d);
			if (operr == operrOK) {
				if (GetComma(codeline)) return msg.Error(errorWrongComma, codeline);
				codeline.AddCode(0xD6, d);
				codeline.SetStates(6);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		// restore initial position
		codeline.curtoken = curtoken;
	}
	// SUB r
	operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0x90 + optools.GetSubCode(src));
		codeline.SetStates(4);
		return true;
	}
//	// SUB undoc
//	operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(src), 0x90 + optools.GetSubCode(src));
//		codeline.SetStates(8);
//		return true;
//	}
	// SUB n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK)  {
		codeline.AddCode(0xD6, d);
		codeline.SetStates(6);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}


/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionXOR::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src;
	int d;
	// save current position
	size_t curtoken = codeline.curtoken;
	OperandError operr = optools.GetReg8(codeline, src,REGFLAGS::A);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			// XOR A,r
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0xA8 + optools.GetSubCode(src));
				codeline.SetStates(4);
				return true;
			}
//			// XOR A,undoc
//			operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//			if (operr == operrOK) {
//				codeline.AddCode(optools.GetPrefix(src), 0xA8+ optools.GetSubCode(src));
//				codeline.SetStates(8);
//				return true;
//			}
			// XOR A,(HL)
			operr = optools.GetIndHL(codeline);
			if (operr == operrOK) {
				codeline.AddCode(0xAE);
				codeline.SetStates(6);
				return true;
			}
			// XOR A,(X+d)
			operr = optools.GetIndX(codeline, src, d);
			if (operr == operrOK) {
				codeline.AddCode(optools.GetPrefix(src), 0xAE, d);
				codeline.SetStates(14);
				return true;
			}
			// XOR A,n
			operr = optools.GetNum8(codeline, d);
			if (operr == operrOK) {
				codeline.AddCode(0xEE, d);
				codeline.SetStates(6);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		// restore initial position
		codeline.curtoken = curtoken;
	}
	// XOR r
	operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xA8 + optools.GetSubCode(src));
		codeline.SetStates(4);
		return true;
	}
//	// XOR undoc
//	operr = optools.GetReg8(codeline, src, REGFLAGS::UNDOC);
//	if (operr == operrOK) {
//		codeline.AddCode(optools.GetPrefix(src), 0xA8+ optools.GetSubCode(src));
//		codeline.SetStates(8);
//		return true;
//	}
	// XOR (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xAE);
		codeline.SetStates(6);
		return true;
	}
	// XOR (X+d)
	operr = optools.GetIndX(codeline, src, d);
	if (operr == operrOK) {
		codeline.AddCode(optools.GetPrefix(src), 0xAE, d);
		codeline.SetStates(14);
		return true;
	}
	// XOR n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		codeline.AddCode(0xEE, d);
		codeline.SetStates(6);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}


//MARK: Z-180 new instructions

/*** added to the Z-80 instruction set :

	Mnemo			bytes	cycles	states

	MLT ww			2		13		17      ED / 01 ww1 100 (4C/5C/6C/7C)
	MULT ww			idem MLT ww

	OTDM			2		6		14		ED 8B
	OTDMR			2		8		16		ED 9B
	OTIM			2		6		14		ED 83
	OTIMR			2		8		16		ED 93
	IN0 g',(m)		3		4		12		ED / 00 g 000 (00, 08, 10, 18, 20, 28, 30, 38) / m
	OUT0 (m),g''	3		5		13		ED / 00 g 001 (01, 09, 11, 19, 29, 28, 39, 38) / m
	SLP				2		2		8		ED 	76
	TST g			2		3		7		ED / 00 g 100 (04, 0C, 14, 1C, 24, 2C, 34, 3C)
	TST A,g 		idem TST g
	TST m			3		3		9		ED 64 m
	TST A,m			idem TST m
	TSTIO m			3		4		12		ED 74 m
	TST (C),m		idem TSTIO m

	g   : B(000) C(001) D(010) E(011) H(100) L(101) (HL)(110) A(111)
	g'  : B(000) C(001) D(010) E(011) H(100) L(101) F (110)   A(111)
	g'' : B(000) C(001) D(010) E(011) H(100) L(101)           A(111)
	ww  : BC(00) DE(01) HL(10) SP(11)
	m   : 8-bit data

 Synonyms:
 TSTIO 		= TST (C)
 MLT 		= MULT
 TST g 		= TST A,g
 TST m 		= TST A,m
 TST (HL) 	= TST A,(HL)
 */


/** Assemble instruction at current token, returns false if error
 MLT ww			2		13		17      ED / 01 ww1 100 (4C/5C/6C/7C)
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionMLT::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest;
	OperandError operr = optools.GetReg16(codeline, dest, REGFLAGS::BC|REGFLAGS::DE|REGFLAGS::HL|REGFLAGS::SP);
	if (operr == operrOK) {
		codeline.SetStates(13);
		codeline.AddCode(0xED, 0x4C + optools.GetSubCode(dest));
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}
/** Assemble instruction at current token, returns false if error
 OTIMR			2		8		16		ED 93
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionOTIMR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x93);
	codeline.SetStates(8);
	return true;
}
/** Assemble instruction at current token, returns false if error
 OTDMR			2		8		16		ED 9B
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionOTDMR::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x9B);
	codeline.SetStates(8);
	return true;
}

/** Assemble instruction at current token, returns false if error
 OTIM			2		6		14		ED 83
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionOTIM::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x83);
	codeline.SetStates(6);
	return true;
}
/** Assemble instruction at current token, returns false if error
 OTDM			2		6		14		ED 8B
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionOTDM::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x8B);
	codeline.SetStates(6);
	return true;
}

/** Assemble instruction at current token, returns false if error
 IN0 A B C D E F H L, (num8)
 IN0 g',(m)		3		4		12		ED / 00 g 000 (00, 08, 10, 18, 20, 28, 30, 38) / m
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionIN0::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src;
	int n;
	OperandError operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL | REGFLAGS::F);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetInd16(codeline, n);
			if (operr == operrOK) {
				codeline.AddCode(0xED, 0 + (optools.GetSubCode(src) << 3), n);
				codeline.SetStates(4);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 OUT0 (num8), A B C D E   H L
 OUT0 (m),g''	3		5		13		ED / 00 g 001 (01, 09, 11, 19, 21, 29, 31, 39) / m
 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionOUT0::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType src;
	int n;
	OperandError operr = optools.GetInd16(codeline, n);
	if (operr == operrOK) {
		if (GetComma(codeline)) {
			operr = optools.GetReg8(codeline, src, REGFLAGS::ABCDEHL);
			if (operr == operrOK) {
				codeline.AddCode(0xED, 1 + (optools.GetSubCode(src) << 3), n);
				codeline.SetStates(5);
				return true;
			}
			return msg.Error(errorWrongOperand2, codeline);
		}
		return msg.Error(errorMissingComma, codeline);
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 @param codeline the code line in which assembled codes will be stored
 */
bool InstructionSLP::Assemble(CodeLine& codeline, ErrorList& )
{
	codeline.AddCode(0xED, 0x76);
	codeline.SetStates(2);
	return true;
}

/** Assemble instruction at current token, returns false if error
 tok index 0   1   2   3
 TST A   ,   B/C/D/E/H/L/(HL)/A   ED 04/0C/14/1C/24/2C/34/3C
 TST A   ,   m                    ED 64 m
 TST A   ,   (HL)                 ED 34
 TST (C) ,   m                    ED 74 m

 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionTST::Assemble(CodeLine& codeline, ErrorList& msg)
{
	OperandType dest, src;
	int d;
	// save current position
	size_t curtoken = codeline.curtoken;
	// TST A[,8bit]
	OperandError operr = optools.GetReg8(codeline, dest,REGFLAGS::A);
	if (operr == operrOK) {
		if (!GetComma(codeline)) {
			// TST A
			codeline.AddCode(0xED, 0x3C);
			codeline.SetStates(3);
			return true;
		}
		// TST A,g
		operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
		if (operr == operrOK) {
			codeline.AddCode(0xED, 0x04 + (optools.GetSubCode(src) << 3));
			codeline.SetStates(3);
			return true;
		}
		// TST A,(HL)
		operr = optools.GetIndHL(codeline);
		if (operr == operrOK) {
			codeline.AddCode(0xED, 0x34);
			codeline.SetStates(4);
			return true;
		}
		// TST A,n
		operr = optools.GetNum8(codeline, d);
		if (operr == operrOK) {
			codeline.AddCode(0xED, 0x64, d);
			codeline.SetStates(3);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);;
	}
	// all checked, maybe shortcut form ADD A / ADD B etc
	// error if there is a comma after next operand
	// restore initial position
	codeline.curtoken = curtoken;// normaly not needed?

	// TST g
	operr = optools.GetReg8(codeline, src,REGFLAGS::ABCDEHL);
	if (operr == operrOK) {
		codeline.AddCode(0xED, 0x04 + (optools.GetSubCode(src) << 3));
		codeline.SetStates(3);
		return true;
	}
	// TST (HL)
	operr = optools.GetIndHL(codeline);
	if (operr == operrOK) {
		codeline.AddCode(0xED, 0x34);
		codeline.SetStates(4);
		return true;
	}
	// TST (C),n
	operr = optools.GetIndC(codeline);
	if (operr == operrOK) {
		operr = optools.GetNum8(codeline, d);
		if (operr == operrOK) {
			codeline.AddCode(0xED, 0x74, d);
			codeline.SetStates(4);
			return true;
		}
		return msg.Error(errorWrongOperand2, codeline);;
	}
	// TST n
	operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		codeline.AddCode(0xED, 0x64, d);
		codeline.SetStates(3);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);
}

/** Assemble instruction at current token, returns false if error
 tok index 0   1   2   3
 TSTIO   m                    ED 74 m

 @param codeline the code line in which assembled codes will be stored
 @param msg the message list which will receive any warning or error information
 */
bool InstructionTSTIO::Assemble(CodeLine& codeline, ErrorList& msg)
{
	int d;
	// save current position
	OperandError operr = optools.GetNum8(codeline, d);
	if (operr == operrOK) {
		codeline.AddCode(0xED, 0x74, d);
		codeline.SetStates(4);
		return true;
	}
	return msg.Error(errorWrongOperand1, codeline);;
}

} // namespace Z180
} // namespace MUZ

