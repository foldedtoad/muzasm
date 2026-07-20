//
//  Z80-Operands.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 23/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "Z80-Operands.h"
#include "ParseToken.h"
#include "Expression.h"
#include "CodeLine.h"
#include "Assembler.h"
#include "All-Directives.h"

namespace MUZ {
namespace Z80 {

	// prorotype to avoid warnings
	void InitRegisterMap();

	//MARK: - Maps for register names and sub codes

	//NB: the & = *new declarations avoid exit-time destructors warnings

	/** list of acceptable 8-bit registers */
	std::unordered_map<std::string, OperandType>& registers8 = *new std::unordered_map<std::string, OperandType>;
	/** list of acceptable 16-bit registers */
	std::unordered_map<std::string, OperandType>& registers16 = *new std::unordered_map<std::string, OperandType>;
	/** list of acceptable conditions */
	std::unordered_map<std::string, OperandType>& conditions = *new std::unordered_map<std::string, OperandType>;

	// Maps of sub-encoding for addressing
	std::map<OperandType, int> & regsubcode = *new std::map<OperandType, int>;
	std::map<OperandType, int> & regprefix = *new std::map<OperandType, int>;


	// Initialize registers map
	void InitRegisterMap() {

		// offsets for registers in some encodings
		regsubcode[regB] = 0;
		regsubcode[regC] = 1;
		regsubcode[regD] = 2;
		regsubcode[regE] = 3;
		regsubcode[regH] = 4;
		regsubcode[regL] = 5;
		regsubcode[regF] = 6;
		regsubcode[indHL] = 6;
		regsubcode[regA] = 7;
		//undocumented
		regsubcode[regIXH] = 4;
		regsubcode[regIXL] = 5;
		regsubcode[regIYH] = 4;
		regsubcode[regIYL] = 5;
		regprefix[regIXH] = 0xDD;
		regprefix[regIYH] = 0xFD;
		regprefix[regIXL] = 0xDD;
		regprefix[regIYL] = 0xFD;

		regsubcode[regI] = 0x07;
		regsubcode[regR] = 0x0F;

		regsubcode[regBC] = 0x00;
		regsubcode[regDE] = 0x10;
		regsubcode[regHL] = 0x20;
		regsubcode[regSP] = 0x30;
		regsubcode[regAF] = 0x30; // push,pop

		regsubcode[regIX] = 0x20;//hl
		regsubcode[regIY] = 0x20;//hl

		regprefix[regIX] = 0xDD;
		regprefix[regIY] = 0xFD;

		regsubcode[bit0] = 0x00;
		regsubcode[bit1] = 0x08;
		regsubcode[bit2] = 0x10;
		regsubcode[bit3] = 0x18;
		regsubcode[bit4] = 0x20;
		regsubcode[bit5] = 0x28;
		regsubcode[bit6] = 0x30;
		regsubcode[bit7] = 0x38;

		regsubcode[condNZ] = 0x00;
		regsubcode[condZ]  = 0x08;
		regsubcode[condNC] = 0x10;
		regsubcode[condC]  = 0x18;
		regsubcode[condPO] = 0x20;
		regsubcode[condPE] = 0x28;
		regsubcode[condP]  = 0x30;
		regsubcode[condM]  = 0x38;

		// authorized registers
		registers8["A"] = regA;
		registers8["B"] = regB;
		registers8["C"] = regC;
		registers8["D"] = regD;
		registers8["E"] = regE;
		registers8["H"] = regH;
		registers8["L"] = regL;
		registers8["I"] = regI;
		registers8["R"] = regR;
		registers8["F"] = regF;
		// undocumented
		registers8["IXH"] = regIXH;
		registers8["IXL"] = regIXL;
		registers8["IYH"] = regIYH;
		registers8["IYL"] = regIYL;

		registers16["AF"] = regAF;
		registers16["AF'"] = regAFp;
		registers16["BC"] = regBC;
		registers16["DE"] = regDE;
		registers16["HL"] = regHL;
		registers16["SP"] = regSP;
		registers16["IX"] = regIX;
		registers16["IY"] = regIY;

		conditions["NC"] = condNC;
		conditions["C"] = condC;
		conditions["NZ"] = condNZ;
		conditions["Z"] = condZ;
		conditions["PE"] = condPE;
		conditions["PO"] = condPO;
		conditions["P"] = condP;
		conditions["M"] = condM;

	}

