//
//  AllOperators.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 13/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef AllOperators_h
#define AllOperators_h

#include "Operator.h"

namespace MUZ {

	
	/** Operator definition. Hold a priority and an operator instance. */
	struct OperatorDef {
		int priority = 9999;	// priority 0 is the highest
		Operator* op = &nop;	// no-operation operator
		TokenType type;			// used as a security check at initialization
	};
	
	/** global array for all operators definitions, global to the class. */
	extern OperatorDef allOps[tokenTypeLAST];
	
} //namespace
#endif /* AllOperators_h */
