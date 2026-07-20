//
//  Z80-Operands.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 23/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef Z80_Operands_h
#define Z80_Operands_h

#include "ExpVector.h"
#include "CodeLine.h"
#include "Errors.h"
#include "Expression.h"
#include <map>

namespace MUZ {
namespace Z80 {
		
	/** Enumeration for the possible Z-80 operand types. This is only used in Z-80-Operands.CPP and
		 Z80-Instructions.cpp.
	*/
		enum OperandType {

		// 8 bits registers, returned by reg8() function
		regA, regB, regC, regD, regE, regH, regL,
		regI, regR, regF,

		// 16 bits registers, returned by reg16() function
		regAF, regAFp, regBC, regDE, regHL, regSP, regIX, regIY,

		// undocumented 8-bit, works with IX and IY prefix on instructions targetting H or L
		regIXH, regIXL, regIYH, regIYL,

		// indirect via 16 bits registers
		indHL,		// (HL), generally can be used at same places as A B C D E H L, and is used for (IX+d) / (IY+d) forms
		indBC,      // (BC) only in LD with A
		indDE,		// (DE) only in LD with A
		indSP,		// (SP) only in EX (SP),HL and (X+d) forms
		indC,		// (C), only in IN and OUT


		// bit number 0 to 7
		bit0, bit1, bit2, bit3, bit4, bit5, bit6, bit7,

		// conditions cond
		condNZ, condZ, condNC, condC, condPO, condPE, condP, condM,

		// numeric operands n nn
		num8, num16,

		// indirect via 16 bits number (nn)
		ind16,
	};

	/** Register flags are given to high level analysing functions to accept specific registers.
	 The flags aare in the same order as the register codes in the OperandType enum. A check is done in DEBUG build to ensure
	 this order is respected.*/
	enum REGFLAGS {

		// 8 bits registers, returned by reg8() function
		/** Accept register A. */
		A		= 1 << 0,
		/** Accept register B. */
		B		= 1 << 1,
		/** Accept register C. */
		C		= 1 << 2,
		/** Accept register D. */
		D		= 1 << 3,
		/** Accept register E. */
		E		= 1 << 4,
		/** Accept register H. */
		H		= 1 << 5,
		/** Accept register L. */
		L		= 1 << 6,
		/** Accept register I. */
		I		= 1 << 7,
		/** Accept register R. */
		R		= 1 << 8,
		/** Accept register F. */
		F		= 1 << 9,

		// 16 bits registers, returned by reg16() function
		/** Accept register AF. */
		AF		= 1 << 10,
		/** Accept register AF'. */
		AFp		= 1 << 11,
		/** Accept register BC. */
		BC		= 1 << 12,
		/** Accept register DE. */
		DE		= 1 << 13,
		/** Accept register HL. */
		HL		= 1 << 14,
		/** Accept register SP. */
		SP		= 1 << 15,
		/** Accept register IX. */
		IX		= 1 << 16,
		/** Accept register IY. */
		IY		= 1 << 17,

		// undocumented 8-bits
		/** Accept register IXH. */
		IXH 	= 1 << 18,
		/** Accept register IXL. */
		IXL		= 1 << 19,
		/** Accept register IYH. */
		IYH 	= 1 << 20,
		/** Accept register IYL. */
		IYL		= 1 << 21,

		// Common combinations used by instructions
		/** Combination of flags for normal 8 bit registers.*/
		ABCDEHL = A | B | C | D | E | H | L,
		/** Combination of flags for undocumented 8-bit registers. */
		UNDOC = IXH | IXL | IYH | IYL,
		/** Combination of flags for normal 16-bit registers. */
		BCDESP = BC | DE | SP,
		/** Combination of flags for normal 16-bit registers and SP. */
		BCDEHLSP = BC | DE | HL | SP,
		/** Combination of flags for IX and IY registers. */
		XY = IX | IY,
		/** Combination of flags for HL, IX and IY registers. */
		HLXY = HL | IX | IY,
	};