	// singleton to launch register maps init at run-time
	struct _auto_init_register_map {
		_auto_init_register_map() {
			InitRegisterMap();
		}
	} _runtime_auto_init_register_map;

	//MARK: - Low level tokens analysis for operand types

	/** Parses current token and return the code for a 8-bit register regA to regH, regI or regR and undocumented. */
	bool OperandTools::reg8( ExpVector* tokens, int& curtoken, OperandType& reg8)
	{
		size_t ucurtoken = (size_t)curtoken;
		ParseToken& token =tokens->at(ucurtoken);
		if (token.type != tokenTypeLETTERS) return false;
		std::string source = std::to_upper(token.source);
		if (registers8.count(source)) {
			reg8 = registers8[source];
			curtoken += 1;
			return true;
		}
		return false;
	}

	/** Parses current token and return the code for a 16-bit register regAF, regAFp regBC regDE regHL regSP IX or regIY. */
	bool OperandTools::reg16( ExpVector* tokens, int& curtoken, OperandType& reg16 )
	{
		size_t ucurtoken = (size_t)curtoken;
		ParseToken& token =tokens->at(ucurtoken);
		if (token.type != tokenTypeLETTERS) return false;
		std::string source = std::to_upper(token.source);
		if (registers16.count(source)) {
			reg16 = registers16[source];
			curtoken += 1;
			return true;
		}
		return false;
	}

	/** Parses current token and return the code for an indirect access via(C): indC. */
	bool OperandTools::indirectC( ExpVector* tokens, int& curtoken, OperandType& reg )
	{
		size_t ucurtoken = (size_t)curtoken;
		if (ucurtoken + 2 >= tokens->size() ) return false;
		ParseToken* token = &tokens->at(ucurtoken);
		if (token->type != tokenTypePAROPEN) return false;
		token = &tokens->at(ucurtoken + 1);
		if (token->type != tokenTypeLETTERS) return false;
		if (token->source != "C") return false;
		token = &tokens->at(ucurtoken + 2);
		if (token->type != tokenTypePARCLOSE) return false;
		reg = indC;
		curtoken += 3;
		return true;
	}

	/** Parses current token and return the code for an indirect access via (HL): indHL. */
	bool OperandTools::indirectHL( ExpVector* tokens, int& curtoken, OperandType& reg )
	{
		size_t ucurtoken = (size_t)curtoken;
		if (ucurtoken + 2 >= tokens->size() ) return false;
		ParseToken* token = &tokens->at(ucurtoken);
		if (token->type != tokenTypePAROPEN) return false;
		token = &tokens->at(ucurtoken + 1);
		if (token->type != tokenTypeLETTERS) return false;
		if (token->source != "HL") return false;
		token = &tokens->at(ucurtoken + 2);
		if (token->type != tokenTypePARCLOSE) return false;
		reg = indHL;
		curtoken += 3;
		return true;
	}

	/** Parses current token and return the code for an indirect access via (BC): indBC. */
	bool OperandTools::indirectBC( ExpVector* tokens, int& curtoken, OperandType& reg )
	{
		size_t ucurtoken = (size_t)curtoken;
		if (ucurtoken + 2 >= tokens->size() ) return false;
		ParseToken* token = &tokens->at(ucurtoken);
		if (token->type != tokenTypePAROPEN) return false;
		token = &tokens->at(ucurtoken + 1);
		if (token->type != tokenTypeLETTERS) return false;
		if (token->source != "BC") return false;
		token = &tokens->at(ucurtoken + 2);
		if (token->type != tokenTypePARCLOSE) return false;
		reg = indBC;
		curtoken += 3;
		return true;
	}

