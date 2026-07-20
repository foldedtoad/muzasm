//
//  DefSymbol.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 01/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef DefSymbol_h
#define DefSymbol_h

#include <unordered_map>
#include "Instruction.h"

namespace MUZ {
	
	/** Structure for a #DEFINE symbol. */
	class DefSymbol : public Instruction
	{
	public:
		/** Value of the symbol if it is defined by a string value. */
		std::string value;
		/** true if this symbol is just defined with no value. */
		bool singledefine = false;
	};
	
	typedef std::unordered_map<std::string, DefSymbol*> DefSymbolsMap;
	// Note: 'DefSymbolMap' (no 's' before map)  makes a strange bug in XCode: values are not added to the map
	
} // namespace MUZ
#endif /* DefSymbol_h */