	/** Low level interpreting functions for each family of operand type. Each function parses the current token and returns
		an operand type, and a value when appropriate.
		The indirect family handle the parenthesis tokens and the "+d" part where appropriate.
		The functions handling numbers handle exxpressions until next separator. */
	class OperandTools {

		// Specialized Expression evaluators for restricted types
		ExpressionEvaluator evalString;
		ExpressionEvaluator evalBool;
		ExpressionEvaluator evalNumber;

#ifdef DEBUG
	public:
#endif

		/** Parses current token and return the code for a 8-bit register regA to regH, regI or regR and undocumented. */
		bool reg8( ExpVector* tokens, int& curtoken, OperandType& reg8 );
		/** Parses current token and return the code for a 16-bit register regAF, regAFp regBC regDE regHL regSP IX or regIY. */
		bool reg16( ExpVector* tokens, int& curtoken, OperandType& reg16 );
		/** Parses current token and return the code for an indirect access via(C): indC. */
		bool indirectC( ExpVector* tokens, int& curtoken, OperandType& regC );
		/** Parses current token and return the code for an indirect access via (HL): indHL. */
		bool indirectHL( ExpVector* tokens, int& curtoken, OperandType& regHL );
		/** Parses current token and return the code for an indirect access via (BC): indBC. */
		bool indirectBC( ExpVector* tokens, int& curtoken, OperandType& regBC );
		/** Parses current token and return the code for an indirect access via (DE): indDE. */
		bool indirectDE( ExpVector* tokens, int& curtoken, OperandType& regDE );
		/** Parses current token and return the code for an indirect access via (SP): indSP. */
		bool indirectSP( ExpVector* tokens, int& curtoken, OperandType& regSP );
		/** Parses current token and return the code for an indirect access via (IX+d) and (IY+d): indIX, indIY. May issue a warning or error for invalid value or missing closing parenthesis. */
		OperandError indirectX( ExpVector* tokens, int& curtoken, OperandType& regX, int& value );
		/** Parses current token and return the code for a bit nunmber: bit0 to bit7. May issue an error if out of range. */
		OperandError bitnumber( ExpVector* tokens, int& curtoken, OperandType& bit );
		/** Parses current token and return the code for a condition name: condNZ to condM. */
		OperandError condition( ExpVector* tokens, int& curtoken, OperandType& cond );
		/** Parses current token and return the value for an 8-bit number. Issue a warning if out of range. */
		OperandError number8( ExpVector* tokens, int& curtoken, int& value );
		/** Parses current token and return the value for a 16-bit number. Issue a warning if out of range. */
		OperandError number16( ExpVector* tokens, int& curtoken, int& value );
		/** Computes a 16-bit value from a numeric expression between parenthesis. If parenthesis or a value cannot be found,
		 returns an error code. The last used token index is returned even if the expression doesn't compute a number but
		 have correct parenthesis. */
		OperandError indirect16( ExpVector* tokens, int curtoken, int& value, int& lasttoken);


		/** High level functions for analyzing a CodeLine operands. These functions accepts a combination of flags which tells which registers are accepted. The flags aare in the same order as the register codes in the OperandType enum. A check is done in DEBUG build to ensure
			this order is respected.*/

		// register flags must be in the same order as OperandType registers:
		//	regA, regB, regC, regD, regE, regH, regL,
		//	regI, regR, regF,
		//	regAF, regAFp, regBC, regDE, regHL, regSP, regIX, regIY,
		//  regIXH, regIXL, regIYH, regIYL,

	public:
		// init
		OperandTools();
		~OperandTools();

		/** Tells if a register code is accepted by a flag combination.
		 @param flags flag combination telling which registers are accepted
		 @param reg the code for a register
		 @return true if one registers is accepted by the flag combination
		 */
		bool RegAccept(unsigned int flags, OperandType reg) ;

		/** Gets the subcode for a register code: this is a code for registers which is added to a base in a lot of instructions.*/
		int GetSubCode(OperandType reg);