	/** Parses current token and return the code for an indirect access via (DE): indDE. */
	bool OperandTools::indirectDE( ExpVector* tokens, int& curtoken, OperandType& reg )
	{
		size_t ucurtoken = (size_t)curtoken;
		if (ucurtoken + 2 >= tokens->size() ) return false;
		ParseToken* token = &tokens->at(ucurtoken);
		if (token->type != tokenTypePAROPEN) return false;
		token = &tokens->at(ucurtoken + 1);
		if (token->type != tokenTypeLETTERS) return false;
		if (token->source != "DE") return false;
		token = &tokens->at(ucurtoken + 2);
		if (token->type != tokenTypePARCLOSE) return false;
		reg = indDE;
		curtoken += 3;
		return true;
	}

	/** Parses current token and return the code for an indirect access via (SP): indSP. */
	bool OperandTools::indirectSP( ExpVector* tokens, int& curtoken, OperandType& reg )
	{
		size_t ucurtoken = (size_t)curtoken;
		if (ucurtoken + 2 >= tokens->size() ) return false;
		ParseToken* token = &tokens->at(ucurtoken);
		if (token->type != tokenTypePAROPEN) return false;
		token = &tokens->at(ucurtoken + 1);
		if (token->type != tokenTypeLETTERS) return false;
		if (token->source != "SP") return false;
		token = &tokens->at(ucurtoken + 2);
		if (token->type != tokenTypePARCLOSE) return false;
		reg = indSP;
		curtoken += 3;
		return true;
	}

	/** Parses current token and returns the code for an indirect access via (IX+d) and (IY+d): indIX, indIY.
		Sends back the 'd' in parameter value.
		Does not change current token if returning:
			operrTOKENNUMBER		Not enough tokens
			operrMISSINGPAROPEN		Missing opening parenthesis
			operrREGISTERNAME		Wrong register name
			operrWRONGOP			Not a '+' operator
		Changes current token if returning:
			operrUNSOLVED			Unsolved symbol in 'd' expression, value is 0
			operrOK					value is 'd' expression result
	 */
	OperandError OperandTools::indirectX( ExpVector* tokens, int& curtoken, OperandType& regX, int& value )
	{
		size_t ucurtoken = (size_t)curtoken;
		if (ucurtoken + 4 >= tokens->size() ) return operrTOKENNUMBER;
		ParseToken* token = &tokens->at(ucurtoken);
		if (token->type != tokenTypePAROPEN) return operrMISSINGPAROPEN;
		int indextoken = curtoken + 1;
		if (! reg16(tokens, indextoken, regX )) return operrREGISTERNAME;
		if (regX != regIX && regX != regIY) return operrREGISTERNAME;
		token = &tokens->at(ucurtoken + 2);
		if (token->type != tokenTypeOP_PLUS) return operrWRONGOP;
		// find closing parenthesis
		indextoken = curtoken + 3;// skip '(' regX '+'
		int parlevel = 1;
		for ( ; indextoken < (int)tokens->size() ; indextoken++) {
			token = &tokens->at((size_t)indextoken);
			if (token->type == tokenTypePAROPEN) {
				parlevel += 1;
			} else if (token->type == tokenTypePARCLOSE) {
				parlevel -= 1;
				if (parlevel == 0) break;
			}
		}
		// evaluate the value after "+" and before closing parenthesis
		indextoken = indextoken - 1;
		ParseToken evaluated = evalNumber.Evaluate(*tokens, curtoken + 3, indextoken);
		// skip closing parenthesis
		curtoken = indextoken + 1;
		if (evaluated.unsolved) {
			// could be pass 1, signal unsolved expression
			value = 0;
			return operrUNSOLVED;
		}
		value = evaluated.asAddress();
		return operrOK;
	}

