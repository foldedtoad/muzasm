//
//  Label.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 01/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef Label_h
#define Label_h

#include <unordered_map>
#include <vector>
#include <limits.h>

namespace MUZ {
	
	/** Structure to store a label from source file. */
	class Label
	{
		size_t							file;
		size_t 							line;
	public:
		std::vector<DWORD>				addresses;		// addresses of this label, unique for global label, multiple for local
		bool							equate=false;	// this label is set by a .EQU
		std::vector<struct CodeLine*>	referencers;	// Code lines where it is used
		bool							multiple=false;	// true to authorize more than one address
		
		void ClearAddresses() {
			addresses.clear();
		}
		
		void SetFileLine(size_t thefile, size_t theline) {
			file = thefile;
			line = theline;
		}

		/** returns true if the label is at the given file/line .*/
		bool isAt(size_t thefile, size_t theline) {
			return (file == thefile && line == theline);
		}

		/** Sets a label address value from a .EQU directive. This sets the label with a unique value and equate mode. */
		void Equate(unsigned int integer) {
			equate = true;
			multiple = false;
			ClearAddresses();
			addresses.push_back(integer);
		}
		
		/** Sets an address into the label. If the label accepts multiple values, it is added to existing values, else it replaces the current value. */
		void SetAddress( unsigned int integer ) {
			if (equate) Equate(integer);
			if (!multiple) ClearAddresses();
			addresses.push_back(integer);
		}
		
		/** Tells if the label has assigned addresses. */
		bool empty() {
			return addresses.empty();
		}
		
		/** Adds a referencing code line to the list of referencers. */
		void AddReferencer(struct CodeLine* codeline) {
			referencers.push_back(codeline);
		}
		
		/** Computes the 2-complement displacement value from a given address to the nearest available addresses.
		 	This value is used for JR and DJNZ relative jumps */
		int DeltaFrom(DWORD from) {
			// compute delta
			int delta = (int)AddressFrom(from) - (int)from;
			// then 2-complement
			return ((((int)MEMMAXSIZE) - delta) & ADDRESSMASK);
		}
		
		/** Returns the nearest available address. It returns max possible address if no address is assigned. */
		DWORD AddressFrom(DWORD from) {
			// find closest address
			DWORD closest = INT_MAX;
			for (DWORD addr: addresses) {
				if (abs((int)addr - (int)from) <= abs((int)closest - (int)from)) {
					closest = addr;
				}
			}
			return closest;
		}
	};

	typedef std::unordered_map<std::string, Label*> LabelMap;
} // namespace MUZ
#endif /* Label_h */