		/** Gets the prefix for IX or IY register. */
		int GetPrefix(OperandType reg);

		/** Tests if current token is recognized as an 8-bit register, and go next token. Don't change current token if not found.
			@param codeline the code line to assemble, with curtoken set to the start token for analysis
			@param reg [OUT] returns the code for the 8-bit register when found
			@param regs flag combination telling which 8-bit registers are accepted
			@return errOK if one of the accepted registers has been found
		 */
		MUZ::OperandError GetReg8(struct MUZ::CodeLine& codeline, OperandType& reg, unsigned int regs = 0xFFFFFFFF ) ;

		/** Tests if current token is recognized as an 16-bit register, and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @param reg [OUT] returns the code for the 16-bit register when found
		 @param regs flag combination telling which 16-bit registers are accepted
		 @return errOK if one of the accepted registers has been found
		 */
		MUZ::OperandError GetReg16(struct MUZ::CodeLine& codeline, OperandType& reg, unsigned int regs = 0xFFFFFFFF ) ;

		/** Tests if current token is recognized as (C), and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @return errOK if (C) has been found
		 */
		MUZ::OperandError GetIndC( struct MUZ::CodeLine& codeline ) ;

		/** Tests if current token is recognized as (HL), and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @return errOK if (HL) has been found
		 */
		MUZ::OperandError GetIndHL( struct MUZ::CodeLine& codeline ) ;

		/** Tests if current token is recognized as (HL), and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @return errOK if (BC) has been found
		 */
		MUZ::OperandError GetIndBC( struct MUZ::CodeLine& codeline ) ;

		/** Tests if current token is recognized as (HL), and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @return errOK if (DE) has been found
		 */
		MUZ::OperandError GetIndDE( struct MUZ::CodeLine& codeline );

		/** Tests if current token is recognized as (SP), and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @return errOK if (SP) has been found
		 */
		MUZ::OperandError GetIndSP( struct MUZ::CodeLine& codeline );

		/** Tests if current token is recognized as (IX+d) or (IY+d), and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @param [OUT] regX the code for the register found (regIX or regIY)
		 @param [OUT] value the 8-bit value for "+d" part
		 @return errOK if (IX+d) or (IY+d) has been found
		 */
		MUZ::OperandError GetIndX( struct MUZ::CodeLine& codeline, OperandType& regX, int& value );

		/** Tests if current token is recognized as a bit number (0-7), and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @param [OUT] bit the code for bit0 to bit7
		 @return operrOK if a number between 0 and 7 has been found
		 */
		MUZ::OperandError GetBitNumber( struct MUZ::CodeLine& codeline, OperandType& bit );

		/** Tests if current token is recognized as a condition, and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @param [OUT] cond the code for the condition
		 @return errOK if a conditoin name has been found
		 */
		MUZ::OperandError GetCond( struct MUZ::CodeLine& codeline, OperandType& cond ) ;

		/** Tests if current token is recognized as an 8-bit number, and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @param [OUT] value the 8-bit value found
		 @return operrOK if an 8-bit value has been found
		 */
		MUZ::OperandError GetNum8( struct MUZ::CodeLine& codeline, int& value );

		/** Tests if current token is recognized as an 16-bit number, and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @param [OUT] value the 16-bit value found
		 @return operrOK if an 8-bit value has been found
		 */
		MUZ::OperandError GetNum16( struct MUZ::CodeLine& codeline, int& value );

		/** Tests if current token is recognized as an (16-bit) indirect addressing, and go next token. Don't change current token if not found.
		 @param codeline the code line to assemble, with curtoken set to the start token for analysis
		 @param [OUT] value the 16-bit value found
		 @return operrOK if an 16-bit value has been found between parenthesis
		 */
		MUZ::OperandError GetInd16( struct MUZ::CodeLine& codeline, int& value );
	}; // struct OperandTools

} // namespace Z80
} // namespace MUZ
#endif /* Z80_Operands_h */