	/** Parses current token and returns the code for a bit nunmber: bit0 to bit7.
		Changes current token if returning:
			operrUNSOLVED			Unsolved symbol in 'd' expression, value is 0
			operrOK					value is 'd' expression result
		Doesn't update current token if returning:
			operrNOTBIT				Number is too big for a bit number, or not a number
	 */
	OperandError OperandTools::bitnumber( ExpVector* tokens, int& curtoken, OperandType& bit )
	{
		int lasttoken = -1;
		ParseToken evaluated = evalNumber.Evaluate(*tokens, curtoken, lasttoken);
		if (evaluated.unsolved) {
			bit = bit0;
			curtoken = lasttoken ;
			return operrUNSOLVED;
		}
		if ((evaluated.type == tokenTypeSTRING) || (evaluated.type == tokenTypeDECNUMBER)) {
			int value = evaluated.asAddress();
			if (value < 0 || value > 7) return operrNOTBIT;
			if (value == 0) bit = bit0;
			else if (value == 1) bit = bit1;
			else if (value == 2) bit = bit2;
			else if (value == 3) bit = bit3;
			else if (value == 4) bit = bit4;
			else if (value == 5) bit = bit5;
			else if (value == 6) bit = bit6;
			else bit = bit7;
			curtoken = lasttoken ;
			return operrOK;
		}
		return operrNOTBIT;
	}

	/** Parses current token and return the code for a condition name: condNZ to condM. */
	OperandError OperandTools::condition( ExpVector* tokens, int& curtoken, OperandType& cond )
	{
		ParseToken& token =tokens->at((size_t)curtoken);
		if (token.type != tokenTypeLETTERS) return operrNOTSTRING;
		if (conditions.count(token.source)) {
			cond = conditions[token.source];
			curtoken += 1;
			return operrOK;
		}
		return operrNOTCONDITION;
	}

	/** Parses current token and return the value for an 8-bit number.
	 Changes current token if returning:
		 operrUNSOLVED			Unsolved symbol in expression, value is 0
		 operrOK				value is expression result
	 Doesn't update current token if returning:
		 operrTOOBIG			Number is too big for a 8-bit number
		 operrNOTNUMBER			Not a number
	 */
	OperandError OperandTools::number8( ExpVector* tokens, int& curtoken, int& value )
	{
		int lasttoken = -1;
		ParseToken evaluated = evalNumber.Evaluate(*tokens, curtoken, lasttoken);
		if (evaluated.unsolved) {
			value = 0;
			curtoken = lasttoken + 1;
			return operrUNSOLVED;
		}
		if ((evaluated.type == tokenTypeSTRING) || (evaluated.type == tokenTypeDECNUMBER)) {
			value = evaluated.asAddress();
			if (value > 255) return operrTOOBIG;
			curtoken = lasttoken + 1;
			return operrOK;
		}
		return operrNOTNUMBER;
	}

	/** Parses current token and return the value for a 16-bit number. */
	OperandError OperandTools::number16( ExpVector* tokens, int& curtoken, int& value )
	{
		int lasttoken = -1;
		ParseToken evaluated = evalNumber.Evaluate(*tokens, curtoken, lasttoken);
		if (evaluated.unsolved) {
			value = 0;
			curtoken = lasttoken + 1;
			return operrUNSOLVED;
		}
		if ((evaluated.type == tokenTypeSTRING) || (evaluated.type == tokenTypeDECNUMBER)) {
			value = evaluated.asAddress();
			if (value > 65535) return operrTOOBIG;
			curtoken = lasttoken + 1;
			return operrOK;
		}
		return operrNOTNUMBER;
	}

