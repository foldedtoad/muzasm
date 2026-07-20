/*
 * types.h
 *
 *  Created on: Nov 26, 2018
 *      Author: Francis Piérot
 */

#ifndef SRC_MUZ_TYPES_H_
#define SRC_MUZ_TYPES_H_

#include <limits.h>
#include <string>
#include <vector>

#include "Config.h"

namespace MUZ {

	// Types for memory and content
	typedef unsigned char BYTE;
	typedef unsigned short WORD;	// should be 16-bits
	
#if UINT_MAX == 65535
	typedef unsigned long DWORD;	// long should be 32-bits
#elif UINT_MAX == 4294967295
	typedef unsigned int DWORD;	// int is 32 bits
#else
	typedef unsigned int DWORD;	// 64-bit environment?
#endif

// Adjust constants depending on the mode choosen in Config.h
#ifdef MODE_A16_D8
	// Address is 16-bits and data is 8-bits (Z-80)
	typedef WORD ADDRESSTYPE; // 16-bits
	const int ADDRESSMASK = 0xFFFF;
	typedef unsigned int ADDRESSSIZETYPE;
	typedef BYTE DATATYPE;
	const int DATAMASK = 0xFF;
#elif MODE_A32_D16
	// Address is 32-bits and data is 16-bits
	typedef DWORD ADDRESSTYPE; // 32-bits
	const int ADDRESSMASK = 0xFFFFffff;
	typedef unsigned int ADDRESSSIZETYPE;
	typedef WORD DATATYPE;
	const int DATAMASK = 0xFFFF;
#else
#error An addressing/data mode must be choosen in Config.h
#endif
	
	const ADDRESSSIZETYPE MEMMAXSIZE = (ADDRESSSIZETYPE)ADDRESSMASK + 1;

	struct AddressRange {
		DWORD start = ADDRESSMASK;	// starting address
		DWORD end = 0;				// last address, updated by each write
	};
		

}; // namespace MUZ

#endif /* SRC_MUZ_TYPES_H_ */