	/** Compute a 16-bit value from a numeric expression between parenthesis. If parenthesis or a value cannot be found,
	 returns an error code. The last used token index is returned even if the expression doesn't compute a number but
	 have correct parenthesis. */
	OperandError OperandTools::indirect16( ExpVector* tokens, int curtoken, int& value, int& lasttoken )
	{
		size_t ucurtoken = (size_t)curtoken;
		if (ucurtoken + 2 >= tokens->size() ) return operrTOKENNUMBER;
		ParseToken* token = &tokens->at(ucurtoken);
		if (token->type != tokenTypePAROPEN) return operrMISSINGPAROPEN;
		token = &tokens->at(ucurtoken + 1);
		// find closing parenthesis
		lasttoken = curtoken + 2;
		int parlevel = 1;
		for ( ; lasttoken < (int)tokens->size() ; lasttoken++) {
			token = &tokens->at((size_t)lasttoken);
			if (token->type == tokenTypePAROPEN) {
				parlevel += 1;
			} else if (token->type == tokenTypePARCLOSE) {
				parlevel -= 1;
				if (parlevel == 0) break;
			}
		}
		if (token->type != tokenTypePARCLOSE) return operrMISSINGPARCLOSE;
		// evaluate the tokens between parenthesis
		lasttoken = lasttoken - 1; // back from parenthesis close
		ParseToken evaluated = evalNumber.Evaluate(*tokens, curtoken + 1, lasttoken );
		lasttoken = lasttoken + 1;// skips  closing parenthesis
		if (evaluated.unsolved) {
			value = 0;
			return operrUNSOLVED;
		}
		if ((evaluated.type == tokenTypeSTRING) || (evaluated.type == tokenTypeDECNUMBER)) {
			value = evaluated.asAddress();
			return operrOK;
		}
		return operrNOTNUMBER;
	}

	//MARK: - High level functions for CodeLine operands analysis

	OperandTools::OperandTools()
	{
		evalString.SetDefaultConversion(tokenTypeSTRING);
		evalBool.SetDefaultConversion(tokenTypeBOOL);
		evalNumber.SetDefaultConversion(tokenTypeDECNUMBER);
	}

	OperandTools::~OperandTools()
	{
	}

	// helpers for instruction assembling
	bool OperandTools::RegAccept(unsigned int flags, OperandType reg)
	{
		unsigned int f = 1 << reg;
		return ((f & flags) == f);
	}

	/** Returns the subcode for a register code. Used for instructions accepting a reg8 spec or a reg16 spec.
	 Returns a 0 for any invalid register or addressing code.
	 */
	int OperandTools::GetSubCode( OperandType reg )
	{
		if (regsubcode.count(reg))
			return regsubcode[reg];
		return 0;
	}
	/** Returns the prefix for a register code. Used for instructions accepting IX, IY and undocumented forms.
	 Returns a 0 for any invalid register or addressing code.
	 */
	int OperandTools::GetPrefix( OperandType reg )
	{
		if (regprefix.count(reg))
			return regprefix[reg];
		return 0;
	}

	/** Returns true if current token is recognized as an 8-bit register, and go next token. */
	OperandError OperandTools::GetReg8(CodeLine& codeline, OperandType& reg, unsigned int regs ) {
		if (!EnoughTokensLeft(codeline, 1)) return operrTOKENNUMBER;
		int worktoken = (int)codeline.curtoken;
		if (reg8(&codeline.tokens, worktoken, reg)) {
			if (RegAccept(regs, reg)) {
				codeline.curtoken = (size_t)worktoken;
				return operrOK;
			}
			return operrWRONGREGISTER;
		}
		return operrNOTREGISTER;
	}
	/** Returns true if current token is recognized as an 16-bit register, and go next token. */
	OperandError OperandTools::GetReg16(CodeLine& codeline, OperandType& reg, unsigned int regs  ) {
		if (!EnoughTokensLeft(codeline, 1)) return operrTOKENNUMBER;
		int worktoken = (int)codeline.curtoken;
		if (reg16(&codeline.tokens, worktoken, reg)) {
			if (RegAccept(regs, reg)) {
				codeline.curtoken = (size_t)worktoken;
				return operrOK;
			}
			return operrWRONGREGISTER;
		}
		return operrNOTREGISTER;
	}
	/** Returns true if current token is recognized as (C), and go next token. */
	OperandError OperandTools::GetIndC( CodeLine& codeline ) {
		if (!EnoughTokensLeft(codeline, 3)) return operrTOKENNUMBER;
		OperandType regC;
		int worktoken = (int)codeline.curtoken;
		if (indirectC(&codeline.tokens,worktoken, regC)) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operrWRONGREGISTER;
	}
	/** Returns true if current token is recognized as (HL), and go next token. */
	OperandError OperandTools::GetIndHL( CodeLine& codeline ) {
		if (!EnoughTokensLeft(codeline, 3)) return operrTOKENNUMBER;
		OperandType regHL;
		int worktoken = (int)codeline.curtoken;
		if (indirectHL(&codeline.tokens, worktoken, regHL)) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operrWRONGREGISTER;
	}
	/** Returns true if current token is recognized as (HL), and go next token. */
	OperandError OperandTools::GetIndBC( CodeLine& codeline ) {
		if (!EnoughTokensLeft(codeline, 3)) return operrTOKENNUMBER;
		OperandType regBC;
		int worktoken = (int)codeline.curtoken;
		if (indirectBC(&codeline.tokens, worktoken, regBC)) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operrWRONGREGISTER;
	}
	/** Returns true if current token is recognized as (HL), and go next token. */
	OperandError OperandTools::GetIndDE( CodeLine& codeline ) {
		if (!EnoughTokensLeft(codeline, 3)) return operrTOKENNUMBER;
		OperandType regDE;
		int worktoken = (int)codeline.curtoken;
		if (indirectDE(&codeline.tokens, worktoken, regDE)) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operrWRONGREGISTER;
	}
	/** Returns true if current token is recognized as (SP), and go next token. */
	OperandError OperandTools::GetIndSP( CodeLine& codeline ) {
		if (!EnoughTokensLeft(codeline, 3)) return operrTOKENNUMBER;
		OperandType regSP;
		int worktoken = (int)codeline.curtoken;
		if (indirectSP(&codeline.tokens, worktoken, regSP)) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operrWRONGREGISTER;
	}

	/** Returns true if current token is recognized as (IX+d) or (IY+d), and go next token. */
	OperandError OperandTools::GetIndX(CodeLine& codeline, OperandType& regX, int& value ) {
		if (!EnoughTokensLeft(codeline,5)) return operrTOKENNUMBER;
		int worktoken = (int)codeline.curtoken;
		OperandError operr = indirectX(&codeline.tokens, worktoken, regX, value);
		if (operr == operrOK) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		if (codeline.as->IsFirstPass() && (operr == operrUNSOLVED)) {
			// probably unresolved label, simulate success with neutral value
			value = 0;
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operrWRONGREGISTER;
	}

	/** Returns true if current token is recognized as a bit number (0-7), and go next token. */
	OperandError OperandTools::GetBitNumber(CodeLine& codeline, OperandType& bit ) {
		if (!EnoughTokensLeft(codeline,1)) return operrTOKENNUMBER;
		int worktoken = (int)codeline.curtoken;
		if (reg8(&codeline.tokens, worktoken, bit)) return operrWRONGREGISTER;
		if (reg16(&codeline.tokens, worktoken, bit)) return operrWRONGREGISTER;
		worktoken = (int)codeline.curtoken;
		OperandError operr = bitnumber(&codeline.tokens, worktoken, bit);
		if (operr == operrOK) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		if (codeline.as->IsFirstPass() && operr == operrUNSOLVED) {
			// not number: probably unresolved label, simulate success with neutral value
			bit = bit0;
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operr;
	}

	/** Returns true if current token is recognized as a condition, and go next token. */
	OperandError OperandTools::GetCond( CodeLine& codeline, OperandType& cond ) {
		if (!EnoughTokensLeft(codeline, 1)) return operrTOKENNUMBER;
		int worktoken = (int)codeline.curtoken;
		if (condition(&codeline.tokens, worktoken, cond) == operrOK) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operrNOTCONDITION;
	}

	/** Tests if current token is recognized as an 8-bit number expression, and go next token.
	 In case a number could not be found, the call will return:
	 - operrTOKENNUMBER if there are not enough tokens left
	 - operrWRONGREGISTER if a register name has been found
	 - operrNOTNUMBER if the expression is not a number
	 */
	OperandError OperandTools::GetNum8(CodeLine& codeline, int& value ) {
		if (!EnoughTokensLeft(codeline,1)) return operrTOKENNUMBER;
		OperandType num8;
		// forbid register names
		int worktoken = (int)codeline.curtoken;
		if (reg8(&codeline.tokens, worktoken, num8)) return operrWRONGREGISTER;
		if (reg16(&codeline.tokens, worktoken, num8)) return operrWRONGREGISTER;
		// now only numbers or labels
		worktoken = (int)codeline.curtoken;
		OperandError operr = number8(&codeline.tokens, worktoken, value);
		if (operr == operrOK) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		if (codeline.as->IsFirstPass() && operr == operrUNSOLVED) {
			value = 0;
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		return operrNOTNUMBER;
	}

	/** Tests if current token is recognized as a 16-bit number expression, and go next token.
	 In case a number could not be found, the call will return:
	 - operrTOKENNUMBER if there are not enough tokens left
	 - operrWRONGREGISTER if a register name has been found
	 - operrNOTNUMBER if the expression is not a number
	 */
	OperandError OperandTools::GetNum16(CodeLine& codeline, int& value ) {
		if (!EnoughTokensLeft(codeline,1)) return operrTOKENNUMBER;
		OperandType num16;
		int worktoken = (int)codeline.curtoken;
		if (reg8(&codeline.tokens, worktoken, num16)) return operrWRONGREGISTER;
		if (reg16(&codeline.tokens, worktoken, num16)) return operrWRONGREGISTER;
		worktoken = (int)codeline.curtoken;
		OperandError operr = number16(&codeline.tokens, worktoken, value);
		if (operr == operrOK) {
			codeline.curtoken = (size_t)worktoken;
			return operrOK;
		}
		if (codeline.as->IsFirstPass() && operr == operrUNSOLVED) {
			codeline.curtoken = (size_t)worktoken;
			value = 0;
			return operrOK;
		}
		return operrNOTNUMBER;
	}

	/** Returns true if current token is recognized as an (16-bit) indirect addressing, and go next token.
	 In case a number could not be found, the call will return:
	 - operrTOKENNUMBER if there are not enough tokens left
	 - operrWRONGREGISTER if a register name has been found
	 - operrNOTNUMBER if the expression is not a number
	 */
	OperandError OperandTools::GetInd16(CodeLine& codeline, int& value ) {
		if (!EnoughTokensLeft(codeline,3)) return operrTOKENNUMBER;
		int lasttoken;
		int worktoken = (int)codeline.curtoken;
		OperandError operr = indirect16(&codeline.tokens, worktoken, value, lasttoken);
		if (operr==operrOK) {
			codeline.curtoken = (size_t)lasttoken;
			return operrOK;
		}
		if (operr==operrUNSOLVED && codeline.as->IsFirstPass()) {
			// not number: probably unresolved label, simulate success with neutral value
			value = 0;
			codeline.curtoken = (size_t)lasttoken;
			return operrOK;
		}
		// other errors do not update curtoken
		return operrNOTNUMBER;
	}


} // namespace Z80
} // namespace MUZ
